#!/bin/bash

set -e

echo "==> Cleanup."
rm -rf MIPP-master include tests codegen README.md *.zip
echo "==> Download."
curl -L -O https://github.com/aff3ct/MIPP/archive/refs/heads/master.zip
tar -xvf master.zip
rm *.zip
echo "==> Organize."
mv MIPP-master/README.md README.md
mv MIPP-master/codegen codegen
mv MIPP-master/include include
mv MIPP-master/tests/src tests
rm -rf MIPP-master
echo "==> Done."
