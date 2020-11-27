import os
import sys
import platform

import version

if version.major > 2:
    yes = True
    no = False
else:
    yes = "yes"
    no = "no"


def is_active():

    return True


def get_name():

    return "psvita"


def can_build():

    return True


def get_opts():

    return [
        ("debug_release", "Add debug symbols to release version", "no"),
    ]


def get_flags():

    return [
        ("tools", False),
        ("module_squish_enabled", False),
        ("module_theora_enabled", False),
        ("module_dds_enabled", False),
        ("module_upnp_enabled", False),
        ("module_webm_enabled", False),
        ("builtin_zlib", True),
        ("builtin_libpng", True),
        ("builtin_pcre2_with_jit", False),
    ]


def check(env, key):

    if not (key in env):
        return False
    if version.major > 2:
        return env[key]
    else:
        return env[key] == "yes"


def configure(env):

    env["bits"] = "32"
    env["arch"] = "arm"

    env.Append(CPPPATH=["#platform/psvita"])
    env["CC"] = "arm-vita-eabi-gcc"
    env["CXX"] = "arm-vita-eabi-g++"
    env["LD"] = "arm-vita-eabi-g++"
    env["AR"] = "arm-vita-eabi-ar"
    env["RANLIB"] = "arm-vita-eabi-ranlib"
    env["AS"] = "arm-vita-eabi-as"

    env.Append(CCFLAGS=["-D__psp2__", "-fno-strict-aliasing"])

    env.Append(
        CCFLAGS=[
            "-Wno-maybe-uninitialized",
            "-Wno-switch",
            "-Wno-sign-compare",
            "-Wno-shadow=compatible-local",
            "-Wno-char-subscripts",
            "-Wno-return-local-addr",
            "-Wno-alloc-size-larger-than",
            "-Wno-format-overflow",
        ]
    )
    env.Append(
        CXXFLAGS=[
            "-Wno-class-memaccess",
            "-Wno-psabi",
        ]
    )

    env.Append(LIBS=["pthread"])

    if env["target"] == "release":
        if env["debug_release"] == "yes":
            env.Append(CCFLAGS=["-g2"])
        else:
            env.Append(CCFLAGS=["-O3"])
    elif env["target"] == "release_debug":
        env.Append(CCFLAGS=["-O2", "-ffast-math", "-DDEBUG_ENABLED"])
        if env["debug_release"] == "yes":
            env.Append(CCFLAGS=["-g2"])
    elif env["target"] == "debug":
        env.Append(CCFLAGS=["-g2", "-Wall", "-DDEBUG_ENABLED", "-DDEBUG_MEMORY_ENABLED"])

    env.Append(
        CPPDEFINES=[
            "NO_STATVFS",
            "NO_IOCTL",
            "IP6_UNAVAILABLE",
            "UNIX_SOCKETS_ENABLED",
            "LIBC_FILEIO_ENABLED",
            "PTHREAD_ENABLED",
            "PTHREAD_NO_RENAME",
        ]
    )
