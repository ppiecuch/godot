#!/usr/bin/env python3
"""
gen_opensymbols_data.py — codegen for the OpenSymbols Godot submodule.

Walks <root>/Deepin*/, reads each *.svg, applies on-the-fly viewBox tightening
(font-glyph SVGs ship with viewBox="0 -410 2048 2048" + a vertical-flip <g>
transform that pushes the visible art into the lower 33% of the raster), and
emits two C++ files:

    opensymbols_data.gen.h   — extern declarations (small)
    opensymbols_data.gen.cpp — string-literal payload (large)

Output is deterministic: entries are sorted by glyph name. `--no-tighten`
skips viewBox normalization (faster builds during development; resulting
icons render small).

Invoked by SCons. Also runnable standalone for diagnostics:

    python3 gen_opensymbols_data.py \\
        --src   modules/gdextensions/thirdparty/opensymbols/svg \\
        --out-h /tmp/opensymbols_data.gen.h \\
        --out-cpp /tmp/opensymbols_data.gen.cpp
"""

import argparse
import os
import re
import sys
from pathlib import Path

# Variant directory name -> 1-based index used in the generated array names.
VARIANT_DIRS = [
    ("DeepinOpenSymbol", 1),
    ("DeepinOpenSymbol2", 2),
    ("DeepinOpenSymbol3", 3),
]

# ── viewBox tightening (port of mini-midi-player/_scripts/tighten_svgs.py) ──

_TOKEN_RE = re.compile(r"[MmLlHhVvCcSsQqTtAaZz]|-?\d+\.?\d*(?:[eE][-+]?\d+)?")


def _parse_path_d(d):
    pts = []
    px = py = 0.0
    last_cmd = ""
    tokens = _TOKEN_RE.findall(d)
    i = 0
    while i < len(tokens):
        t = tokens[i]
        if t.isalpha():
            cmd = t
            i += 1
        else:
            cmd = last_cmd or "L"
        args = []
        while i < len(tokens) and not tokens[i].isalpha():
            try:
                args.append(float(tokens[i]))
            except ValueError:
                pass
            i += 1
        last_cmd = cmd
        rel = cmd.islower()
        cu = cmd.upper()
        idx = 0

        def next_pair():
            nonlocal idx
            if idx + 1 >= len(args):
                return None
            p = (args[idx], args[idx + 1])
            idx += 2
            return p

        if cu == "M":
            first = True
            while True:
                p = next_pair()
                if p is None:
                    break
                if rel and not first:
                    p = (px + p[0], py + p[1])
                elif rel and first:
                    p = (px + p[0], py + p[1])
                px, py = p
                pts.append((px, py))
                first = False
        elif cu == "L":
            while True:
                p = next_pair()
                if p is None:
                    break
                if rel:
                    p = (px + p[0], py + p[1])
                px, py = p
                pts.append((px, py))
        elif cu == "H":
            while idx < len(args):
                v = args[idx]
                idx += 1
                px = px + v if rel else v
                pts.append((px, py))
        elif cu == "V":
            while idx < len(args):
                v = args[idx]
                idx += 1
                py = py + v if rel else v
                pts.append((px, py))
        elif cu == "C":
            while True:
                a = next_pair()
                b = next_pair()
                c = next_pair()
                if c is None:
                    break
                if rel:
                    a = (px + a[0], py + a[1])
                    b = (px + b[0], py + b[1])
                    c = (px + c[0], py + c[1])
                pts.extend([a, b, c])
                px, py = c
        elif cu in ("S", "Q"):
            while True:
                a = next_pair()
                b = next_pair()
                if b is None:
                    break
                if rel:
                    a = (px + a[0], py + a[1])
                    b = (px + b[0], py + b[1])
                pts.extend([a, b])
                px, py = b
        elif cu == "T":
            while True:
                p = next_pair()
                if p is None:
                    break
                if rel:
                    p = (px + p[0], py + p[1])
                px, py = p
                pts.append((px, py))
        elif cu == "A":
            while True:
                if idx + 6 >= len(args):
                    break
                _rx, _ry, _rot, _large, _sweep, x, y = args[idx : idx + 7]
                idx += 7
                if rel:
                    x, y = px + x, py + y
                px, py = x, y
                pts.append((px, py))
    return pts


def _parse_matrix(transform):
    m = re.search(r"matrix\(([^)]+)\)", transform)
    if not m:
        return None
    parts = re.split(r"[\s,]+", m.group(1).strip())
    if len(parts) != 6:
        return None
    try:
        return tuple(float(x) for x in parts)
    except ValueError:
        return None


def _apply_matrix(mat, pt):
    a, b, c, d, e, f = mat
    x, y = pt
    return (a * x + c * y + e, b * x + d * y + f)


def _tighten_viewbox(text):
    """Return text with a tightened viewBox (and the redundant <g matrix>
    flattened away if found). Returns the input unchanged when the SVG is
    already tight or doesn't match the font-glyph pattern."""
    vb_match = re.search(r'viewBox="([^"]+)"', text)
    if not vb_match:
        return text
    parts = re.split(r"[\s,]+", vb_match.group(1).strip())
    try:
        vbX, vbY, vbW, vbH = (float(x) for x in parts)
    except ValueError:
        return text
    if vbY > -1:
        return text  # already tight

    coords = []
    for m in re.finditer(r'd="([^"]+)"', text):
        coords.extend(_parse_path_d(m.group(1)))
    if not coords:
        return text

    mt = re.search(r'<g[^>]*transform="([^"]+)"', text)
    matrix = _parse_matrix(mt.group(1)) if mt else None
    if matrix:
        coords = [_apply_matrix(matrix, p) for p in coords]

    xs = [p[0] for p in coords]
    ys = [p[1] for p in coords]
    minx, maxx = min(xs), max(xs)
    miny, maxy = min(ys), max(ys)
    pad = max(maxx - minx, maxy - miny) * 0.04
    nx = minx - pad
    ny = miny - pad
    nw = (maxx - minx) + 2 * pad
    nh = (maxy - miny) + 2 * pad

    def fmt(v):
        s = f"{v:.1f}"
        return s.rstrip("0").rstrip(".") if "." in s else f"{int(v)}"

    new_vb = f'viewBox="{fmt(nx)} {fmt(ny)} {fmt(nw)} {fmt(nh)}"'
    return text[: vb_match.start()] + new_vb + text[vb_match.end() :]


# ── SVG minification ───────────────────────────────────────────────────────

_PROLOG_RE = re.compile(r"<\?xml[^?]*\?>", re.DOTALL)
_DOCTYPE_RE = re.compile(r"<!DOCTYPE[^>]*>", re.DOTALL)
_COMMENT_RE = re.compile(r"<!--.*?-->", re.DOTALL)
_BETWEEN_TAGS_RE = re.compile(r">\s+<")
_INNER_WS_RE = re.compile(r"[ \t\r\n]+")


def _minify(text):
    text = _PROLOG_RE.sub("", text)
    text = _DOCTYPE_RE.sub("", text)
    text = _COMMENT_RE.sub("", text)
    text = _BETWEEN_TAGS_RE.sub("><", text)
    text = _INNER_WS_RE.sub(" ", text).strip()
    return text


# ── C++ string-literal escaping ────────────────────────────────────────────


def _escape_cpp(s):
    out = []
    for ch in s:
        c = ord(ch)
        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\n":
            out.append("\\n")
        elif ch == "\r":
            out.append("\\r")
        elif ch == "\t":
            out.append("\\t")
        elif c < 0x20 or c == 0x7F:
            out.append(f"\\x{c:02x}")
        elif c < 0x80:
            out.append(ch)
        else:
            # Pass UTF-8 bytes through as escape sequences so the literal stays ASCII.
            for b in ch.encode("utf-8"):
                out.append(f"\\x{b:02x}")
    return "".join(out)


# ── Main ───────────────────────────────────────────────────────────────────


def gather(root, tighten):
    """Return list of (variant_index, name, svg_text) tuples sorted by (variant, name)."""
    entries = []
    for dirname, vidx in VARIANT_DIRS:
        d = root / dirname
        if not d.is_dir():
            continue
        files = sorted(d.glob("*.svg"))
        for f in files:
            text = f.read_text(encoding="utf-8", errors="replace")
            if tighten:
                try:
                    text = _tighten_viewbox(text)
                except Exception as ex:
                    print(f"  warn tighten {f.name}: {ex}", file=sys.stderr)
            text = _minify(text)
            entries.append((vidx, f.stem, text))
    return entries


def write_outputs(entries, out_h, out_cpp):
    by_variant = {1: [], 2: [], 3: []}
    for vidx, name, svg in entries:
        by_variant[vidx].append((name, svg))

    counts = {v: len(rows) for v, rows in by_variant.items()}

    h_lines = [
        "// AUTOGENERATED by gen_opensymbols_data.py — do not edit by hand.",
        "#ifndef OPENSYMBOLS_DATA_GEN_H",
        "#define OPENSYMBOLS_DATA_GEN_H",
        "",
        "#include <cstddef>",
        "",
        "namespace opensymbols_data {",
        "",
        "struct Entry {",
        "    const char *name;",
        "    const char *svg;",
        "    size_t svg_len;",
        "};",
        "",
    ]
    for v in (1, 2, 3):
        h_lines.append(f"extern const Entry variant_{v}[{counts[v]}];")
        h_lines.append(f"constexpr int variant_{v}_count = {counts[v]};")
    h_lines += ["", "} // namespace opensymbols_data", "", "#endif // OPENSYMBOLS_DATA_GEN_H", ""]
    out_h.write_text("\n".join(h_lines), encoding="utf-8")

    cpp_lines = [
        "// AUTOGENERATED by gen_opensymbols_data.py — do not edit by hand.",
        '#include "opensymbols_data.gen.h"',
        "",
        "namespace opensymbols_data {",
        "",
    ]
    for v in (1, 2, 3):
        rows = by_variant[v]
        cpp_lines.append(f"const Entry variant_{v}[{counts[v]}] = {{")
        for name, svg in rows:
            esc_name = _escape_cpp(name)
            esc_svg = _escape_cpp(svg)
            byte_len = len(svg.encode("utf-8"))
            cpp_lines.append(f'    {{ "{esc_name}", "{esc_svg}", {byte_len} }},')
        cpp_lines.append("};")
        cpp_lines.append("")
    cpp_lines += ["} // namespace opensymbols_data", ""]
    out_cpp.write_text("\n".join(cpp_lines), encoding="utf-8")

    return counts


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--src", type=Path, required=True, help="thirdparty/opensymbols/svg root")
    ap.add_argument("--out-h", type=Path, required=True)
    ap.add_argument("--out-cpp", type=Path, required=True)
    ap.add_argument("--no-tighten", action="store_true", help="skip viewBox tightening (faster, smaller icons)")
    args = ap.parse_args()
    if not args.src.is_dir():
        sys.exit(f"no such dir: {args.src}")

    args.out_h.parent.mkdir(parents=True, exist_ok=True)
    args.out_cpp.parent.mkdir(parents=True, exist_ok=True)

    entries = gather(args.src, tighten=not args.no_tighten)
    counts = write_outputs(entries, args.out_h, args.out_cpp)
    total = sum(counts.values())
    print(f"opensymbols_data: {counts[1]} + {counts[2]} + {counts[3]} = {total} entries")


if __name__ == "__main__":
    main()
