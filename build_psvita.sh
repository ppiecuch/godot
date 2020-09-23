#!/bin/bash

export VITASDK=/opt/vitasdk

export PATH=$VITASDK/bin:/bin:/sbin:/usr/bin:/opt/local/bin

scons p=psvita target=release use_mingw=true disable_3d=true disable_advanced_gui=true disable_experimental=yes
