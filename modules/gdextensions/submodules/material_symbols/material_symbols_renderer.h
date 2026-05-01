/**************************************************************************/
/*  material_symbols_renderer.h                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

// Thin FreeType wrapper that owns one FT_Library and three FT_Faces (one
// per Material Symbols style). Renders a glyph at a given pixel size with
// variable-axis coordinates (FILL, GRAD, opsz, wght) into a grayscale
// FT_Bitmap that the caller composes into an RGBA Image.
//
// FT_Library is lazy-init'd on first call; call shutdown() once at engine
// teardown to release it. Thread-unsafe (per FreeType's standard contract).

#ifndef MATERIAL_SYMBOLS_RENDERER_H
#define MATERIAL_SYMBOLS_RENDERER_H

#include "core/color.h"
#include "core/image.h"
#include "core/reference.h"
#include "core/typedefs.h"

class MaterialSymbolsRenderer {
public:
	enum AxisIndex {
		AX_FILL = 0,
		AX_GRAD = 1,
		AX_OPSZ = 2,
		AX_WGHT = 3,
		AX_COUNT = 4,
	};

	struct Axes {
		int weight; // 100..700, default 400
		int grade; // -25..200, default 0
		int optical_size; // 20/24/40/48, default 24
		float fill; // 0..1, default 0
	};

	// Render a glyph (codepoint) into a `size_px × size_px` RGBA8 image,
	// composing the FreeType grayscale into the requested colour. Returns
	// an empty Ref on failure.
	static Ref<Image> render(int style, uint32_t codepoint, int size_px,
			const Color &color, const Axes &axes);

	static void shutdown();
};

#endif // MATERIAL_SYMBOLS_RENDERER_H
