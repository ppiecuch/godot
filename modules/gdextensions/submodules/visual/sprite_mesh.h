/*************************************************************************/
/*  sprite_mesh.h                                                        */
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

#ifndef GD_SPRITE_MESH_H
#define GD_SPRITE_MESH_H

#include "core/math/vector3.h"
#include "scene/2d/sprite.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

class SpriteMesh : public Node2D {
	GDCLASS(SpriteMesh, Node2D);

private:
	Ref<Mesh> mesh;
	Ref<Texture> texture;
	Ref<Texture> normal_map;
	Ref<Texture> mask;

	Vector3 mesh_angle;
	Vector3 mesh_scale;

	bool auto_collision_shape;
	Vector<Vector2> mesh_outline;

	bool centered;
	Point2 offset;

	int selected_frame;
	// Description of frames to be precalculated:
	// ------------------------------------------
	// [
	//    {
	//       "rotate":[10, 10, 10],
	//       "scale":[10, 10, 10]
	//    },
	//    {
	//       "repeat":"10",
	//       "rotate":["sin(repcounter)", 10, 10],
	//       "scale":["360/repcount", 10, 10]
	//    }
	// ]
	String frames_builder;

	struct _FrameInfo {
		Basis xform;
		int surface_nr;
		AABB bbox;
	};
	Vector<_FrameInfo> _frames;

	Array _mesh_data;
	Basis _mesh_xform;
	bool _mesh_dirty;
	bool _mesh_xform_dirty;
	int _mesh_active_surface;
	bool _mesh_debug;

	void _get_rects(Rect2 &r_src_rect, Rect2 &r_dst_rect, bool &r_filter_clip) const;

	void _mesh_changed();

protected:
	void _notification(int p_what);

	static void _bind_methods();

	virtual void _validate_property(PropertyInfo &property) const;

public:
#ifdef TOOLS_ENABLED
	virtual Dictionary _edit_get_state() const;
	virtual void _edit_set_state(const Dictionary &p_state);

	virtual void _edit_set_pivot(const Point2 &p_pivot);
	virtual Point2 _edit_get_pivot() const;
	virtual bool _edit_use_pivot() const;
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
#endif

	void _update_mesh_outline(const PoolVector3Array &p_vertices, const PoolIntArray &p_triangles);
	void _update_xform_values();
	void _update_mesh_xform();
	void _update_transform();

	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

	void set_mesh_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_mesh_texture() const;
	void set_mesh_normal_map(const Ref<Texture> &p_texture);
	Ref<Texture> get_mesh_normal_map() const;
	void set_mesh_mask(const Ref<Texture> &p_texture);
	Ref<Texture> get_mesh_mask() const;

	void set_selected_frame(int p_frame);
	int get_selected_frame() const;
	void set_frames_builder(const String &p_input);
	String get_frames_builder() const;

	void set_mesh_rotation(Vector3 p_radians);
	Vector3 get_mesh_rotation() const;
	void set_mesh_scale(const Vector3 &p_scale);
	Vector3 get_mesh_scale() const;
	void set_mesh_orientation(const Basis &p_basis);
	Basis get_mesh_orientation() const;

	void set_mesh_rotation_x_degrees(float p_degrees);
	float get_mesh_rotation_x_degrees() const;
	void set_mesh_rotation_y_degrees(float p_degrees);
	float get_mesh_rotation_y_degrees() const;
	void set_mesh_rotation_z_degrees(float p_degrees);
	float get_mesh_rotation_z_degrees() const;

	void set_auto_collision_shape(bool state);
	bool is_auto_collision_shape();

	void set_centered(bool p_center);
	bool is_centered() const;
	void set_offset(const Point2 &p_offset);
	Point2 get_offset() const;

	Rect2 get_rect() const;
	virtual Rect2 get_anchorable_rect() const;

	SpriteMesh();
};

#endif // GD_SPRITE_MESH_H
