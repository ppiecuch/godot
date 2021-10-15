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

    return "3ds"


def can_build():

    if not "DEVKITPRO" in os.environ:
        return False
    if not "DEVKITARM" in os.environ:
        return False
    if not "CTRULIB" in os.environ:
        return False
    if os.name == "nt":
        return False

    for exe in ["pkg-config", "picasso"]:
        errorval = os.system("%s --version > /dev/null" % exe)
        if errorval:
            print("%s not found... 3ds disabled." % exe)
            return False

    return True  # 3DS enabled


def get_opts():

    return [("debug_release", "Add debug symbols to release version", "no")]


def get_flags():

    return [
        ("tools", False),
        ("module_bullet_enabled", False),
        ("module_squish_enabled", False),
        ("module_theora_enabled", False),
        ("module_dds_enabled", False),
        ("module_pvr_enabled", False),
        ("module_etc1_enabled", False),
        ("module_upnp_enabled", False),
        ("module_webm_enabled", False),
        ("module_webp_enabled", False),
        ("builtin_zlib", False),
        ("builtin_libpng", False),
        ("builtin_pcre2_with_jit", False),
        ("disable_3d", True),
        ("disable_advanced_gui", True),
        ("disable_experimental", True),
        ("feature_multiplethreads_allowed", False),
    ]


def build_shader_gen(target, source, env, for_signature):

    return "picasso -o {} {}".format(target[0], source[0])


def build_shader_header(target, source, env):

    import os

    data = source[0].get_contents()
    data_str = ",".join([str(x) for x in bytearray(data)])
    name = os.path.basename(str(target[0]))[:-2]
    target[0].prepare()
    with open(str(target[0]), "w") as f:
        f.write("/* Auto-generated from {} */\n".format(str(source[0])))
        f.write("static uint8_t shader_builtin_{}[] =\n{{{}}};".format(name, data_str))


def check(env, key):

    if not (key in env):
        return False
    if version.major > 2:
        return env[key]
    else:
        return env[key] == "yes"


def configure(env):

    env.Append(BUILDERS={"PICA": env.Builder(generator=build_shader_gen, suffix=".shbin", src_suffix=".pica")})
    env.Append(BUILDERS={"PICA_HEADER": env.Builder(action=build_shader_header, suffix=".h", src_suffix=".shbin")})

    env["bits"] = "32"
    env["arch"] = "arm"

    env.Append(CPPPATH=["#platform/3ds"])
    env["CC"] = "arm-none-eabi-gcc"
    env["CXX"] = "arm-none-eabi-g++"
    env["LD"] = "arm-none-eabi-g++"
    env["AR"] = "arm-none-eabi-ar"
    env["RANLIB"] = "arm-none-eabi-ranlib"
    env["AS"] = "arm-none-eabi-as"

    env.Append(CCFLAGS=["-D_3DS", "-DARM11"])
    env.Append(CCFLAGS=["-march=armv6k", "-mtune=mpcore", "-mfloat-abi=hard", "-mtp=soft"])
    env.Append(CCFLAGS=["-mword-relocations", "-fomit-frame-pointer", "-fno-strict-overflow"])

    env.Append(
        CCFLAGS=[
            "-Wno-maybe-uninitialized",
            "-Wno-strict-aliasing",
            "-Wno-nonnull",
            "-Wno-switch",
            "-Wno-sign-compare",
            "-Wno-shadow=compatible-local",
        ]
    )
    env.Append(CXXFLAGS=["-fno-exceptions", "-Wno-psabi", "-Wno-class-memaccess"])

    devkitpro_path = os.environ["DEVKITPRO"]
    ctrulib_path = os.environ["CTRULIB"]

    env.Append(CPPPATH=[devkitpro_path + "/portlibs/armv6k/include", devkitpro_path + "/portlibs/3ds/include"])
    env.Append(LIBPATH=[devkitpro_path + "/portlibs/armv6k/lib", devkitpro_path + "/portlibs/3ds/lib"])

    env.Append(LINKFLAGS=["-specs=3dsx.specs", "-g", "-march=armv6k", "-mtune=mpcore", "-mfloat-abi=hard"])
    env.Append(CPPPATH=[ctrulib_path + "/include"])
    env.Append(LIBPATH=[ctrulib_path + "/lib"])
    env.Append(LIBS=["citro3d", "ctru"])

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

    environ = "export DEVKITPRO=%s;" % devkitpro_path

    if not check(env, "builtin_freetype"):
        env.ParseConfig(
            "%s %s/portlibs/3ds/bin/arm-none-eabi-pkg-config freetype2 --cflags --libs" % (environ, devkitpro_path)
        )
    if not check(env, "builtin_libpng"):
        env.ParseConfig(
            "%s %s/portlibs/3ds/bin/arm-none-eabi-pkg-config libpng --cflags --libs" % (environ, devkitpro_path)
        )
    if not check(env, "builtin_zlib"):
        env.ParseConfig(
            "%s %s/portlibs/3ds/bin/arm-none-eabi-pkg-config zlib --cflags --libs" % (environ, devkitpro_path)
        )

    env.Append(
        CPPDEFINES=[
            "NDS_ENABLED",
            "NEED_LONG_INT",
            "UNIX_SOCKETS_ENABLED",
            "IP6_UNAVAILABLE",
            "LIBC_FILEIO_ENABLED",
        ]
    )
