#!/bin/bash

export RPXC_IMAGE="rpi_toolchain"

# update platform repository
(cd platform/frt; git pull)

docker run ${RPXC_IMAGE} > ./bin/rpxc
chmod +x ./bin/rpxc
echo "*** Using gcc/toolchain version $(./bin/rpxc -- arm-linux-gnueabihf-gcc --version)"

device="pi3"

if [ ! -z "$1" ]; then
	device="$1"
	echo "*** Using device: ${device}"
	pause 3
fi

./bin/rpxc -- scons -j2 platform=frt frt_arch=${device} verbose=yes warnings=all werror=yes target=release tools=no
