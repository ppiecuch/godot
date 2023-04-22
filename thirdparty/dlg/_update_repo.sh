#!/bin/bash

set -e

trap "{ if [ -d '_dlg' ]; then rm -rf _dlg; fi; exit 255; }" SIGINT SIGTERM ERR EXIT

git clone --depth=1 --recursive https://github.com/nyorain/dlg.git _dlg

rm -rf LICENSE README.md dlg include dlg.c

mv _dlg/LICENSE .
mv _dlg/README.md .
mv _dlg/src/dlg/dlg.c .
mkdir include && mv _dlg/include/dlg include/
