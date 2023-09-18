# detect_godot3.py
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

import os
import sys
import errno
import platform
import subprocess
import version
import methods
from custom_devices import get_devices

# TODO factor out common bits

sysroot = os.environ["SYSROOT"] if "SYSROOT" in os.environ else ""
pkgenvstr = ""
if sysroot:
    pkgenvstr = (
        "PKG_CONFIG_DIR= PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR="
        + sysroot
        + "/usr/share/pkgconfig:"
        + sysroot
        + "/opt/vc/lib/pkgconfig"
        + " "
    )


def checkexe(exe):
    try:
        output = subprocess.check_output(exe).strip().splitlines()
        for ln in output:
            print("> " + str(ln, "utf-8"))
    except OSError as e:
        if e.errno == errno.ENOENT:
            return False
    return True


def checkpkgcfg(env, cmd):
    env.ParseConfig("%s %s" % (pkgenvstr, cmd))


def using_gcc_min_ver(env, major=-1, minor=-1):
    if methods.using_gcc(env):
        ver = methods.get_compiler_version(env) or [-1, -1]
        if ver[0] > major or (ver[0] == major and ver[1] >= minor):
            return True
    return False


def using_gcc_ver(env, major, minor):
    if methods.using_gcc(env):
        ver = methods.get_compiler_version(env) or [-1, -1]
        if ver[0] == major and ver[1] == minor:
            return True
    return False


def get_opts():
    from SCons.Variables import BoolVariable

    return [
        BoolVariable("use_llvm", "Use llvm compiler", False),
        BoolVariable("use_lto", "Use link time optimization", False),
        BoolVariable("use_static_cpp", "Link libgcc and libstdc++ statically", False),
        ("frt_std", "C++ standard for frt itself (no/auto/c++98/...)", "auto"),
        ("frt_arch", "Architecture (no/arm32v6/arm32v7/arm64v8/amd64/" + "/".join(get_devices()) + ")", "no"),
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
    if not env["use_lto"]:
        return
    if env["use_llvm"]:
        env.Append(CCFLAGS=["-flto=thin"])
        env.Append(LINKFLAGS=["-fuse-ld=lld", "-flto=thin"])
        env["AR"] = "llvm-ar"
        env["RANLIB"] = "llvm-ranlib"
    else:
        env.Append(CCFLAGS=["-flto"])
        env.Append(LINKFLAGS=["-flto"])
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
    env.Append(LIBS=["pthread", "z", "dl"])
    if env["frt_arch"] == "arm32v6" and version.minor >= 4:  # TODO find out exact combination
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
