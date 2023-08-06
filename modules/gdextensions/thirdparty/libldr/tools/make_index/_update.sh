#!/bin/bash

set -e

if [ -d $HOME/Documents/ldraw ]; then
	./mkindex $HOME/Documents/ldraw

	if [ -e parts.db ]; then
	ls -lh parts.db
	gzip -f parts.db
	ls -lh parts.db.gz
	cp -v  parts.db.gz ../libldr/ldrawlib/ldraw_lib.content.gz
	fi

	ls -lh catalog.inl colors.inl debug.inl

	cp -v catalog.inl colors.inl debug.inl ../libldr/ldrawlib/
else
	echo "LDraw folder not found: $HOME/Documents/ldraw"
	exit 1
fi
