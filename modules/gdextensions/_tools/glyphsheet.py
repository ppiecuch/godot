#!/usr/bin/env python3
"""Shared helpers for the glyph-sheet generators (make_sparkline.py, make_vbars.py).

These tools do not touch the engine. They draw a family of glyphs into a PBM laid out
exactly like the hand-edited atlases in fonts.altered/, so the result can be opened next
to the .xcf, aligned at (0, 0), and merged into the drawn half a cell at a time.

Atlas layout (see make_fonts.py for the full description)
--------------------------------------------------------
Each fonts.altered/dos-WxH@24.bmp is a 16x32 grid of cells holding two banks:

    visual top half     the 256 CP437 glyphs in video-reverse   <- leave alone
    visual bottom half  the same glyphs as drawn, white on black <- edit here

make_fonts.py recovers bank 0 by un-reversing the top half, and treats any cell where
the two halves are *not* exact complements as a hand-drawn bank 1 patch. So a new glyph
is added by drawing it into the bottom half only: touching the top half would change the
CP437 glyph itself. The sheets written here therefore carry the new glyphs at their bottom
half positions and leave everything else black.

PBM is written as plain P1, one character per pixel and no separators, so the raster is
readable in any text editor -- but mind the polarity, which is inverted twice over.

P1 stores 1 for a BLACK pixel. The *drawn* (lower) half of the atlas, which is the half
these sheets are pasted into, stores glyphs white-on-black: make_fonts.py takes a set pixel
there to be lit, exactly as it does for the regular glyphs it recovers from the reversed
half. A lit pixel is therefore written here as 0, so that it comes out white.

Only the reversed (upper) half is black-on-white, which is what makes it read as normal
text to a human scrolling the atlas -- but that half is never hand-edited. Getting this
backwards yields sheets that look correct on screen and render as their own complement: a
1/8 bar comes out 7/8 full. The .log is the human-readable copy, where # is a lit pixel.
"""

import datetime
import glob
import os
import re
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_INPUT = os.path.join(HERE, "fonts.altered")
DEFAULT_OUTPUT = os.path.join(HERE, "glyphsheets")

GRID_COLS = 16
GRID_ROWS = 16

# Cells the figlet faces already depend on, mirrored from the GLYPHS table in
# make_figfonts.py. Bank 1 of these codes is either a hand-drawn patch or, where the table
# asks for an inverse, the *absence* of a patch -- either way a generator must not land on
# one. Validated at run time against the real table, this is only the fallback.
FIGLET_CELLS = {0x01, 0x02, 0x04, 0x08, 0x0A, 0x0B, 0x16, 0x17, 0xDC, 0xDD, 0xDE, 0xDF}

# Codes that can never appear in a console string, whatever bank they name.
UNUSABLE_CODES = {0x00, 0x0A, 0x0D}

# CP437 0x80-0x9A: the accented European letters. A debug console never needs \xe9 or
# \xfc, let alone their video-reverse, so this is the longest run of free bank 1 real
# estate in the atlas -- 27 codes against the 16 the Greek block would give, and clear of
# every cell make_figfonts.py reserves. 0x9B-0x9F (the currency symbols) are left alone.
LATIN_FIRST = 0x80
LATIN_LAST = 0x9A
CP437_LATIN = [
    "C-cedilla",
    "u-diaeresis",
    "e-acute",
    "a-circumflex",
    "a-diaeresis",
    "a-grave",
    "a-ring",
    "c-cedilla",
    "e-circumflex",
    "e-diaeresis",
    "e-grave",
    "i-diaeresis",
    "i-circumflex",
    "i-grave",
    "A-diaeresis",
    "A-ring",
    "E-acute",
    "ae",
    "AE",
    "o-circumflex",
    "o-diaeresis",
    "o-grave",
    "u-circumflex",
    "u-grave",
    "y-diaeresis",
    "O-diaeresis",
    "U-diaeresis",
]


def figlet_reserved_cells():
    """The cells make_figfonts.py depends on, read from the real table when possible."""
    try:
        import make_figfonts

        return {cell for (_, cell, _, _) in make_figfonts.GLYPHS.values()}
    except Exception:
        return set(FIGLET_CELLS)


def cp437_name(code):
    """What the code draws today, so the log says which glyph a patch is displacing."""
    if LATIN_FIRST <= code <= LATIN_LAST:
        return CP437_LATIN[code - LATIN_FIRST]
    return "?"


def discover_faces(input_dir):
    """Return [(name, path, char_w, char_h)] for every atlas in input_dir, smallest first."""
    faces = []
    for path in sorted(glob.glob(os.path.join(input_dir, "*.bmp"))):
        match = re.search(r"(\d+)x(\d+)", os.path.basename(path))
        if not match:
            continue
        char_w, char_h = int(match.group(1)), int(match.group(2))
        faces.append(("%dx%d" % (char_w, char_h), path, char_w, char_h))
    faces.sort(key=lambda f: (f[2] * f[3], f[2]))
    return faces


def atlas_size(path):
    """(width, height) of a BMP, so a sheet can be checked against the real atlas."""
    with open(path, "rb") as f:
        header = f.read(26)
    if header[:2] != b"BM":
        raise ValueError("%s: not a BMP" % path)
    width, height = struct.unpack_from("<ii", header, 18)
    return width, abs(height)


class Sheet(object):
    """A one-bit raster the size of a face atlas, addressed by cell code."""

    def __init__(self, char_w, char_h):
        self.char_w = char_w
        self.char_h = char_h
        self.width = GRID_COLS * char_w
        self.height = 2 * GRID_ROWS * char_h
        self.pixels = [[0] * self.width for _ in range(self.height)]

    def blit(self, code, bitmap):
        """Draw one glyph into the drawn (visual bottom) half at `code`."""
        if not 0 <= code < 256:
            raise ValueError("code 0x%02x out of range" % code)
        left = (code % GRID_COLS) * self.char_w
        top = GRID_ROWS * self.char_h + (code // GRID_COLS) * self.char_h
        for y, row in enumerate(bitmap):
            for x, lit in enumerate(row):
                self.pixels[top + y][left + x] = 1 if lit else 0

    def write_pbm(self, path, comments):
        with open(path, "w") as f:
            f.write("P1\n")
            for line in comments:
                f.write("# %s\n" % line)
            f.write("%d %d\n" % (self.width, self.height))
            for row in self.pixels:
                # P1: 1 is black. Lit pixels are white in the drawn half, hence the flip.
                f.write("".join("0" if px else "1" for px in row))
                f.write("\n")


def ascii_art(bitmap, lit="#", dim="."):
    return ["".join(lit if px else dim for px in row) for row in bitmap]


def art_columns(entries, gap="   "):
    """Lay out [(caption, art_lines)] side by side so a family reads as one strip."""
    if not entries:
        return []
    widths = [max(len(c), max((len(l) for l in a), default=0)) for c, a in entries]
    depth = max(len(a) for _, a in entries)
    lines = [gap.join(c.ljust(w) for (c, _), w in zip(entries, widths))]
    for y in range(depth):
        cells = []
        for (_, art), w in zip(entries, widths):
            cells.append((art[y] if y < len(art) else "").ljust(w))
        lines.append(gap.join(cells))
    return lines


class Log(object):
    """Prints as it goes and keeps a copy for the .log file, like the other tools here."""

    def __init__(self):
        self.lines = []

    def __call__(self, text=""):
        print(text)
        self.lines.append(text)

    def quiet(self, text=""):
        """Recorded in the log file only: the per-glyph rasters are long."""
        self.lines.append(text)

    def write(self, path, argv):
        with open(path, "w") as f:
            f.write("# %s\n" % os.path.basename(path))
            f.write("# generated %s\n" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
            f.write("# %s\n\n" % " ".join(argv))
            for line in self.lines:
                f.write(line + "\n")
        print("wrote %s" % path)


def check_codes(codes, log):
    """Warn about destinations that would collide with something already in the atlas."""
    reserved = figlet_reserved_cells()
    ok = True
    for code in codes:
        if code in UNUSABLE_CODES:
            log("NOTICE: 0x%02x can never appear in a console string" % code)
            ok = False
        elif code in reserved:
            log("NOTICE: 0x%02x is already used by a figlet face (make_figfonts.py GLYPHS)" % code)
            ok = False
    return ok


def levels(steps, extent):
    """Pixel extents for `steps` evenly spaced fill levels, 1..steps of steps+1.

    The step count is fixed across faces so one code means the same fraction everywhere,
    but a small cell cannot resolve them all -- 4x6 has only five rows to give away. The
    duplicates that follow are reported rather than silently collapsed.
    """
    return [max(1, int(round(float(i) * extent / (steps + 1)))) for i in range(1, steps + 1)]
