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
        # animation extensions
        "dragonbones",
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
        "penet",
        "httpserver",
        "isotools",
        "keychains",
        # middleware extensions
        "nakama1",
        "discord",
        "playfab",
        "silentwolf",
        "epicservices",
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
        "opensymbols",
        "material_symbols",
        "slugfont",
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
            "gdext_enable_submodules",
            "Enable gdextensions submodules",
            "all" if env["tools"] else "none",
            modules,
        )
    )
    opts.Add(
        (
            "gdext_material_symbols_subset",
            "Path to a Material Symbols subset manifest. Required for non-tools "
            "builds when material_symbols is enabled — without it the module is "
            "disabled at build time. Tools builds ignore this option.",
            "",
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
