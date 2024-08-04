import SCons


def can_build(env, platform):
    return True


def configure(env):
    from SCons.Script import Variables, ListVariable, Help, Exit

    # all submodules available in the extensions
    modules = [
        # core extensions
        "core",
        "editor",
        "visual",
        # low-level extensions
        "benchmark",
        "breakpad",
        "cpufeatures",
        "debugdraw",
        "fastnoise",
        "flashdb",
        "hwinfo",
        "runtimeprofiler",
        "settings",
        "smooth",
        "spacemouse",
        "sqlite",
        "synthbenchmark",
        "threadpool",
        "unqlite",
        "yaml",
        # system components
        "flexbuffers",
        "openclwrapper",
        "sfxr",
        "vgamepad",
        #
        "bulletkit",
        "qurobullet",
        # pgysics and ai extensions
        "detournav",
        "behaviornode",
        "behaviortree",
        "statemachine",
        "daedalus",
        "simpleai",
        "opensteer",
        "ccd",
        # environment components
        "environment",
        # simulation extensions
        "ropesim",
        "hydro",
        "msalibs",
        # graphics components
        "cyberelements",
        "geomfonts",
        "generator",
        "fontengine3d",
        "spinners",
        # network extensions
        "benet",
        "httpserver",
        # middleware extensions
        "nakama1",
        "discord",
        "parseplatform",
        "playfab",
        "silentwolf",
        "qrcodetexture",
        "landiscovery",
        "multipeer",
        "iap",
        # rendering extensions
        "polyvector",
        "vaserenderer",
        "lsystem",
        "ldrdraw",
        "swrender",
        "swsurface",
        # mesh extensions
        "meshlod",
        "meshslicer",
        "scenemerge",
        "texturepacker",
        # ui and gui extensions
        "turbobadger",
        "textui",
        "ofxdatgui",
        "anttweakbar",
        # particles extensions
        "tileengine",
        "timelinefx",
        "sparkparticles",
        "media",
        "goxel",
        "qmap",
        "flowed",
    ]
    opts = Variables()
    opts.Add(
        ListVariable(
            "enable_gdextensions_submodules",
            "Enable gdextensions submodules",
            "all" if env["tools"] else "none",
            modules,
        )
    )
    opts.Update(env)


def get_doc_path():
    return "doc"


def get_doc_classes():
    return [
        "ErrorReporter",
        "AudioStreamSfxr",
        "FastNoiseLite",
        "Noise",
        "NoiseTexture",
    ]
