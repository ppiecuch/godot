#!/bin/bash

# https://github.com/bruvzg/gdsdecomp.git
# https://github.com/pchasco/gd2c-py

set -e

for m in gd_distrand gd_sqlite gd_unqlite gd_chipmunk gd_liquidfun gd_spine gd_vector_graphics gd_error_handler gd_raknet gd_enet gd_imagetools gd_geomtools; do
	echo "+--"
    echo "| checking module: $m"
    echo "+--"
    if [ -e modules/$m/.git ]; then
        pushd modules/$m; git pull --recurse-submodules; popd
    else
        git clone --recursive https://github.com/ppiecuch/$m modules/$m
    fi
done
