#!/bin/bash

set -e

APPDIR="$(cd "$PWD" && pwd)"
SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source "$SCRIPTDIR/build_functions.sh"

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
	CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

export VITASDK=/opt/vitasdk
export PATH=$VITASDK/bin:/bin:/sbin:/usr/bin:/opt/local/bin

if [[ ! -d $VITASDK ]]; then
	# try run this script in docker (using retro_dev image)
	echo "VITASDK not found. Trying to run in Docker using retro_dev image .."
	_run_in_docker retro_dev $(basename "${BASH_SOURCE[0]}")
fi

scons $* -j$CPU p=psvita target=release disable_3d=true disable_advanced_gui=true

mkdir -p bin/templates/psvita
mv -v bin/godot.psvita.opt.arm bin/templates/psvita/
