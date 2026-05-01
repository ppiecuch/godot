#!/bin/bash
# Refresh thirdparty/opensymbols — vendors the upstream svg/ tree from
# https://github.com/leaeasy/deepin-opensymbol-fonts.
#
# OpenSymbol is GPL-2.0 / LGPL-2.1+. We mirror the lot here so the codegen
# step can pick out individual glyphs at build time.
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO="https://github.com/leaeasy/deepin-opensymbol-fonts.git"

echo "Library: opensymbols"
echo "Repo:    $REPO"
echo "Target:  $SCRIPT_DIR"

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

git clone --depth 1 "$REPO" "$tmp/repo" >/dev/null 2>&1

rm -rf "$SCRIPT_DIR/svg"
[ -d "$tmp/repo/svg" ] && cp -R "$tmp/repo/svg" "$SCRIPT_DIR/svg"
[ -f "$tmp/repo/COPYING" ]   && cp "$tmp/repo/COPYING"   "$SCRIPT_DIR/COPYING"
[ -f "$tmp/repo/README.md" ] && cp "$tmp/repo/README.md" "$SCRIPT_DIR/UPSTREAM_README.md"

if [ -d "$SCRIPT_DIR/svg" ]; then
    for d in "$SCRIPT_DIR/svg"/*/; do
        [ -d "$d" ] || continue
        cnt=$(/bin/ls "$d" 2>/dev/null | /usr/bin/wc -l | /usr/bin/tr -d ' ')
        echo "  $(basename "$d"): $cnt SVGs"
    done
fi

# viewBox tightening is performed by the SCons codegen step, not here —
# keep the vendored copy verbatim from upstream.
