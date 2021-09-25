#!/bin/bash

#!/bin/bash

set -e


# Utilities
# ---------

echo_header() {
	printf "\e[1;4m$1\e[0m\n"
}
echo_success() {
	printf "\e[1;4;32m$1\e[0m\n"
}

export -f echo_header
export -f echo_success

# Config
# ------

CPU=$(sysctl -n hw.physicalcpu)

export SCONS="scons -j$CPU verbose=yes warnings=no progress=no"
export OPTIONS="debug_symbols=no"

export IOS_SDK="14.2"
export IOS_LIPO="xcrun lipo"

# Classical

echo_header "*** Starting classical build for iOS..."

$SCONS platform=iphone $OPTIONS arch=arm64 tools=no target=release_debug
$SCONS platform=iphone $OPTIONS arch=arm64 tools=no target=release

$SCONS platform=iphone $OPTIONS arch=x86_64 ios_simulator=yes tools=no target=release_debug
$SCONS platform=iphone $OPTIONS arch=x86_64 ios_simulator=yes tools=no target=release

mkdir -p bin/templates/ios

$IOS_LIPO -create bin/libgodot.iphone.opt.arm64.a bin/libgodot.iphone.opt.x86_64.simulator.a -output bin/templates/ios/libgodot.iphone.opt.a
$IOS_LIPO -create bin/libgodot.iphone.opt.debug.arm64.a bin/libgodot.iphone.opt.debug.x86_64.simulator.a -output bin/templates/ios/libgodot.iphone.opt.debug.a

rm -v \
	bin/libgodot.iphone.opt.arm64.a bin/libgodot.iphone.opt.x86_64.simulator.a \
	bin/libgodot.iphone.opt.debug.arm64.a bin/libgodot.iphone.opt.debug.x86_64.simulate.a

# Look for platform plugins:
if [ -d "platform_plugins/ios" ]; then
	echo_header "*** Building platform plugins"
	(pushd "platform_plugins/ios"
		for plugin in impact apn arkit camera icloud gamecenter inappstore; do
			scons target=release_debug arch=arm64 simulator=no plugin=$plugin version=3.3
		done
	popd)
fi

echo_header "*** iOS build successful"
