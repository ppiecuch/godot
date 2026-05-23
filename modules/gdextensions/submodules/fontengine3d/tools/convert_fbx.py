#!/usr/bin/env python3
"""
Convert FBX font assets to Godot ArrayMesh .tres resources with textures.

Usage:
    ./convert_fbx.py <source_dir> <dest_dir> [options]
    ./convert_fbx.py data_512/fonts_neon/models data_tres/neon
    ./convert_fbx.py data_512 data_tres --all

Options:
    --all           Process all font subdirectories (fonts, fonts_art, fonts_neon, fonts_toon)
    --skip-obj      Don't keep .obj files (mesh .tres only)
    --flatten       Flatten output to single directory (ignore subdirectory structure)
    --normalize     Normalize output filenames to uppercase single-char (A.tres, B.tres, 0.tres)
    --with-textures Copy referenced textures and generate SpatialMaterial .tres files
"""

import logging
import os
import re
import shutil
import sys
import subprocess
import time
from datetime import datetime
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
SUBMODULE_DIR = SCRIPT_DIR.parent
FBX2OBJ = (SUBMODULE_DIR / "../../thirdparty/bin/fbx2obj").resolve()
GODOTPCKTOOL = (SUBMODULE_DIR / "../../../../misc/tools/godotpcktool").resolve()

ANIMATION_KEYWORDS = ("animation", "animations", "anim", "anims")

# Maps FBX stem names to safe filesystem names (not raw characters).
# Characters that are invalid in filenames use descriptive names.
SPECIAL_CHAR_MAP = {
    "exclamation": "excl",
    "interest": "ques",
    "minus": "minus",
    "plus": "plus",
    "slash": "slash",
    "asterisk": "star",
    "question": "ques",
    "dot": "dot",
    "comma": "comma",
    "correct": "check",
    "division": "slash",
    "equal": "equal",
    "greaterminor": "gtlt",
    "parenthesesclose": "rparen",
    "parenthesesopen": "lparen",
    "percentage": "pct",
    "semicolon": "semi",
}

# Common texture directory names (searched relative to FBX location)
TEXTURE_SEARCH_DIRS = [
    "textures",
    "Texture",
    "Textures",
    "../textures",
    "../Texture",
    "../Textures",
    "../../textures",
    "../../Texture",
]

# PBR channel keywords → SpatialMaterial property map
TEXTURE_CHANNELS = {
    "albedo": "albedo_texture",
    "diffuse": "albedo_texture",
    "base": "albedo_texture",
    "normal": "normal_texture",
    "emissive": "emission_texture",
    "emission": "emission_texture",
    "metallic": "metallic_texture",
    "roughness": "roughness_texture",
    "ao": "ao_texture",
    "ambientocclusion": "ao_texture",
    "occlusion": "ao_texture",
}

log = logging.getLogger("convert_fbx")


def setup_logging(dst_dir: Path):
    dst_dir.mkdir(parents=True, exist_ok=True)
    log_file = dst_dir / f"convert_{datetime.now():%Y%m%d_%H%M%S}.log"

    fmt = logging.Formatter("%(asctime)s  %(levelname)-5s  %(message)s", datefmt="%H:%M:%S")

    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.INFO)
    ch.setFormatter(fmt)

    fh = logging.FileHandler(log_file, encoding="utf-8")
    fh.setLevel(logging.DEBUG)
    fh.setFormatter(fmt)

    log.setLevel(logging.DEBUG)
    log.addHandler(ch)
    log.addHandler(fh)

    log.info("Command: %s", " ".join(sys.argv))
    log.info("Log file: %s", log_file)
    return log_file


def is_animation(rel_path: Path) -> bool:
    return any(part.lower() in ANIMATION_KEYWORDS for part in rel_path.parts)


def find_fbx_files(src: Path) -> list[Path]:
    return sorted(src.rglob("*.[fF][bB][xX]"))


def normalize_filename(fbx_path: Path) -> str:
    """Extract the character name from FBX filename and normalize to single char."""
    stem = fbx_path.stem
    # Strip common suffixes: _Model, _static, _Static
    stem = re.sub(r"[_\s]*(Model|static|Static)$", "", stem)

    if len(stem) == 1 and stem.isalnum():
        return stem.upper()

    key = stem.lower()
    if key in SPECIAL_CHAR_MAP:
        return SPECIAL_CHAR_MAP[key]

    if re.match(r"^Font\d+$", stem, re.IGNORECASE):
        return stem

    return stem


def run_fbx2obj(cmd: list[str], rel: Path) -> bool:
    log.debug("cmd: %s", " ".join(cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)

    for line in (result.stdout or "").strip().splitlines():
        log.debug("  stdout: %s", line)
    for line in (result.stderr or "").strip().splitlines():
        if result.returncode == 0:
            log.debug("  %s", line)
        else:
            log.error("  %s", line)

    if result.returncode != 0:
        log.error("FAIL  %s  (exit code %d)", rel, result.returncode)
        return False
    return True


def _find_texture_by_name(filename: str, search_root: Path):
    """Search for a texture file by stem name in a directory tree."""
    stem = Path(filename).stem
    for img in search_root.rglob("*"):
        if img.is_file() and img.stem == stem and img.suffix.lower() in (".png", ".jpg", ".tga", ".tif", ".exr"):
            return img
    return None


def parse_mtl_textures(mtl_path: Path, fbx_dir: Path, src_root: Path = None) -> dict[str, Path]:
    """Parse .mtl file and resolve texture paths relative to source FBX directory.
    Falls back to searching parent directories if relative paths don't resolve."""
    textures = {}
    if not mtl_path.exists():
        return textures

    # Build search roots: fbx_dir, then up to 4 parent levels, then src_root
    search_dirs = []
    d = fbx_dir.resolve()
    for _ in range(5):
        if d not in search_dirs:
            search_dirs.append(d)
        if d.parent == d:
            break
        d = d.parent
    if src_root and src_root.resolve() not in search_dirs:
        search_dirs.append(src_root.resolve())

    with open(mtl_path, "r") as f:
        for line in f:
            line = line.strip()
            for prefix, channel in [
                ("map_Kd", "albedo"),
                ("map_Ks", "metallic"),
                ("map_Bump", "normal"),
                ("bump", "normal"),
                ("map_Ka", "ao"),
            ]:
                if line.lower().startswith(prefix.lower()):
                    tex_rel = line[len(prefix):].strip()
                    if not tex_rel:
                        continue

                    # Try the literal path from FBX dir
                    tex_path = (fbx_dir / tex_rel).resolve()
                    if tex_path.exists():
                        textures[channel] = tex_path
                        continue

                    # Try alternate extensions at the literal path
                    found = False
                    for ext in [".png", ".jpg", ".tga", ".tif"]:
                        alt = tex_path.with_suffix(ext)
                        if alt.exists():
                            textures[channel] = alt
                            found = True
                            break
                    if found:
                        continue

                    # Search by filename in parent directories
                    tex_name = Path(tex_rel).name
                    for sd in search_dirs:
                        result = _find_texture_by_name(tex_name, sd)
                        if result:
                            textures[channel] = result
                            log.debug("  texture found via search: %s → %s", tex_rel, result)
                            found = True
                            break
                    if not found:
                        log.debug("  texture not found: %s (from %s)", tex_rel, mtl_path.name)
    return textures


def find_shared_textures(fbx_path: Path) -> dict[str, Path]:
    """Search for shared textures in standard texture directories near the FBX."""
    textures = {}
    fbx_dir = fbx_path.parent

    for search_dir in TEXTURE_SEARCH_DIRS:
        tex_dir = (fbx_dir / search_dir).resolve()
        if not tex_dir.is_dir():
            continue

        # Check for PBR-organized subdirectories (Albedo/, Normal/, etc.)
        for subdir in tex_dir.iterdir():
            if subdir.is_dir():
                channel_key = subdir.name.lower()
                if channel_key in TEXTURE_CHANNELS:
                    # Find first image in this channel directory
                    for img in sorted(subdir.iterdir()):
                        if img.suffix.lower() in (".png", ".jpg", ".tga", ".tif", ".exr"):
                            prop = TEXTURE_CHANNELS[channel_key]
                            if prop not in textures:
                                textures[prop] = img
                            break

        # Check for textures in flat directory (named with channel keywords)
        for img in sorted(tex_dir.iterdir()):
            if not img.is_file():
                continue
            if img.suffix.lower() not in (".png", ".jpg", ".tga", ".tif", ".exr"):
                continue
            name_lower = img.stem.lower()
            for keyword, prop in TEXTURE_CHANNELS.items():
                if keyword in name_lower and prop not in textures:
                    textures[prop] = img
                    break

        if textures:
            break

    return textures


def copy_textures(textures: dict[str, Path], dst_dir: Path) -> dict[str, str]:
    """Copy texture files to output dir. Returns map of channel → relative filename."""
    copied = {}
    tex_out = dst_dir / "textures"
    tex_out.mkdir(parents=True, exist_ok=True)

    for channel, src_path in textures.items():
        dst_path = tex_out / src_path.name
        if not dst_path.exists():
            shutil.copy2(src_path, dst_path)
            log.debug("  copied texture: %s → %s", src_path.name, channel)
        copied[channel] = f"textures/{src_path.name}"
    return copied


def generate_material_tres(name: str, texture_refs: dict[str, str], dst_dir: Path):
    """Generate a Godot SpatialMaterial .tres file referencing textures."""
    if not texture_refs:
        return

    mat_path = dst_dir / f"{name}_material.tres"
    if mat_path.exists():
        return

    # Build ext_resource and material property sections
    ext_resources = []
    properties = []
    res_id = 1

    channel_to_param = {
        "albedo_texture": ("albedo", "texture"),
        "normal_texture": ("normal", "texture"),
        "emission_texture": ("emission", "texture"),
        "metallic_texture": ("metallic", "texture"),
        "roughness_texture": ("roughness", "texture"),
        "ao_texture": ("ao", "texture"),
    }

    for channel, rel_path in sorted(texture_refs.items()):
        ext_resources.append(
            f'[ext_resource path="{rel_path}" type="Texture" id={res_id}]'
        )
        if channel in channel_to_param:
            cat, prop = channel_to_param[channel]
            if cat == "albedo":
                properties.append(f'{cat}_{prop} = ExtResource( {res_id} )')
            elif cat == "normal":
                properties.append(f'normal_enabled = true')
                properties.append(f'normal_{prop} = ExtResource( {res_id} )')
            elif cat == "emission":
                properties.append(f'emission_enabled = true')
                properties.append(f'emission_{prop} = ExtResource( {res_id} )')
            elif cat == "metallic":
                properties.append(f'metallic_{prop} = ExtResource( {res_id} )')
            elif cat == "ao":
                properties.append(f'ao_enabled = true')
                properties.append(f'ao_{prop} = ExtResource( {res_id} )')
        res_id += 1

    content = '[gd_resource type="SpatialMaterial" load_steps=%d format=2]\n\n' % (len(ext_resources) + 1)
    for er in ext_resources:
        content += er + "\n"
    content += "\n[resource]\n"
    for p in properties:
        content += p + "\n"

    with open(mat_path, "w") as f:
        f.write(content)
    log.info("  material: %s (%d textures)", mat_path.name, len(texture_refs))


def convert(src_dir: Path, dst_dir: Path, skip_obj: bool = False,
            flatten: bool = False, normalize: bool = False, with_textures: bool = False):
    fbx_files = find_fbx_files(src_dir)
    if not fbx_files:
        log.warning("No FBX files found in %s", src_dir)
        return

    log.info("Source:  %s", src_dir)
    log.info("Output:  %s", dst_dir)
    log.info("Found %d FBX file(s)", len(fbx_files))
    log.info("Options: skip_obj=%s flatten=%s normalize=%s with_textures=%s",
             skip_obj, flatten, normalize, with_textures)
    log.info("")

    converted = 0
    skipped = 0
    failed = 0
    textures_found = False
    t_start = time.monotonic()

    for i, fbx in enumerate(fbx_files, 1):
        rel = fbx.relative_to(src_dir)

        if flatten:
            out_dir = dst_dir
        else:
            out_dir = dst_dir / rel.parent
        out_dir.mkdir(parents=True, exist_ok=True)

        if normalize:
            name = normalize_filename(fbx)
        else:
            name = fbx.stem

        progress = f"[{i}/{len(fbx_files)}]"

        if is_animation(rel):
            out_file = out_dir / f"{name}.tres"
            if out_file.exists():
                log.info("%s  SKIP  %s  (exists)", progress, rel)
                skipped += 1
                continue
            log.info("%s  ANIM  %s → %s", progress, rel, name)
            cmd = [str(FBX2OBJ), "-a", "-o", f"{out_dir}/", str(fbx)]
        else:
            out_tres = out_dir / f"{name}.tres"

            if out_tres.exists():
                log.info("%s  SKIP  %s  (exists)", progress, rel)
                skipped += 1
                continue

            log.info("%s  MESH  %s → %s.tres", progress, rel, name)
            cmd = [str(FBX2OBJ), "-t", "-m", "-o", f"{out_dir}/", str(fbx)]

        if run_fbx2obj(cmd, rel):
            # Rename output to normalized name if different from input stem
            if normalize and name != fbx.stem:
                for ext in (".tres", ".obj", ".mtl"):
                    orig = out_dir / f"{fbx.stem}{ext}"
                    dest = out_dir / f"{name}{ext}"
                    if orig.exists() and not dest.exists():
                        orig.rename(dest)
                        log.debug("  renamed %s → %s", orig.name, dest.name)
                    elif orig.exists() and dest.exists() and orig != dest:
                        orig.unlink()

            # Handle textures
            if with_textures and not is_animation(rel):
                mtl_path = out_dir / f"{name}.mtl"
                texture_refs = {}

                # First try: parse .mtl for per-mesh texture references
                if mtl_path.exists():
                    mtl_textures = parse_mtl_textures(mtl_path, fbx.parent, src_dir)
                    if mtl_textures:
                        # Convert channel names to property names
                        for ch, path in mtl_textures.items():
                            prop = TEXTURE_CHANNELS.get(ch, f"{ch}_texture")
                            texture_refs[prop] = path

                # Second try: search standard texture directories
                if not texture_refs:
                    texture_refs = find_shared_textures(fbx)

                if texture_refs:
                    textures_found = True
                    # Copy textures to output and get relative paths
                    copied = copy_textures(texture_refs, out_dir)
                    # Generate material .tres
                    generate_material_tres(name, copied, out_dir)

            converted += 1
        else:
            failed += 1

    # Remove .obj and .mtl files if skip_obj requested
    if skip_obj:
        for pattern in ("*.obj", "*.mtl"):
            for f in dst_dir.rglob(pattern):
                f.unlink()
                log.debug("  removed %s", f.name)

    elapsed = time.monotonic() - t_start
    log.info("")
    log.info("=" * 50)
    log.info("Converted: %d  |  Skipped: %d  |  Failed: %d", converted, skipped, failed)
    if with_textures:
        log.info("Textures:  %s", "found and copied" if textures_found else "none found")
    log.info("Total time: %.1fs", elapsed)
    log.info("=" * 50)

    if failed:
        log.warning("Some files failed — check the log for details.")


def generate_catalog(dst_dir: Path, res_prefix: str = "fontengine3d"):
    """Generate a catalog.json listing all converted font styles and their glyphs.

    The catalog is packed into the .pck alongside the meshes, so the C++ code
    can discover available fonts at runtime instead of hardcoding them.

    Format:
    {
        "fonts": {
            "neon": {
                "glyphs": ["A", "B", ...],
                "has_materials": true,
                "textures": ["albedo", "normal", ...],
                "path": "fontengine3d/fonts_neon/models"
            },
            ...
        }
    }
    """
    import json

    catalog = {"fonts": {}}

    # Scan dst_dir for font style subdirectories
    # Could be: fonts_neon/models/*.tres, fonts_art/80s style/Fbx/Low/*.tres, etc.
    # Or flat: just *.tres in dst_dir directly

    def scan_font_dir(base_dir: Path, style_name: str, rel_base: str):
        """Scan a directory for glyph .tres files and build/merge a font entry."""
        mesh_files = sorted(
            f for f in base_dir.rglob("*.tres")
            if not f.stem.endswith("_material") and not f.stem.startswith("convert_")
        )
        if not mesh_files:
            return

        glyphs = [mf.stem for mf in mesh_files]

        has_materials = any(
            (mf.parent / f"{mf.stem}_material.tres").exists() for mf in mesh_files
        )

        texture_channels = []
        tex_dir = base_dir / "textures" if (base_dir / "textures").is_dir() else None
        if tex_dir is None:
            for sub in base_dir.rglob("textures"):
                if sub.is_dir():
                    tex_dir = sub
                    break
        if tex_dir:
            for img in sorted(tex_dir.iterdir()):
                if img.is_file() and img.suffix.lower() in (".png", ".jpg", ".tga"):
                    name_lower = img.stem.lower()
                    for kw in ("albedo", "normal", "emissive", "emission", "metallic", "ao", "roughness"):
                        if kw in name_lower:
                            texture_channels.append(kw)
                            break

        first_mesh_rel = mesh_files[0].parent.relative_to(dst_dir)
        res_path = f"{res_prefix}/{first_mesh_rel}"

        # Merge into existing entry if same style_name (e.g. multiple subdirs for one font)
        if style_name in catalog["fonts"]:
            existing = catalog["fonts"][style_name]
            for g in glyphs:
                if g not in existing["glyphs"]:
                    existing["glyphs"].append(g)
            existing["glyph_count"] = len(existing["glyphs"])
            existing["has_materials"] = existing["has_materials"] or has_materials
            for tc in texture_channels:
                if tc not in existing["textures"]:
                    existing["textures"].append(tc)
        else:
            catalog["fonts"][style_name] = {
                "glyphs": glyphs,
                "glyph_count": len(glyphs),
                "has_materials": has_materials,
                "textures": texture_channels,
                "path": res_path,
            }

    # Directories that are structural (not font names)
    SKIP_DIRS = {"models", "fbx", "hi", "med", "low", "hi_med_low",
                 "element", "textures", "statics", "inplace"}

    def make_font_name(dir_path: Path, category_dir: Path) -> str:
        """Derive a clean font name from a directory path.

        Rules:
        - Strip 'fonts_' prefix from category dirs (fonts_neon → Neon)
        - Skip structural dirs (models, Fbx, Hi/Med/Low)
        - Capitalize style names
        - Join remaining meaningful parts with '/'
        """
        rel = dir_path.relative_to(category_dir)
        parts = list(rel.parts)

        # Filter out structural directory names
        meaningful = [p for p in parts if p.lower() not in SKIP_DIRS]

        if not meaningful:
            # All parts were structural — use the category name
            cat_name = category_dir.name
            if cat_name.startswith("fonts_"):
                cat_name = cat_name[6:]
            return cat_name.replace("_", " ").title().replace(" ", "")

        return "/".join(meaningful)

    def derive_category_name(dir_name: str) -> str:
        """Strip fonts_ prefix: fonts_neon → Neon, fonts_art → Art, fonts_toon → Toon."""
        if dir_name.startswith("fonts_"):
            return dir_name[6:].replace("_", " ").title().replace(" ", "")
        return dir_name

    def scan_category(category_dir: Path, category_name: str):
        """Scan a font category directory for all font variants."""
        dirs_with_meshes = set()
        for tres in category_dir.rglob("*.tres"):
            if not tres.stem.endswith("_material") and not tres.stem.startswith("convert_"):
                dirs_with_meshes.add(tres.parent)

        for mesh_dir in sorted(dirs_with_meshes):
            font_name = make_font_name(mesh_dir, category_dir)
            if len(dirs_with_meshes) > 1:
                if font_name == category_name:
                    style_key = category_name
                else:
                    style_key = f"{category_name}/{font_name}"
            else:
                style_key = font_name
            scan_font_dir(mesh_dir, style_key, category_dir.name)

    # Determine if dst_dir is a multi-category root (has fonts_* subdirs)
    # or a single category output (e.g. just fonts_neon output)
    subdirs = [d for d in sorted(dst_dir.iterdir()) if d.is_dir()]
    has_category_dirs = any(d.name.startswith("fonts_") for d in subdirs)

    if has_category_dirs:
        # Multi-category: dst_dir contains fonts_neon/, fonts_art/, etc.
        for sub in subdirs:
            category = derive_category_name(sub.name)
            scan_category(sub, category)
    else:
        # Single category: dst_dir IS the font output (e.g. data_tres/fonts_neon/)
        # Derive category from dst_dir name or its parent
        cat_name = dst_dir.name
        if cat_name.startswith("fonts_"):
            cat_name = cat_name[6:]
        category = cat_name.replace("_", " ").title().replace(" ", "")
        scan_category(dst_dir, category)

    # Also check if dst_dir itself contains .tres files (flat mode)
    flat_tres = [f for f in dst_dir.glob("*.tres") if not f.stem.endswith("_material")]
    if flat_tres and not catalog["fonts"]:
        scan_font_dir(dst_dir, "default", "")

    if not catalog["fonts"]:
        log.warning("No fonts found for catalog in %s", dst_dir)
        return None

    catalog_path = dst_dir / "catalog.json"
    with open(catalog_path, "w") as f:
        json.dump(catalog, f, indent=2)

    log.info("Catalog: %s (%d font styles, %d total glyphs)",
             catalog_path.name,
             len(catalog["fonts"]),
             sum(e["glyph_count"] for e in catalog["fonts"].values()))
    return catalog_path


def pack_to_pck(dst_dir: Path, pck_path: Path, res_prefix: str = "fontengine3d"):
    """Pack all .tres and image files from dst_dir into a Godot .pck file.

    The res:// structure will be: res://<res_prefix>/<relative_path>
    e.g. res://fontengine3d/fonts_neon/A.tres
    """
    if not GODOTPCKTOOL.exists():
        log.error("godotpcktool not found at '%s'", GODOTPCKTOOL)
        return False

    # Collect all packable files
    extensions = {".tres", ".png", ".jpg", ".tga", ".tif", ".exr", ".json"}
    files = sorted(
        f for f in dst_dir.rglob("*")
        if f.is_file() and f.suffix.lower() in extensions
    )

    # Exclude log files
    files = [f for f in files if not f.stem.startswith("convert_")]

    if not files:
        log.warning("No files to pack in %s", dst_dir)
        return False

    log.info("")
    log.info("--- Packing to %s ---", pck_path.name)
    log.info("Files: %d", len(files))

    # godotpcktool --remove-prefix strips the local path portion,
    # leaving the res:// path as: res://<whatever comes after the prefix>
    # We want: res://fontengine3d/fonts_neon/A.tres
    # So remove-prefix should strip everything up to (but not including) the style dirs.
    #
    # Strategy: create a temporary symlink/structure, or use --remove-prefix
    # with the dst_dir parent and let the dst_dir name become part of the path.
    #
    # Simplest: remove dst_dir from paths, prepend res_prefix.
    # godotpcktool strips --remove-prefix from the absolute path, so:
    #   file: /abs/path/to/data_256_tres/fonts_neon/A.tres
    #   --remove-prefix /abs/path/to/data_256_tres
    #   → res://fonts_neon/A.tres
    # Then we want res://fontengine3d/fonts_neon/A.tres
    # But godotpcktool doesn't support adding a prefix — only removing.
    #
    # Workaround: create a temp directory with the prefix as a subdirectory,
    # symlink the output there, pack from the temp dir.

    import tempfile
    with tempfile.TemporaryDirectory() as tmpdir:
        link_base = Path(tmpdir) / res_prefix
        link_base.symlink_to(dst_dir)

        # Rebuild file list relative to tmpdir
        tmp_files = [link_base / f.relative_to(dst_dir) for f in files]

        # Remove the pck if it already exists (godotpcktool appends to existing)
        if pck_path.exists():
            pck_path.unlink()

        cmd = [
            str(GODOTPCKTOOL),
            "-p", str(pck_path),
            "-a", "a",
            "--remove-prefix", tmpdir,
        ] + [str(f) for f in tmp_files]

        log.debug("cmd: godotpcktool -p %s -a a --remove-prefix ... (%d files)", pck_path, len(tmp_files))

        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode != 0:
            log.error("godotpcktool failed (exit %d):", result.returncode)
            for line in (result.stderr or result.stdout or "").strip().splitlines():
                log.error("  %s", line)
            return False

    pck_size = pck_path.stat().st_size
    log.info("Packed: %s (%.1f MB, %d files)", pck_path.name, pck_size / 1024 / 1024, len(files))
    return True


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description="Convert FBX font assets to Godot ArrayMesh .tres resources."
    )
    parser.add_argument("source", help="Source directory with FBX files")
    parser.add_argument("dest", help="Output directory for .tres files")
    parser.add_argument("--all", action="store_true",
                        help="Process all font subdirectories")
    parser.add_argument("--skip-obj", action="store_true",
                        help="Remove .obj/.mtl files after conversion (keep .tres only)")
    parser.add_argument("--flatten", action="store_true",
                        help="Flatten output to single directory")
    parser.add_argument("--normalize", action="store_true",
                        help="Normalize filenames to single uppercase char (A.tres, 0.tres)")
    parser.add_argument("--with-textures", action="store_true",
                        help="Copy textures and generate SpatialMaterial .tres files")
    parser.add_argument("--pack", metavar="FILE",
                        help="Pack output into a Godot .pck file (e.g. font3d.pck)")
    parser.add_argument("--res-prefix", default="fontengine3d",
                        help="Resource prefix in the .pck (default: fontengine3d)")

    args = parser.parse_args()

    src_dir = Path(args.source)
    dst_dir = Path(args.dest)

    if not src_dir.is_absolute():
        src_dir = SUBMODULE_DIR / src_dir
    if not dst_dir.is_absolute():
        dst_dir = SUBMODULE_DIR / dst_dir

    if not src_dir.is_dir():
        print(f"Error: source directory '{src_dir}' not found.")
        sys.exit(1)

    if not FBX2OBJ.exists():
        print(f"Error: fbx2obj not found at '{FBX2OBJ}'")
        sys.exit(1)

    setup_logging(dst_dir)

    if args.all:
        subdirs = ["fonts", "fonts_art", "fonts_neon", "fonts_toon"]
        for sub in subdirs:
            sub_src = src_dir / sub
            if sub_src.is_dir():
                sub_dst = dst_dir / sub
                log.info("--- Processing %s ---", sub)
                convert(sub_src, sub_dst, args.skip_obj, args.flatten,
                        args.normalize, args.with_textures)
            else:
                log.info("--- Skipping %s (not found) ---", sub)
    else:
        convert(src_dir, dst_dir, args.skip_obj, args.flatten,
                args.normalize, args.with_textures)

    if args.pack:
        pck_path = Path(args.pack)
        if not pck_path.is_absolute():
            pck_path = SUBMODULE_DIR / pck_path
        generate_catalog(dst_dir, args.res_prefix)
        pack_to_pck(dst_dir, pck_path, args.res_prefix)


if __name__ == "__main__":
    main()
