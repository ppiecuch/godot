#!/bin/bash

set -e

rm -rf client common *.zip
curl -L -O https://github.com/google/breakpad/archive/refs/heads/master.zip
tar -xvf master.zip

mkdir client
mv breakpad-master/src/client/ios client/
mv breakpad-master/src/client/linux client/
mv breakpad-master/src/client/mac client/
mv breakpad-master/src/client/windows client/
mv breakpad-master/src/client/*.h client/
mv breakpad-master/src/client/*.cc client/
mkdir common
mv breakpad-master/src/common/android common/
mv breakpad-master/src/common/linux common/
mv breakpad-master/src/common/mac common/
mv breakpad-master/src/common/windows common/
mv breakpad-master/src/common/*.h common/
mv breakpad-master/src/common/*.cc common/

rm *.zip
rm -rf breakpad-master
