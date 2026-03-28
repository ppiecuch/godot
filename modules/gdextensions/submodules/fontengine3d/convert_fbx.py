#!/usr/bin/env python3
"""
Convert all FBX files in a source folder to OBJ (+MTL) and animation
FBX files to Godot .tres resources, preserving folder structure in a
separate output directory.

Usage:  ./convert_fbx.py <source_dir> <dest_dir>
        ./convert_fbx.py data_512 data_512_out
"""

import logging
import sys
import subprocess
import time
from datetime import datetime
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
FBX2OBJ = (SCRIPT_DIR / "../../thirdparty/bin/fbx2obj").resolve()

ANIMATION_KEYWORDS = ("animation", "animations", "anim", "anims")

log = logging.getLogger("convert_fbx")


def setup_logging(dst_dir: Path):
    """Configure logging to both console and a log file in the output dir."""
    dst_dir.mkdir(parents=True, exist_ok=True)
    log_file = dst_dir / f"convert_{datetime.now():%Y%m%d_%H%M%S}.log"

    fmt = logging.Formatter("%(asctime)s  %(levelname)-5s  %(message)s",
                            datefmt="%H:%M:%S")

    # Console: concise, INFO+
    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.INFO)
    ch.setFormatter(fmt)

    # File: verbose, DEBUG+  (includes fbx2obj stdout/stderr)
    fh = logging.FileHandler(log_file, encoding="utf-8")
    fh.setLevel(logging.DEBUG)
    fh.setFormatter(fmt)

    log.setLevel(logging.DEBUG)
    log.addHandler(ch)
    log.addHandler(fh)

    log.info("Log file: %s", log_file)
    return log_file


def is_animation(rel_path: Path) -> bool:
    """Check if a file lives under an animation directory."""
    return any(part.lower() in ANIMATION_KEYWORDS for part in rel_path.parts)


def find_fbx_files(src: Path) -> list[Path]:
    """Recursively find all .fbx files (case-insensitive), sorted."""
    return sorted(src.rglob("*.[fF][bB][xX]"))


def run_fbx2obj(cmd: list[str], rel: Path) -> bool:
    """Run fbx2obj, log output, return True on success."""
    log.debug("cmd: %s", " ".join(cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)

    for line in (result.stdout or "").strip().splitlines():
        log.debug("  stdout: %s", line)
    for line in (result.stderr or "").strip().splitlines():
        # fbx2obj writes normal progress to stderr
        if result.returncode == 0:
            log.debug("  %s", line)
        else:
            log.error("  %s", line)

    if result.returncode != 0:
        log.error("FAIL  %s  (exit code %d)", rel, result.returncode)
        return False
    return True


def convert(src_dir: Path, dst_dir: Path):
    fbx_files = find_fbx_files(src_dir)
    if not fbx_files:
        log.warning("No FBX files found in %s", src_dir)
        return

    log.info("Source:  %s", src_dir)
    log.info("Output:  %s", dst_dir)
    log.info("Found %d FBX file(s)", len(fbx_files))
    log.info("")

    converted = 0
    skipped = 0
    failed = 0
    t_start = time.monotonic()

    for i, fbx in enumerate(fbx_files, 1):
        rel = fbx.relative_to(src_dir)
        out_dir = dst_dir / rel.parent
        out_dir.mkdir(parents=True, exist_ok=True)
        name = fbx.stem

        progress = f"[{i}/{len(fbx_files)}]"

        if is_animation(rel):
            out_file = out_dir / f"{name}.tres"
            if out_file.exists():
                log.info("%s  SKIP  %s  (exists)", progress, rel)
                skipped += 1
                continue
            log.info("%s  ANIM  %s", progress, rel)
            cmd = [str(FBX2OBJ), "-a", "-o", f"{out_dir}/", str(fbx)]
        else:
            out_file = out_dir / f"{name}.obj"
            if out_file.exists():
                log.info("%s  SKIP  %s  (exists)", progress, rel)
                skipped += 1
                continue
            log.info("%s  OBJ   %s", progress, rel)
            cmd = [str(FBX2OBJ), "-t", "-o", f"{out_dir}/", str(fbx)]

        if run_fbx2obj(cmd, rel):
            converted += 1
        else:
            failed += 1

    elapsed = time.monotonic() - t_start
    log.info("")
    log.info("=" * 50)
    log.info("Converted: %d  |  Skipped: %d  |  Failed: %d", converted, skipped, failed)
    log.info("Total time: %.1fs", elapsed)
    log.info("=" * 50)

    if failed:
        log.warning("Some files failed — check the log for details.")


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <source_dir> <dest_dir>")
        print(f"  e.g. {sys.argv[0]} data_512 data_512_out")
        sys.exit(1)

    src_dir = Path(sys.argv[1])
    dst_dir = Path(sys.argv[2])

    # Resolve relative paths against script directory
    if not src_dir.is_absolute():
        src_dir = SCRIPT_DIR / src_dir
    if not dst_dir.is_absolute():
        dst_dir = SCRIPT_DIR / dst_dir

    if not src_dir.is_dir():
        print(f"Error: source directory '{src_dir}' not found.")
        sys.exit(1)

    if not FBX2OBJ.exists():
        print(f"Error: fbx2obj not found at '{FBX2OBJ}'")
        sys.exit(1)

    setup_logging(dst_dir)
    convert(src_dir, dst_dir)


if __name__ == "__main__":
    main()
