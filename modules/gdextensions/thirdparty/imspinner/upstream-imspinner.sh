#!/bin/bash

set -e

echo "Downloading .."
curl -s -o imspinner_last.h https://raw.githubusercontent.com/dalerank/imspinner/master/imspinner.h
cp imspinner_last.h imspinner_patch.h
out=$(patch --verbose imspinner_patch.h patch.txt || true)
if echo "$out" | grep "FAILED" -q; then
	echo "$out"
	echo "**"
	echo "** Patching failed - check and resolve rejected chunks."
	echo "**"
	exit
else
	echo "$out"
fi

if [ -f imspinner_patch.h.orig ]; then
	echo "imspinner_patch.h.orig exists - check patch results and remove the file."
fi

# prepare a new patch, if all ok so far
diff -Nur imspinner_last.h imspinner_patch.h > patch.txt || true
cp imspinner_patch.h imspinner.h
# cleanup
rm imspinner_last.h imspinner_patch.h

echo "Done."
