#!/usr/bin/env python3
"""
gen_rockcluster_cells_data.py — codegen for the RockCluster generator's embedded
"cell" meshes (Boulder/Sharp/Crystal scatter pieces).

Parses Unity's ForceText .mesh YAML asset format directly (no Unity SDK
involved) — verified against the actual source files:
  - m_VertexData.m_Channels: a fixed 14-slot array (Position, Normal, Tangent,
    Color, TexCoord0-7, BlendWeight, BlendIndices), each {stream, offset,
    format, dimension}. `format` is a VertexAttributeFormat id — only 0
    (Float32) and 2 (UNorm8) appear in the source files; byte size per
    component is looked up from FORMAT_BYTES.
  - m_VertexData._typelessdata: the packed, interleaved per-vertex byte
    buffer, as a hex string. Per-vertex stride is the sum of every channel's
    (dimension * bytes-per-component), including unused channels — needed to
    correctly step between vertices even though only Position/Normal/Color
    are actually extracted.
  - m_IndexBuffer: hex string, m_IndexFormat selects uint16 (0) or uint32 (1).

Coordinate conversion: Unity is left-handed (Y-up, Z-forward); Godot 3.x is
right-handed (Y-up, Z-toward-viewer). Negating Z (positions and normals) plus
reversing each triangle's winding is the standard LH->RH conversion — skipping
this would silently produce mirrored geometry with inverted normals/winding,
exactly the class of bug this generator's normals/winding fixes guard against.

Output: rockcluster_cells_data.gen.{h,cpp} — plain C arrays, no runtime
parsing needed (this script IS the parser, run once by hand; see
submodules/material_symbols/gen_material_symbols_data.py for the sibling
pattern this follows).
"""

import argparse
import re
import struct
import sys
from pathlib import Path

STYLES = [
    ("Type01", "Boulder"),
    ("Type02", "Sharp"),
    ("Type03", "Crystal"),
]

# VertexAttributeFormat id -> bytes per component. Only 0 and 2 are actually
# used anywhere in the source files (verified via a repo-wide grep), but the
# table is complete so an unexpected format fails loudly instead of silently
# miscomputing the stride.
FORMAT_BYTES = {
    0: 4,  # Float32
    1: 2,  # Float16
    2: 1,  # UNorm8
    3: 1,  # SNorm8
    4: 2,  # UNorm16
    5: 2,  # SNorm16
    6: 1,  # UInt8
    7: 1,  # SInt8
    8: 2,  # UInt16
    9: 2,  # SInt16
    10: 4,  # UInt32
    11: 4,  # SInt32
}

CHANNEL_RE = re.compile(r"stream:\s*(\d+)\s*\n\s*offset:\s*(\d+)\s*\n\s*format:\s*(\d+)\s*\n\s*dimension:\s*(\d+)")


class Cell:
    def __init__(self, name, positions, normals, colors, indices):
        self.name = name
        self.positions = positions  # flat [x,y,z, x,y,z, ...]
        self.normals = normals  # flat, same shape
        self.colors = colors  # flat [r,g,b,a, ...]
        self.indices = indices  # flat triangle list


def parse_mesh(path: Path) -> Cell:
    text = path.read_text(encoding="utf-8", errors="replace")

    def find_int(key):
        m = re.search(rf"^\s*{key}:\s*(-?\d+)", text, re.M)
        if not m:
            raise ValueError(f"{path}: missing {key}")
        return int(m.group(1))

    name_m = re.search(r"^\s*m_Name:\s*(.+)$", text, re.M)
    name = name_m.group(1).strip() if name_m else path.stem

    vertex_count = find_int("m_VertexCount")
    data_size = find_int("m_DataSize")
    index_format = find_int("m_IndexFormat")

    chan_block_m = re.search(r"m_Channels:\n((?:\s*-\s*stream:.*\n(?:\s*\w+:.*\n){3})+)", text)
    if not chan_block_m:
        raise ValueError(f"{path}: no m_Channels block found")
    channels = [
        {"stream": int(a), "offset": int(b), "format": int(c), "dimension": int(d)}
        for a, b, c, d in CHANNEL_RE.findall(chan_block_m.group(1))
    ]
    if len(channels) != 14:
        raise ValueError(f"{path}: expected 14 channels, got {len(channels)}")

    stride = sum(ch["dimension"] * FORMAT_BYTES[ch["format"]] for ch in channels)
    if stride * vertex_count != data_size:
        raise ValueError(
            f"{path}: stride*count ({stride}*{vertex_count}={stride * vertex_count}) != m_DataSize ({data_size})"
        )

    hex_m = re.search(r"_typelessdata:\s*([0-9a-fA-F]*)", text)
    raw = bytes.fromhex(hex_m.group(1)) if hex_m and hex_m.group(1) else b""
    if len(raw) != data_size:
        raise ValueError(f"{path}: decoded {len(raw)} vertex-data bytes, expected {data_size}")

    # Unity's channel array is a *fixed* semantic order (Position, Normal,
    # Tangent, Color, TexCoord0-7, BlendWeight, BlendIndices) regardless of
    # which channels a given mesh actually uses — so index 0/1/3 are always
    # Position/Normal/Color, but Color's own format varies per file (plain
    # Float32 in some files, packed UNorm8 in others), so it can't be assumed
    # like Position/Normal's format can.
    pos_ch, norm_ch, color_ch = channels[0], channels[1], channels[3]
    for ch, label in ((pos_ch, "position"), (norm_ch, "normal")):
        if ch["dimension"] != 3 or ch["format"] != 0:
            raise ValueError(f"{path}: unexpected {label} channel layout {ch}")
    has_color = color_ch["dimension"] > 0
    if has_color and color_ch["format"] not in (0, 2):
        raise ValueError(f"{path}: unexpected color channel format {color_ch}")

    positions = []
    normals = []
    colors = []
    for i in range(vertex_count):
        base = i * stride
        px, py, pz = struct.unpack_from("<3f", raw, base + pos_ch["offset"])
        nx, ny, nz = struct.unpack_from("<3f", raw, base + norm_ch["offset"])
        # LH(Unity) -> RH(Godot): negate Z for both position and normal.
        positions += [px, py, -pz]
        normals += [nx, ny, -nz]
        if has_color:
            dim = color_ch["dimension"]
            if color_ch["format"] == 0:
                comps = list(struct.unpack_from(f"<{dim}f", raw, base + color_ch["offset"]))
            else:  # format 2: UNorm8
                comps = [b / 255.0 for b in raw[base + color_ch["offset"] : base + color_ch["offset"] + dim]]
            comps += [1.0] * (4 - len(comps))
            colors += comps[:4]
        else:
            colors += [1.0, 1.0, 1.0, 1.0]

    idx_hex_m = re.search(r"m_IndexBuffer:\s*([0-9a-fA-F]*)", text)
    idx_raw = bytes.fromhex(idx_hex_m.group(1)) if idx_hex_m and idx_hex_m.group(1) else b""
    if index_format == 0:
        indices = list(struct.unpack_from(f"<{len(idx_raw) // 2}H", idx_raw))
    elif index_format == 1:
        indices = list(struct.unpack_from(f"<{len(idx_raw) // 4}I", idx_raw))
    else:
        raise ValueError(f"{path}: unknown m_IndexFormat {index_format}")
    if len(indices) % 3 != 0:
        raise ValueError(f"{path}: index count {len(indices)} not a multiple of 3")

    # Negating one axis flips handedness, which flips winding — reverse it
    # back so triangles stay outward-facing under Godot's convention.
    for i in range(0, len(indices), 3):
        indices[i + 1], indices[i + 2] = indices[i + 2], indices[i + 1]

    cell = Cell(name, positions, normals, colors, indices)
    _check_winding_matches_normals(path, cell)
    return cell


def _tri_normal(positions, a, b, c):
    def pos(i):
        return (positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2])

    p0, p1, p2 = pos(a), pos(b), pos(c)
    ux, uy, uz = p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]
    vx, vy, vz = p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]
    return (uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx)


def _check_winding_matches_normals(path: Path, cell: Cell) -> None:
    """Cross-checks the LH(Unity)->RH(Godot) conversion above (Z-negate position/normal,
    reverse winding) the only way that actually catches a mistake in it: for every
    triangle, its own geometric face normal (derived purely from the converted positions)
    must point the same general direction as its vertices' own converted, stored normals.
    A wrong conversion (e.g. negating the wrong axis, or reversing winding without also
    negating, or vice versa) would flip *every* triangle in a file consistently, which is
    exactly what this catches. Verified once (2026) against all 57 source .mesh files:
    zero mismatches out of 6491 triangles, confirming the conversion itself is correct and
    any winding defect seen at runtime is not from this parsing step."""
    idx = cell.indices
    for i in range(0, len(idx), 3):
        a, b, c = idx[i], idx[i + 1], idx[i + 2]
        fn = _tri_normal(cell.positions, a, b, c)
        for v in (a, b, c):
            vn = (cell.normals[v * 3], cell.normals[v * 3 + 1], cell.normals[v * 3 + 2])
            if fn[0] * vn[0] + fn[1] * vn[1] + fn[2] * vn[2] < 0:
                raise ValueError(
                    f"{path}: triangle ({a},{b},{c}) winding disagrees with vertex {v}'s "
                    f"normal after LH->RH conversion -- conversion bug, not a source-data defect"
                )


def _edge_key(positions, a, b, prec=100000):
    def pos(i):
        return (positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2])

    def quant(p):
        return tuple(round(c * prec) for c in p)

    return (quant(pos(a)), quant(pos(b)))


def _tri_area(positions, a, b, c):
    def pos(i):
        return (positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2])

    p0, p1, p2 = pos(a), pos(b), pos(c)
    ux, uy, uz = p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]
    vx, vy, vz = p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]
    cx, cy, cz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
    return 0.5 * (cx * cx + cy * cy + cz * cz) ** 0.5


def repair_non_manifold_triangles(cell: Cell) -> Cell:
    """Some source cell meshes (confirmed: Boulder/"Cubic" specifically -- Sharp and
    Crystal decode via this exact same code path with zero defects) contain non-manifold
    overlapping triangles -- near-duplicate triangles at matching-but-not-identical
    positions, and at least one confirmed case of an extra triangle spanning a quad's
    other diagonal on top of two already-valid ones. This is a genuine authoring/export
    quirk in the third-party source asset (these chunks are evidently unioned from
    overlapping pieces without a weld/cleanup pass), not a bug in this script's own
    decode math -- verified by the winding-vs-normal cross-check in parse_mesh() passing
    with zero mismatches across all 57 files, Boulder included.

    Originally left as-is (see rockcluster.cpp's _clip_and_cap_cell() for the runtime
    workaround this required -- a convex-hull cap robust to messy input, instead of an
    exact traced boundary loop). Measured directly (not assumed) once this started
    visibly affecting the *rendered* look, not just the cut-plane cap: overlapping
    triangles cause real z-fighting on the un-cut portion of a cell too, which reads as
    "patchy/dark/see-through" on a solid-looking chunk -- averaging ~8.5% of a cell's own
    surface area across the 28 affected cells, up to 28.6% for the worst one.

    Fixed here at the source (once, offline) rather than at runtime: greedily removes
    triangles that share a directed edge with another triangle (the same signature used
    to detect and measure this) until none remain, recomputing edge counts after each
    removal since removing one triangle can resolve or newly reveal others. Aborts and
    leaves the cell's data untouched (with a loud warning) if this would need removing
    more than 40% of a cell's triangles -- a defensive guard against the greedy algorithm
    doing something drastic on a cell shaped differently than the ones this was tuned
    against, rather than silently gutting it.
    """
    triangles = [tuple(cell.indices[i : i + 3]) for i in range(0, len(cell.indices), 3)]
    original_count = len(triangles)
    original_area = sum(_tri_area(cell.positions, *t) for t in triangles)

    def edge_counts(tris):
        counts = {}
        for a, b, c in tris:
            for u, v in ((a, b), (b, c), (c, a)):
                k = _edge_key(cell.positions, u, v)
                counts[k] = counts.get(k, 0) + 1
        return counts

    removed = 0
    while True:
        counts = edge_counts(triangles)
        worst_idx, worst_score = -1, 0
        for i, (a, b, c) in enumerate(triangles):
            edges = [_edge_key(cell.positions, u, v) for u, v in ((a, b), (b, c), (c, a))]
            score = sum(1 for e in edges if counts[e] > 1)
            if score > worst_score:
                worst_idx, worst_score = i, score
        if worst_idx < 0:
            break  # no triangle has a duplicated directed edge left
        if removed + 1 > original_count * 0.4:
            print(f"  {cell.name}: WARNING -- repair would remove >40% of triangles, aborting (data left unrepaired)")
            return cell
        del triangles[worst_idx]
        removed += 1

    if removed == 0:
        return cell

    repaired_area = sum(_tri_area(cell.positions, *t) for t in triangles)
    print(
        f"  {cell.name}: removed {removed}/{original_count} non-manifold triangles "
        f"({original_area:.2f} -> {repaired_area:.2f} surface area, "
        f"{(original_area - repaired_area) / original_area * 100:.1f}% reduction)"
    )
    cell.indices = [i for tri in triangles for i in tri]
    return cell


def emit_array(out, ctype, name, values, per_line=12):
    out.write(f"const {ctype} {name}[{len(values)}] = {{\n")
    for i in range(0, len(values), per_line):
        chunk = values[i : i + per_line]
        out.write("    " + ", ".join(repr(v) if isinstance(v, float) else str(v) for v in chunk) + ",\n")
    out.write("};\n\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument(
        "--src",
        type=Path,
        required=True,
        help="source directory containing Type01/Type02/Type03 subfolders of .mesh files",
    )
    ap.add_argument("--out-h", type=Path, required=True)
    ap.add_argument("--out-cpp", type=Path, required=True)
    args = ap.parse_args()

    if not args.src.is_dir():
        sys.exit(f"no such directory: {args.src}")

    style_cells = {}
    for subdir, style_name in STYLES:
        d = args.src / subdir
        if not d.is_dir():
            sys.exit(f"missing source directory: {d}")
        paths = sorted(d.glob("*.mesh"))
        if not paths:
            sys.exit(f"no .mesh files found in {d}")
        cells = [parse_mesh(p) for p in paths]
        if style_name == "Boulder":
            # Sharp/Crystal (Type02/Type03) decode via this exact same code path with zero
            # non-manifold defects found -- the repair pass is Boulder-specific because the
            # defect itself is (see repair_non_manifold_triangles()'s own docstring).
            print(f"{style_name}: repairing non-manifold triangles...")
            cells = [repair_non_manifold_triangles(c) for c in cells]
        style_cells[style_name] = cells
        total_tris = sum(len(c.indices) // 3 for c in cells)
        print(f"{style_name}: {len(cells)} cells, {total_tris} triangles total, avg {total_tris / len(cells):.1f}/cell")

    with open(args.out_h, "w", encoding="utf-8") as h:
        h.write("// AUTOGENERATED by gen_rockcluster_cells_data.py — do not edit by hand.\n")
        h.write("// See this script's own docstring for the source mesh format details.\n")
        h.write("#ifndef ROCKCLUSTER_CELLS_DATA_GEN_H\n#define ROCKCLUSTER_CELLS_DATA_GEN_H\n\n")
        h.write("namespace rockcluster_data {\n\n")
        h.write("struct CellData {\n")
        h.write("    const float *positions; // xyz per vertex\n")
        h.write("    const float *normals; // xyz per vertex\n")
        h.write("    const float *colors; // rgba per vertex\n")
        h.write("    int vertex_count;\n")
        h.write("    const int *indices;\n")
        h.write("    int index_count;\n")
        h.write("};\n\n")
        for _, style_name in STYLES:
            n = len(style_cells[style_name])
            lname = style_name.lower()
            h.write(f"extern const CellData k{style_name}Cells[{n}];\n")
            h.write(f"constexpr int k{style_name}CellCount = {n};\n\n")
        h.write("} // namespace rockcluster_data\n\n#endif // ROCKCLUSTER_CELLS_DATA_GEN_H\n")

    with open(args.out_cpp, "w", encoding="utf-8") as cpp:
        cpp.write("// AUTOGENERATED by gen_rockcluster_cells_data.py — do not edit by hand.\n")
        cpp.write('#include "rockcluster_cells_data.gen.h"\n\n')
        cpp.write("namespace rockcluster_data {\n\n")
        for _, style_name in STYLES:
            cells = style_cells[style_name]
            for i, cell in enumerate(cells):
                prefix = f"k{style_name}_{i}"
                emit_array(cpp, "float", f"{prefix}_positions", cell.positions)
                emit_array(cpp, "float", f"{prefix}_normals", cell.normals)
                emit_array(cpp, "float", f"{prefix}_colors", cell.colors)
                emit_array(cpp, "int", f"{prefix}_indices", cell.indices)
            cpp.write(f"const CellData k{style_name}Cells[{len(cells)}] = {{\n")
            for i, cell in enumerate(cells):
                prefix = f"k{style_name}_{i}"
                vcount = len(cell.positions) // 3
                icount = len(cell.indices)
                cpp.write(
                    f"    {{ {prefix}_positions, {prefix}_normals, {prefix}_colors, {vcount}, {prefix}_indices, {icount} }}, // {cell.name}\n"
                )
            cpp.write("};\n\n")
        cpp.write("} // namespace rockcluster_data\n")

    print(f"wrote {args.out_h} and {args.out_cpp}")


if __name__ == "__main__":
    main()
