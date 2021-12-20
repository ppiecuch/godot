def can_build(env, platform):
    return env["tools"] or env["module_gdyaml_enabled"]


def configure(env):
    pass
