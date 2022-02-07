/*************************************************************************/
/*  blitter.h                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include "core/image.h"
#include "core/object.h"
#include "core/reference.h"

class BlitterOps {
public:
	static _FORCE_INLINE_ Rect2 calculate_new_rect_from_mins_max(const Vector2 &p_base_size, const Vector2 &p_mins, const Vector2 &p_max) {
		return Rect2(Point2((p_base_size.x * p_mins.x), (p_base_size.y * p_mins.y)), Size2((p_base_size.x * p_max.x) - (p_base_size.x * p_mins.x), (p_base_size.y * p_max.y) - (p_base_size.y * p_mins.y)));
	}

	static _FORCE_INLINE_ Vector2 calculate_new_size_from_mins_max(const Vector2 &p_base_size, const Vector2 &p_mins, const Vector2 &p_max) {
		return Vector2((p_base_size.x * p_max.x) - (p_base_size.x * p_mins.x), (p_base_size.y * p_max.y) - (p_base_size.y * p_mins.y));
	}

	template <bool use_modulate, bool invert_alpha, bool translucent>
	static _FORCE_INLINE_ Ref<Image> blit_rect(const Ref<Image> p_src, const Rect2 &p_src_rect, const Ref<Image> p_dest, const Point2 &p_dest_point, const Color &p_modulate) {
		Ref<Image> ret = memnew(Image(p_dest->get_width(), p_dest->get_height(), p_dest->has_mipmaps(), p_dest->get_format(), p_dest->get_data()));

		int srcdsize = p_src->get_data().size();
		ERR_FAIL_COND_V(srcdsize == 0, ret;);

		Rect2 rrect = Rect2(0, 0, p_src->get_width(), p_src->get_height()).clip(p_src_rect);

		int width = ret->get_width();
		int height = ret->get_height();

		for (int i = 0; i < rrect.size.y; i++) {
			if (i < 0 || i >= height)
				continue;
			for (int j = 0; j < rrect.size.x; j++) {
				if (j < 0 || j >= width)
					continue;

				Color src_pixel = p_src->get_pixel(rrect.position.x + j, rrect.position.y + i);
				if (use_modulate) {
					src_pixel.r *= p_modulate.r;
					src_pixel.g *= p_modulate.g;
					src_pixel.b *= p_modulate.b;
				}

				if (translucent) {
					if (use_modulate) {
						src_pixel.a *= p_modulate.a;
					}

					if (src_pixel.a == 0.0) {
						continue;
					} else if (src_pixel.a == 1.0) {
						if (invert_alpha) {
							src_pixel.a = 0.0f;
						}
						ret->set_pixel(p_dest_point.x + j, p_dest_point.y + i, src_pixel);
					} else {
						Color blended_pixel = src_pixel.blend(p_dest->get_pixel(p_src_rect.position.x + j, p_src_rect.position.y + i));
						if (invert_alpha) {
							blended_pixel.a = 1.0 - src_pixel.a;
						}
						ret->set_pixel(p_dest_point.x + j, p_dest_point.y + i, blended_pixel);
					}
				} else {
					if (invert_alpha) {
						src_pixel.a = 0.0f;
					} else {
						src_pixel.a = 1.0f;
					}
					ret->set_pixel(p_dest_point.x + j, p_dest_point.y + i, src_pixel);
				}
			}
		}
		return ret;
	}
};

class Blitter : public Object {
	GDCLASS(Blitter, Object)

	static Blitter *singleton;

protected:
	static void _bind_methods();

public:
	static Blitter *get_singleton();

	Rect2 calculate_new_rect_from_mins_max(const Vector2 &p_base_size, const Vector2 &p_mins, const Vector2 &p_max);
	Vector2 calculate_new_size_from_mins_max(const Vector2 &p_base_size, const Vector2 &p_mins, const Vector2 &p_max);
	Ref<Image> blit_rect_modulate(const Ref<Image> p_src, const Rect2 &p_src_rect, const Ref<Image> p_dest, const Point2 &p_dest_point, const Color &p_modulate);
	Ref<Image> blit_rect_modulate_inverted_alpha(const Ref<Image> p_src, const Rect2 &p_src_rect, const Ref<Image> p_dest, const Point2 &p_dest_point, const Color &p_modulate);
	Ref<Image> blit_rect_modulate_inverted_alpha_translucent(const Ref<Image> p_src, const Rect2 &p_src_rect, const Ref<Image> p_dest, const Point2 &p_dest_point, const Color &p_modulate);

	Blitter();
};
