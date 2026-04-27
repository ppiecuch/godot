#!/usr/bin/env python


def can_build(env, platform):
    # Editor only module
    return env["tools"]


def configure(env):
    pass


def is_enabled():
    # Enable manually with `module_gdscript_transpiler_enabled=yes` option.
    return True
