#!/bin/bash

set -e

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
	CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

echo_header() {
	if [[ "$TERM" =~ "xterm" ]]; then
		printf "\e[1;4m$1\e[0m\n"
	else
		printf "[$1]\n"
	fi
}

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

mkdir -p bin/templates

echo_header "*** Building server/release engine for Linux ..."
scons -j${CPU} p=server target=release tools=no
mv -v bin/godot_server.x11.opt.64 bin/templates/

echo_header "*** Building template/release engine for Linux ..."
scons -j${CPU} p=linux target=release tools=no
mv -v bin/godot.x11.opt.64 bin/templates/
