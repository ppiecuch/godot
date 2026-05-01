# Material Symbols — vendored variable fonts

Source: https://github.com/google/material-design-icons (variablefont/)

Three OpenType variable fonts shipping the Material Symbols glyph set, each
covering ~3000 icons with five axes of variation:

| Axis        | Tag    | Range                                  |
|-------------|--------|----------------------------------------|
| Style       | (font) | Outlined / Rounded / Sharp             |
| Fill        | `FILL` | 0.0 (outline) … 1.0 (filled)           |
| Weight      | `wght` | 100 … 700                              |
| Grade       | `GRAD` | -25 / 0 / +200                         |
| Opt. size   | `opsz` | 20 / 24 / 40 / 48                      |

The renderer lives in `submodules/material_symbols`. This directory only
holds the upstream font binaries and per-style codepoint maps (one line per
icon: `name codepoint_hex`). Total binary cost ≈ 5–7 MB after embedding.

## License

Apache-2.0 — see `LICENSE` (refreshed by `./update.sh`). Bundling these
fonts into a redistributed binary requires preserving the LICENSE file and
attributing Google in NOTICE / about screens.

## Refreshing

```
./update.sh
```

Then re-run `scons` to pick up the refreshed binaries (regen of
`submodules/material_symbols/material_symbols_data.gen.{h,cpp}`).
