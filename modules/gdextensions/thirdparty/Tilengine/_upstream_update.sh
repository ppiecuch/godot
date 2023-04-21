#!/bin/bash

set -e

trap "{ if [ -d '_tileeng' ]; then rm -rf _tileeng; fi; exit 255; }" SIGINT SIGTERM ERR EXIT

rm -rf _tileeng
git clone --depth=1 --recursive --no-single-branch https://github.com/megamarc/Tilengine.git _tileeng
rm -rf Tilengine.pdf include src
mv _tileeng/Tilengine.pdf .
mv _tileeng/include .
mv _tileeng/src .

rm -rf _tileeng

echo "**"
echo "** PATCHING .."
echo "**"

patch -p 1 < patch.txt
