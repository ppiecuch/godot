import SCons


def can_build(env, platform):
    return True


def configure(env):
    from SCons.Script import Variables, ListVariable, Help, Exit

    # all submodules available in the extensions
    modules = [
        # media formats and visual components
        "media",
        "visual",
        # low-level extensions
        "benchmark",
        "breakpad",
        "consoleaddons",
        "cpufeatures",
        "debugdraw",
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
        "fastnoise",
        "flexbuffers",
        "openclwrapper",
        "sfxr",
        "vgamepaddesign",
        # navigation/ai extensions
        "detournav",
        "behaviornode",
        "behaviortree",
        "statemachine",
        "mazegen",
        "daedalus",
        "simpleai",
        "opensteer",
        "ccd",
        # environment components
        "environment",
        # physics and simulation extensions
        "ropesim",
        "hydro",
        "msalibs",
        # network extensions
        "benet",
        "httpserver",
        "isotools",
        "keychains",
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
        "thorvg",
        "albmpgfx",
        "canvasraster",
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
        # components and wrappers
        "cyberelements",
        "flowed",
        "geomfonts",
        "symbolfonts",
        "generator",
        "ggoxel",
        "fontengine3d",
        "gqmaps",
        "spinners",
        "tileengine",
        # ui and gui extensions
        "glitehtml",
        "turbobadger",
        "textui",
        "ofxdatgui",
        "anttweakbar",
        # particles extensions
        "bulletkit",
        "qurobullet",
        "timelinefx",
        "sparkparticles",
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
