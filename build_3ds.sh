#!/bin/bash

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

export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export CTRULIB=$DEVKITPRO/libctru

export PATH=$DEVKITARM/bin:$DEVKITPRO/tools/bin:/bin:/sbin:/usr/bin:/opt/local/bin

scons p=3ds target=release use_mingw=true disable_3d=true disable_advanced_gui=true disable_experimental=yes
