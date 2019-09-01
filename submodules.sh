#!/bin/bash

for m in gd_sqlite gd_chipmunk gd_liquidfun gd_spine gd_vector_graphics gd_error_handler; do
    echo "***"
    echo "*** checking module $m"
    echo "***"
    if [ -e modules/$m/.git ]; then
        pushd modules/$m; git pull; popd
    else
        git clone --recursive https://github.com/ppiecuch/$m modules/$m
    fi
done
