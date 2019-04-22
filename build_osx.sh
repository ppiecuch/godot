#!/bin/bash

set -e

scons platform=osx

echo "*** Packaging app ..."

cp -rv misc/dist/osx_tools.app bin/Godot-master.app
mkdir -p bin/Godot-master.app/Contents/MacOS
mv bin/godot.osx.tools.64 bin/Godot-master.app/Contents/MacOS/Godot
