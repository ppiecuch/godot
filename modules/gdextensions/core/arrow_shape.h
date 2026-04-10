/**************************************************************************/
/*  arrow_shape.h                                                         */
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

// Arrow2D — draws a filled arrow polygon from the node origin toward a target
// position or a tracked target node.
//
// Ported from the arrow_shape GDScript addon and extended with:
//   - has_target() / clear_target()       — explicit null-target handling
//   - double_headed                        — arrowhead at both ends
//   - color_end                            — per-vertex gradient tip color
//   - dashed                               — dashed shaft line fallback
//   - min_length                           — suppress drawing below threshold
//   - NOTIFICATION_VISIBILITY_CHANGED      — pause _process when hidden
//
// Usage:
//   var a = Arrow2D.new()
//   a.target = Vector2(200, 100)
//   a.color  = Color(1, 0.5, 0)
//   add_child(a)
//
//   # Track a moving node:
//   a.target_node_path = $enemy.get_path()

#ifndef ARROW_SHAPE_H
#define ARROW_SHAPE_H

#include "core/color.h"
#include "core/math/vector2.h"
#include "scene/2d/node_2d.h"

class Arrow2D : public Node2D {
	GDCLASS(Arrow2D, Node2D);

private:
	// ── Target ────────────────────────────────────────────────────────────────
	NodePath target_node_path;
	ObjectID target_node_id = 0; // safe handle; 0 = none
	Vector2 target;
	bool has_target_flag = false;

	// Change-detection caches
	Vector2 cached_target;
	Vector2 cached_pos;

	// ── Shape ─────────────────────────────────────────────────────────────────
	int width = 3;
	float start_offset = 0.0f;
	float end_offset = 0.0f;
	float side_offset = 0.0f;
	int arrow_size = 6;

	// ── Color ─────────────────────────────────────────────────────────────────
	Color color = Color(1, 1, 1, 1); // shaft / tail color
	Color color_end = Color(1, 1, 1, 1); // tip color (gradient when != color)

	// ── Flags ─────────────────────────────────────────────────────────────────
	bool editor_only = false;
	bool double_headed = false; // also draw arrowhead at the shaft start
	bool dashed = false; // draw shaft as dashed line instead of polygon

	// ── Thresholds ────────────────────────────────────────────────────────────
	float min_length = 1.0f; // skip drawing when arrow is shorter than this

	// ── Internal helpers ──────────────────────────────────────────────────────
	bool _arrow_should_draw() const;
	Node2D *_get_target_node() const;

	// Writes the 7 vertices of a single arrow head+shaft into p_pts / p_cols.
	// p_pts must have room for 7 entries starting at offset.
	// If p_flip is true the head points toward the START (double_headed tail).
	void _build_arrow(Vector2 *p_pts, Color *p_cols,
			const Vector2 &p_arrowvec, const Vector2 &p_sidedir,
			float p_arrowlen, bool p_flip) const;

	// Draws the dashed-line fallback (shaft only, no polygon fill).
	void _draw_dashed(const Vector2 &p_from, const Vector2 &p_to);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// ── target_node_path ──────────────────────────────────────────────────────
	void set_target_node_path(const NodePath &p_path);
	NodePath get_target_node_path() const { return target_node_path; }

	// ── target ────────────────────────────────────────────────────────────────
	void set_target(const Vector2 &p_target);
	Vector2 get_target() const { return target; }
	void clear_target();
	bool has_target() const { return has_target_flag; }

	// ── shape ─────────────────────────────────────────────────────────────────
	void set_width(int p_width);
	int get_width() const { return width; }

	void set_start_offset(float p_value);
	float get_start_offset() const { return start_offset; }

	void set_end_offset(float p_value);
	float get_end_offset() const { return end_offset; }

	void set_side_offset(float p_value);
	float get_side_offset() const { return side_offset; }

	void set_arrow_size(int p_value);
	int get_arrow_size() const { return arrow_size; }

	// ── color ─────────────────────────────────────────────────────────────────
	void set_color(const Color &p_color);
	Color get_color() const { return color; }

	void set_color_end(const Color &p_color);
	Color get_color_end() const { return color_end; }

	// ── flags ─────────────────────────────────────────────────────────────────
	void set_editor_only(bool p_value);
	bool get_editor_only() const { return editor_only; }

	void set_double_headed(bool p_value);
	bool get_double_headed() const { return double_headed; }

	void set_dashed(bool p_value);
	bool get_dashed() const { return dashed; }

	// ── thresholds ────────────────────────────────────────────────────────────
	void set_min_length(float p_value);
	float get_min_length() const { return min_length; }

	Arrow2D() = default;
};

#endif // ARROW_SHAPE_H
