/**************************************************************************/
/*  material_symbols_resources.c                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

// Embed the three Material Symbols variable-font TTFs via INCBIN. Paths are
// resolved against INCBIN_ROOT (set to the gdextensions module directory by
// SCsub). The matching extern declarations live in material_symbols_data.gen.h.

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

INCBIN(ttf_outlined, "thirdparty/material_symbols/MaterialSymbolsOutlined.ttf");
INCBIN(ttf_rounded, "thirdparty/material_symbols/MaterialSymbolsRounded.ttf");
INCBIN(ttf_sharp, "thirdparty/material_symbols/MaterialSymbolsSharp.ttf");
