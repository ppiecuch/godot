#!/bin/bash

set -e

DOCKER_IMAGE="amberelec-build-godot:2022-10-20"

if [ ! -d "/app" ]; then
	if [[ ! -d platform/frt ]]; then
		(cd platform && git clone --depth=1 https://github.com/ppiecuch/frt)
	else
		(cd platform/frt && git pull) # update platform repository
	fi

	# toolchain not found - run docker image
	if ! command -v docker &> /dev/null
	then
		echo "*** Docker is not found - cannot run build script."
		exit
	fi
	docker_state=$(docker info >/dev/null 2>&1)
	if [[ $? -ne 0 ]]; then
		echo "*** Docker does not seem to be running, run it first."
		exit 1
	fi

	APPDIR="$(cd "$PWD" && pwd)"
	SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	NAME="$(basename "${BASH_SOURCE[0]}")"

	echo "*** Running docker toolchain $DOCKER_IMAGE"
	echo "    with script: $NAME"
	echo "    with appdir $APPDIR"
	docker run --rm -t -v "$APPDIR:/app" $DOCKER_IMAGE "/app/${SCRIPTDIR/$APPDIR/}/$NAME" "$@"

	exit
fi

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
	CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

scons -j${CPU} "$@" platform=frt frt_arch=arm64v8 frt_cross=aarch64-libreelec-linux-gnueabi target=release disable_3d=true

mkdir -p bin/templates/frt
mv -v bin/godot.frt.opt.arm64v8 bin/templates/frt/
