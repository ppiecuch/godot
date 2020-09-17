/*************************************************************************/
/*  voronoi.h                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "lib/src/jc_voronoi.h"

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

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) < 40900 \
	&& !defined(__clang__) && !defined(_MSC_VER)
	template <typename U>
	using rebind = std::allocator<U>;
#endif
};

template <typename K, typename V>
using map = std::map<K, V, std::less<K>, GodotAllocator<std::pair<const K, V> > >;

template <typename T>
using vector = std::vector<T, GodotAllocator<T> >;

} // namespace voronoi_detail

class VoronoiEdge;
class VoronoiSite;
class VoronoiDiagram;

class VoronoiEdge : public Object {
	GDCLASS(VoronoiEdge, Object)

public:
	const jcv_edge *_edge;
	const VoronoiDiagram *_diagram;

	VoronoiEdge() = default;
	inline VoronoiEdge(const jcv_edge *edge, const VoronoiDiagram *diagram) :
			_edge(edge), _diagram(diagram) {
	}

	~VoronoiEdge() = default;

	Vector<Variant> sites() const;
	Vector2 start() const;
	Vector2 end() const;

protected:
	static void _bind_methods();
};

class VoronoiSite : public Object {
	GDCLASS(VoronoiSite, Object)

public:
	const jcv_site *_site;
	const VoronoiDiagram *_diagram;

	VoronoiSite() = default;
	inline VoronoiSite(const jcv_site *site, const VoronoiDiagram *diagram) :
			_site(site), _diagram(diagram) {
	}

	~VoronoiSite() = default;

	int index() const;
	Vector2 center() const;
	Vector<Variant> edges() const;
	Vector<Variant> neighbors() const;

protected:
	static void _bind_methods();
};

class VoronoiDiagram : public Reference {
	GDCLASS(VoronoiDiagram, Reference)

public:
	jcv_diagram _diagram;

	voronoi_detail::vector<Variant> _edges;
	voronoi_detail::vector<Variant> _sites;

	voronoi_detail::map<std::uintptr_t, VoronoiEdge *> _edges_by_address;
	voronoi_detail::map<int, VoronoiSite *> _sites_by_index;

	VoronoiDiagram();
	~VoronoiDiagram();

	void build_objects();

	Vector<Variant> edges() const;
	Vector<Variant> sites() const;

protected:
	static void _bind_methods();
};

class Voronoi : public Reference {
	GDCLASS(Voronoi, Reference)

	jcv_rect _boundaries;
	bool _has_boundaries;
	voronoi_detail::vector<jcv_point> _points;

public:
	Voronoi() = default;
	~Voronoi() = default;

	void set_points(Vector<Vector2> points);
	void set_boundaries(Rect2 boundaries);
	void relax_points(int iterations);
	Ref<VoronoiDiagram> generate_diagram() const;

protected:
	static void _bind_methods();
};

#endif // VORONOI_H
