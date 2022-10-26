#!/bin/bash
~/Private/Projekty/0.shared/common-dev-tools/res_tools/c-embed/c-embed \
    water_2d_res \
    *.png caust/*.bmp drops/*.tga noise/*.tga

sed -i '' 's/embed_/water2d_/g' *.c *.h
sed -i '' 's/water2d_water2d_/embed_water2d_/g' *.c *.h
