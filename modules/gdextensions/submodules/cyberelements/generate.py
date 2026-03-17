#!/usr/bin/env python3
"""Generate optimized cyberelements.json from react-cyber-elements TSX source.

Fetches TSX components from GitHub (or uses local checkout), extracts SVG markup,
resolves CSS classes to fill types, and preserves gradient/clipPath definitions
for future rendering support.

Usage:
    python3 generate.py                        # fetch from GitHub
    python3 generate.py --local PATH           # use local react-cyber-elements/src/
    python3 generate.py --existing JSON        # re-optimize existing React-tree JSON
    python3 generate.py --pretty               # pretty-print for debugging

Output: cyberelements.json — Preprocessed JSON array, one entry per element:

    {
        "viewBox": [0, 0, W, H],              ← 4 ints (no string parsing needed)
        "paths": [
            {"fill": 0, "d": "M742 863c..."},  ← fill: 0=primary, 1=secondary, 2+=color index
        ],
        "colors": ["#b3b3b3"],                 ← extra literal colors (indexed by fill-2)
        "defs": [                               ← gradient/clipPath definitions (preserved)
            {"type": "linearGradient", "id": "...", "attrs": {...}, "stops": [...]},
            {"type": "clipPath", "id": "...", "children": "..."}
        ]
    }

Fill type encoding:
    0 = primary fill (default #252626, user-customizable)
    1 = secondary fill (default #767676, user-customizable)
    2+ = index into element's "colors" array (literal non-customizable colors)
"""

import json
import os
import re
import sys

# --- Configuration ---

GITHUB_RAW_BASE = (
    "https://raw.githubusercontent.com/thiswallz/react-cyber-elements/master/src"
)
ELEMENT_COUNT = 90
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LOCAL_TSX_DIR = os.path.join(SCRIPT_DIR, "react-cyber-elements", "src")
OUTPUT_PATH = os.path.join(SCRIPT_DIR, "cyberelements.json")

# Default colors in the original SVGs (mapped to fill type 0=primary, 1=secondary)
PRIMARY_COLORS = {"#252626", "#231f1f"}
SECONDARY_COLORS = {"#767676"}


# --- JSX → HTML conversion ---


def camel_to_css(name):
    return re.sub(r"([A-Z])", lambda m: "-" + m.group(1).lower(), name)


def convert_style_block(m):
    style_content = m.group(1)
    pairs = re.findall(
        r"([\w-]+):\s*[\"']?([^\"',$}]+)[\"']?\s*[,}]?", style_content
    )
    css = "; ".join(f"{camel_to_css(k)}: {v.strip()}" for k, v in pairs)
    return f'style="{css}"'


def extract_svg_from_tsx(content):
    """Extract SVG from TSX, converting JSX → HTML attributes."""
    m = re.search(r"(<svg[\s\S]*?</svg>)", content)
    if not m:
        return None
    svg = m.group(1)

    for old, new in {
        "className=": "class=",
        'xmlSpace="preserve"': 'xml:space="preserve"',
        'fillRule="evenodd"': 'fill-rule="evenodd"',
        'clipRule="evenodd"': 'clip-rule="evenodd"',
        "strokeWidth=": "stroke-width=",
        "strokeLinecap=": "stroke-linecap=",
        "strokeLinejoin=": "stroke-linejoin=",
        "xlinkHref=": "xlink:href=",
        "stopColor=": "stop-color=",
        "stopOpacity=": "stop-opacity=",
    }.items():
        svg = svg.replace(old, new)

    svg = re.sub(r"\s*\{\.\.\.props\}", "", svg)
    svg = re.sub(r"style=\{\{([^}]*)\}\}", convert_style_block, svg)
    svg = re.sub(r">\s*\{\s*\"([^\"]*)\"\s*\}\s*<", r">\1<", svg)
    svg = re.sub(r'>\s*\{\s*\n\s*"([^"]*)"\s*\n\s*\}\s*<', r">\1<", svg)
    svg = re.sub(r"\s*width=\{[\d]+\}", "", svg)
    svg = re.sub(r"\s*height=\{[\d]+\}", "", svg)
    svg = re.sub(r"(\w+)=\{([\d.]+)\}", r'\1="\2"', svg)

    return svg


# --- CSS class → fill color resolution ---


def parse_css_defs(css_text):
    """Parse CSS class definitions, return dict: class_name → fill_color_hex."""
    class_fills = {}
    for m in re.finditer(r"([^{]+)\{([^}]+)\}", css_text):
        selectors = m.group(1)
        body = m.group(2)
        fill_match = re.search(r"fill:\s*([^;]+)", body)
        if not fill_match:
            continue
        fill_val = fill_match.group(1).strip()
        for sel in selectors.split(","):
            sel = sel.strip().lstrip(".")
            if sel:
                class_fills[sel] = fill_val
    return class_fills


def resolve_fill_type(color_hex, extra_colors):
    """Map a color hex to fill type index. 0=primary, 1=secondary, 2+=literal."""
    if not color_hex or color_hex.startswith("url("):
        # Gradient reference — store as literal so we preserve the url()
        if color_hex and color_hex not in extra_colors:
            extra_colors.append(color_hex)
        if color_hex:
            return 2 + extra_colors.index(color_hex), extra_colors
        return 0, extra_colors
    color_lower = color_hex.lower().strip()
    if re.match(r"^#[0-9a-f]{3}$", color_lower):
        color_lower = "#" + "".join(c * 2 for c in color_lower[1:])
    if color_lower in PRIMARY_COLORS:
        return 0, extra_colors
    if color_lower in SECONDARY_COLORS:
        return 1, extra_colors
    named = {"black": "#000000", "white": "#ffffff", "gray": "#808080", "grey": "#808080"}
    if color_lower in named:
        color_lower = named[color_lower]
    if color_lower not in extra_colors:
        extra_colors.append(color_lower)
    return 2 + extra_colors.index(color_lower), extra_colors


# --- Gradient/defs extraction ---


def extract_defs(svg):
    """Extract gradient and clipPath definitions from <defs> block as structured data."""
    defs = []

    for tag_name in ("linearGradient", "radialGradient"):
        # Match both self-closing: <linearGradient attrs />
        # and child-bearing: <linearGradient attrs> <stop/> ... </linearGradient>
        pattern = (
            r"<" + tag_name + r"\s+([^>]*?)"
            r"(?:"
            r"/>"  # self-closing form
            r"|"
            r">([\s\S]*?)</" + tag_name + r">"  # child-bearing form
            r")"
        )
        for m in re.finditer(pattern, svg):
            attrs = _parse_xml_attrs(m.group(1))
            body = m.group(2) if m.group(2) else ""
            stops = _parse_gradient_stops(body) if body else []
            entry = {"type": tag_name, "attrs": attrs}
            if stops:
                entry["stops"] = stops
            defs.append(entry)

    # clipPath (preserve as raw SVG snippet for future use)
    # Match both self-closing and child-bearing forms
    for m in re.finditer(
        r"(<clipPath\s+[^>]*(?:/>|>[\s\S]*?</clipPath>))", svg
    ):
        clip_id_match = re.search(r'id="([^"]*)"', m.group(1))
        entry = {"type": "clipPath", "raw": m.group(1)}
        if clip_id_match:
            entry["id"] = clip_id_match.group(1)
        defs.append(entry)

    return defs


def _parse_xml_attrs(attr_str):
    """Parse XML attribute string into dict."""
    attrs = {}
    for m in re.finditer(r'([\w:-]+)="([^"]*)"', attr_str):
        attrs[m.group(1)] = m.group(2)
    return attrs


def _parse_gradient_stops(stops_str):
    """Parse <stop> elements from gradient body."""
    stops = []
    for m in re.finditer(r"<stop\s+([^>]*?)/?>", stops_str):
        attrs = _parse_xml_attrs(m.group(1))
        # Expand inline style="stop-color: #xxx; stop-opacity: 1" into attrs
        style_match = re.search(r'style="([^"]*)"', m.group(1))
        if style_match:
            for pair in style_match.group(1).split(";"):
                pair = pair.strip()
                if ":" in pair:
                    k, v = pair.split(":", 1)
                    attrs[k.strip()] = v.strip()
            del attrs["style"]  # remove raw style string, we've expanded it
        stops.append(attrs)
    return stops


# --- SVG → preprocessed element ---


def preprocess_svg(svg):
    """Convert SVG to preprocessed element with resolved fill types and preserved defs."""
    # viewBox
    vb_match = re.search(r'viewBox="([^"]+)"', svg)
    viewbox = (
        [int(float(x)) for x in vb_match.group(1).split()]
        if vb_match
        else [0, 0, 100, 100]
    )

    # Parse CSS defs
    style_match = re.search(r"<style[^>]*>(.*?)</style>", svg, re.DOTALL)
    css_text = style_match.group(1).strip() if style_match else ""
    class_fills = parse_css_defs(css_text)

    # Extract gradient/clipPath defs
    svg_defs = extract_defs(svg)

    # Extract and resolve paths
    extra_colors = []
    paths = []
    for m in re.finditer(r"<path\s+([^>]*?)/>", svg, re.DOTALL):
        attrs = m.group(1)
        d_match = re.search(r'd="([^"]*)"', attrs)
        if not d_match:
            continue

        fill_color = None
        cls_match = re.search(r'class="([^"]*)"', attrs)
        if cls_match:
            for cls in cls_match.group(1).split():
                if cls in class_fills:
                    fill_color = class_fills[cls]
                    break
        style_match = re.search(r'style="([^"]*)"', attrs)
        if style_match and not fill_color:
            sfill = re.search(r"fill:\s*([^;]+)", style_match.group(1))
            if sfill:
                fill_color = sfill.group(1).strip()

        if fill_color is None:
            fill_color = "#252626"

        fill_type, extra_colors = resolve_fill_type(fill_color, extra_colors)
        path = {"fill": fill_type, "d": d_match.group(1)}

        # Preserve additional path attributes for future use
        clip_match = re.search(r'clip-path="([^"]*)"', attrs)
        if clip_match:
            path["clipPath"] = clip_match.group(1)
        opacity_match = re.search(r'(?:fill-)?opacity:\s*([^;"\s]+)', attrs)
        if opacity_match:
            try:
                path["opacity"] = float(opacity_match.group(1))
            except ValueError:
                pass

        paths.append(path)

    result = {"viewBox": viewbox, "paths": paths}
    if extra_colors:
        result["colors"] = extra_colors
    if svg_defs:
        result["defs"] = svg_defs
    return result


# --- Re-optimize existing React-tree JSON ---


def optimize_existing_json(json_path):
    """Re-optimize existing cyberelements.json (React component tree format).
    Note: the React tree format doesn't contain gradient defs — use --local for full data.
    """
    with open(json_path, "r") as f:
        data = json.load(f)

    elements = []
    for el in data:
        result = {"viewBox": [0, 0, 100, 100], "paths": []}
        raw_paths = []
        css_text = _extract_css_from_tree(el)
        viewbox = _extract_viewbox_from_tree(el)
        _walk_tree(el, raw_paths)

        class_fills = parse_css_defs(css_text)
        extra_colors = []
        paths = []
        for rp in raw_paths:
            fill_color = None
            if "class" in rp:
                for cls in rp["class"].split():
                    if cls in class_fills:
                        fill_color = class_fills[cls]
                        break
            if "style" in rp and "fill" in rp["style"]:
                if fill_color is None:
                    fill_color = rp["style"]["fill"]
            if fill_color is None:
                fill_color = "#252626"
            fill_type, extra_colors = resolve_fill_type(fill_color, extra_colors)
            paths.append({"fill": fill_type, "d": rp["d"]})

        result = {"viewBox": viewbox, "paths": paths}
        if extra_colors:
            result["colors"] = extra_colors
        # Note: no defs from React tree — warn user
        elements.append(result)

    return elements


def _extract_viewbox_from_tree(obj):
    if isinstance(obj, dict):
        props = obj.get("props", obj)
        if isinstance(props, dict) and "viewBox" in props:
            return [int(float(x)) for x in props["viewBox"].split()]
        for v in obj.values():
            r = _extract_viewbox_from_tree(v)
            if r != [0, 0, 100, 100]:
                return r
    return [0, 0, 100, 100]


def _extract_css_from_tree(obj):
    if isinstance(obj, dict):
        props = obj.get("props", obj)
        if isinstance(props, dict):
            ch = props.get("children")
            if isinstance(ch, str) and "{" in ch:
                return ch
            for v in props.values():
                r = _extract_css_from_tree(v)
                if r:
                    return r
    elif isinstance(obj, list):
        for item in obj:
            r = _extract_css_from_tree(item)
            if r:
                return r
    return ""


def _walk_tree(obj, paths):
    if not isinstance(obj, dict):
        return
    props = obj.get("props", obj)
    if not isinstance(props, dict):
        return
    if "d" in props:
        path = {"d": props["d"]}
        if "className" in props:
            path["class"] = props["className"]
        if "style" in props and isinstance(props["style"], dict):
            style = {}
            for jk, ck in {
                "fill": "fill",
                "stroke": "stroke",
                "fillOpacity": "fill-opacity",
                "strokeWidth": "stroke-width",
            }.items():
                if jk in props["style"]:
                    style[ck] = str(props["style"][jk])
            if style:
                path["style"] = style
        paths.append(path)
    children = props.get("children")
    if isinstance(children, list):
        for c in children:
            _walk_tree(c, paths)
    elif isinstance(children, dict):
        _walk_tree(children, paths)


# --- Source fetching ---


def fetch_tsx_from_github(index):
    import urllib.request

    url = f"{GITHUB_RAW_BASE}/CyberEl{index}.tsx"
    try:
        with urllib.request.urlopen(url, timeout=15) as resp:
            return resp.read().decode("utf-8")
    except Exception as e:
        print(f"  WARN: Failed to fetch CyberEl{index}: {e}")
        return None


def read_tsx_local(src_dir, index):
    path = os.path.join(src_dir, f"CyberEl{index}.tsx")
    if not os.path.exists(path):
        return None
    with open(path, "r") as f:
        return f.read()


# --- Main ---


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description="Generate preprocessed cyberelements.json"
    )
    parser.add_argument(
        "--local", metavar="PATH", help="Local react-cyber-elements/src/ directory"
    )
    parser.add_argument(
        "--existing",
        metavar="JSON",
        help="Re-optimize existing React-tree JSON (no gradient defs)",
    )
    parser.add_argument("--output", metavar="PATH", default=OUTPUT_PATH)
    parser.add_argument("--pretty", action="store_true", help="Pretty-print JSON")
    args = parser.parse_args()

    elements = []

    if args.existing:
        print(f"Re-optimizing {args.existing} ...")
        print("  NOTE: React-tree JSON lacks gradient defs. Use --local for full data.")
        elements = optimize_existing_json(args.existing)
        print(f"  Processed {len(elements)} elements")

    elif args.local:
        print(f"Reading from {args.local} ...")
        for i in range(1, ELEMENT_COUNT + 1):
            content = read_tsx_local(args.local, i)
            if not content:
                print(f"  WARN: Missing CyberEl{i}.tsx")
                continue
            svg = extract_svg_from_tsx(content)
            if not svg:
                print(f"  WARN: No SVG in CyberEl{i}.tsx")
                continue
            elements.append(preprocess_svg(svg))
            print(f"  Extracted CyberEl{i}")

    elif os.path.isdir(LOCAL_TSX_DIR):
        # Auto-detect local TSX source
        print(f"Using local TSX at {LOCAL_TSX_DIR} ...")
        for i in range(1, ELEMENT_COUNT + 1):
            content = read_tsx_local(LOCAL_TSX_DIR, i)
            if not content:
                print(f"  WARN: Missing CyberEl{i}.tsx")
                continue
            svg = extract_svg_from_tsx(content)
            if not svg:
                print(f"  WARN: No SVG in CyberEl{i}.tsx")
                continue
            elements.append(preprocess_svg(svg))
            print(f"  Extracted CyberEl{i}")

    else:
        print("Fetching from GitHub ...")
        for i in range(1, ELEMENT_COUNT + 1):
            content = fetch_tsx_from_github(i)
            if not content:
                continue
            svg = extract_svg_from_tsx(content)
            if not svg:
                print(f"  WARN: No SVG in CyberEl{i}")
                continue
            elements.append(preprocess_svg(svg))
            print(f"  Fetched CyberEl{i}")

    if not elements:
        print("ERROR: No elements extracted!")
        sys.exit(1)

    # Write output
    if args.pretty:
        json_str = json.dumps(elements, indent=2, ensure_ascii=False)
    else:
        json_str = json.dumps(elements, separators=(",", ":"), ensure_ascii=False)

    with open(args.output, "w") as f:
        f.write(json_str)

    size = os.path.getsize(args.output)
    total_paths = sum(len(e["paths"]) for e in elements)
    total_defs = sum(len(e.get("defs", [])) for e in elements)
    extra_color_count = sum(len(e.get("colors", [])) for e in elements)
    gradient_count = sum(
        1
        for e in elements
        for d in e.get("defs", [])
        if d["type"] in ("linearGradient", "radialGradient")
    )
    clip_count = sum(
        1 for e in elements for d in e.get("defs", []) if d["type"] == "clipPath"
    )

    print(f"\nGenerated {args.output}")
    print(f"  Elements: {len(elements)}")
    print(f"  Total paths: {total_paths}")
    print(f"  Extra literal colors: {extra_color_count}")
    print(f"  Gradient defs: {gradient_count}")
    print(f"  ClipPath defs: {clip_count}")
    print(f"  File size: {size:,} bytes ({size / 1024:.0f} KB)")


if __name__ == "__main__":
    main()
