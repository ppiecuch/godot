#!/bin/bash

# https://github.com/Cruel/godot/tree/3ds
# https://gbatemp.net/threads/wip-godot-engine-for-3ds-homebrew.446385/

# How to install
# --------------
# required:
#  - libctru
#  - devkitARM
#  - 3ds-zlib
#  - 3ds-libpng

export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export CTRULIB=$DEVKITPRO/libctru

export PATH=$DEVKITARM/bin:$PATH

scons p=3ds disable_3d=true target=release use_mingw=true
