/**************************************************************************/
/*  figure_2d.h                                                           */
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

#ifndef FIGURE_2D_H
#define FIGURE_2D_H

#include "core/rid.h"
#include "scene/2d/node_2d.h"
#include "scene/animation/animation_player_bone_transform.h"
#include "scene/resources/skin.h"

class Figure2D : public Node2D {
	GDCLASS(Figure2D, Node2D);

private:
	struct Bone {
		String name;

		bool enabled;
		int parent;
		int sort_index; //used for re-sorting process order

		bool disable_rest;
		Transform rest;

		Transform pose;
		Transform pose_global;

		bool custom_pose_enable;
		Transform custom_pose;

		float global_pose_override_amount;
		bool global_pose_override_reset;
		Transform global_pose_override;

		List<uint32_t> nodes_bound;

		Bone() {
			parent = -1;
			enabled = true;
			disable_rest = false;
			custom_pose_enable = false;
			global_pose_override_amount = 0;
			global_pose_override_reset = false;
		}
	};

	Vector<Bone> bones;
	Vector<int> process_order;
	bool process_order_dirty;

	void _make_dirty();
	bool dirty;

	uint64_t version;

	// bind helpers
	Array _get_bound_child_nodes_to_bone(int p_bone) const {
		Array bound;
		List<Node *> children;
		get_bound_child_nodes_to_bone(p_bone, &children);

		for (int i = 0; i < children.size(); i++) {
			bound.push_back(children[i]);
		}
		return bound;
	}

	Basis _figure_xform;
	AABB _figure_aabb;
	bool _figure_xform_dirty;
	bool _figure_dirty;
	Vector<Vector2> _figure_lines;

	Vector3 figure_angle;
	Vector3 figure_scale;
	Point2 figure_offset;

	void _update_xform_values();
	void _update_transform();

	void _update_process_order();
	void _update_figure_shape();

	class BoneTransform : public AnimationPlayerBoneTransform {
		Figure2D *_fg = 0;

	public:
		BoneTransform(Figure2D *p_figure) :
				_fg(p_figure) {}
		virtual String get_bone_name(int p_bone) const { return _fg->get_bone_name(p_bone); }
		virtual int get_bone_parent(int p_bone) const { return _fg->get_bone_parent(p_bone); }
		virtual Transform get_bone_pose(int p_bone) const { return _fg->get_bone_pose(p_bone); }
		virtual void set_bone_pose(int p_bone, const Transform &p_pose) { _fg->set_bone_pose(p_bone, p_pose); }
		virtual int find_bone(const String &p_name) const { return _fg->find_bone(p_name); }
		virtual void clear_pose() {
			for (int i = 0; i < _fg->get_bone_count(); i++) {
				_fg->set_bone_pose(i, Transform());
			}
		}
		virtual void update_skeleton() { notification(NOTIFICATION_UPDATE_FIGURE); }
	} _bone_transformer;

protected:
	bool _get(const StringName &p_path, Variant &r_ret) const;
	bool _set(const StringName &p_path, const Variant &p_value);
	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);
	static void _bind_methods();

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
	enum {

		NOTIFICATION_UPDATE_FIGURE = 50
	};

	void set_figure_rotation(Vector3 p_radians);
	Vector3 get_figure_rotation() const;
	void set_figure_scale(const Vector3 &p_scale);
	Vector3 get_figure_scale() const;
	void set_figure_orientation(const Basis &p_basis);
	Basis get_figure_orientation() const;

	void set_figure_rotation_x_degrees(float p_degrees);
	float get_figure_rotation_x_degrees() const;
	void set_figure_rotation_y_degrees(float p_degrees);
	float get_figure_rotation_y_degrees() const;
	void set_figure_rotation_z_degrees(float p_degrees);
	float get_figure_rotation_z_degrees() const;

	Rect2 get_rect() const;
	Point2 get_figure_offset() const;
	void set_figure_offset(const Point2 &p_offset);

	// skeleton creation api
	void add_bone(const String &p_name);
	int find_bone(const String &p_name) const;
	String get_bone_name(int p_bone) const;

	bool is_bone_parent_of(int p_bone_id, int p_parent_bone_id) const;

	void set_bone_parent(int p_bone, int p_parent);
	int get_bone_parent(int p_bone) const;

	void unparent_bone_and_rest(int p_bone);

	void set_bone_disable_rest(int p_bone, bool p_disable);
	bool is_bone_rest_disabled(int p_bone) const;

	int get_bone_count() const;

	void set_bone_rest(int p_bone, const Transform &p_rest);
	Transform get_bone_rest(int p_bone) const;
	Transform get_bone_global_pose(int p_bone) const;

	void clear_bones_global_pose_override();
	void set_bone_global_pose_override(int p_bone, const Transform &p_pose, float p_amount, bool p_persistent = false);

	void set_bone_enabled(int p_bone, bool p_enabled);
	bool is_bone_enabled(int p_bone) const;

	void bind_child_node_to_bone(int p_bone, Node *p_node);
	void unbind_child_node_from_bone(int p_bone, Node *p_node);
	void get_bound_child_nodes_to_bone(int p_bone, List<Node *> *p_bound) const;

	void clear_bones();

	// posing api

	void set_bone_pose(int p_bone, const Transform &p_pose);
	Transform get_bone_pose(int p_bone) const;

	void set_bone_custom_pose(int p_bone, const Transform &p_custom_pose);
	Transform get_bone_custom_pose(int p_bone) const;

	void localize_rests(); // used for loaders and tools
	int get_process_order(int p_idx);

public:
	Figure2D();
	~Figure2D();

	friend class BoneTransform;
};

#endif // FIGURE_2D_H
