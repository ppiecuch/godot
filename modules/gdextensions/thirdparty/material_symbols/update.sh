#!/bin/bash
# Refresh thirdparty/material_symbols — fetches Google's three Material
# Symbols variable fonts (Outlined, Rounded, Sharp) + codepoint maps.
#
# License: Apache-2.0 (https://github.com/google/material-design-icons/blob/master/LICENSE)
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
RAW="https://raw.githubusercontent.com/google/material-design-icons/master/variablefont"
LICENSE_URL="https://raw.githubusercontent.com/google/material-design-icons/master/LICENSE"

# URL-encoded "[FILL,GRAD,opsz,wght]"
SUFFIX="%5BFILL%2CGRAD%2Copsz%2Cwght%5D"

echo "Library: material_symbols (Material Symbols variable fonts)"
echo "Repo:    google/material-design-icons (variablefont/)"
echo "Target:  $SCRIPT_DIR"

mkdir -p "$SCRIPT_DIR/codepoints"

curl -fsSL "$LICENSE_URL" -o "$SCRIPT_DIR/LICENSE"
echo "  fetched LICENSE"

for style in Outlined Rounded Sharp; do
    ttf_url="$RAW/MaterialSymbols${style}${SUFFIX}.ttf"
    cp_url="$RAW/MaterialSymbols${style}${SUFFIX}.codepoints"
    ttf_out="$SCRIPT_DIR/MaterialSymbols${style}.ttf"
    cp_out="$SCRIPT_DIR/codepoints/${style}.codepoints"
    curl -fsSL "$ttf_url" -o "$ttf_out"
    curl -fsSL "$cp_url" -o "$cp_out"
    ttf_size=$(stat -f '%z' "$ttf_out" 2>/dev/null || stat -c '%s' "$ttf_out")
    cp_lines=$(/usr/bin/wc -l < "$cp_out" | /usr/bin/tr -d ' ')
    echo "  ${style}: $((ttf_size / 1024)) KB TTF, ${cp_lines} glyphs"
done

echo "Done. Re-run scons to regenerate material_symbols_data.gen.{h,cpp}."
