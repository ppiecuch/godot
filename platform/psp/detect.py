import os, sys, errno
import platform
import subprocess

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

    return "psp"


def can_build():

    if not "PSPSDK" in os.environ:
        return False

    return True  # PSP enabled


def get_opts():

    return [("debug_release", "Add debug symbols to release version", "no")]


def get_flags():

    return [
        ("tools", False),
        ("module_squish_enabled", False),
        ("module_theora_enabled", False),
        ("module_dds_enabled", False),
        ("module_pvr_enabled", False),
        ("module_etc1_enabled", False),
        ("module_webm_enabled", False),
        ("module_upnp_enabled", False),
        ("module_mbedtls", False),
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


def checkexe(exe):

    try:
        output = subprocess.check_output(exe).strip().splitlines()
        for ln in output:
            print("> " + ln)
    except OSError as e:
        if e.errno == errno.ENOENT:
            return False
    return True


def configure(env):

    env["bits"] = "32"
    env["arch"] = "arm"

    env.Append(CPPPATH=["#platform/psp"])
    env["CC"] = "psp-gcc"
    env["CXX"] = "psp-g++"
    env["LD"] = "psp-g++"
    env["AR"] = "psp-ar"
    env["RANLIB"] = "psp-ranlib"
    env["AS"] = "psp-as"

    if checkexe([env["CC"], "--version"]):
        print("*** Using psp toolchain.")

    env.Append(CCFLAGS=["-G0"])
    env.Append(
        CCFLAGS=[
            "-Wno-maybe-uninitialized",
            "-Wno-strict-aliasing",
            "-fno-strict-overflow",
            "-Wno-switch",
            "-Wno-sign-compare",
            "-Wno-shadow=compatible-local",
        ]
    )
    env.Append(CXXFLAGS=["-Wno-psabi", "-Wno-class-memaccess"])

    pspdev_path = os.environ["PSPSDK"]

    env.Append(CPPPATH=[pspdev_path + "/psp/sdk/include"])
    env.Append(LIBPATH=[pspdev_path + "/psp/sdk/lib"])

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

    if not check(env, "builtin_freetype"):
        env.ParseConfig("psp-pkg-config freetype2 --cflags --libs")
    if not check(env, "builtin_libpng"):
        env.ParseConfig("psp-pkg-config libpng --cflags --libs")
    if not check(env, "builtin_zlib"):
        env.ParseConfig("psp-pkg-config zlib --cflags --libs")

    env.Append(LIBS=["c", "g", "pspuser", "pspkernel"])

    env.Append(
        CPPDEFINES=[
            "PTHREAD_ENABLED",
            "PTHREAD_NO_RENAME",
            "UNIX_SOCKETS_ENABLED",
            "IP6_UNAVAILABLE",
            "NO_GETADDRINFO",
            "NO_STATVFS",
            "LIBC_FILEIO_ENABLED",
        ]
    )
