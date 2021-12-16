#!/bin/bash

set -e

(cd modules; for m in $(ls -d gd_*|xargs); do
	(cd $m; if [ -e .git ]; then
		echo "+---"
		echo -e "| checking module \033[1;4m$m\033[0m ($(git remote get-url origin))"
		echo "+---"
		git status -uno | grep -v "nothing to commit" | grep -v "(use " | grep .
		echo ""
	fi)
done)
