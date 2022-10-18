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

//  Reference:
//  ----------
//   - https://github.com/armadillu/ofxHistoryPlot/blob/master/src/ofxHistoryPlot.cpp
//   - https://www.desultoryquest.com/blog/real-time-plotting-on-windows-using-opengl
//   - https://github.com/TetsuakiBaba/ofxGraph
//   - https://github.com/fulltrend/ofxChart/tree/master/ofxChart
//

#include "sr_graph.h"

#include "common/gd_core.h"
#include "core/error_macros.h"
#include "core/int_types.h"
#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/math/transform_2d.h"
#include "core/os/memory.h"
#include "core/variant.h"
#include "core/vector.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/material.h"

#include "misc/c++/handle_map.h"

/// Graph public interface

sr_graph_t sr_setup_graph(real_t minx, real_t maxx, real_t miny, real_t maxy, real_t ratio, const Color &bg, const String &label = "");
void sr_cleanup();
void sr_add_axes(sr_graph_t hgraph, const Color &color, bool axis_on_side);
void sr_add_grid(sr_graph_t hgraph, real_t stepx, real_t stepy, const Color &color, bool from_zero);
int sr_add_curve(sr_graph_t hgraph, const Vector<real_t> &xs, const Vector<real_t> &ys, const Color &color);
void sr_update_curve(sr_graph_t hgraph, int curve_id, const Vector<real_t> &xs, const Vector<real_t> &ys);
int sr_add_hist(sr_graph_t hgraph, unsigned bins, const Vector<real_t> &ys, real_t spacing, const Color &color);
void sr_update_hist(sr_graph_t hgraph, int hist_id, const Vector<real_t> &ys);
int sr_add_points(sr_graph_t hgraph, const Vector<real_t> &xs, const PoolRealArray &ys, real_t size, const Color &color);
void sr_update_points(sr_graph_t hgraph, int points_id, const Vector<real_t> &xs, const Vector<real_t> &ys);
int sr_add_stack(sr_graph_t hgraph, real_t weight, const Vector<real_t> &vs, char *pal);
void sr_update_stack(sr_graph_t hgraph, int stack_id, const Vector<real_t> &vs);
void sr_draw(CanvasItem *canvas, sr_graph_t hgraph, Size2 frame);
void sr_draw(CanvasItem *canvas, Size2 frame);
unsigned *sr_palette(int pal, int num_colors);

/// Internal structs.

typedef struct {
	PoolVector2Array buffer;
	Color color;
	real_t param0;
	unsigned param1;
} _sr_curve;

typedef struct {
	String label;
	Color color;
	real_t minx, maxx;
	real_t miny, maxy;
	real_t ratio;
	struct {
		PoolVector2Array buffer;
		Color color;
		Mesh::PrimitiveType primitive;
	} axes;
	struct {
		PoolVector2Array buffer;
		Color color;
		Mesh::PrimitiveType primitive;
	} grid;
	Vector<_sr_curve> curves;
	Vector<_sr_curve> curvespoints;
	Vector<_sr_curve> points;
	Vector<_sr_curve> hists;
	bool _dirty;
	Ref<ArrayMesh> _mesh;
} _sr_graph;

typedef enum {
	pal_warm,
	pal_cool,
	pal_neon,
} _sr_pal;

typedef enum {
	orient_vert,
	orient_horiz,
} _sr_orientation;

/// Forward declarations.

static handle_map<_sr_graph *> _handles(1, 32);

static _FORCE_INLINE_ _sr_graph *_from_handle(sr_graph_t hgraph) {
	const Id_T t = make_handle(hgraph);
	if (_handles.is_valid(t)) {
		return _handles[t];
	} else {
		return nullptr;
	}
}

static void _sr_get_line(real_t p0x, real_t p0y, real_t p1x, real_t p1y, real_t ratio, PoolVector2Array &points);
static void _sr_get_rectangle(real_t p0x, real_t p0y, real_t p1x, real_t p1y, real_t w, PoolVector2Array &points);
static void _sr_get_point(real_t p0x, real_t p0y, real_t radius, real_t ratio, PoolVector2Array &points);
static void _sr_generate_axis(_sr_orientation orientation, real_t ratio, real_t mini, real_t maxi, bool axis_on_side, bool reverse, PoolVector2Array &axis_data);
static int _sr_generate_curve(const _sr_graph *graph, const Vector<real_t> &xs, const Vector<real_t> &ys, _sr_curve *curve);
static int _sr_generate_points(const _sr_graph *graph, const Vector<real_t> &xs, const Vector<real_t> &ys, _sr_curve *curve);
static void _sr_generate_hist(const _sr_graph *graph, const Vector<real_t> &ys, _sr_curve *curve);
static void _sr_draw(CanvasItem *canvas, _sr_graph *graph, Size2 frame);

/// Exposed functions.

sr_graph_t sr_setup_graph(real_t minx, real_t maxx, real_t miny, real_t maxy, real_t ratio, const Color &bg, const String &label) {
	// create a graph with the given infos
	_sr_graph *graph = memnew(_sr_graph);
	graph->label = label;
	graph->color = bg;
	graph->minx = minx;
	graph->maxx = maxx;
	graph->miny = miny;
	graph->maxy = maxy;
	graph->ratio = ratio;
	graph->axes = { PoolVector2Array(), bg };
	graph->grid = { PoolVector2Array(), bg };
	graph->_dirty = true;

	return _handles.insert(graph).value;
}

void sr_cleanup() {
	for (_sr_graph *h : _handles) {
		memdelete(h);
	}
	_handles.reset();
}

void sr_add_axes(sr_graph_t hgraph, const Color &color, bool axis_on_side) {
	_sr_graph *graph = _from_handle(hgraph);
	// generate data for axis
	PoolVector2Array data;
	_sr_generate_axis(orient_horiz, graph->ratio, graph->miny, graph->maxy, axis_on_side, graph->minx > graph->maxx, data);
	_sr_generate_axis(orient_vert, graph->ratio, graph->minx, graph->maxx, axis_on_side, graph->miny > graph->maxy, data);

	graph->axes = { data, color, Mesh::PRIMITIVE_LINES };
	graph->_dirty = true;
}

void sr_add_grid(sr_graph_t hgraph, real_t stepx, real_t stepy, const Color &color, bool from_zero) {
	_sr_graph *graph = _from_handle(hgraph);
	PoolVector2Array data;

	const real_t ax = 2 / (graph->maxx - graph->minx);
	const real_t bx = -1 - ax * graph->minx;
	const real_t ay = 2 / (graph->maxy - graph->miny);
	const real_t by = -1 - ay * graph->miny;

	if (stepx != 0 && graph->maxx != graph->minx) {
		const real_t shift_h = Math::abs(ax) * Math::abs(stepx);
		if (from_zero) {
			real_t x_zero = bx;
			while (x_zero < -1) {
				x_zero += shift_h;
			}
			while (x_zero > 1) {
				x_zero -= shift_h;
			}
			for (real_t xi = x_zero; xi >= -1 - 0.0001; xi -= shift_h) {
				_sr_get_line(xi, -1, xi, 1, graph->ratio, data);
			}
			for (real_t xi = x_zero + shift_h; xi <= 1 + 0.0001; xi += shift_h) {
				_sr_get_line(xi, -1, xi, 1, graph->ratio, data);
			}
		} else {
			for (real_t x = -1; x <= 1; x += shift_h) {
				_sr_get_line(x, -1, x, 1, graph->ratio, data);
			}
			_sr_get_line(1, -1, 1, 1, graph->ratio, data);
		}
	}
	if (stepy != 0 && graph->maxy != graph->miny) {
		const real_t shift_v = Math::abs(ay) * Math::abs(stepy);
		if (from_zero) {
			real_t y_zero = by;
			while (y_zero < -1) {
				y_zero += shift_v;
			}
			while (y_zero > 1) {
				y_zero -= shift_v;
			}
			for (real_t yi = y_zero; yi >= -1 - 0.0001; yi -= shift_v) {
				_sr_get_line(-1, yi, 1, yi, graph->ratio, data);
			}
			for (real_t yi = y_zero + shift_v; yi <= 1 + 0.0001; yi += shift_v) {
				_sr_get_line(-1, yi, 1, yi, graph->ratio, data);
			}
		} else {
			for (real_t y = -1; y < 1; y += shift_v) {
				_sr_get_line(-1, y, 1, y, graph->ratio, data);
			}
			_sr_get_line(-1, 1, 1, 1, graph->ratio, data);
		}
	}
	graph->grid = { data, color, Mesh::PRIMITIVE_LINES };
	graph->_dirty = true;
}

int sr_add_curve(sr_graph_t hgraph, const Vector<real_t> &xs, const Vector<real_t> &ys, const Color &color) {
	_sr_graph *graph = _from_handle(hgraph);
	// generate the lines
	_sr_curve curve;
	curve.color = color;
	_sr_generate_curve(graph, xs, ys, &curve);
	graph->curves.push_back(curve);
	// generate the points junctions
	_sr_curve curvepoints;
	curvepoints.color = color;
	_sr_generate_points(graph, xs, ys, &curvepoints);
	const int cid = graph->curves.size();
	graph->curvespoints.push_back(curvepoints);
	graph->_dirty = true;
	return cid;
}

void sr_update_curve(sr_graph_t hgraph, int curve_id, const Vector<real_t> &xs, const Vector<real_t> &ys) {
	_sr_graph *graph = _from_handle(hgraph);
	// update the lines
	_sr_curve *curve = &graph->curves.write[curve_id];
	curve->buffer = PoolVector2Array();
	_sr_generate_curve(graph, xs, ys, curve);
	// update the points junctions
	_sr_curve *curvepoints = &graph->curvespoints.write[curve_id];
	curvepoints->buffer = PoolVector2Array();
	_sr_generate_points(graph, xs, ys, curvepoints);
	graph->_dirty = true;
}

int sr_add_points(sr_graph_t hgraph, const Vector<real_t> &xs, const Vector<real_t> &ys, const real_t size, const Color &color) {
	_sr_graph *graph = _from_handle(hgraph);
	ERR_FAIL_NULL_V_MSG(graph, -1, "Invalid graph handle");
	_sr_curve curve;
	curve.color = color;
	curve.param0 = size;
	_sr_generate_points(graph, xs, ys, &curve);
	graph->points.push_back(curve);
	return graph->points.size() - 1;
}

void sr_update_points(sr_graph_t hgraph, int curve_id, const Vector<real_t> &xs, const Vector<real_t> &ys) {
	_sr_graph *graph = _from_handle(hgraph);
	ERR_FAIL_NULL_MSG(graph, "Invalid graph handle");
	ERR_FAIL_INDEX_MSG(curve_id, graph->points.size(), "Invalid curve indec");
	_sr_curve *curve = &graph->points.write[curve_id];
	curve->buffer = PoolVector2Array();
	_sr_generate_points(graph, xs, ys, curve);
}

int sr_add_hist(sr_graph_t hgraph, unsigned bins, const Vector<real_t> &ys, real_t spacing, const Color &color) {
	_sr_graph *graph = _from_handle(hgraph);
	ERR_FAIL_NULL_V_MSG(graph, -1, "Invalid graph handle");
	_sr_curve curve;
	curve.color = color;
	curve.param0 = spacing;
	curve.param1 = bins;
	_sr_generate_hist(graph, ys, &curve);
	graph->hists.push_back(curve);
	return graph->hists.size() - 1;
}

void sr_update_hist(sr_graph_t hgraph, int curve_id, const Vector<real_t> &ys) {
	_sr_graph *graph = _from_handle(hgraph);
	ERR_FAIL_NULL_MSG(graph, "Invalid graph handle");
	ERR_FAIL_INDEX_MSG(curve_id, graph->points.size(), "Invalid curve indec");
	_sr_curve *curve = &graph->hists.write[curve_id];
	curve->buffer = PoolVector2Array();
	_sr_generate_hist(graph, ys, curve);
}

void sr_draw(CanvasItem *canvas, sr_graph_t hgraph, Size2 frame) {
	_sr_graph *graph = _from_handle(hgraph);
	_sr_draw(canvas, graph, frame);
}

void sr_draw(CanvasItem *canvas, Size2 frame) {
	for (_sr_graph *graph : _handles) {
		_sr_draw(canvas, graph, frame);
	}
}

/// Internal functions.

static void _sr_get_line(real_t p0x, real_t p0y, real_t p1x, real_t p1y, real_t ratio, PoolVector2Array &points) {
	points.push_back({ p0x, p0y });
	points.push_back({ p1x, p1y });
}

static void _sr_get_rectangle(const real_t p0x, const real_t p0y, const real_t p1x, const real_t p1y, const real_t w, PoolVector2Array &points) {
	const real_t wx = w * 0.5;
	const real_t ax = p0x - wx;
	const real_t ay = p0y;
	const real_t bx = p0x + wx;
	const real_t by = p0y;
	const real_t cx = p1x + wx;
	const real_t cy = p1y;
	const real_t dx = p1x - wx;
	const real_t dy = p1y;

	points.push_back({ ax, ay });
	points.push_back({ bx, by });
	points.push_back({ cx, cy });
	points.push_back({ ax, ay });
	points.push_back({ cx, cy });
	points.push_back({ dx, dy });
}

static void _sr_get_point(real_t p0x, real_t p0y, real_t radius, real_t ratio, PoolVector2Array &points) {
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

	points.push_back({ ax, ay });
	points.push_back({ bx, by });
	points.push_back({ cx, cy });
	points.push_back({ ax, ay });
	points.push_back({ cx, cy });
	points.push_back({ dx, dy });
}

static void _sr_generate_axis(_sr_orientation orientation, real_t ratio, real_t mini, real_t maxi, bool axis_on_side, bool reverse, PoolVector2Array &axis_data) {
	real_t hy;
	// three positions.
	if (axis_on_side || 0 <= MIN(mini, maxi)) {
		// axis on the bottom
		hy = -1;
		if (!axis_on_side && maxi < mini) {
			hy *= -1;
		}
	} else if (0 >= MAX(maxi, mini)) {
		// axis on the top
		hy = 1;
		if (maxi < mini) {
			hy *= -1;
		}
	} else {
		// need to find 0 y coord.
		hy = -2 * (mini / (maxi - mini)) - 1;
	}

	const real_t ld = 0.03;
	const real_t rv = orientation == orient_vert ? ratio : 1;
	const real_t hx0 = -1 + (reverse ? 1.5 * ld * rv : 0);
	const real_t hx1 = 1 - (reverse ? 0 : 1.5 * ld * rv);
	const real_t ord = reverse ? hx0 : hx1;
	const real_t sn = reverse ? -1 : 1;

	if (orientation == orient_vert) {
		// |     |   \
		// |     |    \
		// |     |  +--+
		// +--+  |  |
		//   /   |  |
		//  /    |  |
		_sr_get_line(hy, hx0, hy, hx1, ratio, axis_data);
		_sr_get_line(hy, ord, hy + ld, ord, ratio, axis_data);
		_sr_get_line(hy + ld, ord, hy, sn, ratio, axis_data);
	} else {
		// -----+   /  |  \   +-----
		//      |  /   |   \  |
		//      | /    |    \ |
		//      +      |     +
		_sr_get_line(hx0, hy, hx1, hy, ratio, axis_data);
		_sr_get_line(ord, hy, ord, hy + ld * ratio, ratio, axis_data);
		_sr_get_line(sn, hy, ord, hy + ld * ratio, ratio, axis_data);
	}
}

static int _sr_generate_curve(const _sr_graph *graph, const Vector<real_t> &xs, const Vector<real_t> &ys, _sr_curve *curve) {
	if (xs.size() == 0 || ys.size() == 0) {
		return 0;
	}

	const real_t ax = 2 / (graph->maxx - graph->minx);
	const real_t bx = -1 - ax * graph->minx;
	const real_t ay = 2 / (graph->maxy - graph->miny);
	const real_t by = -1 - ay * graph->miny;

	PoolVector2Array data;
	real_t x0 = ax * xs[0] + bx;
	real_t y0 = ay * ys[0] + by;
	for (int i = 1; i < MIN(xs.size(), ys.size()); ++i) {
		const real_t x1 = ax * xs[i] + bx;
		const real_t y1 = ay * ys[i] + by;
		_sr_get_line(x0, y0, x1, y1, graph->ratio, data);
		x0 = x1;
		y0 = y1;
	}

	curve->buffer = data;

	return data.size();
}

static int _sr_generate_points(const _sr_graph *graph, const Vector<real_t> &xs, const Vector<real_t> &ys, _sr_curve *curve) {
	if (ys.size() == 0 || xs.size() == 0) {
		return 0;
	}
	const real_t ax = 2 / (graph->maxx - graph->minx);
	const real_t bx = -1 - ax * graph->minx;
	const real_t ay = 2 / (graph->maxy - graph->miny);
	const real_t by = -1 - ay * graph->miny;

	PoolVector2Array curve_data;

	for (int i = 0; i < MIN(xs.size(), ys.size()); ++i) {
		const real_t x0 = ax * xs[i] + bx;
		const real_t y0 = ay * ys[i] + by;
		_sr_get_point(x0, y0, curve->param0, graph->ratio, curve_data);
	}

	curve->buffer = curve_data;

	return curve_data.size();
}

static void _sr_generate_hist(const _sr_graph *graph, const Vector<real_t> &ys, _sr_curve *curve) {
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

	PoolVector2Array data;

	const real_t ax = 2 / (graph->maxx - graph->minx);
	const real_t bx = -1 - ax * graph->minx;
	const real_t ay = 2 / (graph->maxy - graph->miny);
	const real_t by = -1 - ay * graph->miny;

	const real_t bin_width = MAX(0, (2.0 / curve->param1 - curve->param0));

	for (unsigned i = 0; i < curve->param1; ++i) {
		if (bin_counts[i] == 0) {
			continue;
		}
		real_t x0 = ax * (graph->minx + (i + 0.5) * bin_size) + bx;
		real_t y0 = by;
		real_t y1 = ay * bin_counts[i] + by;
		_sr_get_rectangle(x0, y0, x0, y1, bin_width, data);
	}
	curve->buffer = data;
}

static void _sr_draw(CanvasItem *canvas, _sr_graph *graph, Size2 frame) {
	ERR_FAIL_NULL_MSG(graph, "Invalid graph handle");
	auto _add_layer = [](Mesh::PrimitiveType primitive, const PoolVector2Array &buffer, const Color &color, Ref<ArrayMesh> &mesh) {
		if (buffer.size()) {
			Array mesh_array;
			mesh_array.resize(VS::ARRAY_MAX);
			mesh_array[VS::ARRAY_VERTEX] = buffer;
			PoolColorArray colors;
			colors.resize(buffer.size());
			colors.fill(color);
			mesh_array[VS::ARRAY_COLOR] = colors;
			mesh->add_surface_from_arrays(primitive, mesh_array);
		}
	};

	// rebuild if necessery
	if (graph->_dirty) {
		if (graph->_mesh) {
			graph->_mesh->clear_mesh();
		} else {
			graph->_mesh = newref(ArrayMesh);
		}
		_add_layer(graph->grid.primitive, graph->grid.buffer, graph->grid.color, graph->_mesh); // grid
		for (int i = 0; i < graph->hists.size(); ++i) { // histograms
			_add_layer(Mesh::PRIMITIVE_TRIANGLES, graph->hists[i].buffer, graph->hists[i].color, graph->_mesh);
		}
		for (int i = 0; i < graph->curves.size(); ++i) { // curves
			_add_layer(Mesh::PRIMITIVE_LINES, graph->curves[i].buffer, graph->curves[i].color, graph->_mesh);
			_add_layer(Mesh::PRIMITIVE_POINTS, graph->curvespoints[i].buffer, graph->curvespoints[i].color, graph->_mesh);
		}
		_add_layer(graph->axes.primitive, graph->axes.buffer, graph->axes.color, graph->_mesh); // axes
		for (int i = 0; i < graph->points.size(); ++i) { // points
			_add_layer(Mesh::PRIMITIVE_TRIANGLES, graph->points[i].buffer, graph->points[i].color, graph->_mesh);
		}
		graph->_dirty = false;
	}

	canvas->draw_mesh(graph->_mesh, Ref<Texture>(), Ref<Texture>(), Ref<Texture>(), Transform2D().translated(Vector2(1, 1)).scaled(frame)); // draw everything
}

// get rgb color from palette: warm, cool or neon.
unsigned *sr_palette(int pal, int num_colors) {
	static unsigned warm[][12] = {
		{ 0xfdb25f },
		{ 0xffc96b, 0xf47942 },
		{ 0xffc96b, 0xf47942, 0xab412c },
		{ 0xffd773, 0xf99851, 0xef5833, 0x923e2d },
		{ 0xffe67a, 0xfcaf5e, 0xf47942, 0xda492d, 0x773a2d },
		{ 0xffe77e, 0xfebd65, 0xf78f4d, 0xf16137, 0xc1452c, 0x6f382e },
		{ 0xffe782, 0xffc76a, 0xfaa055, 0xf47942, 0xef512f, 0xae422c, 0x67382e },
		{ 0xffe886, 0xffd06f, 0xfbad5d, 0xf68a4b, 0xf16739, 0xde4a2d, 0xa0402c, 0x5f362f },
		{ 0xffe98b, 0xffd873, 0xfeb862, 0xf99851, 0xf47942, 0xef5833, 0xca462d, 0x913d2d, 0x573530 },
		{ 0xffe98e, 0xffdf76, 0xffc267, 0xfaa458, 0xf68749, 0xf26a3a, 0xee4c2d, 0xb9442c, 0x843c2d, 0x4f3330 },
		{ 0xffe98e, 0xffe278, 0xffc76a, 0xfbad5d, 0xf8934f, 0xf47942, 0xf05d35, 0xde4a2d, 0xae422c, 0x7f3b2d, 0x4f3330 },
		{ 0xffe98e, 0xffe479, 0xffe479, 0xfeb460, 0xfa9c53, 0xf58547, 0xf58547, 0xee5531, 0xd1492d, 0xa5402c, 0x7a3a2d, 0x4f3330 }
	};
	static unsigned cool[][12] = {
		{ 0x41bed1 },
		{ 0x78cccf, 0x0b88b4 },
		{ 0x78cccf, 0x0b88b4, 0x00357e },
		{ 0x96d5cd, 0x0caac9, 0x07669e, 0x002c71 },
		{ 0xb2dfcc, 0x3bbcd0, 0x0b88b4, 0x044f8f, 0x042561 },
		{ 0xbae0cd, 0x5cc5cf, 0x0da0c3, 0x096fa4, 0x044186, 0x05255c },
		{ 0xc3e2cc, 0x75cbcf, 0x10b1ce, 0x0b88b4, 0x0b88b4, 0x01377f, 0x052257 },
		{ 0xcce5cc, 0x86d1ce, 0x33bbd1, 0x0c9ac0, 0x0a75a7, 0x065090, 0x002f79, 0x051f53 },
		{ 0xd4e8cc, 0x96d5cd, 0x52c2d0, 0x0caac9, 0x0b88b4, 0x07669e, 0x034589, 0x012b70, 0x061d4e },
		{ 0xdcebcb, 0xa3dacd, 0x67c7cf, 0x0db6d1, 0x0b97be, 0x0b78a9, 0x065a96, 0x023d83, 0x042868, 0x061c49 },
		{ 0xdcebcb, 0xa9dccd, 0x75cbcf, 0x33bbd1, 0x33bbd1, 0x33bbd1, 0x096ba2, 0x065090, 0x01377f, 0x032766, 0x032766 },
		{ 0xdcebcb, 0xaeddcd, 0x7ecece, 0x48c0d1, 0x10adcc, 0x0b94bc, 0x0b7bab, 0x09639b, 0x034a8b, 0x00337c, 0x032663, 0x061c49 }
	};
	static unsigned neon[][12] = {
		{ 0xf3648b },
		{ 0xf78495, 0xad3e8c },
		{ 0xf78495, 0xad3e8c, 0x502370 },
		{ 0xf9989b, 0xde4c86, 0x7d3392, 0x461e63 },
		{ 0xfcaca4, 0xf2618a, 0xad3e8c, 0x612b8a, 0x3c1955 },
		{ 0xfdb3a5, 0xf5738f, 0xd04888, 0x8a3590, 0x58277c, 0x3a1951 },
		{ 0xfeb9a7, 0xf68294, 0xea5084, 0xad3e8c, 0x703093, 0x512472, 0x37164c },
		{ 0xffbfaa, 0xf88f98, 0xf25e89, 0xc84688, 0x93378f, 0x632c8c, 0x4b2069, 0x331548 },
		{ 0xffc5ac, 0xf9999c, 0xf46e8d, 0xde4c86, 0xad3e8c, 0x7c3292, 0x5b2981, 0x451e62, 0x301443 },
		{ 0xffcbae, 0xfaa2a0, 0xf57a91, 0xf05384, 0xc44488, 0x97388e, 0x692f94, 0x552578, 0x411c5c, 0x2d133f },
		{ 0xffcbae, 0xfba6a2, 0xf68294, 0xf25e89, 0xd64a87, 0xad3e8c, 0x853491, 0x632c8c, 0x512472, 0x3f1b59, 0x2d133f },
		{ 0xffcbae, 0xfbaaa3, 0xf78997, 0xf3688c, 0xe54f85, 0xbf4489, 0x9c398d, 0x753193, 0x5e2985, 0x4d226d, 0x3d1a57, 0x2d133f }
	};
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
		case 12: {
			switch (pal) {
				case pal_warm:
					return warm[num_colors - 1];
				case pal_cool:
					return cool[num_colors - 1];
				case pal_neon:
					return neon[num_colors - 1];
				default: {
					WARN_PRINT("Undefined palette");
					return nullptr;
				}
			}
		} break;
		default: {
			WARN_PRINT("Undefined palette");
			return nullptr;
		}
	}
}

/// Godot Control

int SRGraph::get_plot_history_size() const {
	return plot_history_size;
}

void SRGraph::set_plot_history_size(int p_size) {
	plot_history_size = p_size;
}

void SRGraph::set_grid(bool p_visible) {
	show_grid = p_visible;
	update();
}

bool SRGraph::get_grid() const {
	return show_grid;
}

void SRGraph::set_grid_color(const Color &p_color) {
	grid_color = p_color;
	update();
}

Color SRGraph::get_grid_color() const {
	return grid_color;
}

void SRGraph::set_axes(bool p_visible) {
	show_axes = p_visible;
	update();
}

bool SRGraph::get_axes() const {
	return show_axes;
}

void SRGraph::set_axes_color(const Color &p_color) {
	axes_color = p_color;
	update();
}

Color SRGraph::get_axes_color() const {
	return axes_color;
}

sr_graph_t SRGraph::add_graph(const Point2 &p_min, const Point2 &p_max, real_t p_ratio, const Color &p_bg, const String &p_label) {
	return sr_setup_graph(p_min.x, p_max.x, p_min.y, p_max.y, p_ratio, p_bg, p_label);
}

int SRGraph::add_curve(sr_graph_t p_graph, const Vector<real_t> &p_xs, const Vector<real_t> &p_ys, const Color &p_color) {
	int id = sr_add_curve(p_graph, p_xs, p_ys, p_color);
	update();
	return id;
}

void SRGraph::update_curve(sr_graph_t p_graph, int p_curve, const Vector<real_t> &p_xs, const Vector<real_t> &p_ys) {
	sr_update_curve(p_graph, p_curve, p_xs, p_ys);
	update();
}

void SRGraph::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_EXIT_TREE: {
			sr_cleanup();
		} break;
		case NOTIFICATION_DRAW: {
			sr_draw(this, get_size());
		} break;
	}
}

void SRGraph::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_plot_history_size", "size"), &SRGraph::set_plot_history_size);
	ClassDB::bind_method(D_METHOD("get_plot_history_size"), &SRGraph::get_plot_history_size);

	ClassDB::bind_method(D_METHOD("set_axes", "visible"), &SRGraph::set_axes);
	ClassDB::bind_method(D_METHOD("get_axes"), &SRGraph::get_axes);
	ClassDB::bind_method(D_METHOD("set_axes_color", "color"), &SRGraph::set_axes_color);
	ClassDB::bind_method(D_METHOD("get_axes_color"), &SRGraph::get_axes_color);
	ClassDB::bind_method(D_METHOD("set_grid", "visible"), &SRGraph::set_grid);
	ClassDB::bind_method(D_METHOD("get_grid"), &SRGraph::get_grid);
	ClassDB::bind_method(D_METHOD("set_grid_color", "color"), &SRGraph::set_grid_color);
	ClassDB::bind_method(D_METHOD("get_grid_color"), &SRGraph::get_grid_color);

	ClassDB::bind_method(D_METHOD("add_graph", "min_xy", "max_xy", "ratio", "bg", "label"), &SRGraph::add_graph, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_curve", "graph_id", "xs", "ys", "color"), &SRGraph::add_curve);
	ClassDB::bind_method(D_METHOD("update_curve", "graph_id", "curve_id", "xs", "ys"), &SRGraph::update_curve);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "history_size"), "set_plot_history_size", "get_plot_history_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axes"), "set_axes", "get_axes");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "axes_color"), "set_axes_color", "get_axes_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "grid"), "set_grid", "get_grid");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "grid_color"), "set_grid_color", "get_grid_color");
}

SRGraph::SRGraph() {
	plot_history_size = 100;
	show_grid = true;
	grid_color = Color(0.8, 0.8, 0.8, 0.25);
	show_axes = false;
	axes_color = Color(1, 0, 0, 0.8);
	set_size(Size2(150, 150));
}

SRGraph::~SRGraph() {}
