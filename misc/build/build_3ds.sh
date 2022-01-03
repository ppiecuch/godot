#!/bin/bash

set -e

# https://github.com/Cruel/godot/tree/3ds
# https://gbatemp.net/threads/wip-godot-engine-for-3ds-homebrew.446385/

# How to install
# --------------
# required:
#  - devkitARM
#  - 3ds-zlib
#  - 3ds-libpng
#  - libctru
#  - citra3d
#  - picasso

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
	CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export CTRULIB=$DEVKITPRO/libctru

export PATH=$DEVKITARM/bin:$DEVKITPRO/tools/bin:/bin:/sbin:/usr/bin:/opt/local/bin

scons -j$CPU p=3ds target=release disable_3d=true disable_advanced_gui=true

mkdir -p bin/templates/3ds
mv -v bin/godot.3ds.opt.arm bin/templates/3ds/
