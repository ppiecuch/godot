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

# Godot gradle builds require JDK 17
if [[ "$OSTYPE" == "darwin"* ]]; then
	JAVA17=$(/usr/libexec/java_home -v 17 2>/dev/null)
	if [ -z "$JAVA17" ] || [ ! -d "$JAVA17" ]; then
		echo ""
		echo "ERROR: JDK 17 is required for Godot gradle builds but was not found."
		echo ""
		echo "  Installed JDKs:"
		/usr/libexec/java_home -V 2>&1 | grep -v "^$" | sed 's/^/    /'
		echo ""
		echo "  Install JDK 17 via one of:"
		echo "    brew install openjdk@17"
		echo "    https://adoptium.net/"
		echo ""
		exit 1
	fi
	export JAVA_HOME="$JAVA17"
	echo "Using JDK 17: $JAVA_HOME"
elif [[ "$OSTYPE" == "linux"* ]]; then
	if [ -z "$JAVA_HOME" ]; then
		for jdk in /usr/lib/jvm/java-17-* /usr/lib/jvm/temurin-17-*; do
			if [ -d "$jdk" ]; then
				export JAVA_HOME="$jdk"
				break
			fi
		done
	fi
	if [ -z "$JAVA_HOME" ] || ! "$JAVA_HOME/bin/java" -version 2>&1 | grep -q '"17\.'; then
		echo "ERROR: JDK 17 is required. Set JAVA_HOME to a JDK 17 installation."
		exit 1
	fi
	echo "Using JDK 17: $JAVA_HOME"
fi

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
				for plugin in godot-direct godot-google-play-billing godot-bluetooth godot-device-info; do
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
