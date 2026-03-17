#!/usr/bin/env python3
"""Fetch latest imspinner.h from upstream and generate Godot integration files.

Downloads from: https://github.com/dalerank/imspinner
Applies patches to remove ImGui dependency.
Generates complete Godot integration file with configurable spinner parameters.

Output files:
  imspinner.h              : patched version for Godot
  imspinner.orig.h         : unmodified upstream reference
  patch.txt                : unified diff of applied patches
  imspinner_generated.h    : enum, defaults table, configurable draw function
"""

import os
import re
import sys
import urllib.request
import difflib

UPSTREAM_URL = "https://raw.githubusercontent.com/dalerank/imspinner/master/imspinner.h"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def fetch_upstream():
    print("Downloading latest imspinner.h ...")
    content = urllib.request.urlopen(UPSTREAM_URL, timeout=30).read().decode("utf-8")
    print(f"  {len(content)} bytes, {content.count(chr(10))} lines")
    return content


def apply_patches(src):
    patches = 0
    if not src.startswith("/* clang-format off */"):
        src = "/* clang-format off */\n" + src
        patches += 1
    orig = src
    src = re.sub(r"#ifdef __has_include\s*\n.*?#endif\s*//\s*__has_include\s*\n", "", src, flags=re.DOTALL)
    src = src.replace('#include "imgui.h"\n', "")
    src = src.replace('#include "imgui_internal.h"\n', "")
    src = re.sub(r"// imgui headers\s*\n", "", src)
    if src != orig:
        patches += 1
    if "sizeof(detail::spinner_draw_funcs)" in src:
        src = src.replace("sizeof(detail::spinner_draw_funcs)", "e_st_count")
        patches += 1
    if "unsigned char* bitmap;" in src and "const unsigned char* bitmap;" not in src:
        src = src.replace("unsigned char* bitmap;", "const unsigned char* bitmap;")
        patches += 1
    src = src.rstrip()
    if not src.endswith("/* clang-format on */"):
        src += "\n/* clang-format on */\n"
        patches += 1
    else:
        src += "\n"
    print(f"  Patches: {patches}")
    return src


# ---------------------------------------------------------------------------
# Spinner function parsing
# ---------------------------------------------------------------------------

def extract_spinner_functions(src):
    pattern = r"inline\s+void\s+(Spinner\w+)\s*\(([^)]*(?:\([^)]*\))*[^)]*)\)"
    seen = set()
    funcs = []
    for m in re.finditer(pattern, src, re.DOTALL):
        name = m.group(1)
        if name in seen:
            continue
        seen.add(name)
        params = [_parse_param(p) for p in _split_params(m.group(2).strip())]
        funcs.append({"name": name, "params": params})
    return funcs


def _split_params(s):
    params, depth, cur = [], 0, ""
    for ch in s:
        if ch in "({": depth += 1
        elif ch in ")}": depth -= 1
        if ch == "," and depth == 0:
            params.append(cur.strip()); cur = ""
        else: cur += ch
    if cur.strip(): params.append(cur.strip())
    return params


def _parse_param(p):
    default = None
    if "=" in p:
        idx = p.index("=")
        decl, default = p[:idx].strip(), p[idx+1:].strip()
    else:
        decl = p.strip()
    tokens = decl.split()
    pname = tokens[-1].lstrip("&*") if len(tokens) >= 2 else decl
    ptype = " ".join(tokens[:-1]) if len(tokens) >= 2 else ""
    return {"type": ptype, "name": pname, "default": default, "raw": p}


def to_enum(name):
    s = re.sub(r"(?<=[a-z0-9])([A-Z])", r"_\1", name)
    s = re.sub(r"(?<=[A-Z])([A-Z][a-z])", r"_\1", s)
    return s.upper()


# ---------------------------------------------------------------------------
# Classify parameters for the configurable defaults
# ---------------------------------------------------------------------------

def classify_param(p, idx):
    """Return (config_field, default_value, type_tag) for a parameter."""
    n = p["name"].lower()
    t = p["type"]
    d = p["default"]

    if idx == 0:  # label — always generated
        return None, None, None

    # Map to a small set of configurable fields
    if "radius" in n:
        return "radius", d or "16.f", "float"
    if n == "thickness" or n == "thickness_drop":
        return "thickness" if n == "thickness" else "thickness_drop", d or "2.f", "float"
    if n == "speed":
        return "speed", d or "2.8f", "float"
    if n in ("color", "color1"):
        return "color", d or "white", "ImColor"
    if n in ("bg", "color2", "altcolor"):
        return "bg_color", d or "half_white", "ImColor"
    if n == "angle":
        return "angle", d or "IM_PI", "float"
    if n in ("dots", "arcs", "bars", "lines", "layers"):
        return n, d or "6", "int"
    if n == "mode":
        return "mode", d or "0", "int"
    if n == "minth" or n == "minthickness":
        return "min_thickness", d or "-1.f", "float"
    if "bool" in t:
        return n, d or "false", "bool"

    # Everything else: pass through with its default
    return None, None, None


SPECIAL_CALLS = {
    # Functions needing hand-written calls due to unusual signatures
    "SpinnerFillingMem": 'static ImColor _filling_bg = cfg.bg_color; ImSpinner::SpinnerFillingMem("{name}", cfg.radius, cfg.thickness, cfg.color, _filling_bg, cfg.speed * data->velocity);',
    "SpinnerCamera": 'ImSpinner::SpinnerCamera("{name}", cfg.radius, cfg.thickness, _spinner_camera_leaf, cfg.speed * data->velocity, cfg.bars, cfg.mode);',
    "SpinnerTextFading": 'ImSpinner::SpinnerTextFading("{name}", "Loading...", cfg.radius, cfg.color, cfg.speed * data->velocity, 0);',
}

def resolve_arg(p, idx, func_name):
    """Return the C++ expression for this argument in the draw call."""
    n = p["name"].lower()
    t = p["type"]
    d = p["default"]

    if idx == 0:
        return f'"{func_name}"'

    # Configurable fields — read from cfg
    if "radius" in n:
        return "cfg.radius"
    if n == "thickness":
        return "cfg.thickness"
    if n == "thickness_drop":
        return d or "cfg.thickness"
    if n == "speed":
        return "cfg.speed * data->velocity"
    if n in ("color", "color1"):
        return "cfg.color"
    if n in ("bg", "color2", "altcolor"):
        return "cfg.bg_color"
    if n == "angle":
        return "cfg.angle"
    if n in ("dots", "arcs", "bars", "lines"):
        return f"cfg.{n}"
    if n == "mode":
        return "cfg.mode"
    if n == "minth" or n == "minthickness":
        return "cfg.min_thickness"
    if n == "layers":
        return "cfg.layers"
    if n == "middledots" or n == "mdots":
        return "cfg.dots / 2"

    # Float pointer state
    if "float*" in t or "float *" in t or n == "nextdot":
        return "&data->nextdot1"

    # Pass through default
    if d:
        val = d.replace("half_white", "ImColor(255, 255, 255, 128)")
        return val

    # Type-based fallback
    if "ImColor" in t or "color" in n:
        return "white"
    if "float" in t:
        return "1.f"
    if "int" in t or "size_t" in t:
        return "0"
    if "bool" in t:
        return "false"
    if "char" in t:
        return '""'
    return "0"


# ---------------------------------------------------------------------------
# Extra variants (popular spinners with different params)
# ---------------------------------------------------------------------------

def generate_extra_variants():
    return [
        ("SPINNER_ANG_90_BG", "SpinnerAng", '"SpinnerAng90Bg", cfg.radius, cfg.thickness * 3, cfg.color, ImColor(255, 255, 255, 128), cfg.speed * data->velocity, IM_PI / 2'),
        ("SPINNER_ANG_90_NOBG", "SpinnerAng", '"SpinnerAng90NoBg", cfg.radius, cfg.thickness * 3, cfg.color, ImColor(255, 255, 255, 0), cfg.speed * data->velocity, IM_PI / 2'),
        ("SPINNER_ANG_270", "SpinnerAng", '"SpinnerAng270", cfg.radius, cfg.thickness, cfg.color, ImColor(255, 255, 255, 128), cfg.speed * data->velocity, 270.f / 360.f * 2 * IM_PI'),
        ("SPINNER_ING_YANG_REVERSED", "SpinnerIngYang", '"SpinnerIngYangR", cfg.radius, cfg.thickness * 2.5f, true, 0.1f, cfg.color, ImColor(255, 0, 0), cfg.speed * data->velocity, IM_PI * 0.8f'),
        ("SPINNER_ANG_TWIN_INNER", "SpinnerAngTwin", '"SpinnerAngTwinInner", cfg.radius - 3, cfg.radius, cfg.thickness, ImColor(255, 0, 0), cfg.color, cfg.speed * data->velocity, IM_PI / 2'),
        ("SPINNER_ANG_TWIN_DOUBLE", "SpinnerAngTwin", '"SpinnerAngTwinDouble", cfg.radius, cfg.radius - 3, cfg.thickness, ImColor(255, 0, 0), cfg.color, cfg.speed * data->velocity, IM_PI / 2, 2'),
        ("SPINNER_BOUNCE_BALL_SHADOW", "SpinnerBounceBall", '"SpinnerBounceBallShadow", cfg.radius, cfg.thickness * 3, cfg.color, cfg.speed * data->velocity, 1, true'),
        ("SPINNER_BAR_CHART_SINE_HSV", "SpinnerBarChartSine", '"SpinnerBarChartSineHSV", cfg.radius, cfg.thickness * 2, ImColor::HSV(data->hue * 0.005f, 0.8f, 0.8f), cfg.speed * data->velocity, 4, 1'),
        ("SPINNER_RAINBOW_HSV", "SpinnerRainbow", '"SpinnerRainbowHSV", cfg.radius, cfg.thickness, ImColor::HSV(++data->hue * 0.005f, 0.8f, 0.8f), cfg.speed * data->velocity'),
        ("SPINNER_DOTS_NOBG", "SpinnerDots", '"SpinnerDotsNoBg", &data->nextdot2, cfg.radius, cfg.thickness * 2, cfg.color, 0.3f * data->velocity, 12, 0'),
        ("SPINNER_VDOTS_HSV", "SpinnerVDots", '"SpinnerVDotsHSV", cfg.radius, cfg.thickness * 2, ImColor::HSV(data->hue * 0.001f, 0.8f, 0.8f), ImColor::HSV(data->hue * 0.0011f, 0.8f, 0.8f), cfg.speed * data->velocity, 12, 6'),
        ("SPINNER_FILLED_ARC_FADE_6", "SpinnerFilledArcFade", '"SpinnerFilledArcFade6", cfg.radius, cfg.color, cfg.speed * data->velocity, 6'),
        ("SPINNER_FILLED_ARC_FADE_8", "SpinnerFilledArcFade", '"SpinnerFilledArcFade8", cfg.radius, cfg.color, cfg.speed * data->velocity, 8'),
        ("SPINNER_MOD_CIRCLE_MODE1", "SpinnerModCircle", '"SpinnerModCircleM1", cfg.radius, cfg.thickness, cfg.color, cfg.speed * data->velocity, 1'),
    ]


# ---------------------------------------------------------------------------
# Code generation
# ---------------------------------------------------------------------------

def generate_header(funcs):
    variants = []

    # Primary variants: one per function
    for func in funcs:
        enum = to_enum(func["name"])
        if func["name"] in SPECIAL_CALLS:
            call = SPECIAL_CALLS[func["name"]].format(name=func["name"])
        else:
            args = ", ".join(resolve_arg(p, i, func["name"]) for i, p in enumerate(func["params"]))
            call = f'ImSpinner::{func["name"]}({args});'
        variants.append({"enum": enum, "call": call})

    # Extra variants
    for enum, func_name, args in generate_extra_variants():
        call = f"ImSpinner::{func_name}({args});"
        variants.append({"enum": enum, "call": call})

    L = []
    L.append("// Auto-generated by upstream-imspinner.py — DO NOT EDIT")
    L.append(f"// {len(variants)} spinner variants from {len(funcs)} ImSpinner functions")
    L.append("")
    L.append("#ifndef IMSPINNER_GENERATED_H")
    L.append("#define IMSPINNER_GENERATED_H")
    L.append("")

    # --- Config struct ---
    L.append("// Runtime-configurable spinner parameters.")
    L.append("// Users can override any field; unset fields use sensible defaults.")
    L.append("struct SpinnerConfig {")
    L.append("    float radius = 16.f;")
    L.append("    float thickness = 2.f;")
    L.append("    float speed = 4.f;")
    L.append("    ImColor color = ImColor(1.f, 1.f, 1.f, 1.f);")
    L.append("    ImColor bg_color = ImColor(1.f, 1.f, 1.f, 0.5f);")
    L.append("    float angle = IM_PI;")
    L.append("    int dots = 12;")
    L.append("    int arcs = 4;")
    L.append("    int bars = 6;")
    L.append("    int lines = 8;")
    L.append("    int layers = 1;")
    L.append("    int mode = 0;")
    L.append("    float min_thickness = -1.f;")
    L.append("};")
    L.append("")

    # --- Enum ---
    L.append(f"// {len(variants)} spinner variants")
    L.append("enum SpinnerVariant {")
    for i, v in enumerate(variants):
        L.append(f"    /* {i:3d} */ {v['enum']},")
    L.append(f"    /* {len(variants):3d} */ SPINNER_VARIANT_COUNT,")
    L.append("};")
    L.append("")

    # --- Name table ---
    L.append("static const char *SPINNER_VARIANT_NAMES[] = {")
    for v in variants:
        L.append(f'    "{v["enum"]}",')
    L.append("};")
    L.append("")

    # --- Animation state ---
    L.append("// Per-instance animation state")
    L.append("struct SpinnerAnimState {")
    L.append("    float velocity = 1.f;")
    L.append("    int hue = 0;")
    L.append("    float nextdot1 = 0.f;")
    L.append("    float nextdot2 = 0.f;")
    L.append("};")
    L.append("")

    # --- Draw function ---
    L.append("// Helper for SpinnerCamera (needs function pointer, not lambda)")
    L.append("static ImColor _spinner_camera_leaf(int i) { return ImColor::HSV(i * 0.1f, 0.8f, 0.8f); }")
    L.append("")
    L.append("// Draw the specified spinner variant with configurable parameters.")
    L.append("static inline void draw_spinner_variant(int p_variant, SpinnerAnimState *data, const SpinnerConfig &cfg) {")
    L.append("    using ImSpinner::white;")
    L.append("    using ImSpinner::half_white;")
    L.append("    using ImSpinner::red;")
    L.append("    using ImSpinner::PI_2;")
    L.append("")
    L.append("    data->nextdot1 -= 0.07f;")
    L.append("    data->nextdot2 -= 0.2f * data->velocity;")
    L.append("")
    L.append("    switch (p_variant) {")
    for v in variants:
        L.append(f"        case {v['enum']}:")
        L.append(f"            {v['call']}")
        L.append(f"            break;")
    L.append("        default: break;")
    L.append("    }")
    L.append("}")
    L.append("")
    L.append("#endif // IMSPINNER_GENERATED_H")
    L.append("")
    return "\n".join(L), variants


# ---------------------------------------------------------------------------

def main():
    os.chdir(SCRIPT_DIR)

    upstream = fetch_upstream()
    with open("imspinner.orig.h", "w") as f:
        f.write(upstream)

    patched = apply_patches(upstream)
    with open("imspinner.h", "w") as f:
        f.write(patched)

    orig_lines = upstream.splitlines(keepends=True)
    patch_lines = patched.splitlines(keepends=True)
    with open("patch.txt", "w") as f:
        f.writelines(difflib.unified_diff(orig_lines, patch_lines,
                     fromfile="imspinner.orig.h", tofile="imspinner.h"))

    funcs = extract_spinner_functions(patched)
    print(f"  Functions: {len(funcs)}")

    header, variants = generate_header(funcs)
    with open("imspinner_generated.h", "w") as f:
        f.write(header)
    print(f"  Variants: {len(variants)}")

    print(f"\nDone: imspinner.h ({patched.count(chr(10))} lines), "
          f"imspinner_generated.h ({len(variants)} variants)")


if __name__ == "__main__":
    main()
