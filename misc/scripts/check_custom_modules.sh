#!/bin/bash

set -e

(cd modules; for m in $(ls -d gd_*|xargs); do
	(cd $m; if [ -e .git ]; then
		echo "+---"
		echo "| checking module $m ($(git remote get-url origin))"
		echo "+---"
		git status $m | grep -v "nothing to commit" | grep .
		echo ""
	fi)
done)
