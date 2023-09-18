# custom_devices.py
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

import os
import sys
import errno

# preconfigured device configuration
def get_devices():
    return ["gcw0", "odroid", "rg351"]
