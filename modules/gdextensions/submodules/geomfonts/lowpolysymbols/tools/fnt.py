#!/usr/bin/env python3

"""
LowPolySymbols OBJ to C header converter.

Converts a 3D OBJ font file (LowPolySymbols.obj) into a C header with
triangle vertex data suitable for mesh-based rendering.

The OBJ contains named objects (A-Z, 0-9, ', _). Each object is a 3D glyph
with world-space coordinates. This script:
  1. Parses each named object
  2. Normalizes vertices to local coordinates (origin at bottom-left)
  3. Triangulates faces (fan triangulation for n-gons)
  4. Outputs widths[], sizes[], vdataoffsets[], vdata[][3] arrays

Unlike bluntfont (2D, float[2]), this font preserves Z depth (float[3]).

Usage:
    python3 fnt.py LowPolySymbols.obj > lowpoly_vdata.h
"""

import sys
import re


def parse_obj(content):
    """Parse OBJ file into named objects with vertices and faces."""
    # Split at object boundaries: "# object NAME"
    # The pattern is: #\n# object NAME\n#\n
    raw_objects = re.split(r"(?:^|\n)#\n# object ", content)

    objects = []
    global_verts = []  # OBJ vertices are global (1-indexed)
    global_vnorms = []
    vert_count = 0

    for i, raw in enumerate(raw_objects):
        if i == 0:
            # Header section, may contain the first object
            if "# object " in raw:
                idx = raw.find("# object ")
                name = raw[idx + 9 :].split("\n")[0].strip()
                raw = raw[idx + len("# object ") + len(name) :]
            else:
                continue
        else:
            name = raw.split("\n")[0].strip()

        lines = raw.split("\n")

        local_verts = []
        faces = []
        vert_start = len(global_verts)

        for line in lines:
            line = line.strip()
            if line.startswith("v "):
                parts = line.split()
                x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
                global_verts.append((x, y, z))
                local_verts.append((x, y, z))
            elif line.startswith("f "):
                parts = line.split()[1:]
                # Face indices are global in OBJ (1-based)
                indices = []
                for p in parts:
                    vi = int(p.split("/")[0]) - 1  # convert to 0-based
                    indices.append(vi)
                faces.append(indices)

        objects.append(
            {
                "name": name,
                "verts": local_verts,
                "faces": faces,
                "vert_start": vert_start,
            }
        )

    return objects, global_verts


def triangulate_faces(faces):
    """Fan triangulation of n-gon faces into triangles."""
    tris = []
    for face in faces:
        for t in range(len(face) - 2):
            tris.append((face[0], face[t + 1], face[t + 2]))
    return tris


def normalize_glyph(verts):
    """Normalize glyph vertices to local coordinates.
    Returns (normalized_verts, width, height, depth)."""
    if not verts:
        return [], 0, 0, 0

    xs = [v[0] for v in verts]
    ys = [v[1] for v in verts]
    zs = [v[2] for v in verts]

    min_x, max_x = min(xs), max(xs)
    min_y = min(ys)
    min_z, max_z = min(zs), max(zs)

    width = max_x - min_x
    height = max_z - min_z
    depth = max(ys) - min_y

    # Normalize: shift so min_x=0, min_z=0, center Y around 0
    mid_y = (min_y + max(ys)) / 2.0
    normalized = []
    for x, y, z in verts:
        normalized.append((x - min_x, y - mid_y, z - min_z))

    return normalized, width, height, depth


def main():
    if len(sys.argv) != 2:
        print("Usage: %s LowPolySymbols.obj" % sys.argv[0], file=sys.stderr)
        sys.exit(1)

    with open(sys.argv[1], "r") as f:
        content = f.read()

    objects, global_verts = parse_obj(content)

    # Map named objects to ASCII codes
    # Known glyphs: A-Z (65-90), 0-9 (48-57), _ (95), ' (39)
    # Multiple '_' objects exist — use the first one for ASCII 95,
    # assign remaining sequentially to unused slots for symbols.

    # Build a mapping: determine which ASCII codes we have
    NUM_GLYPHS = 128
    glyph_data = [None] * NUM_GLYPHS  # (width, tri_verts_3d)

    # Assign known named objects
    underscore_assigned = False
    unknown_idx = 33  # Start assigning unknowns from '!' (33)

    # Track which symbol slots we've used for the extra '_' objects
    symbol_sequence = []
    # The OBJ has objects in order. After 0-9 and ', the remaining '_' objects
    # appear to be symbol glyphs in a specific order based on their position.
    # We'll map them to specific ASCII codes based on object index.

    # First pass: figure out the ordering
    # Objects 0-36: known (_,A-Z,0-9)
    # Object 37-56: symbols/padding (mostly '_' named, one '\'' at 40)
    # Based on visual inspection of OBJ positions, these map to:
    SYMBOL_MAP = {
        37: ord("!"),  # exclamation
        38: ord("."),  # period
        39: ord(","),  # comma
        40: ord("'"),  # apostrophe (named)
        41: ord('"'),  # double quote
        42: ord("&"),  # ampersand
        43: ord("?"),  # question
        44: ord(":"),  # colon
        45: ord("#"),  # hash
        46: ord("@"),  # at
        47: ord("%"),  # percent
        48: ord("-"),  # dash
        49: ord("+"),  # plus
        50: ord("="),  # equals
        51: ord("$"),  # dollar
        52: ord("/"),  # slash
        53: ord("<"),  # less than
        54: ord(">"),  # greater than
        55: ord("("),  # left paren
        56: ord(")"),  # right paren
    }

    for i, obj in enumerate(objects):
        name = obj["name"]
        verts = obj["verts"]
        faces = obj["faces"]

        if not verts or not faces:
            continue

        # Determine ASCII code
        if i in SYMBOL_MAP:
            ascii_code = SYMBOL_MAP[i]
        elif len(name) == 1 and name != "_":
            ascii_code = ord(name)
        elif name == "_" and not underscore_assigned:
            ascii_code = ord("_")
            underscore_assigned = True
        elif name == "_":
            # Extra underscore — check symbol map
            if i in SYMBOL_MAP:
                ascii_code = SYMBOL_MAP[i]
            else:
                continue  # skip unmapped extras
        else:
            continue

        if ascii_code >= NUM_GLYPHS:
            continue

        # Normalize and triangulate
        norm_verts, width, height, depth = normalize_glyph(verts)
        tris = triangulate_faces(faces)

        # Build triangle vertex stream using global vertex indices
        tri_stream = []
        for i0, i1, i2 in tris:
            # Convert global indices to local
            local_i0 = i0 - obj["vert_start"]
            local_i1 = i1 - obj["vert_start"]
            local_i2 = i2 - obj["vert_start"]
            if 0 <= local_i0 < len(norm_verts) and 0 <= local_i1 < len(norm_verts) and 0 <= local_i2 < len(norm_verts):
                tri_stream.append(norm_verts[local_i0])
                tri_stream.append(norm_verts[local_i1])
                tri_stream.append(norm_verts[local_i2])

        glyph_data[ascii_code] = (width, tri_stream)

    # Also create lowercase mappings (point to uppercase data)
    for c in range(ord("a"), ord("z") + 1):
        upper = c - 32  # 'a'-32 = 'A'
        if glyph_data[upper] is not None and glyph_data[c] is None:
            glyph_data[c] = glyph_data[upper]

    # Space glyph: empty with standard width
    if glyph_data[ord(" ")] is None:
        glyph_data[ord(" ")] = (20.0, [])

    # Compute arrays
    widths = [0.0] * NUM_GLYPHS
    sizes = [0] * NUM_GLYPHS
    streams = [[] for _ in range(NUM_GLYPHS)]

    for i in range(NUM_GLYPHS):
        if glyph_data[i] is not None:
            widths[i] = glyph_data[i][0]
            sizes[i] = len(glyph_data[i][1])
            streams[i] = glyph_data[i][1]

    totalsize = sum(sizes)

    def ffmt(x):
        """Format float as C float literal, ensuring 'f' suffix is valid."""
        s = "%.6g" % x
        if "." not in s and "e" not in s and "E" not in s:
            s += ".0"
        return s + "f"

    # Output C header
    print("// Machine-generated from %s by tools/fnt.py, do not edit." % sys.argv[1])
    print("#ifdef __clang__")
    print("#pragma clang diagnostic push")
    print('#pragma clang diagnostic ignored "-Wmissing-braces"')
    print('#pragma clang diagnostic ignored "-Wconversion"')
    print("#endif")
    print("")
    print("// clang-format off")
    print("#define LP_VDATASZ %d" % totalsize)
    print("#define LP_NUMGLYPHS %d" % NUM_GLYPHS)
    print("")

    # Widths
    print("// Glyph widths in local units.")
    print("static float lp_widths[LP_NUMGLYPHS] = {")
    for i in range(0, NUM_GLYPHS, 8):
        vals = ", ".join(ffmt(widths[j]) for j in range(i, min(i + 8, NUM_GLYPHS)))
        print("  %s," % vals)
    print("};")
    print("")

    # Sizes
    print("// Glyph sizes in number of triangle vertices (multiple of 3).")
    print("static int lp_sizes[LP_NUMGLYPHS] = {")
    line = "  "
    for i, sz in enumerate(sizes):
        line += "%d, " % sz
        if (i + 1) % 16 == 0:
            print(line)
            line = "  "
    if line.strip():
        print(line)
    print("};")
    print("")

    # Offsets
    print("// Vertex data offsets for each glyph.")
    print("static int lp_vdataoffsets[LP_NUMGLYPHS] = {")
    offset = 0
    line = "  "
    for i, sz in enumerate(sizes):
        line += "%d, " % offset
        offset += sz
        if (i + 1) % 8 == 0:
            print(line)
            line = "  "
    if line.strip():
        print(line)
    print("};")
    print("")

    # Vertex data (3D)
    print("// Vertex data for all glyphs combined (x, y, z).")
    print("static float lp_vdata[LP_VDATASZ][3] = {")
    for i in range(NUM_GLYPHS):
        if streams[i]:
            # Print comment with char
            if 32 <= i < 127:
                print("// '%s' (%d)" % (chr(i), i))
            else:
                print("// (%d)" % i)
            for v in streams[i]:
                print("  {%s, %s, %s}," % (ffmt(v[0]), ffmt(v[1]), ffmt(v[2])))
    print("};")
    print("// clang-format on")
    print("")
    print("#ifdef __clang__")
    print("#pragma clang diagnostic pop")
    print("#endif")

    # Print stats to stderr
    print(
        "\n// Stats: %d glyphs, %d total vertices, %d triangles"
        % (sum(1 for g in glyph_data if g is not None), totalsize, totalsize // 3),
        file=sys.stderr,
    )
    assigned = [i for i in range(NUM_GLYPHS) if glyph_data[i] is not None and sizes[i] > 0]
    print(
        "// Assigned glyphs: %s" % " ".join(chr(c) if 32 < c < 127 else "(%d)" % c for c in assigned),
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
