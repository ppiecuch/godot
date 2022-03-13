#!/bin/bash

set -e

source build_functions.sh

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
	CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

if [ ! -d /opt/vitasdk ]; then
	# try run in docker
	_run_in_docker retro_dev
fi

export VITASDK=/opt/vitasdk
export PATH=$VITASDK/bin:/bin:/sbin:/usr/bin:/opt/local/bin

scons $* p=psvita target=release disable_3d=true disable_advanced_gui=true

mkdir -p bin/templates/psvita
mv -v bin/godot.psvita.opt.arm bin/templates/psvita/
