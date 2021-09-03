FROM sdthirlwall/raspberry-pi-cross-compiler

# Install BCM firmware
RUN curl -L https://github.com/raspberrypi/firmware/tarball/master \
	| tar --strip-components 1 --directory $SYSROOT --wildcards -xzf - "*/opt/vc"

# Jessie is no longer available from raspbian.org
RUN sed -i "s~archive.raspbian.org/raspbian~mirrors.sohu.com/raspbian/raspbian~g" $SYSROOT/etc/apt/sources.list

# Install some native and raspbian build-time tools
RUN install-debian --update pkg-config scons \
	&& install-raspbian --update libgles2-mesa-dev libpng-dev libz-dev
