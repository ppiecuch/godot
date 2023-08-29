#!/bin/bash

set -e

if [ ! -d .git ]; then
	echo "*** Missing root .git folder"
	exit 1
fi

dt=$(date '+%Y%m%d_%H'h'%M')
fn="source-tree-$dt.tar.bz2"

trap "{ if [ -f "$fn" ]; then rm "$fn"; fi; exit 255; }" SIGINT SIGTERM ERR EXIT

if [ -f "$fn" ]; then
	rm "$fn"
fi

echo "** Creating archive $dt .."
git archive master | bzip2 > "$fn"

if [ -f .cache/last-export ]; then
	source .cache/last-export
fi

# export.sh rsync=user@host:path

if [ $# -gt 0 ]; then
	echo "# Created $(date)" > .cache/last-export
	for var in "$@"; do
		eval "$var"
		echo "$var" >> .cache/last-export
	done
fi

if [ ! -z "$rsync" ]; then
	echo "** Running rsync -av $fn $rsync"
	rsync -av "$fn" "$rsync"
fi

if [ ! -z "$sshcmd" ]; then
	ssh $sshcmd
fi

echo "** Cleanup .."
rm "$fn"
