#!/bin/bash

CROSS="/opt/gcw0-toolchain/usr/bin"

CC="mipsel-gcw0-linux-uclibc-gcc"
CXX="mipsel-gcw0-linux-uclibc-g++"
LD="mipsel-gcw0-linux-uclibc-g++"
AR="mipsel-gcw0-linux-uclibc-ar"
STRIP="mipsel-gcw0-linux-uclibc-strip"

if [ ! -e "$CROSS/$CC" ]; then
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

	DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	NAME="$(basename "${BASH_SOURCE[0]}")"
	VERSION=2014-08-20

	echo "*** Running docker toolchain $VERSION (with script $NAME).."
	docker run --rm -t -v "$DIR:/app" gcw_zero_dev:$VERSION "./$NAME"
	exit
fi

PATH=/usr/bin:/bin:/sbin:/usr/local/bin:$CROSS
scons -j 2 platform=frt frt_arch=gcw0 target=release disable_experimental=yes
