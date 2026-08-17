#!/usr/bin/env python3
"""Generate the console glyph data from a folder of dual-bank CP437 atlases.

Usage: make_fonts.py [INPUT_DIR] [-o DATA_HEADER] [-d DECL_HEADER]

Source atlases
--------------
Each file in INPUT_DIR (default fonts.altered/) is a 16x32 glyph grid holding two banks
of 256 CP437 glyphs. BMP rows are stored bottom-up (positive biHeight), so the *visual*
layout is:

    visual top half     the glyphs in video-reverse
    visual bottom half  the glyphs as drawn, white on black

For an untouched face the two halves are exact pixel complements. Where they are not,
the drawn half has been hand-edited to carry a custom glyph -- the half-height
connectors the figlet faces smush with -- while the reversed half still shows the
original CP437 shape. That difference is all that is needed to tell the two apart, so
the patched codes are detected by comparison and never hard-coded.

Output layout
-------------
bank 0  the 256 regular glyphs, white on black, recovered by un-reversing the visual
        top half. Emitted as a 16x16 glyph raster, exactly as the engine uploads it.

bank 1  only the custom glyphs, emitted as individual glyph blocks plus the code each
        one belongs to. The console places every patch at its own code index in the
        lower half of the atlas texture. Bank 1 codes without a patch are rendered by
        inverting the bank 0 glyph at draw time, which is what makes the \\x01 bank
        toggle behave like the invert marker it replaced.

A face whose halves are perfect complements simply yields no patches.
"""

import argparse
import datetime
import glob
import os
import re
import struct
import sys


class Log:
    """Echoes the run summary to stdout and keeps it for the log file.

    The generated headers carry no record of which atlas produced them, so the log is
    the only place a later reader can see what the last regeneration actually found --
    in particular which glyphs were detected as hand-edited.
    """

    def __init__(self):
        self.lines = []

    def __call__(self, text=""):
        print(text)
        self.lines.append(text)

    def write(self, path, argv):
        with open(path, "w", encoding="utf-8", newline="\n") as out:
            out.write("# Summary of the last make_fonts.py run -- regenerated, do not edit.\n")
            out.write("# %s\n" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
            out.write("# %s\n\n" % " ".join(argv))
            for line in self.lines:
                out.write(line.rstrip() + "\n")
        print("wrote %s" % path)


GRID_COLS = 16
GRID_ROWS = 16

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_INPUT = os.path.join(HERE, "fonts.altered")
ENGINE_DIR = os.path.normpath(os.path.join(HERE, "..", "..", "..", "scene", "debugconsole"))
DEFAULT_OUT = os.path.join(ENGINE_DIR, "dos_font_data_8.h")
DEFAULT_DECL = os.path.join(ENGINE_DIR, "dos_font_data.h")


def discover_faces(input_dir):
    """Find every atlas in input_dir, deriving the cell size from the file name."""
    faces = []
    for path in sorted(glob.glob(os.path.join(input_dir, "*.bmp"))):
        stem = os.path.splitext(os.path.basename(path))[0]
        match = re.search(r"(\d+)x(\d+)", stem)
        if not match:
            print("skipping %s: no <w>x<h> in the file name" % path)
            continue
        char_w, char_h = int(match.group(1)), int(match.group(2))
        faces.append(("font_%dx%d" % (char_w, char_h), path, char_w, char_h))
    if not faces:
        raise SystemExit("no atlases found in %s" % input_dir)
    faces.sort(key=lambda f: (-f[3], -f[2]))
    return faces


def read_bmp(path):
    """Return (width, height, rows) with rows in visual top-down order, 1 byte/pixel."""
    with open(path, "rb") as f:
        data = f.read()
    if data[:2] != b"BM":
        raise ValueError("%s: not a BMP" % path)
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    bpp = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    if compression != 0:
        raise ValueError("%s: compressed BMPs are not supported" % path)
    if bpp not in (8, 24, 32):
        raise ValueError("%s: unsupported bit depth %d" % (path, bpp))
    step = bpp // 8
    stride = ((width * step + 3) // 4) * 4
    rows = []
    for y in range(abs(height)):
        base = offset + y * stride
        rows.append([data[base + x * step] for x in range(width)])
    if height > 0:  # bottom-up storage
        rows.reverse()
    return width, abs(height), rows


def glyph(rows, char_w, char_h, code, row_offset):
    """Extract one glyph as a tuple of rows of booleans (True = lit pixel)."""
    gr, gc = divmod(code, GRID_COLS)
    top = row_offset + gr * char_h
    left = gc * char_w
    return tuple(tuple(rows[top + y][left + x] > 127 for x in range(char_w)) for y in range(char_h))


def invert(bitmap):
    return tuple(tuple(not px for px in row) for row in bitmap)


def load_face(path, char_w, char_h):
    """Return (regular, patches): the 256 regular glyphs and the hand-edited ones."""
    width, height, rows = read_bmp(path)
    if width != GRID_COLS * char_w or height != 2 * GRID_ROWS * char_h:
        raise ValueError(
            "%s: expected a %dx%d two-bank atlas, got %dx%d"
            % (path, GRID_COLS * char_w, 2 * GRID_ROWS * char_h, width, height)
        )
    half = height // 2

    regular, patches = {}, {}
    for code in range(256):
        reversed_half = glyph(rows, char_w, char_h, code, 0)
        drawn_half = glyph(rows, char_w, char_h, code, half)
        # The reversed half is never hand-edited, so un-reversing it always yields the
        # regular glyph. An untouched cell of the drawn half repeats that glyph, so where
        # the two disagree the cell carries a hand-drawn one.
        regular[code] = invert(reversed_half)
        if drawn_half != regular[code]:
            # Custom glyphs are drawn the way the rest of the sheet reads, black on white,
            # so they come back the same way the reversed half does: by un-reversing them.
            patches[code] = invert(drawn_half)
    return regular, patches


def raster(regular, char_w, char_h):
    """Flatten the 256 glyphs into a 16x16 glyph raster, one byte per pixel."""
    out = bytearray()
    for grow in range(GRID_ROWS):
        for y in range(char_h):
            for gcol in range(GRID_COLS):
                line = regular[grow * GRID_COLS + gcol][y]
                out.extend(0xFF if px else 0x00 for px in line)
    return bytes(out)


def emit_array(handle, name, payload, per_line=16):
    if not payload:
        # A zero-length array is not valid C; the patch count guards every read.
        handle.write("const unsigned char %s[1] = { 0x00 };\n\n" % name)
        return
    handle.write("const unsigned char %s[] = {\n" % name)
    for start in range(0, len(payload), per_line):
        chunk = payload[start : start + per_line]
        handle.write("\t" + " ".join("0x%02x," % b for b in chunk) + "\n")
    handle.write("};\n\n")


def licence_header(out_path):
    """Reuse the licence block already present in the generated file, if any."""
    if not os.path.exists(out_path):
        return ""
    with open(out_path, "r", encoding="latin-1") as f:
        text = f.read()
    marker = "/**************************************************************************/"
    end = text.rfind(marker)
    return text[: end + len(marker)] + "\n\n" if end != -1 else ""


def write_declarations(path, faces):
    """Emit the companion header declaring everything in the generated data file."""
    with open(path, "w", encoding="latin-1") as out:
        out.write(licence_header(path))
        out.write(
            "/* Generated by modules/gdextensions/_tools/make_fonts.py -- do not edit.\n"
            "   Declarations for the glyph data in dos_font_data_8.h.\n\n"
            "   font_<w>x<h>              bank 0: a %d x %d glyph raster of the regular CP437\n"
            "                             glyphs, one byte per pixel (0x00 or 0xff).\n"
            "   font_<w>x<h>_patch        bank 1: FONT_<W>X<H>_PATCH_COUNT glyph blocks of\n"
            "                             w * h bytes, in the order of the code table below.\n"
            "   font_<w>x<h>_patch_codes  the CP437 code each patch block belongs to.\n\n"
            "   Bank 1 codes without a patch are rendered by inverting the bank 0 glyph at\n"
            "   draw time. A face with no hand-edited glyphs has a patch count of 0. */\n\n" % (GRID_COLS, GRID_ROWS)
        )
        out.write("#ifndef DOS_FONT_DATA_H\n#define DOS_FONT_DATA_H\n\n")
        for name, _, _, _, _, patches in faces:
            codes = sorted(patches)
            out.write(
                "#define %s_PATCH_COUNT %d%s\n"
                % (
                    name.upper(),
                    len(codes),
                    (" /* %s */" % " ".join("0x%02x" % c for c in codes)) if codes else "",
                )
            )
        out.write('\n#ifdef __cplusplus\nextern "C" {\n#endif\n\n')
        for name, _, char_w, char_h, _, _ in faces:
            out.write(
                "extern const unsigned char %s[], %s_patch[], %s_patch_codes[]; /* %dx%d */\n"
                % (name, name, name, char_w, char_h)
            )
        out.write("\n#ifdef __cplusplus\n}\n#endif\n\n#endif // DOS_FONT_DATA_H\n")


def report_patch_consistency(faces, log):
    """Warn when the faces do not carry the same set of hand-edited glyphs.

    The console picks a face from the panel geometry, so a patch that exists only in
    some atlases makes the very same log line render differently -- or fall back to the
    inverted bank 0 glyph -- depending on the screen it lands on. That is a mistake in
    the atlases rather than in this tool, so it is reported and not corrected here.
    """
    if len(faces) < 2:
        return True
    sets = {name: frozenset(patches) for name, _, _, _, _, patches in faces}
    union = frozenset().union(*sets.values())
    if len(set(sets.values())) == 1:
        log("all %d faces patch the same %d glyph(s)" % (len(faces), len(union)))
        return True

    log("")
    log("NOTICE: the atlases do not patch the same glyphs.")
    log("        union of all patched codes: %s" % " ".join("0x%02x" % c for c in sorted(union)))
    for name, _, _, _, _, patches in faces:
        missing = sorted(union - set(patches))
        if missing:
            log(
                "        %-10s %d/%d patched, missing %s"
                % (name, len(patches), len(union), " ".join("0x%02x" % c for c in missing))
            )
    log("        Draw the missing glyphs in the corresponding fonts.altered/*.bmp")
    log("        (bottom/drawn half only) and re-run this tool.")
    log("")
    return False


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", nargs="?", default=DEFAULT_INPUT, help="folder holding the atlases")
    parser.add_argument("-o", "--output", default=DEFAULT_OUT)
    parser.add_argument("-d", "--declarations", default=DEFAULT_DECL)
    parser.add_argument(
        "-l", "--log", default=os.path.join(HERE, "make_fonts.log"), help="where to write the summary of this run"
    )
    args = parser.parse_args()

    log = Log()
    faces = []
    for name, path, char_w, char_h in discover_faces(args.input):
        regular, patches = load_face(path, char_w, char_h)
        faces.append((name, path, char_w, char_h, regular, patches))
        log(
            "%-10s %-22s custom glyphs: %s"
            % (
                name,
                os.path.basename(path),
                " ".join("0x%02x" % c for c in sorted(patches)) or "none",
            )
        )

    consistent = report_patch_consistency(faces, log)

    with open(args.output, "w", encoding="latin-1") as out:
        out.write(licence_header(args.output))
        out.write(
            "/* Generated by modules/gdextensions/_tools/make_fonts.py from %s/ -- do not\n"
            "   edit. Bank 0 holds the 256 regular CP437 glyphs; bank 1 holds only the\n"
            "   hand-edited glyphs listed in the *_patch_codes tables. See\n"
            "   scene/debugconsole/CONSOLE.md section 7. */\n\n" % os.path.basename(os.path.normpath(args.input))
        )
        for name, path, char_w, char_h, regular, patches in faces:
            codes = sorted(patches)
            out.write('/* %s: %dx%d glyphs from "%s". */\n' % (name, char_w, char_h, os.path.basename(path)))
            emit_array(out, name, raster(regular, char_w, char_h))
            blocks = bytearray()
            for code in codes:
                for row in patches[code]:
                    blocks.extend(0xFF if px else 0x00 for px in row)
            emit_array(out, name + "_patch", bytes(blocks))
            emit_array(out, name + "_patch_codes", bytes(codes))

    total = sum(GRID_COLS * cw * GRID_ROWS * ch for _, _, cw, ch, _, _ in faces)
    log("wrote %s (%d bytes of bank 0 glyph data)" % (args.output, total))

    write_declarations(args.declarations, faces)
    log("wrote %s" % args.declarations)

    log.write(args.log, sys.argv)

    # Non-zero on an inconsistent set so a build step can notice, while still emitting
    # usable data: the console copes with a missing patch, it just looks different.
    return 0 if consistent else 1


if __name__ == "__main__":
    sys.exit(main())
