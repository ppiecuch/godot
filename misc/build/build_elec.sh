#!/bin/bash

set -e

DOCKER_IMAGE="351elec/351elec-build:latest"

# update platform repository
(cd platform/frt; git pull)

if [ ! -d "/app" ]; then
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

	echo "*** Running docker toolchain $DOCKER_IMAGE (with script $NAME).."
	docker run --rm -t -v "$APPDIR:/app" $DOCKER_IMAGE "./${SCRIPTDIR/$APPDIR/}/$NAME"

	exit
fi

PATH=/usr/bin:/bin:/sbin:/usr/local/bin
scons -j2 platform=frt frt_arch=rg351 target=release disable_3d=true

mkdir -p bin/templates/frt
mv -v bin/godot.frt.opt.aarch64.rg351 bin/templates/frt/
