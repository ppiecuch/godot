#!/bin/bash

set -e

trap "{ if [ -d '_yamlcpp' ]; then rm -rf _yamlcpp; fi; exit 255; }" SIGINT SIGTERM ERR EXIT

git clone --depth=1 --recursive --no-single-branch https://github.com/jbeder/yaml-cpp _yamlcpp
rm -rf include src
mv _yamlcpp/include .
mv _yamlcpp/src .

rm -rf _yamlcpp
