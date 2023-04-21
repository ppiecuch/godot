#!/bin/bash

set -e

trap "{ if [ -d '_clwrapper' ]; then rm -rf _clwrapper; fi; exit 255; }" SIGINT SIGTERM ERR EXIT

rm -rf _clwrapper
git clone --depth=1 --recursive --no-single-branch https://github.com/ProjectPhysX/OpenCL-Wrapper.git _clwrapper
rm -rf README.md src
mv _clwrapper/README.md .
mv _clwrapper/src .

rm -rf _clwrapper

echo "**"
echo "** PATCHING .."
echo "**"

patch -p 1 < patch.txt
