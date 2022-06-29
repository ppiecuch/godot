/*************************************************************************/
/*  polyvector.h                                                         */
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

#ifndef POLYVECTOR_H
#define POLYVECTOR_H

#include <map>
#include <vector>

#include "core/os/os.h"
#include "scene/2d/canvas_item.h"
#include "scene/3d/mesh_instance.h"
#include "scene/animation/animation_player.h"
#include "scene/resources/curve.h"

#include "earcut/earcut.h"
#include "resource_importer_swf.h"

#ifdef DEBUG_ENABLED
#define POLYVECTOR_DEBUG
#endif

using Coord = float;
using Point = Vector2;

namespace mapbox {
namespace util {
template <>
struct nth<0, Vector2> {
	inline static Coord get(const Vector2 &v) { return v.x; };
};
template <>
struct nth<1, Vector2> {
	inline static Coord get(const Vector2 &v) { return v.y; };
};
} //namespace util
} //namespace mapbox

#ifndef _3D_DISABLED
class PolyVector : public VisualInstance {
	GDCLASS(PolyVector, VisualInstance)

	Ref<JSONVector> data_vec_file;
	Ref<SpatialMaterial> material_default;
	MeshInstanceMap mesh_display;

	real_t frame_time;
	int8_t curve_quality;
	Vector2 position_offset;
	real_t layer_depth;
	real_t max_tessellation_angle;

	List<PolyVectorFrame> frame_data;
	List<PolyVectorCharacter> dictionary_data;
	real_t fps;

#ifdef POLYVECTOR_DEBUG
	OS *os;
	bool debug_wireframe;
	Ref<SpatialMaterial> material_debug;
	double triangulation_time;
	double mesh_update_time;
	uint32_t vertex_count;
#endif

protected:
	static void _bind_methods();

public:
	void draw_current_frame();

	void clear_mesh_data();
	void clear_mesh_instances();

	void set_vector_image(const Ref<JSONVector> &p_vector);
	Ref<JSONVector> get_vector_image() const;
	void set_time(real_t p_time);
	real_t get_time();
	void set_curve_quality(int8_t p_quality);
	int8_t get_curve_quality();
	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset();
	void set_layer_separation(real_t p_separation);
	real_t get_layer_separation();
	void set_albedo_colour(Color);
	Color get_albedo_colour();
	void set_material_unshaded(bool);
	bool get_material_unshaded();

	void set_max_tessellation_angle(real_t);
	real_t get_max_tessellation_angle();

	virtual AABB get_aabb() const;
	virtual PoolVector<Face3> get_faces(uint32_t p_usage_flags) const;

#ifdef POLYVECTOR_DEBUG
	void set_debug_wireframe(bool);
	bool get_debug_wireframe();

	double get_triangulation_time();
	double get_mesh_update_time();
	uint32_t get_vertex_count();
#endif

	PolyVector();
	~PolyVector();
};
#endif // _3D_DISABLED

class PolyVector2D : public CanvasItem {
	GDCLASS(PolyVector2D, CanvasItem)

	Ref<JSONVector> data_vec_file;
	Size2 viewSize;

	real_t frame_time;

	List<PolyVectorFrame> frame_data;
	List<PolyVectorCharacter> dictionary_data;
	real_t fps;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
#ifdef TOOLS_ENABLED
	void _edit_set_position(const Point2 &p_position);
	Point2 _edit_get_position() const;
	void _edit_set_scale(const Size2 &p_scale);
	Size2 _edit_get_scale() const;
#endif

	Transform2D get_transform() const;

	void set_vector_image(const Ref<JSONVector> &p_vector);
	Ref<JSONVector> get_vector_image() const;
	void set_time(real_t p_time);
	real_t get_time();

	PolyVector2D();
};

#endif // POLYVECTOR_H
