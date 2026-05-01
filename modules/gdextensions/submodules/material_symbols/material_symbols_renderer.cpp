/**************************************************************************/
/*  material_symbols_renderer.cpp                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "material_symbols_renderer.h"

#include "material_symbols_data.gen.h"

#include "core/print_string.h"
#include "core/typedefs.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H

namespace {

struct State {
	FT_Library library = nullptr;
	FT_Face faces[3] = { nullptr, nullptr, nullptr };
	int axis_index[3][MaterialSymbolsRenderer::AX_COUNT] = {
		{ -1, -1, -1, -1 },
		{ -1, -1, -1, -1 },
		{ -1, -1, -1, -1 },
	};
	bool init_failed = false;
};

State *get_state() {
	static State s;
	return &s;
}

bool ensure_library(State *s) {
	if (s->library) {
		return true;
	}
	if (s->init_failed) {
		return false;
	}
	if (FT_Init_FreeType(&s->library) != 0) {
		s->init_failed = true;
		ERR_PRINT("MaterialSymbols: FT_Init_FreeType failed");
		return false;
	}
	return true;
}

bool ensure_face(State *s, int style) {
	if (style < 0 || style >= 3) {
		return false;
	}
	if (s->faces[style]) {
		return true;
	}
	if (!ensure_library(s)) {
		return false;
	}
	const unsigned char *data = nullptr;
	unsigned int size = 0;
	switch (style) {
		case 0:
			data = ttf_outlined_data;
			size = ttf_outlined_size;
			break;
		case 1:
			data = ttf_rounded_data;
			size = ttf_rounded_size;
			break;
		case 2:
			data = ttf_sharp_data;
			size = ttf_sharp_size;
			break;
		default:
			return false;
	}
	FT_Face face = nullptr;
	if (FT_New_Memory_Face(s->library, data, (FT_Long)size, 0, &face) != 0) {
		ERR_PRINT(vformat("MaterialSymbols: FT_New_Memory_Face failed for style %d", style));
		return false;
	}
	s->faces[style] = face;

	// Resolve axis name → index lookup. Material Symbols ships FILL, GRAD, opsz, wght.
	FT_MM_Var *mm = nullptr;
	if (FT_Get_MM_Var(face, &mm) == 0 && mm) {
		for (FT_UInt i = 0; i < mm->num_axis; i++) {
			const FT_Var_Axis &ax = mm->axis[i];
			// `tag` is a 4-char-code uint32. Compare against the four axes we care about.
			uint32_t t = (uint32_t)ax.tag;
			if (t == FT_MAKE_TAG('F', 'I', 'L', 'L')) {
				s->axis_index[style][MaterialSymbolsRenderer::AX_FILL] = (int)i;
			} else if (t == FT_MAKE_TAG('G', 'R', 'A', 'D')) {
				s->axis_index[style][MaterialSymbolsRenderer::AX_GRAD] = (int)i;
			} else if (t == FT_MAKE_TAG('o', 'p', 's', 'z')) {
				s->axis_index[style][MaterialSymbolsRenderer::AX_OPSZ] = (int)i;
			} else if (t == FT_MAKE_TAG('w', 'g', 'h', 't')) {
				s->axis_index[style][MaterialSymbolsRenderer::AX_WGHT] = (int)i;
			}
		}
		FT_Done_MM_Var(s->library, mm);
	}
	return true;
}

} // namespace

Ref<Image> MaterialSymbolsRenderer::render(int style, uint32_t codepoint, int size_px,
		const Color &color, const Axes &axes) {
	if (size_px <= 0 || codepoint == 0) {
		return Ref<Image>();
	}
	State *s = get_state();
	if (!ensure_face(s, style)) {
		return Ref<Image>();
	}
	FT_Face face = s->faces[style];

	// Set variable-axis coordinates. FreeType wants 16.16 fixed-point, with
	// axis ordering matching the face's mm_var. We assemble an array sized
	// to the face's actual axis count, default values for any axis we don't
	// recognise, and fill in our 4 known axes by their resolved indices.
	FT_MM_Var *mm = nullptr;
	if (FT_Get_MM_Var(face, &mm) == 0 && mm) {
		FT_Fixed coords[16];
		FT_UInt n = MIN(mm->num_axis, (FT_UInt)16);
		for (FT_UInt i = 0; i < n; i++) {
			coords[i] = mm->axis[i].def; // default
		}
		const int *aidx = s->axis_index[style];
		if (aidx[AX_FILL] >= 0 && (FT_UInt)aidx[AX_FILL] < n) {
			coords[aidx[AX_FILL]] = (FT_Fixed)((double)axes.fill * 65536.0);
		}
		if (aidx[AX_GRAD] >= 0 && (FT_UInt)aidx[AX_GRAD] < n) {
			coords[aidx[AX_GRAD]] = (FT_Fixed)((double)axes.grade * 65536.0);
		}
		if (aidx[AX_OPSZ] >= 0 && (FT_UInt)aidx[AX_OPSZ] < n) {
			coords[aidx[AX_OPSZ]] = (FT_Fixed)((double)axes.optical_size * 65536.0);
		}
		if (aidx[AX_WGHT] >= 0 && (FT_UInt)aidx[AX_WGHT] < n) {
			coords[aidx[AX_WGHT]] = (FT_Fixed)((double)axes.weight * 65536.0);
		}
		FT_Set_Var_Design_Coordinates(face, n, coords);
		FT_Done_MM_Var(s->library, mm);
	}

	if (FT_Set_Pixel_Sizes(face, 0, (FT_UInt)size_px) != 0) {
		return Ref<Image>();
	}
	if (FT_Load_Char(face, (FT_ULong)codepoint, FT_LOAD_RENDER) != 0) {
		return Ref<Image>();
	}
	const FT_Bitmap &bm = face->glyph->bitmap;
	if (bm.width == 0 || bm.rows == 0) {
		return Ref<Image>();
	}

	// Centre the glyph in a size_px × size_px RGBA bitmap.
	const int W = size_px;
	const int H = size_px;
	const int gx = ((int)W - (int)bm.width) / 2;
	const int gy = ((int)H - (int)bm.rows) / 2;

	PoolVector<uint8_t> px;
	px.resize(W * H * 4);
	{
		PoolVector<uint8_t>::Write w = px.write();
		uint8_t *dst = w.ptr();
		for (int i = 0; i < W * H * 4; i++) {
			dst[i] = 0;
		}
		const uint8_t r = (uint8_t)CLAMP((int)round(color.r * 255.0f), 0, 255);
		const uint8_t g = (uint8_t)CLAMP((int)round(color.g * 255.0f), 0, 255);
		const uint8_t b = (uint8_t)CLAMP((int)round(color.b * 255.0f), 0, 255);
		const float a_factor = color.a;
		for (unsigned int y = 0; y < bm.rows; y++) {
			const int dy = gy + (int)y;
			if (dy < 0 || dy >= H) {
				continue;
			}
			const uint8_t *src_row = bm.buffer + y * bm.pitch;
			for (unsigned int x = 0; x < bm.width; x++) {
				const int dx = gx + (int)x;
				if (dx < 0 || dx >= W) {
					continue;
				}
				const uint8_t alpha = src_row[x];
				const size_t off = (size_t)((dy * W) + dx) * 4;
				dst[off + 0] = r;
				dst[off + 1] = g;
				dst[off + 2] = b;
				dst[off + 3] = (uint8_t)CLAMP((int)round((float)alpha * a_factor), 0, 255);
			}
		}
	}
	Ref<Image> img;
	img.instance();
	img->create(W, H, false, Image::FORMAT_RGBA8, px);
	return img;
}

void MaterialSymbolsRenderer::shutdown() {
	State *s = get_state();
	for (int i = 0; i < 3; i++) {
		if (s->faces[i]) {
			FT_Done_Face(s->faces[i]);
			s->faces[i] = nullptr;
		}
	}
	if (s->library) {
		FT_Done_FreeType(s->library);
		s->library = nullptr;
	}
}
