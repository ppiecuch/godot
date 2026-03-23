# detect.py
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2025  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

import os
import sys
import errno
import platform
import subprocess
import version
import methods


def checkexe(exe):
    try:
        output = subprocess.check_output(exe).strip().splitlines()
        for ln in output:
            print("> " + str(ln, "utf-8"))
    except OSError as e:
        if e.errno == errno.ENOENT:
            return False
    return True


def get_name():
    return "FRT"


def is_active():
    return True


def can_build():
    import os

    if os.name != "posix":
        return False
    return True


def get_opts():
    from SCons.Variables import BoolVariable

    return [
        BoolVariable("use_llvm", "Use llvm compiler", False),
        BoolVariable("use_static_cpp", "Link libgcc and libstdc++ statically", False),
        ("frt_arch", "Architecture (no/arm32v6/arm32v7/arm64v8/amd64)", "no"),
        ("frt_cross", "Cross compilation (no/auto/<triple>)", "no"),
    ]


def get_flags():
    return [
        ("tools", False),
    ]


def configure_compiler(env):
    if env["use_llvm"]:
        env["CC"] = "clang"
        env["CXX"] = "clang++"
        env["LD"] = "clang++"
        env.extra_suffix += ".llvm"


def configure_lto(env):
    if not env["use_llvm"] and env["lto"] and env["lto"] != "none":
        env.Append(CCFLAGS=["-flto"])
        env.Append(LINKFLAGS=["-flto"])
        if env["frt_cross"] == "no":
            env["AR"] = "gcc-ar"
            env["RANLIB"] = "gcc-ranlib"
        env.extra_suffix += ".lto"


def configure_arch(env):
    if env["frt_arch"] == "arm32v6":
        env.Append(CCFLAGS=["-march=armv6", "-mfpu=vfp", "-mfloat-abi=hard"])
        env.extra_suffix += ".arm32v6"
    elif env["frt_arch"] == "arm32v7":
        env.Append(CCFLAGS=["-march=armv7-a", "-mfpu=neon-vfpv4", "-mfloat-abi=hard"])
        env.extra_suffix += ".arm32v7"
    elif env["frt_arch"] == "arm64v8":
        env.Append(CCFLAGS=["-march=armv8-a"])
        env.extra_suffix += ".arm64v8"
    elif env["frt_arch"] == "amd64":
        env.extra_suffix += ".amd64"
    elif env["frt_arch"] != "no":
        env.extra_suffix += "." + env["frt_arch"]


def configure_cross(env):
    if env["frt_cross"] == "no":
        env["FRT_PKG_CONFIG"] = "pkg-config"
        return
    if env["frt_cross"] == "auto":
        triple = {
            "arm32v6": "arm-linux-gnueabihf",
            "arm32v7": "arm-linux-gnueabihf",
            "arm64v8": "aarch64-linux-gnu",
            "amd64": "x86_64-linux-gnu",
        }[env["frt_arch"]]
    else:
        triple = env["frt_cross"]
    if env["use_llvm"]:
        env.Append(CCFLAGS=["-target", triple])
        env.Append(LINKFLAGS=["-target", triple])
    else:
        env["CC"] = triple + "-gcc"
        env["CXX"] = triple + "-g++"
        env["AR"] = triple + "-gcc-ar"
        env["RANLIB"] = triple + "-gcc-ranlib"
    env["FRT_PKG_CONFIG"] = triple + "-pkg-config"
    if not checkexe([env["CC"], "--version"]):
        print("*** Cannot find %s toolchain." % env["CC"])


def configure_target(env):
    if env["target"] == "release":
        env.Append(CCFLAGS=["-O2", "-ffast-math", "-fomit-frame-pointer"])
    elif env["target"] == "release_debug":
        env.Append(CCFLAGS=["-O2", "-ffast-math"])
    elif env["target"] == "debug":
        env.Append(CCFLAGS=["-g2"])


def configure_misc(env):
    env.Append(CPPPATH=["#platform/frt"])
    env.Append(CPPFLAGS=["-DUNIX_ENABLED", "-DGLES2_ENABLED", "-DGLES_ENABLED", "-DJOYDEV_ENABLED"])
    if env["disable_3d"]:
        env.Append(CPPFLAGS=["-DGLES3_DISABLED"])
    env.Append(CPPFLAGS=["-DFRT_ENABLED"])
    env.Append(CFLAGS=["-std=gnu11"])  # for libwebp (maybe more in the future)
    env.Append(LIBS=["pthread", "z", "dl", "EGL"])
    if env["frt_arch"] == "arm32v6" and version.minor >= 4:
        env.Append(LIBS=["atomic"])
    if env["CXX"] == "clang++":
        env["CC"] = "clang"
        env["LD"] = "clang++"
    if env["use_static_cpp"]:
        env.Append(LINKFLAGS=["-static-libgcc", "-static-libstdc++"])
    env["ENV"]["PATH"] = os.getenv("PATH")
    env["ENV"]["LD_LIBRARY_PATH"] = os.getenv("LD_LIBRARY_PATH")


def configure(env):
    configure_compiler(env)
    configure_lto(env)
    configure_arch(env)
    configure_cross(env)
    configure_target(env)
    configure_misc(env)
