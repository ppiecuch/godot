#!/bin/bash

for m in gd_sqlite gd_chipmunk gd_liquidfun gd_spine gd_error_handler; do
    echo "*** checking module $m"
	if [ -e modules/$m/.git ]; then
		( cd modules/$m; git pull )
	else
		git clone https://github.com/ppiecuch/$m modules/$m
	fi
done
