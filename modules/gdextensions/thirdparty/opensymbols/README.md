# OpenSymbols (Deepin OpenSymbol fonts) — vendored data

Source: https://github.com/leaeasy/deepin-opensymbol-fonts

This directory ships the per-glyph SVG export of the Deepin OpenSymbol font set
(a Wingdings/OpenSymbol-style fork by Deepin). Three font variants are
included:

| Variant directory     | Glyph count |
|-----------------------|-------------|
| `DeepinOpenSymbol/`   | 224         |
| `DeepinOpenSymbol2/`  | 217         |
| `DeepinOpenSymbol3/`  | 208         |

Each variant's `.map` file is a JSON dictionary mapping the glyph name (used as
the lookup key in `submodules/opensymbols`) to its Unicode codepoint in the
original font.

## License

OpenSymbol upstream is **GPL-2.0** with portions under **LGPL-2.1+** — see
`COPYING` (when present; fetched by `update.sh`). Statically linking these
glyphs into a Godot binary brings the whole binary under the (L)GPL terms. This
fork accepts those terms; redistributors must preserve the COPYING text and
respect the (L)GPL.

## Refreshing

Run `./update.sh` to clone the upstream repo and refresh the `svg/` tree and
`COPYING`. Any local patches under `../../patches/opensymbols/*.patch` are
re-applied. The script never touches generated headers — the Godot SCons build
takes the `svg/` files as input and produces
`submodules/opensymbols/opensymbols_data.gen.{h,cpp}` on demand.
