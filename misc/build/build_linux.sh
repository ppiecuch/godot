#!/bin/bash

set -e

if [ $(uname) == "Darwin" ]; then
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
	VERSION=latest

	echo "*** Running docker toolchain $VERSION (with script $NAME).."
	docker run --rm -t -v "$APPDIR:/app" linux_dev:$VERSION "./${SCRIPTDIR/$APPDIR/}/$NAME"

	exit
fi

PATH=/usr/bin:/bin:/sbin:/usr/local/bin
scons -j2 p=server target=release tools=no

mv -v bin/godot_server.x11.opt.64 bin/templates/
