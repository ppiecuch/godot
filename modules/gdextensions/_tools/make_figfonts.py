#!/usr/bin/env python3
"""Generate the embedded debug-console FIGlet font sources from FIGfont files.

Usage: make_figfonts.py [INPUT_DIR] [-o OUTPUT_DIR] [-l LOG] [-p PATCH_DIR]

INPUT_DIR defaults to figfonts/ next to this script. Every .flf (FIGlet) and .tlf (TOIlet)
file in the folder is converted to scene/debugconsole/figlet_font_<name>.cpp, where <name>
is the lower-case file stem with the non-alphanumeric characters removed, so that
"ANSI Regular.flf" becomes figlet_font_ansiregular.cpp holding `Figlet::Banner ansiregular`.

Modern FIGfonts draw with Unicode box-drawing and block characters. The console renders
CP437 code pages, so every glyph character is translated to its CP437 code here, at
generation time; the `mappingFrom`/`mappingTo` runtime translation of the Banner class is
therefore not used by generated fonts anymore.

Shapes that the CP437 translation cannot deliver -- the quadrant blocks, the heavy
half-lines and the halves whose cell the sheet gives to their inverse -- are described by
the fixed `GLYPHS` table below, which names the glyph sheet cell (code and bank) drawing
each one. Nothing is detected from the sheets, and a font needing a shape the table does
not describe fails generation instead of being silently mangled.

Those shapes are emitted as placeholder codes rather than as their final console code: a
shape and its inverse share one drawn cell, so they are the same code in different banks,
and they have to stay distinct characters while the FIGlet renderer smushes glyphs, which
counts one byte per output column. Each generated font therefore carries a small
placeholder -> (code, bank) table the console applies on output.

figfonts_glyphs.log lists the cells every face depends on.
"""

import argparse
import datetime
import glob
import os
import re
import sys
import unicodedata

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_INPUT = os.path.join(HERE, "figfonts")
ENGINE_DIR = os.path.normpath(os.path.join(HERE, "..", "..", "..", "scene", "debugconsole"))

# The FIGfont specification: the 95 printable ASCII codes, then the seven "German" codes.
REQUIRED_CODES = list(range(32, 127)) + [196, 214, 220, 228, 246, 252, 223]

MAX_HEIGHT = 11  # Figlet::maxHeight
MAX_CODE = 256  # Figlet::maxTableSize

# Unicode spaces used as filler by the patorjk.com fonts.
SPACES = "\u00a0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u3000"

# Characters CP437 cannot encode, folded onto the closest code it has.
FOLD = {
    "\u2501": "\u2500",
    "\u2503": "\u2502",  # heavy horizontal/vertical
    "\u250f": "\u250c",
    "\u2513": "\u2510",  # heavy corners
    "\u2517": "\u2514",
    "\u251b": "\u2518",
    "\u2523": "\u251c",
    "\u252b": "\u2524",  # heavy tees
    "\u2533": "\u252c",
    "\u253b": "\u2534",
    "\u254b": "\u253c",  # heavy cross
}

# Per-font deviations from the plain CP437 translation, kept so that regenerating a font
# does not change how it has always looked in the console.
STYLE = {
    # Calvin S is only three rows tall: double horizontals make it unreadable, so the
    # double box drawing characters keep their double stems but lose the double rails.
    "calvins": {
        "\u2550": "\u2500",
        "\u2554": "\u2553",
        "\u2557": "\u2556",
        "\u255a": "\u2559",
        "\u255d": "\u255c",
        "\u2560": "\u255f",
        "\u2563": "\u2562",
        "\u2566": "\u2565",
        "\u2569": "\u2568",
        "\u256c": "\u256b",
    },
    # maxiwi is a pixel font, one cell per pixel: full blocks make it a solid slab, the
    # small square keeps the pixels apart.
    "maxiwi": {"\u2588": "\u25a0"},
}

# Every shape a FIGfont draws that the plain CP437 translation cannot deliver, and the
# console glyph cell that draws it. This table is the single source of truth; nothing is
# detected from the glyph sheets. Each entry is
#
#     character: (placeholder, code, bank, comment)
#
# `code`/`bank` name a cell of the sheets in fonts.altered/: bank 0 is the drawn upper
# half, bank 1 the lower half, which the console renders by inverting bank 0 unless an
# explicit patch is drawn there. A shape and its inverse therefore share a single drawn
# cell, which is why ten quadrant shapes need only five drawings.
#
# `placeholder` is the byte the generated font is rendered with. A shape and its inverse
# have to stay distinct characters while the FIGlet renderer smushes glyphs together, and
# that renderer counts exactly one byte per output column, so the bank cannot be spelled
# out in the glyph rows. The console resolves the placeholder to `code` on output and
# wraps the bank 1 ones in the \x01 bank toggle, see TextConsole::logf().
#
# Placeholders are frozen: renumbering one changes every generated font. They must avoid
# 0x00, 0x01 (the bank toggle), 0x0A and 0x0D (which would split the rendered lines) and
# stay below 0x20, which the CP437 translation never produces.
GLYPHS = {
    "\u25c6": (0x02, 0x04, 0, "\u25c6 BLACK DIAMOND"),
    "\u259d": (0x03, 0x08, 0, "\u259d QUADRANT UPPER RIGHT"),
    "\u2599": (0x04, 0x08, 1, "\u2599 QUADRANT UPPER LEFT AND LOWER LEFT AND LOWER RIGHT"),
    "\u2596": (0x05, 0x0A, 0, "\u2596 QUADRANT LOWER LEFT"),
    "\u259c": (0x06, 0x0A, 1, "\u259c QUADRANT UPPER LEFT AND UPPER RIGHT AND LOWER RIGHT"),
    "\u259a": (0x07, 0x0F, 1, "\u259a QUADRANT UPPER LEFT AND LOWER RIGHT"),
    "\u259e": (0x08, 0x0F, 0, "\u259e QUADRANT UPPER RIGHT AND LOWER LEFT"),
    "\u2598": (0x09, 0xDE, 0, "\u2598 QUADRANT UPPER LEFT"),
    "\u259f": (0x0B, 0xDE, 1, "\u259f QUADRANT UPPER RIGHT AND LOWER LEFT AND LOWER RIGHT"),
    "\u2597": (0x0C, 0xDF, 0, "\u2597 QUADRANT LOWER RIGHT"),
    "\u259b": (0x0E, 0xDF, 1, "\u259b QUADRANT UPPER LEFT AND UPPER RIGHT AND LOWER LEFT"),
    "\u2584": (0x0F, 0xDC, 0, "\u2584 LOWER HALF BLOCK"),
    "\u2580": (0x10, 0xDC, 1, "\u2580 UPPER HALF BLOCK"),
    "\u258c": (0x11, 0xDD, 0, "\u258c LEFT HALF BLOCK"),
    "\u2590": (0x12, 0xDD, 1, "\u2590 RIGHT HALF BLOCK"),
    "\u257a": (0x13, 0x01, 1, "\u257a BOX DRAWINGS HEAVY RIGHT"),
    "\u2578": (0x14, 0x02, 1, "\u2578 BOX DRAWINGS HEAVY LEFT"),
    "\u257b": (0x15, 0x16, 1, "\u257b BOX DRAWINGS HEAVY DOWN"),
    "\u2579": (0x16, 0x17, 1, "\u2579 BOX DRAWINGS HEAVY UP"),
}

HEADER_RE = re.compile(r"^[ft]lf2a(.)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)\s+(\d+)")


def discover_fonts(input_dir):
    """Find the FIGfont inputs, deriving the C++ face name from the file name."""
    paths = sorted(glob.glob(os.path.join(input_dir, "*.flf")) + glob.glob(os.path.join(input_dir, "*.tlf")))
    fonts = []
    for path in paths:
        stem = os.path.splitext(os.path.basename(path))[0]
        name = re.sub(r"[^0-9a-zA-Z]+", "", stem).lower()
        if not name:
            print("skipping %s: empty face name" % path)
            continue
        fonts.append((name, path))
    if not fonts:
        raise SystemExit("no FIGfont files found in %s" % input_dir)
    return fonts


def parse_header(line, path):
    match = HEADER_RE.match(line)
    if not match:
        raise ValueError("%s: bad FIGfont header: %r" % (path, line))
    height = int(match.group(2))
    if height > MAX_HEIGHT:
        raise ValueError("%s: height %d exceeds Figlet::maxHeight (%d)" % (path, height, MAX_HEIGHT))
    return {
        "hardblank": match.group(1),
        "height": height,
        "baseline": int(match.group(3)),
        "max_length": int(match.group(4)),
        "old_layout": int(match.group(5)),
        "comment_lines": int(match.group(6)),
    }


def parse_figfont(path):
    """Read a .flf/.tlf file following the FIGfont specification."""
    with open(path, "r", encoding="utf-8", newline="") as f:
        lines = f.read().split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    lines = [line.rstrip("\r") for line in lines]
    if not lines:
        raise ValueError("%s: empty file" % path)

    header = parse_header(lines[0], path)
    height = header["height"]
    index = 1 + header["comment_lines"]
    if index >= len(lines):
        raise ValueError("%s: no glyph data" % path)

    def read_glyph(at, code):
        if at + height > len(lines):
            raise ValueError("%s: glyph %d is truncated" % (path, code))
        rows = []
        for raw in lines[at : at + height]:
            # The end mark is the last character of the line and is never a space, so
            # trailing blanks past it can go; some fonts vary the mark per character.
            row = raw.rstrip(" \t")
            if not row:
                raise ValueError("%s: glyph %d has a line without an end mark" % (path, code))
            rows.append(row.rstrip(row[-1]))
        return rows, at + height

    glyphs = []
    for code in REQUIRED_CODES:
        if index >= len(lines):
            break  # some fonts stop after the printable ASCII range
        rows, index = read_glyph(index, code)
        glyphs.append((code, rows))

    while index < len(lines):
        tag = lines[index].strip()
        index += 1
        if not tag:
            continue
        token = tag.split()[0]
        try:
            code = int(token, 0) if not token.lstrip("-").isdigit() else int(token)
        except ValueError:
            raise ValueError("%s: bad code tag: %r" % (path, tag))
        rows, index = read_glyph(index, code)
        if 0 <= code < MAX_CODE:
            glyphs.append((code, rows))

    return header, glyphs


def resolve_char(char, style):
    """Map one source character to the byte the generated font renders it with.

    Returns None when neither GLYPHS nor CP437 can express the character.
    """
    char = style.get(char, char)
    if char in GLYPHS:
        return GLYPHS[char][0]
    if char in SPACES:
        return 0x20
    try:
        return FOLD.get(char, char).encode("cp437")[0]
    except UnicodeEncodeError:
        return None


def translate(rows, name, used):
    """Translate one glyph from Unicode to console bytes, padded to a rectangle.

    The left/right space counts are taken from the source text, where the Unicode fillers
    are not spaces, matching what the console expects for packing.
    """
    style = STYLE.get(name, {})
    width = max(len(row) for row in rows) if rows else 0
    out_rows, lspaces, rspaces = [], [], []
    for row in rows:
        row = row + " " * (width - len(row))
        lspaces.append(len(row) - len(row.lstrip(" ")))
        rspaces.append(len(row) - len(row.rstrip(" ")))
        out = bytearray()
        for char in row:
            code = resolve_char(char, style)
            if style.get(char, char) in GLYPHS:
                used.add(style.get(char, char))
            out.append(code)
        out_rows.append(bytes(out))
    return out_rows, lspaces, rspaces, width


def validate_glyphs(fonts):
    """Fail before writing anything if a font needs a shape GLYPHS does not describe."""
    unknown = set()
    for name, _, _, glyphs in fonts:
        style = STYLE.get(name, {})
        for _, rows in glyphs:
            for row in rows:
                for char in row:
                    if resolve_char(char, style) is None:
                        unknown.add(style.get(char, char))
    if not unknown:
        return
    taken = set(entry[0] for entry in GLYPHS.values()) | {0x00, 0x01, 0x0A, 0x0D}
    free = [code for code in range(0x02, 0x20) if code not in taken]
    lines = ["FIGfont input needs shapes that are not in GLYPHS:"]
    for char in sorted(unknown, key=ord):
        try:
            unicode_name = unicodedata.name(char)
        except ValueError:
            unicode_name = "?"
        lines.append("  U+%04X %s %s" % (ord(char), char, unicode_name))
    lines.append("Add an entry naming the cell of fonts.altered/ that draws it.")
    lines.append("Free placeholders: %s" % " ".join("0x%02X" % code for code in free))
    raise SystemExit("\n".join(lines))


def licence_header(out_path):
    """Reuse the licence block already present in the generated file, if any."""
    if not os.path.exists(out_path):
        return ""
    with open(out_path, "r", encoding="latin-1") as f:
        text = f.read()
    marker = "/**************************************************************************/"
    end = text.rfind(marker)
    return text[: end + len(marker)] + "\n\n" if end != -1 else ""


def c_string(data):
    out = ['"']
    for byte in data:
        if byte == 0x22 or byte == 0x5C:
            out.append("\\" + chr(byte))
        elif 0x20 <= byte < 0x7F:
            out.append(chr(byte))
        else:
            out.append("\\%03o" % byte)  # octal: never eats the following character
    out.append('"')
    return "".join(out)


def glyph_comment(code):
    if code == 32:
        return 'letter "space"'
    if 33 <= code < 127:
        return 'letter N. %d " %s "' % (code, chr(code))
    return "letter N. %d" % code


def write_font(path, name, header, glyphs, used):
    with open(path, "w", encoding="latin-1", newline="\n") as out:
        out.write(licence_header(path))
        out.write('#include "figlet_font.h"\n\n')
        out.write("namespace Figlet {\n\n")
        out.write(
            "static char const Hardblank = '%s';\n"
            % (header["hardblank"] if 0x20 <= ord(header["hardblank"]) < 0x7F else "\\%03o" % ord(header["hardblank"]))
        )
        out.write("static unsigned const FontHeight = %d;\n" % header["height"])
        out.write("static unsigned const FontMaxLen = %d;\n\n" % max(g[3] for g in glyphs))
        out.write("// clang-format off\n\n")
        out.write("static FontFiglet characters[] = {\n")

        for index, (code, rows, spaces, width) in enumerate(glyphs):
            lspaces, rspaces = spaces
            comma = "," if index + 1 < len(glyphs) else ""
            out.write("\t// %s\n" % glyph_comment(code))
            out.write("\t{ %d,\n" % code)
            out.write("\t\t{ %s },\n" % ", ".join(str(v) for v in lspaces))
            out.write("\t\t{ %s },\n" % ", ".join(str(v) for v in rspaces))
            out.write("\t\t{\t")
            for row_index, row in enumerate(rows):
                if row_index:
                    out.write("\t\t\t")
                suffix = " } }%s\n" % comma if row_index + 1 == len(rows) else ",\n"
                out.write(c_string(row) + suffix)
            if index + 1 < len(glyphs):
                out.write("\n")

        out.write("};\n\n")
        out.write("// clang-format on\n\n")
        out.write("static unsigned const FontSize = sizeof(characters) / sizeof(characters[0]);\n")
        if used:
            entries = sorted(used, key=lambda char: GLYPHS[char][0])
            out.write("\n// Shapes CP437 has no code for. The glyphs above render them as placeholder\n")
            out.write("// codes so that a shape and its inverse stay distinct while smushing; the\n")
            out.write("// console resolves each one to the console glyph below, taking it from glyph\n")
            out.write("// bank 1 where listed. See TextConsole::logf().\n")
            for char in entries:
                placeholder, code, bank, comment = GLYPHS[char]
                # The generated sources are latin-1, so name the shape instead of drawing it.
                out.write(
                    "//   \\%03o -> \\%03o bank %d   U+%04X %s\n"
                    % (placeholder, code, bank, ord(char), comment.split(" ", 1)[1])
                )
            out.write("static char const Placeholders[] = %s;\n" % c_string([GLYPHS[char][0] for char in entries]))
            out.write("static char const PlaceholderGlyphs[] = %s;\n" % c_string([GLYPHS[char][1] for char in entries]))
            bank1 = [GLYPHS[char][0] for char in entries if GLYPHS[char][2]]
            out.write("static char const PlaceholderBank1[] = %s;\n\n" % c_string(bank1))
            out.write(
                "Banner %s(characters, Hardblank, FontHeight, FontMaxLen, FontSize,\n"
                "\t\tFIGLET_FULLWIDTH, Placeholders, PlaceholderGlyphs, PlaceholderBank1);\n" % name
            )
        else:
            out.write("Banner %s(characters, Hardblank, FontHeight, FontMaxLen, FontSize, FIGLET_FULLWIDTH);\n" % name)
        out.write("} // namespace Figlet\n")


def write_log(path, report, summary, argv):
    """Write the run summary plus the glyph cells the generated fonts depend on.

    Both belong to the same regeneration, so they share one file: the summary says what
    was produced, the table below says which drawn cells of fonts.altered/ those fonts
    need in order to render correctly.
    """
    with open(path, "w", encoding="utf-8", newline="\n") as out:
        out.write("# Summary of the last make_figfonts.py run -- regenerated, do not edit.\n")
        out.write("# %s\n" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        out.write("# %s\n\n" % " ".join(argv))
        for line in summary:
            out.write(line.rstrip() + "\n")

        out.write("\n")
        out.write("# Glyph cells the generated FIGlet fonts need from fonts.altered/\n")
        out.write("#\n")
        out.write("# Taken from the GLYPHS table below; nothing here is detected from the\n")
        out.write("# sheets. `code`/`bank` name a cell of every dos-<w>x<h> sheet: bank 0 is the\n")
        out.write("# drawn upper half of the sheet, bank 1 the lower half, which the console\n")
        out.write("# renders by inverting bank 0 unless a patch is drawn there. A shape and its\n")
        out.write("# inverse therefore share one drawn cell.\n")
        out.write("#\n")
        out.write("# %-5s %-4s %-4s %-52s %s\n" % ("place", "code", "bank", "shape", "used by"))
        for char in sorted(report, key=lambda c: GLYPHS[c][0]):
            placeholder, code, bank, comment = GLYPHS[char]
            out.write(
                "# 0x%02X  0x%02X  %d     %-52s %s\n"
                % (placeholder, code, bank, comment, ", ".join(sorted(report[char])))
            )
        if not report:
            out.write("# (none)\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", nargs="?", default=DEFAULT_INPUT, help="folder holding the FIGfont files")
    parser.add_argument("-o", "--output-dir", default=ENGINE_DIR)
    parser.add_argument(
        "-l",
        "--log",
        default=os.path.join(HERE, "make_figfonts.log"),
        help="where to write the run summary and the glyph cell report",
    )
    args = parser.parse_args()

    summary = []

    def report_line(text):
        print(text)
        summary.append(text)

    fonts = []
    for name, source in discover_fonts(args.input):
        header, glyphs = parse_figfont(source)
        fonts.append((name, source, header, glyphs))
    validate_glyphs(fonts)

    total = 0
    report = {}  # character -> [face, ...]
    for name, source, header, raw_glyphs in fonts:
        used = set()
        glyphs = []
        for code, rows in raw_glyphs:
            cells, lspaces, rspaces, width = translate(rows, name, used)
            glyphs.append((code, cells, (lspaces, rspaces), width))
        out_path = os.path.join(args.output_dir, "figlet_font_%s.cpp" % name)
        write_font(out_path, name, header, glyphs, used)
        total += len(glyphs)
        report_line(
            "%-12s %-20s glyphs: %3d  height: %2d  width: %2d  shapes: %2d  bytes: %6d"
            % (
                name,
                os.path.basename(source),
                len(glyphs),
                header["height"],
                max(g[3] for g in glyphs),
                len(used),
                os.path.getsize(out_path),
            )
        )
        for char in used:
            report.setdefault(char, []).append(name)

    cells = set(GLYPHS[char][1] for char in report)
    report_line("wrote %d figlet glyphs; %d shapes out of %d drawn glyph cells" % (total, len(report), len(cells)))

    write_log(args.log, report, summary, sys.argv)
    print("wrote %s" % args.log)


if __name__ == "__main__":
    sys.exit(main())
