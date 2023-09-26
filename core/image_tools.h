/**************************************************************************/
/*  image_tools.h                                                         */
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

#ifndef IMAGE_TOOLS_H
#define IMAGE_TOOLS_H

#include "core/dictionary.h"
#include "core/image.h"
#include "core/vector.h"

class ImageTools {
public:
	static void checker_board(Image *p_src, int p_cell, const Color &grid_color1, const Color &grid_color2, bool details = true, const Color &details_color = Color(1, 1, 1, 1));
	static Ref<Image> neighbor_tracing(const Image *p_src);
	static void fix_alpha_edges(Image *p_src);
	static void fix_tex_bleed(Image *p_src);
	static void normalmap_to_xy(Image *p_src);
	static void bumpmap_to_normalmap(Image *p_src, float p_bump_scale = 1.0);
	static Vector<Rect2> unpack_region(const Image *p_src, real_t p_distance_between_tiles_perc = 1, real_t p_minimum_tile_area_to_save_perc = 1, real_t p_alpha_threshold = 0.2, Ref<Image> p_debug_image = Ref<Image>());
	enum SeamlessAxis {
		FE_XY,
		FE_X,
		FE_Y,
	};
	enum SeamlessStampMode {
		FE_STAMPING,
		FE_SPLATMODE,
	};
	static Ref<Image> make_seamless(const Image *p_src, SeamlessStampMode p_stamp_mode = FE_STAMPING, real_t p_stamper_radius = 0.45, real_t p_stamp_density = 0.4, real_t p_hardness = 0.6, real_t p_stamp_noise_mask = 1, real_t p_randomize = 0.25, int p_stamp_rotate = 1, SeamlessAxis p_to_loop = FE_XY);
};

#endif // IMAGE_TOOLS_H
