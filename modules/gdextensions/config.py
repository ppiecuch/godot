import SCons


def can_build(env, platform):
    return True


def configure(env):
    from SCons.Script import Variables, ListVariable, Help, Exit

    # all submodules available in the extensions
    modules = [
        "core",
        "visual",
        "editor",
        "cpufeatures",
        "breakpad",
        "benchmark",
        "bulletkit",
        "blitter",
        "benchmark",
        "detournav",
        "environment",
        "fastnoise",
        "smooth",
        "debugdraw",
        "cyberelements",
        "geomfonts",
        "generator",
        "ropesim",
        "statemachine",
        "benet",
        "httpserver",
        "behaviornode",
        "behaviortree",
        "daedalus",
        "simpleai",
        "flexbuffers",
        "ccd",
        "settings",
        "flashdb",
        "sqlite",
        "unqlite",
        "threadpool",
        "openclwrapper",
        "sfxr",
        "vgamepad",
        "iap",
        "nakama1",
        "discord",
        "parseplatform",
        "playfab",
        "silentwolf",
        "qrcodetexture",
        "landiscovery",
        "multipeer",
        "polyvector",
        "vaserenderer",
        "lsystem",
        "timelinefx",
        "meshlod",
        "meshslicer",
        "scenemerge",
        "texturepacker",
        "ldrdraw",
        "opensteer",
        "tileengine",
        "sparkparticles",
        "turbobadger",
        "spinners",
        "spacemouse",
        "media",
        "hwinfo",
        "synthbenchmark",
        "runtimeprofiler",
        "goxel",
        "textui",
        "textwidgets",
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
