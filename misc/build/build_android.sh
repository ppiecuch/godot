#!/bin/bash

set -e

if ! command -v scons &> /dev/null
then
	export PATH=$PATH:/opt/local/bin
fi

# Config
# ------

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
	CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

export ANDROID_SDK_ROOT=$HOME/Library/Android/sdk
export ANDROID_NDK_ROOT=$HOME/Library/Android/ndk

# Godot gradle builds require JDK 17 to 23.
# JDK 24+ is not supported by Gradle 8.x (class file major version 68).
MIN_JDK=17
MAX_JDK=23

get_java_major_version() {
	"$1/bin/java" -version 2>&1 | head -1 | sed 's/.*"\([0-9]*\)\..*/\1/'
}

is_jdk_compatible() {
	local jdk_home="$1"
	[ -d "$jdk_home" ] && [ -x "$jdk_home/bin/java" ] || return 1
	local ver=$(get_java_major_version "$jdk_home")
	[ -n "$ver" ] && [ "$ver" -ge $MIN_JDK ] && [ "$ver" -le $MAX_JDK ] 2>/dev/null
}

# Try to find a compatible JDK
if ! is_jdk_compatible "$JAVA_HOME"; then
	unset JAVA_HOME
	if [[ "$OSTYPE" == "darwin"* ]]; then
		# Try preferred versions via java_home, validate each result
		for ver in 17 21 23 22 20 19 18; do
			candidate=$(/usr/libexec/java_home -v $ver 2>/dev/null)
			if is_jdk_compatible "$candidate"; then
				export JAVA_HOME="$candidate"
				break
			fi
		done
		# Try common Homebrew locations as fallback
		if [ -z "$JAVA_HOME" ]; then
			for ver in 17 21 23 22 20 19 18; do
				for prefix in /opt/homebrew/opt /usr/local/opt; do
					candidate="$prefix/openjdk@$ver/libexec/openjdk.jdk/Contents/Home"
					if is_jdk_compatible "$candidate"; then
						export JAVA_HOME="$candidate"
						break 2
					fi
				done
			done
		fi
	elif [[ "$OSTYPE" == "linux"* ]]; then
		for jdk in /usr/lib/jvm/java-17-* /usr/lib/jvm/java-21-* /usr/lib/jvm/temurin-17-* /usr/lib/jvm/temurin-21-*; do
			if is_jdk_compatible "$jdk"; then
				export JAVA_HOME="$jdk"
				break
			fi
		done
	fi
fi

if [ -z "$JAVA_HOME" ] || ! is_jdk_compatible "$JAVA_HOME"; then
	echo ""
	echo "ERROR: JDK $MIN_JDK to $MAX_JDK is required for Godot gradle builds."
	if [ -n "$JAVA_HOME" ] && [ -x "$JAVA_HOME/bin/java" ]; then
		echo "  Found JDK $(get_java_major_version "$JAVA_HOME") at $JAVA_HOME (not compatible)"
	fi
	if [[ "$OSTYPE" == "darwin"* ]]; then
		echo ""
		echo "  Installed JDKs:"
		/usr/libexec/java_home -V 2>&1 | grep -v "^$" | sed 's/^/    /'
		echo ""
		echo "  Install a compatible JDK via:"
		echo "    brew install openjdk@17"
	fi
	echo ""
	exit 1
fi
echo "Using JDK $(get_java_major_version "$JAVA_HOME"): $JAVA_HOME"

# Build env info is saved to the template output dir after templates are built (see below).

export SCONS="scons -j$CPU verbose=yes warnings=no progress=no"
export OPTIONS="debug_symbols=yes debug_experimental=no"

# Utilities
# ---------

echo_header() {
	if [[ "$TERM" =~ "xterm" ]]; then
		printf "\e[1;4m$1\e[0m\n"
	else
		printf "[$1]\n"
	fi
}
echo_success() {
	if [[ "$TERM" =~ "xterm" ]]; then
		printf "\e[1;4;32m$1\e[0m\n"
	else
		printf "*** $1\n"
	fi
}
echo_bold() {
	if [[ "$TERM" =~ "xterm" ]]; then
		printf "\e[1m$1\e[0m\n"
	else
		printf "** $1 **\n"
	fi
}

export -f echo_header
export -f echo_success

# Building
# --------

cmd=""
if [ "$1" == "skip_plugins" ] || [ "$1" == "build_x86" ]; then
	cmd="$1"
	shift
fi

# Cleanup
rm -rfv \
	bin/libgodot.android.*.so

echo_header "*** Starting classical build for Android..."

if [ -n "$JAVA_HOME" ]; then
	echo_bold "JAVA home at: ${JAVA_HOME}"
fi

$SCONS $* platform=android android_arch=armv7 $OPTIONS tools=no target=release_debug
$SCONS $* platform=android android_arch=armv7 $OPTIONS tools=no target=release

$SCONS $* platform=android android_arch=arm64v8 $OPTIONS tools=no target=release_debug
$SCONS $* platform=android android_arch=arm64v8 $OPTIONS tools=no target=release

if [ "$cmd" == "build_x86" ]; then
	$SCONS $* platform=android android_arch=x86 $OPTIONS tools=no target=release_debug
	$SCONS $* platform=android android_arch=x86 $OPTIONS tools=no target=release

	$SCONS $* platform=android android_arch=x86_64 $OPTIONS tools=no target=release_debug
	$SCONS $* platform=android android_arch=x86_64 $OPTIONS tools=no target=release
fi

(cd platform/android/java && ./gradlew generateGodotTemplates)

template_dir="${TEMPLATE_OUT_DIR}"
if [ ! -d "$template_dir" ]; then
	template_dir="$(pwd)/bin/templates/android"
fi

mkdir -p "$template_dir"

# Save build environment info for downstream builds (e.g. app export scripts).
cat > "${template_dir}/build_env.txt" <<BUILD_ENV_EOF
JAVA_HOME=$JAVA_HOME
JAVA_VERSION=$(get_java_major_version "$JAVA_HOME")
JAVA_VERSION_MIN=$MIN_JDK
JAVA_VERSION_MAX=$MAX_JDK
BUILD_DATE=$(date -u +%Y-%m-%dT%H:%M:%SZ)
BUILD_ENV_EOF

mv -v \
	bin/android_source.zip bin/android_debug.apk bin/android_release.apk bin/godot-lib.debug.aar bin/godot-lib.release.aar \
	"${template_dir}/"

# Look for platform plugins:
if [ "$cmd" != "skip_plugins" ]; then
	if [ -d "platform_plugins/android" ]; then
		for build in debug release; do
			echo_header "*** Building platform plugins ($build)"
			godot_lib="$(pwd)/platform/android/java/app/libs/${build}/godot-lib.${build}.aar"
			gradle=$(grep distributionUrl "$(pwd)/platform/android/java/gradle/wrapper/gradle-wrapper.properties" | sed 's/.*gradle-\([0-9.]*\)-.*/\1/')
			if [ -z "$gradle" ]; then
				echo_header "*** Cannot detect gradle version"
				exit 1
			fi
			install_dir=""
			if [ ! -z "${template_dir}" ]; then
				install_dir="${template_dir}/plugins/${build}"
				mkdir -p "${install_dir}"
			fi
			(pushd "platform_plugins/android"
				for plugin in godot-direct godot-google-play-billing godot-bluetooth godot-device-info godot-pad; do
				(if [[ $plugin == -* ]]; then
					echo_bold "*** Skipping plugin: $plugin"
				elif [ -d $plugin ]; then
					echo_bold "Building  plugin: $plugin"
					pushd $plugin
					if [ -e gd_build_plugin.sh ]; then
						if [ -f "gradle/wrapper/gradle-wrapper.properties" ]; then
							ver=$(grep distributionUrl "gradle/wrapper/gradle-wrapper.properties" | sed 's/.*gradle-\([0-9.]*\)-.*/\1/')
						else
							ver=$(grep distributionUrl $(find . -name gradle-wrapper.properties) | sed 's/.*gradle-\([0-9.]*\)-.*/\1/')
						fi
						if [ "$ver" != "$gradle" ]; then
							echo_bold "*** Plugin gradle differ from main build: $gradle <> $ver. Please update."
							exit 1
						fi
						./gd_build_plugin.sh "$godot_lib" "$build" "$install_dir"
					else
						echo_header "*** Cannot find a gd_build_plugin.sh script for the plugin: $plugin"
						exit 1
					fi
					popd
				fi)
				done
			popd)
		done
	fi
fi

echo_header "*** Android build successful"
