#!/bin/bash
set -e

tp="$HOME/Private/Projekty/0.shared/common-dev-tools/texture_tools/texpack/bin/texpack"

if [ ! -e "$tp" ]; then
    echo "*** texpack not found."
    exit 1
fi

fmt='{"%1$s","%2$s","%3$s",%4$u,%5$u,%6$u,%7$u,%8$u,%9$f,%10$f,%11$f,%12$f,%13$u,%14$u,%15$f,%16$f,%17$u,%18$u},'
rm -f _flist.txt

ls caust/*.bmp drops/*.tga noise/*.tga env.png >> _flist.txt

atlas="water_2d_atlas"

# --
$tp -i -l _flist.txt -o ${atlas}_ -F $fmt -b 3 -H 2048 -W 2048 -E > ${atlas}.h

grep "#define" ${atlas}.h > ${atlas}_id.h
ls -l ${atlas}_0.*

# --
rm _flist.txt
