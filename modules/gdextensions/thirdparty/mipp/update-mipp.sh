#!/bin/bash

set -e

rm -rf include tests README.md *.zip
curl -L -O https://github.com/aff3ct/MIPP/archive/refs/heads/master.zip
tar -xvf master.zip
rm *.zip
mv MIPP-master/README.md README.md
mv MIPP-master/src include
mv MIPP-master/tests/src tests
rm -rf MIPP-master
