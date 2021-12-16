def can_build(env, platform):
    return True


def configure(env):
    from SCons.Script import Variables, ListVariable, Help, Exit

    modules = [
        "core",
        "bulletkit",
        "visual",
        "environment",
        "smooth",
        "debugdraw",
        "statemachine",
        "benet",
        "behaviornode",
        "settings",
        "sfxr",
        "vgamepad",
    ]
    opts = Variables()
    opts.Add(ListVariable("enable_gdextensions_submodules", "Enable gdextensions submodules", "all", modules))
    opts.Update(env)


def get_doc_path():
    return "doc"


def get_doc_classes():
    return [
        "ErrorReporter",
        "AudioStreamSfxr",
    ]
