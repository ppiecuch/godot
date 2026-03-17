/**************************************************************************/
/*  voronoi.h                                                             */
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

#ifndef VORONOI_H
#define VORONOI_H

#include <cstdint>
#include <map>
#include <type_traits>
#include <vector>

#include <core/math/rect2.h>
#include <core/math/vector2.h>
#include <core/object.h>
#include <core/reference.h>
#include <core/variant.h>
#include <core/vector.h>

#include "voronoi/jc_voronoi.h"
#include "voronoi/jc_voronoi_clip.h"

namespace voronoi_detail {

template <typename T>
struct GodotAllocator {
	using value_type = T;

	constexpr GodotAllocator() noexcept {}

	template <typename U>
	GodotAllocator(const GodotAllocator<U> &) noexcept {}

	template <typename U>
	bool operator==(const GodotAllocator<U> &) const noexcept {
		return true;
	}

	template <typename U>
	bool operator!=(const GodotAllocator<U> &) const noexcept {
		return false;
	}

	inline T *allocate(size_t n) const {
		return reinterpret_cast<T *>(memalloc(sizeof(T) * n));
	}

	inline void deallocate(T *ptr, size_t) const noexcept {
		memfree(ptr);
	}

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) < 40900 && !defined(__clang__) && !defined(_MSC_VER)
	template <class U>
	struct rebind {
		typedef std::allocator<U> other;
	};
#endif
};

template <typename K, typename V>
using map = std::map<K, V, std::less<K>, GodotAllocator<std::pair<const K, V>>>;

template <typename T>
using vector = std::vector<T, GodotAllocator<T>>;

} // namespace voronoi_detail

class VoronoiEdge;
class VoronoiSite;
class VoronoiDiagram;

class VoronoiEdge : public Object {
	GDCLASS(VoronoiEdge, Object)

	const jcv_edge *_edge;
	const VoronoiDiagram *_diagram;

protected:
	static void _bind_methods();

public:
	Vector<Variant> sites() const;
	Vector2 start() const;
	Vector2 end() const;

	VoronoiEdge() = default;
	inline VoronoiEdge(const jcv_edge *edge, const VoronoiDiagram *diagram) :
			_edge(edge), _diagram(diagram) {
	}

	~VoronoiEdge() = default;
};

class VoronoiSite : public Object {
	GDCLASS(VoronoiSite, Object)

	const jcv_site *_site;
	const VoronoiDiagram *_diagram;

protected:
	static void _bind_methods();

public:
	int index() const;
	Vector2 center() const;
	Vector<Variant> edges() const;
	Vector<Variant> neighbors() const;

	VoronoiSite() = default;
	inline VoronoiSite(const jcv_site *site, const VoronoiDiagram *diagram) :
			_site(site), _diagram(diagram) {
	}

	~VoronoiSite() = default;
};

class VoronoiDiagram : public Reference {
	GDCLASS(VoronoiDiagram, Reference)

	friend class VoronoiEdge;
	friend class VoronoiSite;
	friend class Voronoi;

	jcv_diagram _diagram;

	voronoi_detail::vector<Variant> _edges;
	voronoi_detail::vector<Variant> _sites;

	voronoi_detail::map<std::uintptr_t, VoronoiEdge *> _edges_by_address;
	voronoi_detail::map<int, VoronoiSite *> _sites_by_index;

protected:
	static void _bind_methods();

public:
	void build_objects();

	Vector<Variant> edges() const;
	Vector<Variant> sites() const;

	VoronoiDiagram();
	~VoronoiDiagram();
};

class Voronoi : public Reference {
	GDCLASS(Voronoi, Reference)

	jcv_rect _boundaries;
	bool _has_boundaries;
	voronoi_detail::vector<jcv_point> _points, _cpoints;

protected:
	static void _bind_methods();

public:
	void set_points(Vector<Vector2> points);
	void set_boundaries(Rect2 boundaries);
	void set_clip_points(Vector<Vector2> points);
	void relax_points(int iterations);
	Ref<VoronoiDiagram> generate_diagram() const;

	Voronoi() = default;
	~Voronoi() = default;
};

#endif // VORONOI_H

// Voronoi Godot integration
// =========================
//
// Integration of [JCash's Voronoi implementation](https://github.com/JCash/voronoi) as a Godot V3.1.1 module.
//
// Install
// -------
//
// Download the contents of this git repository into Godot's module folder and re-build Godot.
// The module folder needs to be named `voronoi`, otherwise Godot won't compile properly.
//
// `git clone https://github.com/rakai93/godot_voronoi.git voronoi`
//
// Example usage
// -------------
//
// # Create voronoi generator
// var generator = Voronoi.new()
// generator.set_points(list_of_vector2)
// # optional : set boundaries for diagram, otherwise boundaries are computed based on points
// generator.set_boundaries(rect2_bounds)
// # optional : relax points N times, resulting in more equal sites
// generator.relax_points(2)
//
// #Generate diagram
// var diagram = generator.generate_diagram()
//
// # Iterate over sites
// for site in diagram.sites():
//   draw_circle(site.center())
//
// # Iterate over edges
// for edge in diagram.edges():
//   draw_line(edge.start(), edge.end())
