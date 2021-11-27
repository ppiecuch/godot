#!/bin/bash

set -e

trap \
 "{ rm -rf stb_image.h stb_image_write.h ; exit 255; }" \
 SIGINT SIGTERM ERR EXIT

curl -o stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -o stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

gcc -I ../../thirdparty/misc -o qoiconv qoiconv.c

rm -rf stb_image.h stb_image_write.h
