#!/bin/bash

set -e

# Compiler requirments:
# ---------------------
# port install mpfr
#

export PSPSDK=/opt/pspdev

export PATH=$PSPSDK/bin:/bin:/sbin:/usr/bin:/opt/local/bin

scons p=psp target=release use_mingw=true disable_3d=true disable_advanced_gui=true

mkdir -p bin/templates/psp
mv -v bin/godot.psp.opt.arm bin/templates/psp/
