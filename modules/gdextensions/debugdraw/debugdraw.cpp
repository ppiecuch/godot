/*************************************************************************/
/*  debugdraw.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "debugdraw.h"

#include "core/os/main_loop.h"
#include "core/print_string.h"
#include "scene/main/viewport.h"
#include "scene/resources/theme.h"
#include "servers/visual_server.h"

DebugDraw *DebugDraw::singleton = NULL;

DebugDraw::DebugDraw() :
		ready(false) {
	ERR_FAIL_COND(singleton);
	singleton = this;
}

DebugDraw::~DebugDraw() {
	if (ready) {
		auto *vs = VS::get_singleton();

		for (auto *e = drawings.front(); e; e = e->next())
			vs->free(e->get().canvas_item);

		auto *st = SceneTree::get_singleton();
		st->disconnect("idle_frame", this, "_idle_frame");

		vs->free(canvas);
	}

	singleton = NULL;
}

bool DebugDraw::init() {
	auto *st = SceneTree::get_singleton();
	ERR_FAIL_NULL_V(st, false);

	auto *vs = VS::get_singleton();
	Viewport *viewport = st->get_root()->get_viewport();

	ERR_FAIL_NULL_V(viewport, false);

	canvas = vs->canvas_create();
	vs->viewport_attach_canvas(viewport->get_viewport_rid(), canvas);
	vs->viewport_set_canvas_stacking(viewport->get_viewport_rid(), canvas, (~0U) >> 1, (~0U) >> 1);

	default_font = Theme::get_default()->get_font("_", "_");

	st->connect("idle_frame", this, "_idle_frame");
	return ready = true;
}

void DebugDraw::circle(const Vector2 &position, float radius, const Color &color, float duration) {
	if (ready || init()) {
		auto *vs = VS::get_singleton();
		Drawing d = { vs->canvas_item_create(), duration };
		vs->canvas_item_set_parent(d.canvas_item, canvas);
		vs->canvas_item_add_circle(d.canvas_item, position, radius, color);
		drawings.push_back(d);
	}
}

void DebugDraw::line(const Vector2 &a, const Vector2 &b, const Color &color, float width, float duration) {
	if (ready || init()) {
		auto *vs = VS::get_singleton();
		Drawing d = { vs->canvas_item_create(), duration };
		vs->canvas_item_set_parent(d.canvas_item, canvas);
		vs->canvas_item_add_line(d.canvas_item, a, b, color, width);
		drawings.push_back(d);
	}
}

void DebugDraw::rect(const Rect2 &rect, const Color &color, float width, float duration) {
	if (ready || init()) {
		Vector2 tl = rect.position;
		Vector2 tr = rect.position + Vector2(rect.size.x, 0);
		Vector2 bl = rect.position + Vector2(0, rect.size.y);
		Vector2 br = rect.position + rect.size;

		auto *vs = VS::get_singleton();
		Drawing d = { vs->canvas_item_create(), duration };
		vs->canvas_item_set_parent(d.canvas_item, canvas);
		vs->canvas_item_add_line(d.canvas_item, tl, tr, color, width);
		vs->canvas_item_add_line(d.canvas_item, tr, br, color, width);
		vs->canvas_item_add_line(d.canvas_item, br, bl, color, width);
		vs->canvas_item_add_line(d.canvas_item, bl, tl, color, width);
		drawings.push_back(d);
	}
}

void DebugDraw::area(const Rect2 &rect, const Color &color, float duration) {
	if (ready || init()) {
		auto *vs = VS::get_singleton();
		Drawing d = { vs->canvas_item_create(), duration };
		vs->canvas_item_set_parent(d.canvas_item, canvas);
		vs->canvas_item_add_rect(d.canvas_item, rect, color);
		drawings.push_back(d);
	}
}

void DebugDraw::print(const String &text, const Color &color, float duration) {
	if (ready || init()) {
		auto *vs = VS::get_singleton();
		Drawing d = { vs->canvas_item_create(), duration };
		vs->canvas_item_set_parent(d.canvas_item, canvas);
		default_font->draw(d.canvas_item, Vector2(1, 1), text, color.inverted());
		default_font->draw(d.canvas_item, Vector2(), text, color);
		auto offset = (prints.size() + 1) * default_font->get_height();
		vs->canvas_item_set_transform(d.canvas_item, Transform2D(.0f, Vector2(10.f, 10.f + offset)));
		prints.push_back(d);
	}
}

void DebugDraw::clear() {
	auto *vs = VS::get_singleton();

	// clear drawings
	while (auto *e = drawings.front()) {
		vs->free(e->get().canvas_item);
		e->erase();
	}

	// clear prints
	while (auto *e = prints.front()) {
		vs->free(e->get().canvas_item);
		e->erase();
	}
}

void DebugDraw::_idle_frame() {
	auto *vs = VS::get_singleton();
	auto *st = SceneTree::get_singleton();
	const float delta = st->get_idle_process_time();

	// remove dead drawings
	for (auto *e = drawings.front(); e;) {
		auto &d = e->get();

		if (d.time_left < .0f) {
			vs->free(d.canvas_item);
			auto old = e;
			e = e->next();
			old->erase();
		} else {
			d.time_left -= delta;
			e = e->next();
		}
	}

	// remove dead prints
	uint32_t print_count = 0;

	for (auto *e = prints.front(); e;) {
		auto &d = e->get();

		if (d.time_left < .0f) {
			vs->free(d.canvas_item);
			auto old = e;
			e = e->next();
			old->erase();
		} else {
			auto offset = (print_count + 1) * default_font->get_height();
			vs->canvas_item_set_transform(d.canvas_item, Transform2D(.0f, Vector2(10.f, 10.f + offset)));
			++print_count;
			d.time_left -= delta;
			e = e->next();
		}
	}
}

DebugDraw *DebugDraw::get_singleton() {
	return singleton;
}

void DebugDraw::_bind_methods() {
	ClassDB::bind_method(D_METHOD("circle", "position:Vector2", "radius:real", "color:Color", "duration:real"), &DebugDraw::circle, DEFVAL(.0f));
	ClassDB::bind_method(D_METHOD("line", "a:Vector2", "b:Vector2", "color:Color", "width:real", "duration:real"), &DebugDraw::line, DEFVAL(1.f), DEFVAL(.0f));
	ClassDB::bind_method(D_METHOD("rect", "rect:Rect2", "color:Color", "width:real", "duration:real"), &DebugDraw::rect, DEFVAL(1.f), DEFVAL(.0f));
	ClassDB::bind_method(D_METHOD("area", "rect:Rect2", "color:Color", "duration:real"), &DebugDraw::area, DEFVAL(.0f));
	ClassDB::bind_method(D_METHOD("print", "text:String", "color:Color", "duration:real"), &DebugDraw::print, DEFVAL(.0f));

	ClassDB::bind_method(D_METHOD("clear"), &DebugDraw::clear);

	ClassDB::bind_method(D_METHOD("_idle_frame"), &DebugDraw::_idle_frame);
}
