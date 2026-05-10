/**************************************************************************/
/*  godot_draw.cpp                                                        */
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

#include "core/math/geometry.h"
#include "servers/visual_server.h"

#include "godot_draw.h"

void gd_draw_circle(const RID &p_to_rid, const Point2 &p_pos, float p_radius, int p_vertices, const Color &p_color) {
	Vector<Vector2> points;
	for (int i = 0; i < p_vertices; i++) {
		points.push_back(p_pos + Vector2(Math::cos(i * Math_PI * 2 / 24.0), Math::sin(i * Math_PI * 2 / 24.0)) * p_radius);
	}

	int vertex_count = points.size();
	VisualServer::get_singleton()->canvas_item_add_line(p_to_rid, p_pos, points[0], p_color, 1.0f);
	for (int i = 0; i < vertex_count; i++) {
		Vector2 p = points[i];
		Vector2 n = points[(i + 1) % vertex_count];
		VisualServer::get_singleton()->canvas_item_add_line(p_to_rid, p, n, p_color, 1.0f);
	}

	Vector<Color> col;
	col.push_back(p_color);
	VisualServer::get_singleton()->canvas_item_add_polygon(p_to_rid, points, col);
}

void gd_draw_rect(const RID &p_to_rid, float p_width, float p_height, const Color &p_color) {
	Vector<Vector2> points;

	const real_t hx = p_width * 0.5f;
	const real_t hy = p_height * 0.5f;
	points.push_back(Vector2(hx, hy));
	points.push_back(Vector2(-hx, hy));
	points.push_back(Vector2(-hx, -hy));
	points.push_back(Vector2(hx, -hy));

	int vertex_count = points.size();
	for (int i = 0; i < vertex_count; i++) {
		Vector2 p = points[i];
		Vector2 n = points[(i + 1) % vertex_count];
		VisualServer::get_singleton()->canvas_item_add_line(p_to_rid, p, n, p_color, 1.0f);
	}

	Vector<Color> col;
	col.push_back(p_color);
	VisualServer::get_singleton()->canvas_item_add_polygon(p_to_rid, points, col);
}

void gd_draw_arrow(const RID &p_to_rid, const Vector2 &start, const Vector2 &end, const Color &p_color, float p_width) {
	Vector2 norm = (end - start).normalized();
	VisualServer::get_singleton()->canvas_item_add_line(p_to_rid, start, end, p_color, p_width);
	VisualServer::get_singleton()->canvas_item_add_line(p_to_rid, end, end - norm.rotated(Math_PI * 0.17f) * 4.0f, p_color, p_width);
	VisualServer::get_singleton()->canvas_item_add_line(p_to_rid, end, end - norm.rotated(-Math_PI * 0.17f) * 4.0f, p_color, p_width);
}
