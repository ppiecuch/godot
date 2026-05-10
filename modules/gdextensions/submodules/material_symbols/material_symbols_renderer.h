/**************************************************************************/
/*  material_symbols_renderer.h                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
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
