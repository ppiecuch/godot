/*************************************************************************/
/*  blitter.cpp                                                          */
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

#include "blitter.h"

Blitter *Blitter::singleton = NULL;

void Blitter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("calculate_new_rect_from_mins_max", "p_base_size", "p_mins", "p_max"), &Blitter::calculate_new_rect_from_mins_max);
	ClassDB::bind_method(D_METHOD("calculate_new_size_from_mins_max", "p_base_size", "p_mins", "p_max"), &Blitter::calculate_new_size_from_mins_max);
	ClassDB::bind_method(D_METHOD("blit_rect_modulate", "p_src:Image", "p_src_rect", "p_dest_point", "p_modulate"), &Blitter::blit_rect_modulate);
	ClassDB::bind_method(D_METHOD("blit_rect_modulate_inverted_alpha", "p_src:Image", "p_src_rect", "p_dest_point", "p_modulate"), &Blitter::blit_rect_modulate_inverted_alpha);
	ClassDB::bind_method(D_METHOD("blit_rect_modulate_inverted_alpha_translucent", "p_src:Image", "p_src_rect", "p_dest_point", "p_modulate"), &Blitter::blit_rect_modulate_inverted_alpha_translucent);
}

Rect2 Blitter::calculate_new_rect_from_mins_max(const Vector2 &p_base_size, const Vector2 &p_mins, const Vector2 &p_max) {
	return BlitterOps::calculate_new_rect_from_mins_max(p_base_size, p_mins, p_max);
}

Vector2 Blitter::calculate_new_size_from_mins_max(const Vector2 &p_base_size, const Vector2 &p_mins, const Vector2 &p_max) {
	return BlitterOps::calculate_new_size_from_mins_max(p_base_size, p_mins, p_max);
}

Ref<Image> Blitter::blit_rect_modulate(const Ref<Image> p_src, const Rect2 &p_src_rect, const Ref<Image> p_dest, const Point2 &p_dest_point, const Color &p_modulate) {
	return BlitterOps::blit_rect<true, false, false>(p_src, p_src_rect, p_dest, p_dest_point, p_modulate);
}

Ref<Image> Blitter::blit_rect_modulate_inverted_alpha(const Ref<Image> p_src, const Rect2 &p_src_rect, const Ref<Image> p_dest, const Point2 &p_dest_point, const Color &p_modulate) {
	return BlitterOps::blit_rect<true, true, false>(p_src, p_src_rect, p_dest, p_dest_point, p_modulate);
}

Ref<Image> Blitter::blit_rect_modulate_inverted_alpha_translucent(const Ref<Image> p_src, const Rect2 &p_src_rect, const Ref<Image> p_dest, const Point2 &p_dest_point, const Color &p_modulate) {
	return BlitterOps::blit_rect<true, true, true>(p_src, p_src_rect, p_dest, p_dest_point, p_modulate);
}

Blitter::Blitter() {
	singleton = this;
}

Blitter::~Blitter() {
	singleton = nullptr;
}
