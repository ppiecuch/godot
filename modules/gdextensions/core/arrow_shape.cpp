/**************************************************************************/
/*  arrow_shape.cpp                                                       */
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

#include "arrow_shape.h"

#include "core/engine.h"
#include "core/object.h"
#include "core/os/os.h"
#include "scene/2d/node_2d.h"

// ── Helpers ──────────────────────────────────────────────────────────────────

bool Arrow2D::_arrow_should_draw() const {
	if (!editor_only) {
		return true;
	}
	return Engine::get_singleton()->is_editor_hint();
}

Node2D *Arrow2D::_get_target_node() const {
	if (target_node_id == 0) {
		return nullptr;
	}
	Object *obj = ObjectDB::get_instance(target_node_id);
	return Object::cast_to<Node2D>(obj);
}

// Writes 7 vertices of a single arrowhead+shaft into p_pts / p_cols.
// When p_flip is true the head points toward the START of p_arrowvec
// (used for the second head in double_headed mode).
//
// Vertex layout (p_flip = false, head at END):
//   0  left  shaft start
//   1  left  shoulder (base of head, left)
//   2  left  wing tip
//   3  TIP
//   4  right wing tip
//   5  right shoulder
//   6  right shaft start
void Arrow2D::_build_arrow(Vector2 *p_pts, Color *p_cols,
		const Vector2 &p_arrowvec, const Vector2 &p_sidedir,
		float p_arrowlen, bool p_flip) const {
	const Vector2 sidevec = p_sidedir * (width * 0.5f);
	const Vector2 pointvec = (p_arrowvec / p_arrowlen) * (width * (float)arrow_size);

	const Vector2 startoffset = (p_arrowvec / p_arrowlen) * start_offset;
	const Vector2 endoffset = -(p_arrowvec / p_arrowlen) * end_offset;
	const Vector2 sideoff = p_sidedir * side_offset;

	if (!p_flip) {
		// Head at END (normal arrow)
		p_pts[0] = sideoff + startoffset + sidevec;
		p_pts[1] = sideoff + endoffset + sidevec + p_arrowvec - pointvec;
		p_pts[2] = sideoff + endoffset + (float)arrow_size * sidevec + p_arrowvec - pointvec;
		p_pts[3] = sideoff + endoffset + p_arrowvec; // tip
		p_pts[4] = sideoff + endoffset - (float)arrow_size * sidevec + p_arrowvec - pointvec;
		p_pts[5] = sideoff + endoffset - sidevec + p_arrowvec - pointvec;
		p_pts[6] = sideoff + startoffset - sidevec;
	} else {
		// Head at START (flipped — for double_headed tail)
		p_pts[0] = sideoff + endoffset + p_arrowvec - sidevec;
		p_pts[1] = sideoff + startoffset - sidevec + pointvec;
		p_pts[2] = sideoff + startoffset - (float)arrow_size * sidevec + pointvec;
		p_pts[3] = sideoff + startoffset; // tip at origin end
		p_pts[4] = sideoff + startoffset + (float)arrow_size * sidevec + pointvec;
		p_pts[5] = sideoff + startoffset + sidevec + pointvec;
		p_pts[6] = sideoff + endoffset + p_arrowvec + sidevec;
	}

	// Gradient: shaft vertices (0,1,5,6) use color; head vertices (2,3,4) use color_end.
	// For the flipped head the roles swap so the tip (index 3) always gets color_end.
	for (int i = 0; i < 7; ++i) {
		p_cols[i] = color;
	}
	// Tip + wing vertices always get color_end
	p_cols[2] = color_end;
	p_cols[3] = color_end;
	p_cols[4] = color_end;
}

// Draws a dashed line from p_from to p_to using the current color.
void Arrow2D::_draw_dashed(const Vector2 &p_from, const Vector2 &p_to) {
	const float dash_len = (float)width * 3.0f;
	const float gap_len = (float)width * 1.5f;
	const float step = dash_len + gap_len;

	const Vector2 delta = p_to - p_from;
	const float total = delta.length();
	if (total < CMP_EPSILON) {
		return;
	}
	const Vector2 dir = delta / total;

	float pos = 0.0f;
	while (pos < total) {
		float end = MIN(pos + dash_len, total);
		draw_line(p_from + dir * pos, p_from + dir * end, color, (float)width);
		pos += step;
	}
}

// ── Notification ─────────────────────────────────────────────────────────────

void Arrow2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_READY: {
			set_process(is_visible_in_tree());
		} break;

		case NOTIFICATION_VISIBILITY_CHANGED: {
			set_process(is_visible_in_tree());
		} break;

		case NOTIFICATION_DRAW: {
			if (!_arrow_should_draw() || !has_target_flag) {
				return;
			}

			// All coordinates are local — draw() operates in local space.
			// arrowvec = target - node_global_pos, but since we draw locally,
			// target must be stored in local coords relative to this node.
			// set_target() stores the raw value; callers pass global positions
			// when tracking a node, so we convert via to_local().
			//
			// For the Vector2 property (set via editor or script), the value is
			// treated as a local-space offset from the node origin, matching the
			// original GDScript behaviour where the arrow was drawn in local space.

			const Vector2 arrowvec = target;
			const float arrowlen = arrowvec.length();

			if (arrowlen < min_length) {
				// Below threshold — draw a small dot at the target instead
				if (arrowlen >= CMP_EPSILON) {
					draw_circle(arrowvec, (float)width * 0.5f, color);
				}
				return;
			}

			const Vector2 sidedir = Vector2(arrowvec.y, -arrowvec.x) / arrowlen;

			if (dashed) {
				// Shaft only — no filled polygon
				const Vector2 arrowdir = arrowvec / arrowlen;
				const Vector2 startoffset = arrowdir * start_offset;
				const Vector2 endoffset = arrowdir * (arrowlen - end_offset - width * (float)arrow_size);
				const Vector2 sideoff = sidedir * side_offset;
				_draw_dashed(sideoff + startoffset, sideoff + sideoff + endoffset);
				return;
			}

			// ── Normal polygon arrow ──────────────────────────────────────────
			Vector<Vector2> points;
			Vector<Color> colors;

			if (double_headed) {
				points.resize(14);
				colors.resize(14);
				_build_arrow(points.ptrw(), colors.ptrw(), arrowvec, sidedir, arrowlen, false);
				_build_arrow(points.ptrw() + 7, colors.ptrw() + 7, arrowvec, sidedir, arrowlen, true);
				draw_polygon(points, colors);
			} else {
				points.resize(7);
				colors.resize(7);
				_build_arrow(points.ptrw(), colors.ptrw(), arrowvec, sidedir, arrowlen, false);
				draw_polygon(points, colors);
			}
		} break;

		case NOTIFICATION_PROCESS: {
			// Track a moving target node — convert global → local for draw
			if (Node2D *tnode = _get_target_node()) {
				const Vector2 local_target = to_local(tnode->get_global_position());
				set_target(local_target);
			}
			// Redraw if the arrow node itself moved
			const Vector2 pos = get_global_position();
			if (pos != cached_pos) {
				cached_pos = pos;
				update();
			}
		} break;
	}
}

// ── Setters ──────────────────────────────────────────────────────────────────

void Arrow2D::set_target_node_path(const NodePath &p_path) {
	target_node_path = p_path;
	target_node_id = 0;

	if (p_path.is_empty()) {
		has_target_flag = false;
		update();
		return;
	}

	Node *node = has_node(p_path) ? get_node(p_path) : nullptr;
	if (Node2D *n2d = Object::cast_to<Node2D>(node)) {
		target_node_id = n2d->get_instance_id();
		has_target_flag = true;
	} else {
		has_target_flag = false;
	}
	update();
}

void Arrow2D::set_target(const Vector2 &p_target) {
	if (p_target == cached_target && has_target_flag) {
		return;
	}
	target = p_target;
	cached_target = p_target;
	has_target_flag = true;
	update();
}

void Arrow2D::clear_target() {
	has_target_flag = false;
	target_node_id = 0;
	target_node_path = NodePath();
	update();
}

void Arrow2D::set_width(int p_width) {
	width = p_width;
	update();
}

void Arrow2D::set_start_offset(float p_value) {
	start_offset = p_value;
	update();
}

void Arrow2D::set_end_offset(float p_value) {
	end_offset = p_value;
	update();
}

void Arrow2D::set_side_offset(float p_value) {
	side_offset = p_value;
	update();
}

void Arrow2D::set_arrow_size(int p_value) {
	arrow_size = p_value;
	update();
}

void Arrow2D::set_color(const Color &p_color) {
	color = p_color;
	update();
}

void Arrow2D::set_color_end(const Color &p_color) {
	color_end = p_color;
	update();
}

void Arrow2D::set_editor_only(bool p_value) {
	editor_only = p_value;
	update();
}

void Arrow2D::set_double_headed(bool p_value) {
	double_headed = p_value;
	update();
}

void Arrow2D::set_dashed(bool p_value) {
	dashed = p_value;
	update();
}

void Arrow2D::set_min_length(float p_value) {
	min_length = p_value;
	update();
}

// ── Binding ───────────────────────────────────────────────────────────────────

void Arrow2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node_path", "path"), &Arrow2D::set_target_node_path);
	ClassDB::bind_method(D_METHOD("get_target_node_path"), &Arrow2D::get_target_node_path);

	ClassDB::bind_method(D_METHOD("set_target", "target"), &Arrow2D::set_target);
	ClassDB::bind_method(D_METHOD("get_target"), &Arrow2D::get_target);
	ClassDB::bind_method(D_METHOD("clear_target"), &Arrow2D::clear_target);
	ClassDB::bind_method(D_METHOD("has_target"), &Arrow2D::has_target);

	ClassDB::bind_method(D_METHOD("set_width", "width"), &Arrow2D::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &Arrow2D::get_width);

	ClassDB::bind_method(D_METHOD("set_start_offset", "value"), &Arrow2D::set_start_offset);
	ClassDB::bind_method(D_METHOD("get_start_offset"), &Arrow2D::get_start_offset);

	ClassDB::bind_method(D_METHOD("set_end_offset", "value"), &Arrow2D::set_end_offset);
	ClassDB::bind_method(D_METHOD("get_end_offset"), &Arrow2D::get_end_offset);

	ClassDB::bind_method(D_METHOD("set_side_offset", "value"), &Arrow2D::set_side_offset);
	ClassDB::bind_method(D_METHOD("get_side_offset"), &Arrow2D::get_side_offset);

	ClassDB::bind_method(D_METHOD("set_arrow_size", "value"), &Arrow2D::set_arrow_size);
	ClassDB::bind_method(D_METHOD("get_arrow_size"), &Arrow2D::get_arrow_size);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &Arrow2D::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &Arrow2D::get_color);

	ClassDB::bind_method(D_METHOD("set_color_end", "color"), &Arrow2D::set_color_end);
	ClassDB::bind_method(D_METHOD("get_color_end"), &Arrow2D::get_color_end);

	ClassDB::bind_method(D_METHOD("set_editor_only", "value"), &Arrow2D::set_editor_only);
	ClassDB::bind_method(D_METHOD("get_editor_only"), &Arrow2D::get_editor_only);

	ClassDB::bind_method(D_METHOD("set_double_headed", "value"), &Arrow2D::set_double_headed);
	ClassDB::bind_method(D_METHOD("get_double_headed"), &Arrow2D::get_double_headed);

	ClassDB::bind_method(D_METHOD("set_dashed", "value"), &Arrow2D::set_dashed);
	ClassDB::bind_method(D_METHOD("get_dashed"), &Arrow2D::get_dashed);

	ClassDB::bind_method(D_METHOD("set_min_length", "value"), &Arrow2D::set_min_length);
	ClassDB::bind_method(D_METHOD("get_min_length"), &Arrow2D::get_min_length);

	ADD_GROUP("Target", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node_path"), "set_target_node_path", "get_target_node_path");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "target"), "set_target", "get_target");

	ADD_GROUP("Shape", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "width"), "set_width", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "start_offset"), "set_start_offset", "get_start_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "end_offset"), "set_end_offset", "get_end_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "side_offset"), "set_side_offset", "get_side_offset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "arrow_size"), "set_arrow_size", "get_arrow_size");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_length"), "set_min_length", "get_min_length");

	ADD_GROUP("Color", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color_end"), "set_color_end", "get_color_end");

	ADD_GROUP("Flags", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editor_only"), "set_editor_only", "get_editor_only");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "double_headed"), "set_double_headed", "get_double_headed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "dashed"), "set_dashed", "get_dashed");
}
