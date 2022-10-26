import os, version, errno, sys, subprocess, platform
import cg_builders
from platform_methods import run_in_subprocess

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


def checkexe(exe):
    try:
        output = subprocess.check_output(exe).strip().splitlines()
        for ln in output:
            print("> " + str(ln))
    except OSError as e:
        if e.errno == errno.ENOENT:
            return False
    return True


def can_build():
    if not "VITASDK" in os.environ:
        return False
    for exe in ["arm-vita-eabi-pkg-config"]:
        if checkexe("%s --version"):
            print("%s not found... psvita disabled." % exe)
            return False
    if os.name == "nt":
        return False
    return True


def get_opts():

    return [
        ("debug_release", "Add debug symbols to release version", "no"),
        ("taihen_logging", "Enable verbose logging in taiHEN", "no"),
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

    if checkexe(["arm-vita-eabi-gcc", "--version"]):
        env["CC"] = "arm-vita-eabi-gcc"
        env["CXX"] = "arm-vita-eabi-g++"
        env["LD"] = "arm-vita-eabi-g++"
        env["AR"] = "arm-vita-eabi-ar"
        env["RANLIB"] = "arm-vita-eabi-ranlib"
        env["AS"] = "arm-vita-eabi-as"
        print("*** Using arm-vita-eabi-gcc toolchain by default.")

    env.Append(CPPPATH=["#platform/psvita"])
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
        BUILDERS={
            "GLES2_TO_CG": env.Builder(
                action=run_in_subprocess(cg_builders.build_cg_headers), suffix="cg.gen.h", src_suffix=".glsl"
            )
        }
    )

    if not check(env, "builtin_freetype"):
        env.ParseConfig("arm-vita-eabi-pkg-config freetype2 --cflags --libs")
    if not check(env, "builtin_libpng"):
        env.ParseConfig("arm-vita-eabi-pkg-config libpng --cflags --libs")
    if not check(env, "builtin_zlib"):
        env.ParseConfig("arm-vita-eabi-pkg-config zlib --cflags --libs")

    env.Append(
        CPPDEFINES=[
            "PSVITA_ENABLED",
            "PTHREAD_ENABLED",
            "PTHREAD_NO_RENAME",
            "LIBC_FILEIO_ENABLED",
            "NO_STATVFS",
            "NO_IOCTL",
            "IP6_UNAVAILABLE",
            "UNIX_SOCKETS_ENABLED",
        ]
    )
