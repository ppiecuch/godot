## Building with GCW-Zero (GCW0) toolchain

Experimental build of _Godot3_ template with gcw-zero toolchain (_mipsel_ architecture).

```
$ ls -lh bin/godot.frt.opt.mipsel.gcw0
-rwxr-xr-x  1 piecuchp  staff    70M Aug 24 16:34 bin/godot.frt.opt.mipsel.gcw0
```

(this compilation is quite big since it contains about 25 of my own modules - some of them are quite big like _ffmpeg_ - propably plain vanilla _Godot_ would be half of this).

I donot have any GCW0 compatible device so I have no idea if it works or not (propably _not_) - I was only interested if _Godot_ can be build with this toolchain.

__Btw:__ requires changes from my https://github.com/ppiecuch/frt and https://github.com/ppiecuch/godot - not much but still (commits are prefixed with _gcw0_ if anyone is interested).

### Dockerized toolchain:

To make building process easier and more cross-platform, I am using dockerized __GCW0__ toolchain. _Dockerfile_ for building an image is like this:

```
FROM i386/ubuntu:xenial

MAINTAINER Pawel Piecuch <piecuch.pawel@gmail.com>

ARG version

LABEL version=$version
LABEL description="GCW Zero development toolchain"

RUN mkdir -p /opt

RUN apt-get update \
  && apt-get install -y bzip2 less curl scons libx11-dev nano make ca-certificates \
  && apt-get autoclean \
  && rm -rf /var/lib/apt/lists/* \
  && echo "*** Fetching toolchain $version ..." \
  && cd /opt \
  && curl -sL http://www.gcw-zero.com/files/opendingux-gcw0-toolchain.$version.tar.bz2 | tar xj

VOLUME ["/app"]
WORKDIR /app/

CMD ["uname", "-a"]
```

And building script is like this:

```
#!/bin/bash

set -e

version=2014-08-20
docker build --build-arg version=$version -t gcw_zero_dev:$version .
```

### frt platform

To simplify toolchain configuration I have added gcw0 configuration as another device (so it is enough to specify at call ```scons platform=frt frt_arch=gcw0```:

```
	elif env['frt_arch'] == 'gcw0':
		env.Append(CPPDEFINES=['__GCW0__', 'PTHREAD_NO_RENAME'])
		# we need /usr/include for X11 headers only - there are part of the gcw0-toolchain
		# docker image; no other headers should be installed/available
		env.Append(CPPFLAGS=['-I/usr/include', '-fno-strict-aliasing'])
		# it is needed by ffmepg, but we can add this here anyhow
		env.Append(LIBS=["iconv"])
		env['CC'] = 'mipsel-linux-gcc'
		env['CXX'] = 'mipsel-linux-g++'
		env['LD'] = 'mipsel-linux-g++'
		env['AR'] = 'mipsel-linux-ar'
		env['STRIP'] = 'mipsel-linux-strip'
		env['arch'] = 'mipsel'
		env.extra_suffix += '.gcw0'
```

(that change and some others necessery for gcw0 are in my fork at https://github.com/ppiecuch/frt - look there for actual version)

### Build script

Having a build image in place we can build an application (eg. using ```(build_gcw0.sh)[https://github.com/ppiecuch/godot/blob/master/build_gcw0.sh]``` script):

```
#!/bin/bash

CROSS="/opt/gcw0-toolchain/usr/bin"

CC="mipsel-gcw0-linux-uclibc-gcc"
CXX="mipsel-gcw0-linux-uclibc-g++"
LD="mipsel-gcw0-linux-uclibc-g++"
AR="mipsel-gcw0-linux-uclibc-ar"
STRIP="mipsel-gcw0-linux-uclibc-strip"

if [ ! -e "$CROSS/$CC" ]; then
	# toolchain not found - run docker image
	if ! command -v docker &> /dev/null
	then
		echo "*** Docker is not found - cannot run build script."
		exit
	fi
	docker_state=$(docker info >/dev/null 2>&1)
	if [[ $? -ne 0 ]]; then
		echo "*** Docker does not seem to be running, run it first."
		exit 1
	fi

	DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	NAME="$(basename "${BASH_SOURCE[0]}")"
	VERSION=2014-08-20

	echo "*** Running docker toolchain $VERSION (with script $NAME).."
	docker run --rm -t -v "$DIR:/app" gcw_zero_dev:$VERSION "./$NAME"
	exit
fi

PATH=/usr/bin:/bin:/sbin:/usr/local/bin:$CROSS
scons -j 2 platform=frt frt_arch=gcw0 target=release
```

### Reference:

 - frt platform: https://github.com/efornara/frt
 - Custom export templates for Raspberry Pi: https://www.reddit.com/r/godot/comments/b0yaar/frt_custom_export_templates_for_raspberry_pi_and/
 - gcw-zero toolchain: http://www.gcw-zero.com/develop
