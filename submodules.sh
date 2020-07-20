#!/bin/bash

# Misc repos:
# -----------
# https://github.com/pchasco/gd2c-py


set -e

ext=" \
 https://github.com/bruvzg/gdsdecomp.git \
"

for m in \
	gd_distrand gd_behavior_tree \
	gd_sqlite gd_unqlite \
	gd_chipmunk gd_liquidfun \
	gd_spine gd_dragonbones \
	gd_vector_graphics gd_polyvector \
	gd_raknet gd_enet \
	gd_goost; do
	echo "+--"
    echo "| checking module: $m"
    echo "+--"
    if [ -e modules/$m/.git ]; then
        pushd modules/$m; git pull --recurse-submodules; popd
    else
        git clone --recursive https://github.com/ppiecuch/$m modules/$m
    fi
done
