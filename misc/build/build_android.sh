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

export SCONS="scons -j$CPU verbose=yes warnings=no progress=no"
export OPTIONS="debug_symbols=no debug_experimental=no"

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
	template_dir="bin/templates/android"
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
			install_dir=""
			if [ ! -z "${TEMPLATE_OUT_DIR}" ]; then
				install_dir="${TEMPLATE_OUT_DIR}/plugins/${build}"
				mkdir -p "${install_dir}"
			fi
			(pushd "platform_plugins/android"
				for plugin in godot-direct godot-google-play-billing godot-bluetooth godot-device-info; do
				(if [ "$plugin" == "godot-google-play-billing" ]; then
					echo_bold "*** Skipping plugin: godot-google-play-billing"
				elif [ -d $plugin ]; then
					echo_bold "Building  plugin: $plugin"
					pushd $plugin
					if [ -e gd_build_plugin.sh ]; then
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
