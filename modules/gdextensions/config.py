import SCons


def can_build(env, platform):
    return True


def configure(env):
    from SCons.Script import Variables, ListVariable, Help, Exit

    modules = [
        "core",
        "visual",
        "bulletkit",
        "blitter",
        "benchmark",
        "environment",
        "fastnoise",
        "smooth",
        "debugdraw",
        "geomfonts",
        "generator",
        "statemachine",
        "benet",
        "behaviornode",
        "flexbuffers",
        "ccd",
        "settings",
        "thread_pool",
        "sfxr",
        "vgamepad",
        "iap",
        "nakama1",
        "qrcodetexture",
        "landiscovery",
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
    ]
