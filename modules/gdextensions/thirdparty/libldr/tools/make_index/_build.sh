#!/bin/bash

set -e

QT=$HOME/Qt/5.15-static/clang_64

if [ -d "$QT" ]; then
	$QT/bin/qmake -r Tools.pro
else
	echo "Qt not found at $QT"
	exit 1
fi
