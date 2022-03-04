/*************************************************************************/
/*  sr_graph.cpp                                                         */
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

#include "sr_graph.h"

#include "core/math/math_funcs.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/material.h"

#include <assert.h> /* for assert */
#include <stddef.h> /* for size_t */
#include <string.h> /* for strcmp */
#include <sys/types.h>

/// Godot Control

void SRGraph::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_plot_buf_size", "size"), &SRGraph::set_plot_buf_size);
	ClassDB::bind_method(D_METHOD("get_plot_buf_size"), &SRGraph::get_plot_buf_size);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "plot_buf_size"), "set_plot_buf_size", "get_plot_buf_size");
	ADD_GROUP("Size", "size_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "size_width"), "set_plot_size_width", "get_plot_size_width");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "size_height"), "set_plot_size_height", "get_plot_size_height");
	ADD_GROUP("", "");
	ADD_GROUP("Margins", "margin_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "margin_left"), "set_plot_left_margin", "get_plot_left_margin");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "margin_right"), "set_plot_right_margin", "get_plot_right_margin");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "margin_top"), "set_plot_top_margin", "get_plot_top_margin");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "margin_bottom"), "set_plot_bottom_margin", "get_plot_bottom_margin");
	ADD_GROUP("", "");
}

SRGraph::SRGraph() {
	plot_buf_size = 100;
	plot_size[0] = 1;
	plot_size[1] = 0.1;
	plot_margin[0] = 0; // left
	plot_margin[1] = 0; // right
	plot_margin[2] = 0.9; // top
	plot_margin[3] = 0; // bottom
}

SRGraph::~SRGraph() {}

/// Materials

static const String _sr_vstr = R"(
	attribute vec3 v;
	uniform float ratio;
	varying vec2 coords;
	void vertex() {
		int id = int(v.z);
		vec2 final_ratio = ratio < 1.0 ? vec2(1.0, ratio) : vec2(1.0/ratio, 1.0);
		gl_Position.xy = v.xy * final_ratio;
		gl_Position.zw = vec2(0.0, 1.0);
		bool uzero = (id == 0) || (id == 3) || (id == 5);
		bool vzero = (id == 0) || (id == 3) || (id == 1);
		coords = vec2(uzero ? -1.0 : 1.0, vzero ? -1.0 : 1.0);
	}
)";

// Quads program
static const String _sr_fstr = R"(
	varying vec2 coords;
	uniform vec3 color;
	void fragment() {
		gl_FragColor.rgb = color;
		gl_FragColor.a = 1.0;
	}
)";

// Lines program
static const String _sr_lfstr = R"(
	varying vec2 coords;
	uniform vec3 color;
	void fragment() {
		gl_FragColor.rgb = color;
		gl_FragColor.a = 1.0 - smoothstep(0.5, 1.0, abs(coords.y));
	}
)";

// Points program
static const String _sr_pfstr = R"(
	varying vec2 coords;
	uniform vec3 color;
	uniform bool smoothing;
	void fragment() {
		gl_FragColor = vec4(color, 1.0 - smoothstep(smoothing ? 0.5 : 0.9, 1.0, length(coords)));
	}
)";

enum {
	SH_QUADS,
	SH_LINES,
	SH_POINTS
};

static Ref<Shader> _shaders[3];

/// Internal structs.

typedef struct {
	PoolRealArray buffer;
	Color color;
	real_t param0;
	unsigned param1;
} _sr_curve;

typedef enum {
	lt,
	rt,
	bt,
	tp
} _sr_margin;

typedef struct {
	Color color;
	real_t minx;
	real_t maxx;
	real_t miny;
	real_t maxy;
	real_t ratio;
	real_t margins[4]; // l+r+b+t
	PoolRealArray buffer_axes;
	Color color_axes;
	PoolRealArray buffer_grid;
	Color color_grid;
	Vector<_sr_curve> curves;
	Vector<_sr_curve> curvespoints;
	Vector<_sr_curve> points;
	Vector<_sr_curve> hists;
	bool _dirty;
	Ref<ArrayMesh> _mesh;
} _sr_graph;

typedef struct {
	PoolRealArray buffer_quad;
	unsigned cid;
	unsigned pcid;
	unsigned lcid;
	unsigned rid;
	unsigned prid;
	unsigned lrid;
	unsigned psid;
} _sr_internal_state;

typedef enum {
	RG_VERTICAL,
	RG_HORIZONTAL
} _sr_orientation;

/// Internal variables.

static bool _sr_is_init = false;
static _sr_internal_state _sr_state;
static Vector<_sr_graph> _sr_graphs;

/// Foreward declarations.

static void _sr_internal_setup();
static void _sr_get_line(real_t p0x, real_t p0y, real_t p1x, real_t p1y, real_t w, real_t ratio, PoolRealArray &points);
static void _sr_get_rectangle(real_t p0x, real_t p0y, real_t p1x, real_t p1y, real_t w, PoolRealArray &points);
static void _sr_get_point(real_t p0x, real_t p0y, real_t radius, real_t ratio, PoolRealArray &points);
static void _sr_generate_axis(_sr_orientation orientation, real_t margins[4], real_t ratio, real_t width, real_t mini, real_t maxi, bool axis_on_side, bool reverse, PoolRealArray &axis_data);
static void _sr_generate_curve(const _sr_graph *graph, const PoolRealArray &xs, const PoolRealArray &ys, _sr_curve *curve);
static void _sr_generate_points(const _sr_graph *graph, const PoolRealArray &xs, const PoolRealArray &ys, _sr_curve *curve);
static void _sr_generate_hist(const _sr_graph *graph, const PoolRealArray &ys, _sr_curve *curve);

/// Exposed functions.

int sr_setup(real_t minx, real_t maxx, real_t miny, real_t maxy, real_t ratio, const real_t margins[4], real_t bg_r, real_t bg_g, real_t bg_b) {
	// If we haven't initialized our GD stuff, do it.
	if (!_sr_is_init) {
		_sr_internal_setup();
	}
	// Create a graph with the given infos.
	_sr_graph graph;
	graph.color = Color{ bg_r, bg_g, bg_b };
	graph.minx = minx;
	graph.maxx = maxx;
	graph.miny = miny;
	graph.maxy = maxy;
	graph.ratio = Math::abs(ratio);
	for (int m = 0; m < 4; ++m) {
		graph.margins[m] = MIN(2, MAX(0, Math::abs(margins[m] * 2)));
	}
	graph.buffer_axes = PoolRealArray();
	graph.color_axes = graph.color;
	graph.buffer_grid = PoolRealArray();
	graph.color_grid = graph.color;

	graph._dirty = false;

	// Store it.
	_sr_graphs.push_back(graph);
	return _sr_graphs.size() - 1;
}

void sr_add_axes(int graph_id, real_t width, real_t axis_r, real_t axis_g, real_t axis_b, bool axis_on_side) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return;
	}
	_sr_graph *graph = &_sr_graphs.write[graph_id];
	/// Generate data for axis.
	PoolRealArray axis_data;
	_sr_generate_axis(RG_HORIZONTAL, graph->margins, graph->ratio, width, graph->miny, graph->maxy, axis_on_side, graph->minx > graph->maxx, axis_data);
	_sr_generate_axis(RG_VERTICAL, graph->margins, graph->ratio, width, graph->minx, graph->maxx, axis_on_side, graph->miny > graph->maxy, axis_data);

	graph->color_axes = Color{ axis_r, axis_g, axis_b };
	graph->buffer_axes = axis_data;
}

void sr_add_grid(int graph_id, real_t stepx, real_t stepy, real_t width, real_t lines_r, real_t lines_g, real_t lines_b, bool free_zero) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return;
	}
	_sr_graph *graph = &_sr_graphs.write[graph_id];
	PoolRealArray grid_data;

	const real_t ax = (2 - (graph->margins[lt] + graph->margins[rt])) / (graph->maxx - graph->minx);
	const real_t bx = -1 + graph->margins[rt] - ax * graph->minx;
	const real_t ay = (2 - (graph->margins[bt] + graph->margins[tp])) / (graph->maxy - graph->miny);
	const real_t by = -1 + graph->margins[bt] - ay * graph->miny;

	if (stepx != 0 && graph->maxx != graph->minx) {
		real_t shift_H = Math::abs(ax) * Math::abs(stepx);
		if (free_zero) {
			real_t x_zero = bx;
			while (x_zero < -1 + graph->margins[lt]) {
				x_zero += shift_H;
			}
			while (x_zero > 1.0f - graph->margins[rt]) {
				x_zero -= shift_H;
			}
			for (real_t xi = x_zero; xi >= -1.0f + graph->margins[lt] - 0.0001; xi -= shift_H) {
				_sr_get_line(xi, -1 + graph->margins[bt], xi, 1 - graph->margins[tp], width, graph->ratio, grid_data);
			}
			for (real_t xi = x_zero + shift_H; xi <= 1 - graph->margins[rt] + 0.0001; xi += shift_H) {
				_sr_get_line(xi, -1 + graph->margins[bt], xi, 1 - graph->margins[tp], width, graph->ratio, grid_data);
			}
		} else {
			for (real_t x = -1 + graph->margins[lt]; x <= 1.0f - graph->margins[rt]; x += shift_H) {
				_sr_get_line(x, -1 + graph->margins[bt], x, 1 - graph->margins[tp], width, graph->ratio, grid_data);
			}
			_sr_get_line(1 - graph->margins[rt], -1 + graph->margins[bt], 1.0f - graph->margins[rt], 1 - graph->margins[tp], width, graph->ratio, grid_data);
		}
	}
	if (stepy != 0 && graph->maxy != graph->miny) {
		real_t shiftV = Math::abs(ay) * Math::abs(stepy);
		if (free_zero) {
			real_t y_zero = by;
			while (y_zero < -1 + graph->margins[bt]) {
				y_zero += shiftV;
			}
			while (y_zero > 1 - graph->margins[tp]) {
				y_zero -= shiftV;
			}
			for (real_t yi = y_zero; yi >= -1.0f + graph->margins[bt] - 0.0001; yi -= shiftV) {
				_sr_get_line(-1 + graph->margins[lt], yi, 1 - graph->margins[rt], yi, width, graph->ratio, grid_data);
			}
			for (real_t yi = y_zero + shiftV; yi <= 1.0f - graph->margins[tp] + 0.0001; yi += shiftV) {
				_sr_get_line(-1 + graph->margins[lt], yi, 1 - graph->margins[rt], yi, width, graph->ratio, grid_data);
			}
		} else {
			for (real_t y = -1 + graph->margins[bt]; y < 1 - graph->margins[tp]; y += shiftV) {
				_sr_get_line(-1 + graph->margins[lt], y, 1 - graph->margins[rt], y, width, graph->ratio, grid_data);
			}
			_sr_get_line(-1 + graph->margins[lt], 1 - graph->margins[tp], 1 - graph->margins[rt], 1 - graph->margins[tp], width, graph->ratio, grid_data);
		}
	}
	graph->color_grid = Color{ lines_r, lines_g, lines_b };
	graph->buffer_grid = grid_data;
}

int sr_add_curve(int graph_id, const PoolRealArray &xs, const PoolRealArray &ys, real_t width, real_t color_r, real_t color_g, real_t color_b) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return -1;
	}
	_sr_graph *graph = &_sr_graphs.write[graph_id];
	// Generate the lines.
	_sr_curve curve;
	curve.color = Color{ color_r, color_g, color_b };
	curve.param0 = width;
	_sr_generate_curve(graph, xs, ys, &curve);
	graph->curves.push_back(curve);
	// Generate the points junctions.
	_sr_curve curvepoints;
	curvepoints.color = Color{ color_r, color_g, color_b };
	curvepoints.param0 = width;
	_sr_generate_points(graph, xs, ys, &curvepoints);
	graph->curvespoints.push_back(curvepoints);
	return graph->curves.size() - 1;
}

void sr_update_curve(int graph_id, int curve_id, const PoolRealArray &xs, const PoolRealArray &ys) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return;
	}
	_sr_graph *graph = &_sr_graphs.write[graph_id];
	if (curve_id < 0 || curve_id >= graph->curves.size()) {
		return;
	}
	// Update the lines.
	_sr_curve *curve = &graph->curves.write[curve_id];
	curve->buffer = PoolRealArray();
	_sr_generate_curve(graph, xs, ys, curve);
	// Update the points junctions.
	_sr_curve *curvepoints = &graph->curvespoints.write[curve_id];
	curvepoints->buffer = PoolRealArray();
	_sr_generate_points(graph, xs, ys, curvepoints);
}

int sr_add_points(const int graph_id, const PoolRealArray &xs, const PoolRealArray &ys, const real_t size, const real_t color_r, const real_t color_g, const real_t color_b) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return -1;
	}
	_sr_graph *graph = &_sr_graphs.write[graph_id];
	_sr_curve curve;
	curve.color = Color{ color_r, color_g, color_b };
	curve.param0 = size;
	_sr_generate_points(graph, xs, ys, &curve);
	graph->points.push_back(curve);
	return graph->points.size() - 1;
}

void sr_update_points(const int graph_id, const int curve_id, const PoolRealArray &xs, const PoolRealArray &ys) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return;
	}
	_sr_graph *graph = &_sr_graphs.write[graph_id];
	if (curve_id < 0 || curve_id >= graph->points.size()) {
		return;
	}
	_sr_curve *curve = &graph->points.write[curve_id];
	curve->buffer = PoolRealArray();
	_sr_generate_points(graph, xs, ys, curve);
}

int sr_add_hist(int graph_id, unsigned bins, const PoolRealArray &ys, real_t spacing, real_t color_r, real_t color_g, real_t color_b) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return -1;
	}
	_sr_graph *graph = &_sr_graphs.write[graph_id];
	_sr_curve curve;
	curve.color = Color{ color_r, color_g, color_b };
	curve.param0 = spacing;
	curve.param1 = bins;
	_sr_generate_hist(graph, ys, &curve);
	graph->hists.push_back(curve);
	return graph->hists.size() - 1;
}

void sr_update_hist(int graph_id, int curve_id, const PoolRealArray &ys) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return;
	}
	_sr_graph *graph = &_sr_graphs.write[graph_id];
	if (curve_id < 0 || curve_id >= graph->hists.size()) {
		return;
	}
	_sr_curve *curve = &graph->hists.write[curve_id];
	curve->buffer = PoolRealArray();
	_sr_generate_hist(graph, ys, curve);
}

void sr_draw(CanvasItem *canvas, int graph_id, real_t ratio) {
	if (graph_id < 0 || graph_id >= _sr_graphs.size()) {
		return;
	}

	_sr_graph *graph = &_sr_graphs.write[graph_id];

	const real_t smoothing1 = 1, smoothing0 = 0;
	const real_t final_ratio = (ratio == 0) ? 1 : ratio / graph->ratio;

	auto _add_layer = [final_ratio](const PoolRealArray &buffer, const Color &col, const Ref<Shader> &sh, Ref<ArrayMesh> &mesh) {
		static Array mesh_array;
		mesh_array.resize(VS::ARRAY_MAX);
		mesh_array[VS::ARRAY_VERTEX] = buffer;
		const int idx = mesh->get_surface_count();
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array());
		Ref<ShaderMaterial> mat = memnew(ShaderMaterial);
		mat->set_shader(sh);
		mat->set_shader_param("color", col);
		mat->set_shader_param("ratio", final_ratio);
		mesh->surface_set_material(idx, mat);
		return mat;
	};

	// Rebuild if necessery
	if (graph->_dirty) {
		graph->_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		_add_layer(_sr_state.buffer_quad, graph->color, _shaders[0], graph->_mesh); // Quad
		_add_layer(graph->buffer_grid, graph->color_grid, _shaders[0], graph->_mesh); // Grid
		for (int i = 0; i < graph->hists.size(); ++i) { // Histograms
			_add_layer(graph->hists[i].buffer, graph->hists[i].color, _shaders[0], graph->_mesh);
		}
		for (int i = 0; i < graph->curves.size(); ++i) { // Curves
			_add_layer(graph->curves[i].buffer, graph->curves[i].color, _shaders[1], graph->_mesh);
			_add_layer(graph->curvespoints[i].buffer, graph->curvespoints[i].color, _shaders[2], graph->_mesh)
					->set_shader_param("smoothing", smoothing1);
		}
		_add_layer(graph->buffer_axes, graph->color_axes, _shaders[1], graph->_mesh); // Axes
		for (int i = 0; i < graph->points.size(); ++i) { // Points
			_add_layer(graph->points[i].buffer, graph->points[i].color, _shaders[2], graph->_mesh)
					->set_shader_param("smoothing", smoothing0);
		}
		graph->_dirty = false;
	}

	canvas->draw_mesh(graph->_mesh, Ref<Texture>()); // Draw everything
}

/// Internal functions.

static void _sr_get_line(real_t p0x, real_t p0y, real_t p1x, real_t p1y, real_t w, real_t ratio, PoolRealArray &points) {
	// Compute normal vector.
	real_t dirx = p1x - p0x, diry = p1y - p0y;
	const real_t dir_norm = Math::sqrt(dirx * dirx + diry * diry);
	if (dir_norm != 0) {
		dirx /= dir_norm;
		diry /= dir_norm;
	}

	const real_t norx = -diry;
	const real_t nory = dirx;
	const real_t sdx = w;
	const real_t sdy = ratio * w;
	const real_t dNx = sdx * norx;
	const real_t dNy = sdy * nory;

	const real_t ax = p0x - dNx;
	const real_t ay = p0y - dNy;
	const real_t bx = p1x - dNx;
	const real_t by = p1y - dNy;
	const real_t cx = p1x + dNx;
	const real_t cy = p1y + dNy;
	const real_t dx = p0x + dNx;
	const real_t dy = p0y + dNy;

	points.push_back(ax);
	points.push_back(ay);
	points.push_back((points.size() / 3) % 6);
	points.push_back(bx);
	points.push_back(by);
	points.push_back((points.size() / 3) % 6);
	points.push_back(cx);
	points.push_back(cy);
	points.push_back((points.size() / 3) % 6);
	points.push_back(ax);
	points.push_back(ay);
	points.push_back((points.size() / 3) % 6);
	points.push_back(cx);
	points.push_back(cy);
	points.push_back((points.size() / 3) % 6);
	points.push_back(dx);
	points.push_back(dy);
	points.push_back((points.size() / 3) % 6);
}

static void _sr_get_rectangle(const real_t p0x, const real_t p0y, const real_t p1x, const real_t p1y, const real_t w, PoolRealArray &points) {
	const real_t wx = w * 0.5;
	const real_t ax = p0x - wx;
	const real_t ay = p0y;
	const real_t bx = p0x + wx;
	const real_t by = p0y;
	const real_t cx = p1x + wx;
	const real_t cy = p1y;
	const real_t dx = p1x - wx;
	const real_t dy = p1y;

	points.push_back(ax);
	points.push_back(ay);
	points.push_back((points.size() / 3) % 6);
	points.push_back(bx);
	points.push_back(by);
	points.push_back((points.size() / 3) % 6);
	points.push_back(cx);
	points.push_back(cy);
	points.push_back((points.size() / 3) % 6);
	points.push_back(ax);
	points.push_back(ay);
	points.push_back((points.size() / 3) % 6);
	points.push_back(cx);
	points.push_back(cy);
	points.push_back((points.size() / 3) % 6);
	points.push_back(dx);
	points.push_back(dy);
	points.push_back((points.size() / 3) % 6);
}

static void _sr_get_point(real_t p0x, real_t p0y, real_t radius, real_t ratio, PoolRealArray &points) {
	const real_t wx = radius;
	const real_t wy = radius * ratio;
	const real_t ax = p0x - wx;
	const real_t ay = p0y - wy;
	const real_t bx = p0x + wx;
	const real_t by = p0y - wy;
	const real_t cx = p0x + wx;
	const real_t cy = p0y + wy;
	const real_t dx = p0x - wx;
	const real_t dy = p0y + wy;

	points.push_back(ax);
	points.push_back(ay);
	points.push_back((points.size() / 3) % 6);
	points.push_back(bx);
	points.push_back(by);
	points.push_back((points.size() / 3) % 6);
	points.push_back(cx);
	points.push_back(cy);
	points.push_back((points.size() / 3) % 6);
	points.push_back(ax);
	points.push_back(ay);
	points.push_back((points.size() / 3) % 6);
	points.push_back(cx);
	points.push_back(cy);
	points.push_back((points.size() / 3) % 6);
	points.push_back(dx);
	points.push_back(dy);
	points.push_back((points.size() / 3) % 6);
}

static void _sr_generate_axis(_sr_orientation orientation, real_t margins[4], real_t ratio, real_t width, real_t mini, real_t maxi, bool axis_on_side, bool reverse, PoolRealArray &axis_data) {
	real_t hy;
	// Three positions.
	if (axis_on_side || 0 <= MIN(mini, maxi)) {
		// Axis on the bottom
		hy = -1 + margins[bt];
		if (!axis_on_side && maxi < mini) {
			hy *= -1;
		}
	} else if (0 >= MAX(maxi, mini)) {
		// Axis on the top
		hy = 1 - margins[tp];
		if (maxi < mini) {
			hy *= -1;
		}
	} else {
		// Need to find 0 y coord.
		hy = -(2 - margins[bt] - margins[tp]) * (mini / (maxi - mini)) + margins[bt] - 1;
	}

	const real_t ld = MIN(0.03, 0.3 * (margins[rt] + margins[lt] + margins[bt] + margins[tp]) / 4);
	const real_t rv = orientation == RG_VERTICAL ? ratio : 1;
	const real_t rh = orientation == RG_HORIZONTAL ? ratio : 1;
	const int sh = orientation == RG_VERTICAL ? 0 : 1;
	const real_t hx0 = -1 + margins[lt] - (reverse ? 1.5 * ld * rv : 0.75 * width * rv);
	const real_t hx1 = 1 - margins[rt] + (reverse ? 0.75 * width * rv : 1.5 * ld * rv);
	const real_t ord = reverse ? hx0 : hx1;
	const real_t sn = reverse ? -1 : 1;
	const real_t s0 = sn * (1 + Math::sqrt(2.0) * rv) * width;
	const real_t s1 = sn * ld * rv;
	const real_t s2 = (-sh * 2 + 1) * sn * (ld - Math::sqrt(2.0) * width) * rh;
	const real_t s3 = (-sh * 2 + 1) * sn * (ld + Math::sqrt(2.0) * width) * rh;

	auto axis_data_write = axis_data.write();

	if (orientation == RG_VERTICAL) {
		_sr_get_line(hy, hx0, hy, hx1, width, ratio, axis_data);
		_sr_get_line(hy, ord + sn * width, hy + sn * ld, ord - sn * ld * ratio, width, ratio, axis_data);
	} else {
		_sr_get_line(hx0, hy, hx1, hy, width, ratio, axis_data);
		_sr_get_line(ord + sn * width, hy, ord - sn * ld, hy - sn * ld * ratio, width, ratio, axis_data);
	}
	// Correct vertices positions to join the point of the arrow and the tail angles.
	axis_data_write[axis_data.size() - 3 + sh] = hy;
	axis_data_write[axis_data.size() - 2 - sh] = ord + s0;
	axis_data_write[axis_data.size() - 15 + sh] = hy + s2;
	axis_data_write[axis_data.size() - 12 + sh] = hy + s3;
	axis_data_write[axis_data.size() - 6 + sh] = hy + s3;
	axis_data_write[axis_data.size() - 14 - sh] = ord - s1;
	axis_data_write[axis_data.size() - 11 - sh] = ord - s1;
	axis_data_write[axis_data.size() - 5 - sh] = ord - s1;

	if (orientation == RG_VERTICAL) {
		_sr_get_line(hy, ord + sn * width, hy - sn * ld, ord - sn * ld * ratio, width, ratio, axis_data);
	} else {
		_sr_get_line(ord + sn * width, hy, ord - sn * ld, hy + sn * ld * ratio, width, ratio, axis_data);
	}
	// Correct vertices positions to join the point of the arrow and the tail angles.
	axis_data_write[axis_data.size() - 9 + sh] = hy;
	axis_data_write[axis_data.size() - 18 + sh] = hy;
	axis_data_write[axis_data.size() - 8 - sh] = ord + s0;
	axis_data_write[axis_data.size() - 17 - sh] = ord + s0;
	axis_data_write[axis_data.size() - 15 + sh] = hy - s3;
	axis_data_write[axis_data.size() - 12 + sh] = hy - s2;
	axis_data_write[axis_data.size() - 6 + sh] = hy - s2;
	axis_data_write[axis_data.size() - 14 - sh] = ord - s1;
	axis_data_write[axis_data.size() - 11 - sh] = ord - s1;
	axis_data_write[axis_data.size() - 5 - sh] = ord - s1;
}

static void _sr_generate_curve(const _sr_graph *graph, const PoolRealArray &xs, const PoolRealArray &ys, _sr_curve *curve) {
	if (xs.size() != ys.size() || xs.size() == 0) {
		return;
	}

	const real_t ax = (2 - (graph->margins[lt] + graph->margins[rt])) / (graph->maxx - graph->minx);
	const real_t bx = -1 + graph->margins[lt] - ax * graph->minx;
	const real_t ay = (2 - (graph->margins[bt] + graph->margins[tp])) / (graph->maxy - graph->miny);
	const real_t by = -1 + graph->margins[bt] - ay * graph->miny;

	PoolRealArray curve_data;
	real_t x0 = ax * xs[0] + bx;
	real_t y0 = ay * ys[0] + by;
	for (int i = 1; i < xs.size(); ++i) {
		const real_t x1 = ax * xs[i] + bx;
		const real_t y1 = ay * ys[i] + by;
		_sr_get_line(x0, y0, x1, y1, curve->param0, graph->ratio, curve_data);
		x0 = x1;
		y0 = y1;
	}

	curve->buffer = curve_data;
}

static void _sr_generate_points(const _sr_graph *graph, const PoolRealArray &xs, const PoolRealArray &ys, _sr_curve *curve) {
	if (xs.size() != ys.size() || xs.size() == 0) {
		return;
	}
	const real_t ax = (2 - (graph->margins[lt] + graph->margins[rt])) / (graph->maxx - graph->minx);
	const real_t bx = -1 + graph->margins[lt] - ax * graph->minx;
	const real_t ay = (2 - (graph->margins[bt] + graph->margins[tp])) / (graph->maxy - graph->miny);
	const real_t by = -1 + graph->margins[bt] - ay * graph->miny;

	PoolRealArray curve_data;

	for (int i = 0; i < xs.size(); ++i) {
		const real_t x0 = ax * xs[i] + bx;
		const real_t y0 = ay * ys[i] + by;
		_sr_get_point(x0, y0, curve->param0, graph->ratio, curve_data);
	}

	curve->buffer = curve_data, curve_data.size();
}

static void _sr_generate_hist(const _sr_graph *graph, const PoolRealArray &ys, _sr_curve *curve) {
	if (ys.size() == 0) {
		return;
	}
	const real_t bin_size = (graph->maxx - graph->minx) / (real_t)curve->param1;
	PoolIntArray bin_counts;
	bin_counts.resize(curve->param1);
	auto bin_counts_write = bin_counts.write();
	for (int i = 0; i < ys.size(); ++i) {
		const int j = (unsigned)Math::floor((ys[i] - graph->minx) / bin_size);
		if (j < 0 || j >= bin_counts.size()) {
			continue;
		}
		bin_counts_write[j] += 1;
	}
	bin_counts_write.release();

	PoolRealArray hist_data;

	const real_t ax = (2 - (graph->margins[lt] + graph->margins[rt])) / (graph->maxx - graph->minx);
	const real_t bx = -1 + graph->margins[lt] - ax * graph->minx;
	const real_t ay = (2 - (graph->margins[bt] + graph->margins[tp])) / (graph->maxy - graph->miny);
	const real_t by = -1 + graph->margins[bt] - ay * graph->miny;

	const real_t bin_width = MAX(0, (2 - graph->margins[lt] - graph->margins[rt]) / (real_t)curve->param1 - curve->param0);

	for (unsigned i = 0; i < curve->param1; ++i) {
		if (bin_counts[i] == 0) {
			continue;
		}
		real_t x0 = ax * (graph->minx + (real_t)(i + 0.5) * bin_size) + bx;
		real_t y0 = by;
		real_t y1 = ay * (real_t)bin_counts[i] + by;
		_sr_get_rectangle(x0, y0, x0, y1, bin_width, hist_data);
	}
	curve->buffer = hist_data;
}

static void _sr_internal_setup() {
	// build all materials
	_shaders[0] = Ref<Shader>(memnew(Shader));
	_shaders[0]->set_code(_sr_vstr + _sr_fstr);
	_shaders[1] = Ref<Shader>(memnew(Shader));
	_shaders[1]->set_code(_sr_vstr + _sr_lfstr);
	_shaders[2] = Ref<Shader>(memnew(Shader));
	_shaders[2]->set_code(_sr_vstr + _sr_pfstr);
	_sr_is_init = true;
}

// Get color from palette: warm, cool or neon.
unsigned *rg_palette(const char *name, int num_colors) {
	static unsigned warm[][12] = {
		{},
		{ 0xFDB25F },
		{ 0xFFC96B, 0xF47942 },
		{ 0xFFC96B, 0xF47942, 0xAB412C },
		{ 0xFFD773, 0xF99851, 0xEF5833, 0x923E2D },
		{ 0xFFE67A, 0xFCAF5E, 0xF47942, 0xDA492D, 0x773A2D },
		{ 0xFFE77E, 0xFEBD65, 0xF78F4D, 0xF16137, 0xC1452C, 0x6F382E },
		{ 0xFFE782, 0xFFC76A, 0xFAA055, 0xF47942, 0xEF512F, 0xAE422C, 0x67382E },
		{ 0xFFE886, 0xFFD06F, 0xFBAD5D, 0xF68A4B, 0xF16739, 0xDE4A2D, 0xA0402C, 0x5F362F },
		{ 0xFFE98B, 0xFFD873, 0xFEB862, 0xF99851, 0xF47942, 0xEF5833, 0xCA462D, 0x913D2D, 0x573530 },
		{ 0xFFE98E, 0xFFDF76, 0xFFC267, 0xFAA458, 0xF68749, 0xF26A3A, 0xEE4C2D, 0xB9442C, 0x843C2D, 0x4F3330 },
		{ 0xFFE98E, 0xFFE278, 0xFFC76A, 0xFBAD5D, 0xF8934F, 0xF47942, 0xF05D35, 0xDE4A2D, 0xAE422C, 0x7F3B2D, 0x4F3330 },
		{ 0xFFE98E, 0xFFE479, 0xFFE479, 0xFEB460, 0xFA9C53, 0xF58547, 0xF58547, 0xEE5531, 0xD1492D, 0xA5402C, 0x7A3A2D, 0x4F3330 }
	};
	static unsigned cool[][12] = {
		{ 0x41BED1 },
		{ 0x78CCCF, 0x0B88B4 },
		{ 0x78CCCF, 0x0B88B4, 0x00357E },
		{ 0x96D5CD, 0x0CAAC9, 0x07669E, 0x002C71 },
		{ 0xB2DFCC, 0x3BBCD0, 0x0B88B4, 0x044F8F, 0x042561 },
		{ 0xBAE0CD, 0x5CC5CF, 0x0DA0C3, 0x096FA4, 0x044186, 0x05255C },
		{ 0xC3E2CC, 0x75CBCF, 0x10B1CE, 0x0B88B4, 0x0B88B4, 0x01377F, 0x052257 },
		{ 0xCCE5CC, 0x86D1CE, 0x33BBD1, 0x0C9AC0, 0x0A75A7, 0x065090, 0x002F79, 0x051F53 },
		{ 0xD4E8CC, 0x96D5CD, 0x52C2D0, 0x0CAAC9, 0x0B88B4, 0x07669E, 0x034589, 0x012B70, 0x061D4E },
		{ 0xDCEBCB, 0xA3DACD, 0x67C7CF, 0x0DB6D1, 0x0B97BE, 0x0B78A9, 0x065A96, 0x023D83, 0x042868, 0x061C49 },
		{ 0xDCEBCB, 0xA9DCCD, 0x75CBCF, 0x33BBD1, 0x33BBD1, 0x33BBD1, 0x096BA2, 0x065090, 0x01377F, 0x032766, 0x032766 },
		{ 0xDCEBCB, 0xAEDDCD, 0x7ECECE, 0x48C0D1, 0x10ADCC, 0x0B94BC, 0x0B7BAB, 0x09639B, 0x034A8B, 0x00337C, 0x032663, 0x061C49 }
	};
	static unsigned neon[][12] = {
		{ 0xF3648B },
		{ 0xF78495, 0xAD3E8C },
		{ 0xF78495, 0xAD3E8C, 0x502370 },
		{ 0xF9989B, 0xDE4C86, 0x7D3392, 0x461E63 },
		{ 0xFCACA4, 0xF2618A, 0xAD3E8C, 0x612B8A, 0x3C1955 },
		{ 0xFDB3A5, 0xF5738F, 0xD04888, 0x8A3590, 0x58277C, 0x3A1951 },
		{ 0xFEB9A7, 0xF68294, 0xEA5084, 0xAD3E8C, 0x703093, 0x512472, 0x37164C },
		{ 0xFFBFAA, 0xF88F98, 0xF25E89, 0xC84688, 0x93378F, 0x632C8C, 0x4B2069, 0x331548 },
		{ 0xFFC5AC, 0xF9999C, 0xF46E8D, 0xDE4C86, 0xAD3E8C, 0x7C3292, 0x5B2981, 0x451E62, 0x301443 },
		{ 0xFFCBAE, 0xFAA2A0, 0xF57A91, 0xF05384, 0xC44488, 0x97388E, 0x692F94, 0x552578, 0x411C5C, 0x2D133F },
		{ 0xFFCBAE, 0xFBA6A2, 0xF68294, 0xF25E89, 0xD64A87, 0xAD3E8C, 0x853491, 0x632C8C, 0x512472, 0x3F1B59, 0x2D133F },
		{ 0xFFCBAE, 0xFBAAA3, 0xF78997, 0xF3688C, 0xE54F85, 0xBF4489, 0x9C398D, 0x753193, 0x5E2985, 0x4D226D, 0x3D1A57, 0x2D133F }
	};
	if (strcmp(name, "warm") == 0) {
		switch (num_colors) {
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				return warm[num_colors];
			default:
				return 0;
		}
	} else if (strcmp(name, "cool") == 0) {
		switch (num_colors) {
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				return cool[num_colors];
			default:
				return 0;
		}
	} else if (strcmp(name, "neon") == 0) {
		switch (num_colors) {
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				return neon[num_colors];
			default:
				return 0;
		}
	}
	return 0;
}
