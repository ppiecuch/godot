/**************************************************************************/
/*  gd_softrender_display.cpp                                             */
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

#include "gd_softrender_display.h"

// ============================================================
// SoftRenderDisplay (Node2D)
// ============================================================

SoftRenderDisplay::SoftRenderDisplay() {
	canvas_size = Size2(256, 256);
	centered = true;
	backend = SoftRender::BACKEND_PORTABLEGL;
}

void SoftRenderDisplay::set_canvas_size(const Size2 &p_size) {
	if (canvas_size == p_size) {
		return;
	}
	canvas_size = p_size;
	if (soft_render.is_valid()) {
		soft_render->initialize((int)canvas_size.width, (int)canvas_size.height, backend);
	}
	update();
}

Size2 SoftRenderDisplay::get_canvas_size() const {
	return canvas_size;
}

void SoftRenderDisplay::set_centered(bool p_centered) {
	centered = p_centered;
	update();
}

bool SoftRenderDisplay::is_centered() const {
	return centered;
}

void SoftRenderDisplay::set_backend(int p_backend) {
	if (backend == p_backend) {
		return;
	}
	backend = p_backend;
	if (soft_render.is_valid()) {
		soft_render->initialize((int)canvas_size.width, (int)canvas_size.height, backend);
	}
	update();
}

int SoftRenderDisplay::get_backend() const {
	return backend;
}

Ref<SoftRender> SoftRenderDisplay::get_soft_render() {
	if (soft_render.is_null()) {
		soft_render.instance();
		soft_render->initialize((int)canvas_size.width, (int)canvas_size.height, backend);
	}
	return soft_render;
}

void SoftRenderDisplay::refresh() {
	update();
}

void SoftRenderDisplay::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (soft_render.is_null()) {
				return;
			}
			Ref<ImageTexture> tex = soft_render->get_texture();
			if (tex.is_null()) {
				return;
			}
			Point2 offset;
			if (centered) {
				offset = -canvas_size * 0.5;
			}
			draw_texture(tex, offset);
		} break;
	}
}

#ifdef TOOLS_ENABLED
Dictionary SoftRenderDisplay::_edit_get_state() const {
	Dictionary state;
	state["offset"] = get_position();
	return state;
}

void SoftRenderDisplay::_edit_set_state(const Dictionary &p_state) {
	set_position(p_state["offset"]);
}

bool SoftRenderDisplay::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	Rect2 rect = _edit_get_rect();
	return rect.has_point(p_point);
}

Rect2 SoftRenderDisplay::_edit_get_rect() const {
	if (centered) {
		return Rect2(-canvas_size * 0.5, canvas_size);
	}
	return Rect2(Point2(), canvas_size);
}

void SoftRenderDisplay::_edit_set_rect(const Rect2 &p_rect) {
	set_position(p_rect.position + (centered ? canvas_size * 0.5 : Size2()));
	canvas_size = p_rect.size;
}

bool SoftRenderDisplay::_edit_use_rect() const {
	return true;
}
#endif

void SoftRenderDisplay::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_canvas_size", "size"), &SoftRenderDisplay::set_canvas_size);
	ClassDB::bind_method(D_METHOD("get_canvas_size"), &SoftRenderDisplay::get_canvas_size);
	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &SoftRenderDisplay::set_centered);
	ClassDB::bind_method(D_METHOD("is_centered"), &SoftRenderDisplay::is_centered);
	ClassDB::bind_method(D_METHOD("set_backend", "backend"), &SoftRenderDisplay::set_backend);
	ClassDB::bind_method(D_METHOD("get_backend"), &SoftRenderDisplay::get_backend);
	ClassDB::bind_method(D_METHOD("get_soft_render"), &SoftRenderDisplay::get_soft_render);
	ClassDB::bind_method(D_METHOD("refresh"), &SoftRenderDisplay::refresh);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "canvas_size"), "set_canvas_size", "get_canvas_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "centered"), "set_centered", "is_centered");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "backend", PROPERTY_HINT_ENUM, "PortableGL,Fusion2X"), "set_backend", "get_backend");
}
