#!/bin/bash

set -e

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
        CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

export VITASDK=/opt/vitasdk
export PATH=$VITASDK/bin:/bin:/sbin:/usr/bin:/opt/local/bin

scons $* p=psvita target=release disable_3d=true disable_advanced_gui=true

mkdir -p bin/templates/psvita
mv -v bin/godot.psvita.opt.arm bin/templates/psvita/
