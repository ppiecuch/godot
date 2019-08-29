#!/bin/bash

for m in gd_sqlite gd_chipmunk gd_liquidfun gd_spine gd_error_handler; do
	if [ -e modules/$m ]; then
		git update modules/$m
	else
		git clone https://github.com/ppiecuch/$m modules/
	fi
done
