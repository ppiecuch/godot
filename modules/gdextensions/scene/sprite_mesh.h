/*************************************************************************/
/*  sprite_mesh.h                                                        */
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

#ifndef GLSPRITEMESH_H
#define GLSPRITEMESH_H

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

	Vector3 mesh_angle;
	Vector3 mesh_scale;

	bool centered;
	Point2 offset;

	struct SurfInfo {
		Basis surf_xform; // surface transform
		int surf_id; // surface index
	};
	Vector<SurfInfo> _mesh_xform;
	bool _mesh_dirty;
	bool _mesh_xform_dirty;
	int _mesh_active_surface;

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

	void _update_xform_values();
	void _update_mesh_xform();
	void _update_transform();

	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

	void set_mesh_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_mesh_texture() const;

	void set_mesh_normal_map(const Ref<Texture> &p_texture);
	Ref<Texture> get_mesh_normal_map() const;

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

	void set_centered(bool p_center);
	bool is_centered() const;
	void set_offset(const Point2 &p_offset);
	Point2 get_offset() const;

	Basis new_snapshot();
	void set_snapshot(int p_snapshot);
	void delete_snapshot(int p_snapshot);
	Basis get_snapshot_transform() const;
	int get_snapshot_count() const;

	Rect2 get_rect() const;
	virtual Rect2 get_anchorable_rect() const;

	SpriteMesh();
	~SpriteMesh();
};

#endif // GLSPRITEMESH_H
