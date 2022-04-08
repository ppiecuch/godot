#!/bin/bash

set -e

# extract alpha channel: mogrify -alpha extract *.png

for v in S M L; do
	case $v in
		S) scale='15%'
		;;
		M) scale='25%'
		;;
		L) scale='70%'
		;;
	esac
	echo "Build variant $v .."
	for f in source/*.png; do
		convert -resize $scale $f $v/$(basename $f)
	done
done
