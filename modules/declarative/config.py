def can_build(env, platform):
    # Depends on scene/gui system (always available in Godot 3.x).
    # Can be disabled with module_declarative_enabled=no.
    return True


def configure(env):
    pass


def get_doc_classes():
    return [
        "GUILoader",
    ]


def get_doc_path():
    return "doc_classes"
