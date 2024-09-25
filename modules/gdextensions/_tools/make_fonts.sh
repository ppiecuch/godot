#!/bin/bash

# Convert to 1-bit bmp:
# ---------------------
# > convert fonts.altered/dos-8x8@24.bmp -monochrome -depth 1 fonts.altered/dos-8x8@1.bmp

set -e

DEVTOOLS="~/Private/Projekty/0.shared/common-dev-tools"

$DEVTOOLS/res_tools/c-embed/c-embed \
    dos_font_data \
    fonts.altered/*.bmp

sed -i '' -e 's/embed_1/font_4x6/g' dos_font_data.*
sed -i '' -e 's/embed_2/font_7x9/g' dos_font_data.*
sed -i '' -e 's/embed_3/font_8x12/g' dos_font_data.*
sed -i '' -e 's/embed_4/font_8x16/g' dos_font_data.*
sed -i '' -e 's/embed_5/font_8x8/g' dos_font_data.*

mv -v dos_font_data.c dos_font_data_rgb.h
gcc -o conv_rgb_to_8 conv_rgb_to_8.cpp
./conv_rgb_to_8
