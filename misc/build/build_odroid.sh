#!/bin/bash

set -e

CROSS="/opt/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin"
CC="aarch64-linux-gnu-gcc"

if [ ! -e "$CROSS/$CC" ]; then

	if [[ ! -d platform/frt ]]; then
		(cd platform && git clone --depth=1 https://github.com/ppiecuch/frt)
	else
		(cd platform/frt && git pull) # update platform repository
	fi

	# toolchain not found - run docker image
	if ! command -v docker &> /dev/null
	then
		echo "*** Docker is not found - cannot run build script."
		exit 1
	fi
	docker_state=$(docker info >/dev/null 2>&1)
	if [[ $? -ne 0 ]]; then
		echo "*** Docker does not seem to be running, run it first."
		exit 1
	fi

	APPDIR="$(cd "$PWD" && pwd)"
	SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	NAME="$(basename "${BASH_SOURCE[0]}")"
	VERSION=2020-10-01

	echo "*** Running docker toolchain $VERSION (with script $NAME).."
	docker run --rm -t -v "$APPDIR:/app" odroid_dev:$VERSION "./${SCRIPTDIR/$APPDIR/}/$NAME"

	exit
fi

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
	CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

PATH=/usr/bin:/bin:/sbin:/usr/local/bin:$CROSS
scons -j${CPU} platform=frt frt_arch=odroid target=release disable_3d=true

mkdir -p bin/templates/frt
mv -v bin/godot.frt.opt.aarch64.odroid bin/templates/frt/
