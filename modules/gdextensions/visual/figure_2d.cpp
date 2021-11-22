/*************************************************************************/
/*  figure_2d.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "figure_2d.h"

#include "core/message_queue.h"
#include "core/project_settings.h"
#include "scene/resources/surface_tool.h"

#include <limits>

const Basis FLIP_Y = Basis(1, 0, 0, 0, -1, 0, 0, 0, 1);
const real_t REAL_MIN = std::numeric_limits<real_t>::min();
const real_t REAL_MAX = std::numeric_limits<real_t>::max();

#ifdef TOOLS_ENABLED
Dictionary Figure::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["figure_offset"] = figure_offset;
	return state;
}

void Figure::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_figure_offset(p_state["figure_offset"]);
}

void Figure::_edit_set_pivot(const Point2 &p_pivot) {
	set_figure_offset(get_figure_offset() - p_pivot);
	set_position(get_transform().xform(p_pivot));
}

Point2 Figure::_edit_get_pivot() const {
	return figure_offset;
}

bool Figure::_edit_use_pivot() const {
	return true;
}

bool Figure::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return get_rect().has_point(p_point);
}

Rect2 Figure::_edit_get_rect() const {
	return get_rect();
}

bool Figure::_edit_use_rect() const {
	return true;
}
#endif

void Figure::_update_figure_shape() {
	_figure_lines.clear();
	_figure_aabb = AABB();

	if (!get_bone_count())
		return;

	PoolVector<Vector3> lines;

	Vector<Transform> grests;
	grests.resize(get_bone_count());

	Vector3 pt_min(REAL_MAX, REAL_MAX, REAL_MAX), pt_max(REAL_MIN, REAL_MIN, REAL_MIN);

	for (int i_bone = 0; i_bone < get_bone_count(); i_bone++) {
		const int i = get_process_order(i_bone);
		const int parent = get_bone_parent(i);

		if (parent >= 0) {
			grests.write[i] = grests[parent] * get_bone_rest(i) * get_bone_pose(i);

			Vector3 v0 = _figure_xform.xform(grests[parent].origin);
			Vector3 v1 = _figure_xform.xform(grests[i].origin);

			if (v0.x < pt_min.x)
				pt_min.x = v0.x;
			if (v0.y < pt_min.y)
				pt_min.y = v0.y;
			if (v0.z < pt_min.z)
				pt_min.z = v0.z;

			if (v0.x > pt_max.x)
				pt_max.x = v0.x;
			if (v0.y > pt_max.y)
				pt_max.y = v0.y;
			if (v0.z > pt_max.z)
				pt_max.z = v0.z;

			if (v1.x < pt_min.x)
				pt_min.x = v1.x;
			if (v1.y < pt_min.y)
				pt_min.y = v1.y;
			if (v1.z < pt_min.z)
				pt_min.z = v1.z;

			if (v1.x > pt_max.x)
				pt_max.x = v1.x;
			if (v1.y > pt_max.y)
				pt_max.y = v1.y;
			if (v1.z > pt_max.z)
				pt_max.z = v1.z;

			lines.append(v0);
			lines.append(v1);

		} else {
			grests.write[i] = get_bone_rest(i) * get_bone_pose(i);
		}
	}

	for (int pt = 0; pt < lines.size(); pt++) {
		const Vector3 &v = lines[pt];
		_figure_lines.push_back(Vector2(v.x, v.y));
	}

	_figure_aabb.position = pt_min;
	_figure_aabb.size = pt_max - pt_min;
}

void Figure::_update_xform_values() {
	figure_angle = _figure_xform.get_euler();
	figure_scale = _figure_xform.get_scale();
	_figure_xform_dirty = false;
}

void Figure::_update_transform() {
	_figure_xform.set_euler_scale(figure_angle, figure_scale);

	Transform2D xform(get_rotation(), get_scale(), get_position() + figure_offset);
	VisualServer::get_singleton()->canvas_item_set_transform(get_canvas_item(), xform);

	_figure_dirty = true;

	if (!is_inside_tree())
		return;

	_notify_transform();
	update();
}

void Figure::set_figure_scale(const Vector3 &p_scale) {
	if (_figure_xform_dirty)
		_update_xform_values();
	figure_scale = p_scale * Vector3(1, -1, 1); // by default we y-mirror
	// Avoid having 0 scale values, can lead to errors in physics and rendering.
	if (figure_scale.x == 0)
		figure_scale.x = CMP_EPSILON;
	if (figure_scale.y == 0)
		figure_scale.y = CMP_EPSILON;
	if (figure_scale.z == 0)
		figure_scale.z = CMP_EPSILON;
	_update_transform();
}

Vector3 Figure::get_figure_scale() const {
	if (_figure_xform_dirty)
		((Figure *)this)->_update_xform_values();

	return figure_scale * Vector3(1, -1, 1);
}

void Figure::set_figure_rotation(Vector3 p_radians) {
	if (_figure_xform_dirty)
		_update_xform_values();
	figure_angle = p_radians;
	_update_transform();
}

Vector3 Figure::get_figure_rotation() const {
	if (_figure_xform_dirty)
		((Figure *)this)->_update_xform_values();

	return figure_angle;
}

void Figure::set_figure_rotation_x_degrees(float p_degrees) {
	figure_angle.x = Math::deg2rad(p_degrees);
	_update_transform();
}

void Figure::set_figure_rotation_y_degrees(float p_degrees) {
	figure_angle.y = Math::deg2rad(p_degrees);
	_update_transform();
}

void Figure::set_figure_rotation_z_degrees(float p_degrees) {
	figure_angle.z = Math::deg2rad(p_degrees);
	_update_transform();
}

float Figure::get_figure_rotation_x_degrees() const {
	return Math::rad2deg(figure_angle.x);
}

float Figure::get_figure_rotation_y_degrees() const {
	return Math::rad2deg(figure_angle.y);
}

float Figure::get_figure_rotation_z_degrees() const {
	return Math::rad2deg(figure_angle.z);
}

void Figure::set_figure_orientation(const Basis &p_basis) {
	_figure_xform = p_basis * FLIP_Y;
	_figure_xform_dirty = true;

	if (!is_inside_tree())
		return;

	_notify_transform();
}

Basis Figure::get_figure_orientation() const {
	return _figure_xform * FLIP_Y;
}

bool Figure::_set(const StringName &p_path, const Variant &p_value) {
	String path = p_path;

	if (!path.begins_with("bones/"))
		return false;

	int which = path.get_slicec('/', 1).to_int();
	String what = path.get_slicec('/', 2);

	if (which == bones.size() && what == "name") {
		add_bone(p_value);
		return true;
	}

	ERR_FAIL_INDEX_V(which, bones.size(), false);

	if (what == "parent")
		set_bone_parent(which, p_value);
	else if (what == "rest")
		set_bone_rest(which, p_value);
	else if (what == "enabled")
		set_bone_enabled(which, p_value);
	else if (what == "pose")
		set_bone_pose(which, p_value);
	else if (what == "bound_children") {
		Array children = p_value;

		if (is_inside_tree()) {
			bones.write[which].nodes_bound.clear();

			for (int i = 0; i < children.size(); i++) {
				NodePath npath = children[i];
				ERR_CONTINUE(npath.operator String() == "");
				Node *node = get_node(npath);
				ERR_CONTINUE(!node);
				bind_child_node_to_bone(which, node);
			}
		}
	} else {
		return false;
	}

	return true;
}

bool Figure::_get(const StringName &p_path, Variant &r_ret) const {
	String path = p_path;

	if (!path.begins_with("bones/"))
		return false;

	int which = path.get_slicec('/', 1).to_int();
	String what = path.get_slicec('/', 2);

	ERR_FAIL_INDEX_V(which, bones.size(), false);

	if (what == "name")
		r_ret = get_bone_name(which);
	else if (what == "parent")
		r_ret = get_bone_parent(which);
	else if (what == "rest")
		r_ret = get_bone_rest(which);
	else if (what == "enabled")
		r_ret = is_bone_enabled(which);
	else if (what == "pose")
		r_ret = get_bone_pose(which);
	else if (what == "bound_children") {
		Array children;

		for (const List<uint32_t>::Element *E = bones[which].nodes_bound.front(); E; E = E->next()) {
			Object *obj = ObjectDB::get_instance(E->get());
			ERR_CONTINUE(!obj);
			Node *node = cast_to<Node>(obj);
			ERR_CONTINUE(!node);
			NodePath npath = get_path_to(node);
			children.push_back(npath);
		}

		r_ret = children;
	} else
		return false;

	return true;
}
void Figure::_get_property_list(List<PropertyInfo> *p_list) const {
	for (int i = 0; i < bones.size(); i++) {
		String prep = "bones/" + itos(i) + "/";
		p_list->push_back(PropertyInfo(Variant::STRING, prep + "name"));
		p_list->push_back(PropertyInfo(Variant::INT, prep + "parent", PROPERTY_HINT_RANGE, "-1," + itos(bones.size() - 1) + ",1"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, prep + "rest"));
		p_list->push_back(PropertyInfo(Variant::BOOL, prep + "enabled"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, prep + "pose", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));
		p_list->push_back(PropertyInfo(Variant::ARRAY, prep + "bound_children"));
	}
}

void Figure::_update_process_order() {
	if (!process_order_dirty)
		return;

	Bone *bonesptr = bones.ptrw();
	int len = bones.size();

	process_order.resize(len);
	int *order = process_order.ptrw();
	for (int i = 0; i < len; i++) {
		if (bonesptr[i].parent >= len) {
			//validate this just in case
			ERR_PRINT("Bone " + itos(i) + " has invalid parent: " + itos(bonesptr[i].parent));
			bonesptr[i].parent = -1;
		}
		order[i] = i;
		bonesptr[i].sort_index = i;
	}
	//now check process order
	int pass_count = 0;
	while (pass_count < len * len) {
		//using bubblesort because of simplicity, it won't run every frame though.
		//bublesort worst case is O(n^2), and this may be an infinite loop if cyclic
		bool swapped = false;
		for (int i = 0; i < len; i++) {
			int parent_idx = bonesptr[order[i]].parent;
			if (parent_idx < 0)
				continue; //do nothing because it has no parent
			//swap indices
			int parent_order = bonesptr[parent_idx].sort_index;
			if (parent_order > i) {
				bonesptr[order[i]].sort_index = parent_order;
				bonesptr[parent_idx].sort_index = i;
				//swap order
				SWAP(order[i], order[parent_order]);
				swapped = true;
			}
		}

		if (!swapped)
			break;
		pass_count++;
	}

	if (len && pass_count == len * len) {
		ERR_PRINT("Figure parenthood graph is cyclic");
	}

	process_order_dirty = false;
}

void Figure::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (_figure_dirty) {
				_update_figure_shape();
				_figure_dirty = false;
			}
			if (_figure_lines.size()) {
				draw_multiline(_figure_lines, Color(1, 1, 1, 0.8));
				draw_rect(Rect2(_figure_aabb.position.x, _figure_aabb.position.y, _figure_aabb.size.x, _figure_aabb.size.y), Color::named("yellow"), false);
			}

		} break;

		case NOTIFICATION_UPDATE_FIGURE: {
			Bone *bonesptr = bones.ptrw();
			int len = bones.size();

			_update_process_order();

			const int *order = process_order.ptr();

			for (int i = 0; i < len; i++) {
				Bone &b = bonesptr[order[i]];

				if (b.global_pose_override_amount >= 0.999) {
					b.pose_global = b.global_pose_override;
				} else {
					if (b.disable_rest) {
						if (b.enabled) {
							Transform pose = b.pose;
							if (b.custom_pose_enable) {
								pose = b.custom_pose * pose;
							}
							if (b.parent >= 0) {
								b.pose_global = bonesptr[b.parent].pose_global * pose;
							} else {
								b.pose_global = pose;
							}
						} else {
							if (b.parent >= 0) {
								b.pose_global = bonesptr[b.parent].pose_global;
							} else {
								b.pose_global = Transform();
							}
						}

					} else {
						if (b.enabled) {
							Transform pose = b.pose;
							if (b.custom_pose_enable) {
								pose = b.custom_pose * pose;
							}
							if (b.parent >= 0) {
								b.pose_global = bonesptr[b.parent].pose_global * (b.rest * pose);
							} else {
								b.pose_global = b.rest * pose;
							}
						} else {
							if (b.parent >= 0) {
								b.pose_global = bonesptr[b.parent].pose_global * b.rest;
							} else {
								b.pose_global = b.rest;
							}
						}
					}

					if (b.global_pose_override_amount >= CMP_EPSILON) {
						b.pose_global = b.pose_global.interpolate_with(b.global_pose_override, b.global_pose_override_amount);
					}
				}

				if (b.global_pose_override_reset) {
					b.global_pose_override_amount = 0.0;
				}
			}

			_figure_dirty = true;
			dirty = false;

			update();
		} break;
	}
}

void Figure::clear_bones_global_pose_override() {
	for (int i = 0; i < bones.size(); i += 1) {
		bones.write[i].global_pose_override_amount = 0;
	}
	_make_dirty();
}

void Figure::set_bone_global_pose_override(int p_bone, const Transform &p_pose, float p_amount, bool p_persistent) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].global_pose_override_amount = p_amount;
	bones.write[p_bone].global_pose_override = p_pose;
	bones.write[p_bone].global_pose_override_reset = !p_persistent;
	_make_dirty();
}

Transform Figure::get_bone_global_pose(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	if (dirty)
		const_cast<Figure *>(this)->notification(NOTIFICATION_UPDATE_FIGURE);
	return bones[p_bone].pose_global;
}

// Figure creation api
void Figure::add_bone(const String &p_name) {
	ERR_FAIL_COND(p_name == "" || p_name.find(":") != -1 || p_name.find("/") != -1);

	for (int i = 0; i < bones.size(); i++) {
		ERR_FAIL_COND(bones[i].name == p_name);
	}

	Bone b;
	b.name = p_name;
	bones.push_back(b);
	process_order_dirty = true;
	version++;
	_make_dirty();
	update();
}
int Figure::find_bone(const String &p_name) const {
	for (int i = 0; i < bones.size(); i++) {
		if (bones[i].name == p_name)
			return i;
	}

	return -1;
}
String Figure::get_bone_name(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), "");

	return bones[p_bone].name;
}

bool Figure::is_bone_parent_of(int p_bone, int p_parent_bone_id) const {
	int parent_of_bone = get_bone_parent(p_bone);

	if (-1 == parent_of_bone)
		return false;

	if (parent_of_bone == p_parent_bone_id)
		return true;

	return is_bone_parent_of(parent_of_bone, p_parent_bone_id);
}

int Figure::get_bone_count() const {
	return bones.size();
}

void Figure::set_bone_parent(int p_bone, int p_parent) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ERR_FAIL_COND(p_parent != -1 && (p_parent < 0));

	bones.write[p_bone].parent = p_parent;
	process_order_dirty = true;
	_make_dirty();
}

void Figure::unparent_bone_and_rest(int p_bone) {
	ERR_FAIL_INDEX(p_bone, bones.size());

	_update_process_order();

	int parent = bones[p_bone].parent;
	while (parent >= 0) {
		bones.write[p_bone].rest = bones[parent].rest * bones[p_bone].rest;
		parent = bones[parent].parent;
	}

	bones.write[p_bone].parent = -1;
	process_order_dirty = true;

	_make_dirty();
}

void Figure::set_bone_disable_rest(int p_bone, bool p_disable) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].disable_rest = p_disable;
}

bool Figure::is_bone_rest_disabled(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), false);
	return bones[p_bone].disable_rest;
}

int Figure::get_bone_parent(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), -1);

	return bones[p_bone].parent;
}

void Figure::set_bone_rest(int p_bone, const Transform &p_rest) {
	ERR_FAIL_INDEX(p_bone, bones.size());

	bones.write[p_bone].rest = p_rest;
	_make_dirty();
}
Transform Figure::get_bone_rest(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());

	return bones[p_bone].rest;
}

void Figure::set_bone_enabled(int p_bone, bool p_enabled) {
	ERR_FAIL_INDEX(p_bone, bones.size());

	bones.write[p_bone].enabled = p_enabled;
	_make_dirty();
}
bool Figure::is_bone_enabled(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), false);
	return bones[p_bone].enabled;
}

void Figure::bind_child_node_to_bone(int p_bone, Node *p_node) {
	ERR_FAIL_NULL(p_node);
	ERR_FAIL_INDEX(p_bone, bones.size());

	uint32_t id = p_node->get_instance_id();

	for (const List<uint32_t>::Element *E = bones[p_bone].nodes_bound.front(); E; E = E->next()) {
		if (E->get() == id)
			return; // already here
	}

	bones.write[p_bone].nodes_bound.push_back(id);
}
void Figure::unbind_child_node_from_bone(int p_bone, Node *p_node) {
	ERR_FAIL_NULL(p_node);
	ERR_FAIL_INDEX(p_bone, bones.size());

	uint32_t id = p_node->get_instance_id();
	bones.write[p_bone].nodes_bound.erase(id);
}
void Figure::get_bound_child_nodes_to_bone(int p_bone, List<Node *> *p_bound) const {
	ERR_FAIL_INDEX(p_bone, bones.size());

	for (const List<uint32_t>::Element *E = bones[p_bone].nodes_bound.front(); E; E = E->next()) {
		Object *obj = ObjectDB::get_instance(E->get());
		ERR_CONTINUE(!obj);
		p_bound->push_back(cast_to<Node>(obj));
	}
}

void Figure::clear_bones() {
	bones.clear();
	process_order_dirty = true;
	version++;
	_make_dirty();
}

// posing api

void Figure::set_bone_pose(int p_bone, const Transform &p_pose) {
	ERR_FAIL_INDEX(p_bone, bones.size());

	bones.write[p_bone].pose = p_pose;
	if (is_inside_tree()) {
		_make_dirty();
	}
}
Transform Figure::get_bone_pose(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	return bones[p_bone].pose;
}

void Figure::set_bone_custom_pose(int p_bone, const Transform &p_custom_pose) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	//ERR_FAIL_COND( !is_inside_scene() );

	bones.write[p_bone].custom_pose_enable = (p_custom_pose != Transform());
	bones.write[p_bone].custom_pose = p_custom_pose;

	_make_dirty();
}

Transform Figure::get_bone_custom_pose(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	return bones[p_bone].custom_pose;
}

void Figure::_make_dirty() {
	if (dirty)
		return;

	MessageQueue::get_singleton()->push_notification(this, NOTIFICATION_UPDATE_FIGURE);
	dirty = true;
}

int Figure::get_process_order(int p_idx) {
	ERR_FAIL_INDEX_V(p_idx, bones.size(), -1);
	_update_process_order();
	return process_order[p_idx];
}

void Figure::localize_rests() {
	_update_process_order();

	for (int i = bones.size() - 1; i >= 0; i--) {
		int idx = process_order[i];
		if (bones[idx].parent >= 0) {
			set_bone_rest(idx, bones[bones[idx].parent].rest.affine_inverse() * bones[idx].rest);
		}
	}
}

Rect2 Figure::get_rect() const {
	return Rect2(_figure_aabb.position.x, _figure_aabb.position.y, _figure_aabb.size.x, _figure_aabb.size.y);
}

Point2 Figure::get_figure_offset() const {
	return figure_offset;
}

void Figure::set_figure_offset(const Point2 &p_offset) {
	figure_offset = p_offset;
	_update_transform();
}

void Figure::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_bone", "name"), &Figure::add_bone);
	ClassDB::bind_method(D_METHOD("find_bone", "name"), &Figure::find_bone);
	ClassDB::bind_method(D_METHOD("get_bone_name", "bone_idx"), &Figure::get_bone_name);

	ClassDB::bind_method(D_METHOD("get_bone_parent", "bone_idx"), &Figure::get_bone_parent);
	ClassDB::bind_method(D_METHOD("set_bone_parent", "bone_idx", "parent_idx"), &Figure::set_bone_parent);

	ClassDB::bind_method(D_METHOD("get_bone_count"), &Figure::get_bone_count);

	ClassDB::bind_method(D_METHOD("unparent_bone_and_rest", "bone_idx"), &Figure::unparent_bone_and_rest);

	ClassDB::bind_method(D_METHOD("get_bone_rest", "bone_idx"), &Figure::get_bone_rest);
	ClassDB::bind_method(D_METHOD("set_bone_rest", "bone_idx", "rest"), &Figure::set_bone_rest);

	ClassDB::bind_method(D_METHOD("localize_rests"), &Figure::localize_rests);

	ClassDB::bind_method(D_METHOD("set_bone_disable_rest", "bone_idx", "disable"), &Figure::set_bone_disable_rest);
	ClassDB::bind_method(D_METHOD("is_bone_rest_disabled", "bone_idx"), &Figure::is_bone_rest_disabled);

	ClassDB::bind_method(D_METHOD("bind_child_node_to_bone", "bone_idx", "node"), &Figure::bind_child_node_to_bone);
	ClassDB::bind_method(D_METHOD("unbind_child_node_from_bone", "bone_idx", "node"), &Figure::unbind_child_node_from_bone);
	ClassDB::bind_method(D_METHOD("get_bound_child_nodes_to_bone", "bone_idx"), &Figure::_get_bound_child_nodes_to_bone);

	ClassDB::bind_method(D_METHOD("clear_bones"), &Figure::clear_bones);

	ClassDB::bind_method(D_METHOD("get_bone_pose", "bone_idx"), &Figure::get_bone_pose);
	ClassDB::bind_method(D_METHOD("set_bone_pose", "bone_idx", "pose"), &Figure::set_bone_pose);

	ClassDB::bind_method(D_METHOD("clear_bones_global_pose_override"), &Figure::clear_bones_global_pose_override);
	ClassDB::bind_method(D_METHOD("set_bone_global_pose_override", "bone_idx", "pose", "amount", "persistent"), &Figure::set_bone_global_pose_override, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_bone_global_pose", "bone_idx"), &Figure::get_bone_global_pose);

	ClassDB::bind_method(D_METHOD("get_bone_custom_pose", "bone_idx"), &Figure::get_bone_custom_pose);
	ClassDB::bind_method(D_METHOD("set_bone_custom_pose", "bone_idx", "custom_pose"), &Figure::set_bone_custom_pose);

	ClassDB::bind_method(D_METHOD("set_figure_offset", "offset"), &Figure::set_figure_offset);
	ClassDB::bind_method(D_METHOD("get_figure_offset"), &Figure::get_figure_offset);
	ClassDB::bind_method(D_METHOD("set_figure_scale", "figure_scale"), &Figure::set_figure_scale);
	ClassDB::bind_method(D_METHOD("get_figure_scale"), &Figure::get_figure_scale);
	ClassDB::bind_method(D_METHOD("set_figure_rotation", "figure_rotation"), &Figure::set_figure_rotation);
	ClassDB::bind_method(D_METHOD("get_figure_rotation"), &Figure::get_figure_rotation);
	ClassDB::bind_method(D_METHOD("set_figure_rotation_x_degrees", "figure_rotation_x_degrees"), &Figure::set_figure_rotation_x_degrees);
	ClassDB::bind_method(D_METHOD("get_figure_rotation_x_degrees"), &Figure::get_figure_rotation_x_degrees);
	ClassDB::bind_method(D_METHOD("set_figure_rotation_y_degrees", "figure_rotation_x_degrees"), &Figure::set_figure_rotation_y_degrees);
	ClassDB::bind_method(D_METHOD("get_figure_rotation_y_degrees"), &Figure::get_figure_rotation_y_degrees);
	ClassDB::bind_method(D_METHOD("set_figure_rotation_z_degrees", "figure_rotation_x_degrees"), &Figure::set_figure_rotation_z_degrees);
	ClassDB::bind_method(D_METHOD("get_figure_rotation_z_degrees"), &Figure::get_figure_rotation_z_degrees);
	ClassDB::bind_method(D_METHOD("set_figure_orientation", "figure_orientation"), &Figure::set_figure_orientation);
	ClassDB::bind_method(D_METHOD("get_figure_orientation"), &Figure::get_figure_orientation);

	ADD_GROUP("Figure Transform", "figure_");
	ADD_PROPERTY(PropertyInfo(Variant::BASIS, "figure_orientation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_figure_orientation", "get_figure_orientation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "figure_offset"), "set_figure_offset", "get_figure_offset");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "figure_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_figure_rotation", "get_figure_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "figure_rotation_x_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_figure_rotation_x_degrees", "get_figure_rotation_x_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "figure_rotation_y_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_figure_rotation_y_degrees", "get_figure_rotation_y_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "figure_rotation_z_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_figure_rotation_z_degrees", "get_figure_rotation_z_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "figure_scale"), "set_figure_scale", "get_figure_scale");

	BIND_CONSTANT(NOTIFICATION_UPDATE_FIGURE);
}

Figure::Figure() :
		_bone_transformer(this) {
	figure_offset = Point2(0, 0);
	figure_angle = Vector3(0, 0, 0);
	figure_scale = Vector3(1, -1, 1);
	dirty = false;
	version = 1;
	process_order_dirty = true;

	_figure_xform = Basis(figure_angle, figure_scale);
	_figure_aabb = AABB();
	_figure_xform_dirty = false;
	_figure_dirty = false;

	set_meta(BONE_TRANSFORMER_KEY, &_bone_transformer);
}

Figure::~Figure() {
}
