#!/bin/bash

set -e

ROOT=~/Private/Workspace/embedFiglet
CONV=$ROOT/scripts/FigletFontConvert.rb

ruby $CONV $ROOT/res/figfonts/Calvin\ S.flf > figlet_font_calvins.cpp
ruby $CONV $ROOT/res/figfonts/maxiwi.flf > figlet_font_maxiwi.cpp
ruby $CONV $ROOT/res/figfonts/maxii.flf > figlet_font_maxii.cpp
ruby $CONV $ROOT/res/figfonts/DOS\ Rebel.flf > figlet_font_dosrebel.cpp
ruby $CONV $ROOT/res/figfonts/ANSI\ Regular.flf > figlet_font_ansiregular.cpp

sed -i '' \
	-e 's/Calvin S/calvins/g' \
	-e 's/DOS Rebel/dosrebel/g' \
	-e 's/ANSI Regular/ansiregular/g' \
	-e 's/Figlet.hh/figlet_font.h/g' \
	figlet_font_*.cpp
