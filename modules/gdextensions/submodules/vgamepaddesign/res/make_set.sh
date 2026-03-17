#!/bin/bash

set -e

#trap \
# "{ rm -rf set1 ; exit 255; }" \
# SIGINT SIGTERM ERR EXIT

#~/Private/Projekty/0.shared/common-dev-tools/res_tools/c-embed/c-embed \
#    starfield_res \
#    *.png anim/*.png

#sed -i '' 's/embed_/starfield_/g' *.c *.h
#sed -i '' 's/starfield_starfield_/embed_starfield_/g' *.c *.h

GODOT="../../../.."
TEXPACK="$HOME/Private/Projekty/0.shared/common-dev-tools/texture_tools/texpack"

rm -rf set1 && mkdir set1

cp ../design/controls/*.png set1
rm set1/DPad_Decor.png
mogrify -resize 256\> set1/Joystick_Back.png
mogrify -resize 256\> set1/DPad_*.png
cp ../design/controls/DPad_Decor.png set1
$TEXPACK/bin/texpack -s set1 -o set1
echo "" >> set1_0.cat
if [ -e $GODOT//misc/tools/qoiconv ]; then
  $GODOT/misc/tools/qoiconv set1_0.png set1_0.qoi
fi
