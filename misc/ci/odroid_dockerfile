FROM amd64/ubuntu:bionic

MAINTAINER Pawel Piecuch <piecuch.pawel@gmail.com>

ARG version
ARG root
ARG toolchain

LABEL version=$version
LABEL description="ODROID HK development toolchain"

RUN mkdir -p /opt/usr/include/api

RUN apt-get update \
  && apt-get install -y bzip2 less curl scons pkg-config nano make ca-certificates \
  && apt-get autoclean \
  && rm -rf /var/lib/apt/lists/* \
  && cd /opt \
  && echo "*** Fetching toolchain $version ..." \
  && curl -skL $toolchain | tar xJ \
  && echo "*** Fetching Khronos headers ..." \
  && curl -L https://github.com/KhronosGroup/OpenGL-Registry/tarball/main \
  | tar --strip-components 2 --directory /opt/usr/include/api --wildcards -xzf - "*/api" \
  && curl -L https://github.com/KhronosGroup/EGL-Registry/tarball/main \
  | tar --strip-components 2 --directory /opt/usr/include/api --wildcards -xzf - "*/api"

ENV PKG_CONFIG_PATH=/opt/usr/share/pkgconfig

VOLUME ["/app"]
WORKDIR /app/

CMD ["uname", "-a"]
