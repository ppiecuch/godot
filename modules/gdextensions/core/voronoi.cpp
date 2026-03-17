/**************************************************************************/
/*  voronoi.cpp                                                           */
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

#include <algorithm>
#include <type_traits>

#define JC_VORONOI_IMPLEMENTATION
#define JC_VORONOI_CLIP_IMPLEMENTATION
#include "voronoi.h"

/// VoronoiEdge

Vector<Variant> VoronoiEdge::sites() const {
	Vector<Variant> result;
	result.push_back(_diagram->_sites_by_index.at(_edge->sites[0]->index));
	if (_edge->sites[1] != nullptr)
		result.push_back(_diagram->_sites_by_index.at(_edge->sites[1]->index));
	return result;
}

Vector2 VoronoiEdge::start() const {
	return Vector2(_edge->pos[0].x, _edge->pos[0].y);
}

Vector2 VoronoiEdge::end() const {
	return Vector2(_edge->pos[1].x, _edge->pos[1].y);
}

void VoronoiEdge::_bind_methods() {
	ClassDB::bind_method(D_METHOD("sites"), &VoronoiEdge::sites);
	ClassDB::bind_method(D_METHOD("start"), &VoronoiEdge::start);
	ClassDB::bind_method(D_METHOD("end"), &VoronoiEdge::end);
}

/// VoronoiSite

int VoronoiSite::index() const {
	return _site->index;
}

Vector2 VoronoiSite::center() const {
	return Vector2(_site->p.x, _site->p.y);
}

Vector<Variant> VoronoiSite::edges() const {
	Vector<Variant> result;
	const jcv_graphedge *graphedge = _site->edges;
	while (graphedge) {
		result.push_back(_diagram->_edges_by_address.at(
				reinterpret_cast<std::uintptr_t>(graphedge->edge)));
		graphedge = graphedge->next;
	}
	return result;
}

Vector<Variant> VoronoiSite::neighbors() const {
	Vector<Variant> result;
	const jcv_graphedge *graphedge = _site->edges;
	while (graphedge) {
		if (graphedge->neighbor)
			result.push_back(_diagram->_sites_by_index.at(graphedge->neighbor->index));
		graphedge = graphedge->next;
	}
	return result;
}

void VoronoiSite::_bind_methods() {
	ClassDB::bind_method(D_METHOD("index"), &VoronoiSite::index);
	ClassDB::bind_method(D_METHOD("center"), &VoronoiSite::center);
	ClassDB::bind_method(D_METHOD("edges"), &VoronoiSite::edges);
	ClassDB::bind_method(D_METHOD("neighbors"), &VoronoiSite::neighbors);
}

/// VoronoiDiagram

VoronoiDiagram::VoronoiDiagram() :
		_diagram() {
	memset(&_diagram, 0, sizeof(jcv_diagram));
}

VoronoiDiagram::~VoronoiDiagram() {
	for (auto edge : _edges)
		memdelete(static_cast<Object *>(edge));
	for (auto site : _sites)
		memdelete(static_cast<Object *>(site));

	jcv_diagram_free(&_diagram);
}

Vector<Variant> VoronoiDiagram::edges() const {
	Vector<Variant> result;
	for (auto edge : _edges)
		result.push_back(edge);
	return result;
}

Vector<Variant> VoronoiDiagram::sites() const {
	Vector<Variant> result;
	for (auto site : _sites)
		result.push_back(site);
	return result;
}

void VoronoiDiagram::build_objects() {
	voronoi_detail::vector<Variant> gd_edges;
	const jcv_edge *edge = jcv_diagram_get_edges(&_diagram);
	while (edge) {
		// apparent bug in jcv, egdes where start = end are reported as
		// diagram edges, but do not exist when iterating over sites
		if (edge->pos[0].x != edge->pos[1].x || edge->pos[0].y != edge->pos[1].y) {
			VoronoiEdge *gd_edge = memnew(VoronoiEdge(edge, this));
			gd_edges.push_back(gd_edge);
			_edges_by_address[reinterpret_cast<std::uintptr_t>(edge)] = gd_edge;
		}
		edge = edge->next;
	}
	_edges.swap(gd_edges);

	voronoi_detail::vector<Variant> gd_sites;
	const jcv_site *sites = jcv_diagram_get_sites(&_diagram);
	for (int i = 0; i < _diagram.numsites; i++) {
		VoronoiSite *gd_site = memnew(VoronoiSite(&sites[i], this));
		gd_sites.push_back(gd_site);
		_sites_by_index[sites[i].index] = gd_site;
	}
	_sites.swap(gd_sites);
}

void VoronoiDiagram::_bind_methods() {
	ClassDB::bind_method(D_METHOD("edges"), &VoronoiDiagram::edges);
	ClassDB::bind_method(D_METHOD("sites"), &VoronoiDiagram::sites);
}

/// Voronoi

void Voronoi::set_points(Vector<Vector2> points) {
	assert(points.size());

	// translate Godot Vector2 points into jcv_points
	voronoi_detail::vector<jcv_point> new_points;
	for (int i = 0; i < points.size(); i++)
		new_points.push_back({ points[i].x, points[i].y });

	_points.swap(new_points);
}

void Voronoi::set_boundaries(Rect2 boundaries) {
	_boundaries = jcv_rect{
		jcv_point{ boundaries.position.x, boundaries.position.y },
		jcv_point{ boundaries.position.x + boundaries.size.x, boundaries.position.y + boundaries.size.y }
	};
	_has_boundaries = true;
}

void Voronoi::set_clip_points(Vector<Vector2> points) {
	assert(points.size());

	// translate Godot Vector2 points into jcv_points
	voronoi_detail::vector<jcv_point> new_points;
	for (int i = 0; i < points.size(); i++)
		new_points.push_back({ points[i].x, points[i].y });

	_cpoints.swap(new_points);
}

static void *useralloc(void *ctx, size_t size) {
	return memalloc(size);
}

static void userfree(void *ctx, void *ptr) {
	return memfree(ptr);
}

void Voronoi::relax_points(int iterations = 1) {
	voronoi_detail::vector<jcv_point> new_points;
	for (int j = 0; j < iterations; j++) {
		jcv_diagram diagram;
		memset(&diagram, 0, sizeof(jcv_diagram));
		jcv_diagram_generate_useralloc(
				_points.size(),
				_points.data(),
				_has_boundaries ? &_boundaries : nullptr,
				nullptr,
				nullptr,
				&useralloc,
				&userfree,
				&diagram);
		const jcv_site *sites = jcv_diagram_get_sites(&diagram);
		const int numsites = diagram.numsites;
		for (int i = 0; i < numsites; ++i) {
			const jcv_site *site = &sites[i];
			jcv_point sum = site->p;
			int count = 1;

			const jcv_graphedge *edge = site->edges;

			while (edge) {
				sum.x += edge->pos[0].x;
				sum.y += edge->pos[0].y;
				++count;
				edge = edge->next;
			}

			new_points.push_back({ sum.x / count, sum.y / count });
		}
		jcv_diagram_free(&diagram);
	}
	_points.swap(new_points);
}

Ref<VoronoiDiagram> Voronoi::generate_diagram() const {
	Ref<VoronoiDiagram> result{ memnew(VoronoiDiagram) };

	// setup clippping:
	jcv_clipper *clipper = nullptr;
	if (_cpoints.size()) {
		jcv_clipping_polygon polygon;
		polygon.num_points = _cpoints.size();
		polygon.points = const_cast<jcv_point *>(_cpoints.data());
		jcv_clipper polygonclipper;
		polygonclipper.test_fn = jcv_clip_polygon_test_point;
		polygonclipper.clip_fn = jcv_clip_polygon_clip_edge;
		polygonclipper.fill_fn = jcv_clip_polygon_fill_gaps;
		polygonclipper.ctx = &polygon;
		clipper = &polygonclipper;
	}
	jcv_diagram_generate_useralloc(
			_points.size(),
			_points.data(),
			_has_boundaries ? &_boundaries : nullptr,
			clipper,
			nullptr,
			&useralloc,
			&userfree,
			&(result->_diagram));
	result->build_objects();
	return result;
}

void Voronoi::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_points", "points"), &Voronoi::set_points);
	ClassDB::bind_method(D_METHOD("set_boundaries", "boundaries"), &Voronoi::set_boundaries);
	ClassDB::bind_method(D_METHOD("set_clip_points", "points"), &Voronoi::set_clip_points);
	ClassDB::bind_method(D_METHOD("relax_points", "iterations"), &Voronoi::relax_points);
	ClassDB::bind_method(D_METHOD("generate_diagram"), &Voronoi::generate_diagram);
}
