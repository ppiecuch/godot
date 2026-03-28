/**************************************************************************/
/*  trail_2d.h                                                            */
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

//
// Created by Gen on 2016/1/5.
//

#ifndef GODOT_TRAIL_2D_H
#define GODOT_TRAIL_2D_H

#include "core/math/vector2.h"
#include "core/object.h"
#include "core/typedefs.h"
#include "scene/2d/canvas_item.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/position_2d.h"

#include "queue.h"

class TrailPoint2D : public Node2D {
	GDCLASS(TrailPoint2D, Node2D);

private:
	struct TrailItem {
		Vector2 position;
		int count;

		_FORCE_INLINE_ TrailItem() {
			count = 0;
		};
	};

	bool trail_enable;
	int span_frame;
	int span_count;
	real_t line_width;
	Ref<Gradient> line_color;

	Vector2 old_position;

	gdext::Queue<TrailItem> trail_items;

	void _normal_points(int idx, int total, Vector2 &res1, Vector2 &res2);
	void _update_position(bool minus = false);
	void _update_frame(bool minus = false);

	NodePath target_path;
	CanvasItem *trail_target;
	void _update_trail_target();
	void _on_exit_tree();

	Vector2 gravity;
	Vector2 final_gravity;
	real_t wave;
	real_t wave_scale;
	real_t wave_time_scale;
	real_t time_during;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	virtual Rect2 get_item_rect() const;

	_FORCE_INLINE_ bool get_trail_enable() { return trail_enable; }
	_FORCE_INLINE_ void set_trail_enable(bool p_enable) {
		trail_enable = p_enable;
		span_count = span_frame;
	}

	_FORCE_INLINE_ int get_span_frame() { return span_frame; }
	_FORCE_INLINE_ void set_span_frame(int p_frame) { span_frame = p_frame; }

	_FORCE_INLINE_ real_t get_trail_count() { return trail_items.limit(); }
	_FORCE_INLINE_ void set_trail_count(real_t p_count) { trail_items.alloc(p_count); }

	_FORCE_INLINE_ real_t get_line_width() { return line_width; }
	_FORCE_INLINE_ void set_line_width(real_t p_width) { line_width = p_width < 0 ? 0 : (p_width > 10 ? 10 : p_width); }

	_FORCE_INLINE_ Ref<Gradient> get_line_color() { return line_color; }
	_FORCE_INLINE_ void set_line_color(const Ref<Gradient> &p_color) { line_color = p_color; }

	_FORCE_INLINE_ NodePath get_target_path() { return target_path; }
	_FORCE_INLINE_ void set_target_path(const NodePath &p_path) {
		target_path = p_path;
		_update_trail_target();
	}

	_FORCE_INLINE_ Vector2 get_gravity() { return gravity; }
	_FORCE_INLINE_ void set_gravity(Vector2 p_gravity) { gravity = final_gravity = p_gravity; }

	_FORCE_INLINE_ real_t get_wave() { return wave; }
	_FORCE_INLINE_ void set_wave(real_t p_wave) { wave = p_wave; }
	_FORCE_INLINE_ real_t get_wave_scale() { return wave_scale; }
	_FORCE_INLINE_ void set_wave_scale(real_t p_scale) { wave_scale = p_scale; }
	_FORCE_INLINE_ real_t get_wave_time_scale() { return wave_time_scale; }
	_FORCE_INLINE_ void set_wave_time_scale(real_t p_scale) { wave_time_scale = p_scale; }

	_FORCE_INLINE_ TrailPoint2D() {
		trail_enable = false;
		span_frame = 0;
		line_width = 1;
		trail_items.alloc(30);
		trail_target = nullptr;
		wave = 0;
		time_during = 0;
		wave_scale = 10;
		wave_time_scale = 10;
	}

	_FORCE_INLINE_ ~TrailPoint2D() {
		if (trail_target != nullptr) {
			trail_target->disconnect("tree_exiting", this, "_on_exit_tree");
		}
	}
};

class TrailLine2D : public Node2D {
	GDCLASS(TrailLine2D, Node2D);

private:
	struct TrailItem {
		Vector2 position1, position2;
		int count;

		_FORCE_INLINE_ void offset(Vector2 p_off) {
			position1 += p_off;
			position2 += p_off;
		}

		_FORCE_INLINE_ TrailItem() {
			count = 0;
		};
	};

	Vector2 old_position;

	bool trail_enable;
	int span_frame;
	int span_count;
	Ref<Gradient> line_color;

	gdext::Queue<TrailItem> trail_items;

	Position2D *terminal;
	Point2 terminal_position;

	bool _needs_update;
	Vector<Vector<Point2>> _cache_polys;

	void _update_position(bool minus = false);
	void _update_frame(bool minus = false);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	virtual Rect2 get_item_rect() const;

	_FORCE_INLINE_ Vector2 get_terminal() {
		if (terminal)
			return terminal->get_position();
		else
			return terminal_position;
	}
	_FORCE_INLINE_ void set_terminal(const Vector2 &p_terminal) {
		if (terminal)
			terminal->set_position(p_terminal);
		else
			terminal_position = p_terminal;
	}

	_FORCE_INLINE_ bool get_trail_enable() { return trail_enable; }
	_FORCE_INLINE_ void set_trail_enable(bool p_enable) {
		trail_enable = p_enable;
		span_count = span_frame;
	}

	_FORCE_INLINE_ int get_span_frame() { return span_frame; }
	_FORCE_INLINE_ void set_span_frame(int p_frame) { span_frame = p_frame; }

	_FORCE_INLINE_ real_t get_trail_count() { return trail_items.limit(); }
	_FORCE_INLINE_ void set_trail_count(real_t p_count) { trail_items.alloc(p_count); }

	_FORCE_INLINE_ Ref<Gradient> get_line_color() { return line_color; }
	_FORCE_INLINE_ void set_line_color(const Ref<Gradient> &p_color) { line_color = p_color; }

	_FORCE_INLINE_ TrailLine2D() {
		trail_enable = false;
		span_frame = 0;
		trail_items.alloc(30);
		terminal_position = Vector2(10, 10);
		terminal = nullptr;

		_needs_update = false;
	}
};

#endif // GODOT_TRAIL_2D_H
