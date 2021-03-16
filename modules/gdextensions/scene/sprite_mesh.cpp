/*************************************************************************/
/*  sprite_mesh.cpp                                                      */
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

#include <string>
#include <vector>

#include "core/io/json.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "scene/3d/mesh_instance.h"
#include "scene/resources/mesh.h"
#include "scene/resources/mesh_data_tool.h"

#include "sprite_mesh.h"
#include "tinyexpr.h"

// References:
// -----------
// https://stackoverflow.com/questions/6053522/how-to-recalculate-axis-aligned-bounding-box-after-translate-rotate
// https://github.com/libigl/libigl/issues/514 (Projecting mesh onto a plane)
// https://github.com/pyvista/pyvista-support/issues/20 (Project points/surface to a plane)
// https://github.com/patrikhuber/eos/issues/140
// * https://gamedev.stackexchange.com/questions/10261/android-collision-detection-of-a-3d-object-based-on-a-2d-projection

#define get_mesh_surf_info(mesh) \
	((Dictionary)(mesh->has_meta("_mesh_surf_info") ? (Dictionary)mesh->get_meta("_mesh_surf_info") : Dictionary()))

#ifdef TOOLS_ENABLED
Dictionary SpriteMesh::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["offset"] = offset;
	return state;
}

void SpriteMesh::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_offset(p_state["offset"]);
}

void SpriteMesh::_edit_set_pivot(const Point2 &p_pivot) {
	set_offset(get_offset() - p_pivot);
	set_position(get_transform().xform(p_pivot));
}

Point2 SpriteMesh::_edit_get_pivot() const {
	return Vector2();
}

bool SpriteMesh::_edit_use_pivot() const {
	return true;
}

bool SpriteMesh::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return get_rect().has_point(p_point);
}

Rect2 SpriteMesh::_edit_get_rect() const {
	return get_rect();
}

bool SpriteMesh::_edit_use_rect() const {
	return mesh.is_valid();
}
#endif

void SpriteMesh::_update_mesh_xform() {
	if (mesh.is_valid() && !_mesh_data.empty()) {
		if (ArrayMesh *array_mesh = Object::cast_to<ArrayMesh>(*mesh)) {

			PoolVector3Array vertexes = _mesh_data[VS::ARRAY_VERTEX];
			PoolVector3Array xform_vertexes;
			ERR_FAIL_COND(xform_vertexes.resize(vertexes.size()) != OK);

			auto w = xform_vertexes.write();
			for (int v = 0; v < vertexes.size(); ++v) {
				w[v] = _mesh_xform.xform(vertexes[v]);
			}
			// build transformed mesh
			Array mesh_array = _mesh_data.duplicate();
			mesh_array[VS::ARRAY_VERTEX] = xform_vertexes;
			array_mesh->clear_mesh();
			// array_mesh->surface_remove(active_frame);
			// int surf_id = array_mesh->get_surface_count();
			array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);

			item_rect_changed();
		} else {
			WARN_PRINT("Cannot transform non ArrayMesh object.");
		}
	}
}

void SpriteMesh::_notification(int p_what) {
	if (p_what == NOTIFICATION_DRAW) {
		if (_mesh_dirty) {
			_update_mesh_xform();
			_mesh_dirty = false;
		}
		if (mesh.is_valid()) {
			const AABB &aabb = mesh->get_aabb();

			Size2 s(aabb.size.x, aabb.size.y);

			Point2 ofs = offset;
			if (centered)
				ofs -= Size2(s) / 2;

			Transform2D xform(0, ofs - Point2(aabb.position.x, aabb.position.y));
			draw_mesh(mesh, texture, normal_map, xform);
			if (_mesh_debug) {
				draw_rect(Rect2(ofs, s), Color::named("yellow"), false);
			}
		}
	}
}

void SpriteMesh::set_mesh(const Ref<Mesh> &p_mesh) {

	if (mesh == p_mesh)
		return;

	if (*p_mesh && Object::cast_to<ArrayMesh>(*p_mesh) == nullptr) {
		WARN_PRINT("Not an ArrayMesh object: do not know how to transform this.");
		return;
	}

	mesh = p_mesh;

	_mesh_data.clear();
	if (ArrayMesh *array_mesh = Object::cast_to<ArrayMesh>(*mesh))
		_mesh_data = array_mesh->surface_get_arrays(0);

	_mesh_dirty = true;
	_update_transform();
	item_rect_changed();
	update();
	emit_signal("mesh_changed");
	_change_notify("mesh");
}

Ref<Mesh> SpriteMesh::get_mesh() const {
	return mesh;
}

void SpriteMesh::_update_xform_values() {

	mesh_angle = _mesh_xform.get_euler();
	mesh_scale = _mesh_xform.get_scale();
	_mesh_xform_dirty = false;
}

void SpriteMesh::_update_transform() {

	_mesh_xform.set_euler_scale(mesh_angle, mesh_scale);

	if (!is_inside_tree())
		return;

	_mesh_dirty = true;

	_notify_transform();
	update();
}

void SpriteMesh::set_mesh_rotation(Vector3 p_radians) {

	if (_mesh_xform_dirty)
		_update_xform_values();
	mesh_angle = p_radians;
	_update_transform();
}

Vector3 SpriteMesh::get_mesh_rotation() const {
	if (_mesh_xform_dirty)
		((SpriteMesh *)this)->_update_xform_values();

	return mesh_angle;
}

void SpriteMesh::set_mesh_rotation_x_degrees(float p_degrees) {

	mesh_angle.x = Math::deg2rad(p_degrees);
	_update_transform();
}

float SpriteMesh::get_mesh_rotation_x_degrees() const {

	return Math::rad2deg(mesh_angle.x);
}

void SpriteMesh::set_mesh_rotation_y_degrees(float p_degrees) {

	mesh_angle.y = Math::deg2rad(p_degrees);
	_update_transform();
}

float SpriteMesh::get_mesh_rotation_y_degrees() const {

	return Math::rad2deg(mesh_angle.y);
}

void SpriteMesh::set_mesh_rotation_z_degrees(float p_degrees) {

	mesh_angle.z = Math::deg2rad(p_degrees);
	_update_transform();
}

float SpriteMesh::get_mesh_rotation_z_degrees() const {

	return Math::rad2deg(mesh_angle.z);
}

void SpriteMesh::set_mesh_orientation(const Basis &p_basis) {

	_mesh_xform = p_basis;
	_mesh_xform_dirty = true;

	if (!is_inside_tree())
		return;

	_notify_transform();
}

Basis SpriteMesh::get_mesh_orientation() const {

	return _mesh_xform;
}

void SpriteMesh::set_mesh_scale(const Vector3 &p_scale) {

	if (_mesh_xform_dirty)
		_update_xform_values();
	mesh_scale = p_scale;
	// Avoid having 0 scale values, can lead to errors in physics and rendering.
	if (mesh_scale.x == 0)
		mesh_scale.x = CMP_EPSILON;
	if (mesh_scale.y == 0)
		mesh_scale.y = CMP_EPSILON;
	if (mesh_scale.z == 0)
		mesh_scale.z = CMP_EPSILON;
	_update_transform();
}

Vector3 SpriteMesh::get_mesh_scale() const {
	if (_mesh_xform_dirty)
		((SpriteMesh *)this)->_update_xform_values();

	return mesh_scale;
}

void SpriteMesh::set_mesh_texture(const Ref<Texture> &p_texture) {

	if (texture == p_texture)
		return;
	texture = p_texture;
	update();
	emit_signal("texture_changed");
	_change_notify("texture");
}

Ref<Texture> SpriteMesh::get_mesh_texture() const {
	return texture;
}

void SpriteMesh::set_mesh_normal_map(const Ref<Texture> &p_texture) {

	if (normal_map != p_texture) {
		normal_map = p_texture;
		update();
	}
}

Ref<Texture> SpriteMesh::get_mesh_normal_map() const {

	return normal_map;
}

void SpriteMesh::set_selected_frame(int p_frame) {

	if (selected_frame != p_frame) {
		selected_frame = p_frame;
	}
}

int SpriteMesh::get_selected_frame() const {
	return selected_frame;
}

void SpriteMesh::set_frames_builder(const String &p_input) {

	if (frames_builder != p_input) {
		if (!p_input.empty()) {
			Variant input;
			String err_message;
			int err_line;
			if (OK != JSON::parse(p_input, input, err_message, err_line)) {
				WARN_PRINT(vformat("Failed to parse input JSON data - error at line %d: %s", err_line, err_message));
				return;
			}
			if (input.get_type() == Variant::ARRAY) {
				// process frames
				_frames.clear();
				frames_builder = p_input;
				Array frames = input;
				int frame_nr = 0;
				for (int f = 0; f < frames.size(); f++) {
					if (input.get_type() == Variant::ARRAY) {
						Dictionary frame = frames[f];
						int rep = 1;
						if (frame.has("repeat")) {
						}
						real_t rotation[3] = { 0, 0, 0 };
						real_t scaling[3] = { 0, 0, 0 };
						for (int loop = 0; loop < rep; ++loop) {
							if (frame.has("rotate")) {
								if (frame["rotate"].get_type() == Variant::ARRAY) {
									Array rotates = frame["rotate"];
									for (int c = 0; c < 3; c++) {
										switch (rotates[c].get_type()) {
											case Variant::REAL: {
												rotation[c] = real_t(rotates[c]);
											} break;
											case Variant::STRING: {
												String expr_str = rotates[c];
												te_variable vars[] = { { "repcount", &rep }, { "repcounter", &loop } };
												int err;
												if (te_expr *expr = te_compile(expr_str.utf8().get_data(), vars, 2, &err)) {
													rotation[c] = te_eval(expr);
													te_free(expr);
												} else {
													WARN_PRINT(vformat("Rotate expresion parse error at %d", err));
												}
											} break;
											default: {
												WARN_PRINT("rotate component can be float or String only");
											}
										}
									}
								} else {
									WARN_PRINT("rotate needs to be an Array");
								}
							}
							if (frame.has("scale")) {
								if (frame["scale"].get_type() == Variant::ARRAY) {
								} else {
									WARN_PRINT("scale needs to be an Array");
								}
							}
#ifdef DEBUG_ENABLED
							printf("frame %d) rotate=[%0.2f %0.2f %0.2f] scale=[%0.2f %0.2f %0.2f]\n",
									frame_nr,
									rotation[0], rotation[1], rotation[2],
									scaling[0], scaling[1], scaling[2]);
#endif
							++frame_nr;
						}
					} else {
						WARN_PRINT(vformat("Unknown format of frame %d. Skipping", f));
					}
				}
			} else {
				WARN_PRINT("Input JSON is not an Array.");
			}
		}
	}
}

String SpriteMesh::get_frames_builder() const {

	return frames_builder;
}

void SpriteMesh::set_centered(bool p_center) {

	centered = p_center;
	update();
	item_rect_changed();
}

bool SpriteMesh::is_centered() const {

	return centered;
}

void SpriteMesh::set_offset(const Point2 &p_offset) {

	offset = p_offset;
	update();
	item_rect_changed();
	_change_notify("offset");
}
Point2 SpriteMesh::get_offset() const {

	return offset;
}

Rect2 SpriteMesh::get_rect() const {
	if (mesh.is_null())
		return Rect2(0, 0, 1, 1);

	const AABB &aabb = mesh->get_aabb();

	Size2 s(aabb.size.x, aabb.size.y);

	Point2 ofs = offset;
	if (centered)
		ofs -= Size2(s) / 2;

	if (s == Size2(0, 0))
		s = Size2(1, 1);

	return Rect2(ofs, s);
}

Rect2 SpriteMesh::get_anchorable_rect() const {
	return get_rect();
}

void SpriteMesh::_validate_property(PropertyInfo &property) const {
}

void SpriteMesh::_mesh_changed() {

	// Changes to the mesh need to trigger an update to make
	// the editor redraw the sprite with the updated mesh.
	if (mesh.is_valid()) {
		update();
	}
}

void SpriteMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &SpriteMesh::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &SpriteMesh::get_mesh);
	ClassDB::bind_method(D_METHOD("set_mesh_texture", "mesh_texture"), &SpriteMesh::set_mesh_texture);
	ClassDB::bind_method(D_METHOD("get_mesh_texture"), &SpriteMesh::get_mesh_texture);
	ClassDB::bind_method(D_METHOD("set_mesh_normal_map", "mesh_normal_map"), &SpriteMesh::set_mesh_normal_map);
	ClassDB::bind_method(D_METHOD("get_mesh_normal_map"), &SpriteMesh::get_mesh_normal_map);

	ClassDB::bind_method(D_METHOD("set_selected_frame", "frame"), &SpriteMesh::set_selected_frame);
	ClassDB::bind_method(D_METHOD("get_selected_frame"), &SpriteMesh::get_selected_frame);
	ClassDB::bind_method(D_METHOD("set_frames_builder", "description"), &SpriteMesh::set_frames_builder);
	ClassDB::bind_method(D_METHOD("get_frames_builder"), &SpriteMesh::get_frames_builder);

	ClassDB::bind_method(D_METHOD("set_mesh_rotation", "mesh_rotation"), &SpriteMesh::set_mesh_rotation);
	ClassDB::bind_method(D_METHOD("get_mesh_rotation"), &SpriteMesh::get_mesh_rotation);
	ClassDB::bind_method(D_METHOD("set_mesh_rotation_x_degrees", "mesh_rotation_x_degrees"), &SpriteMesh::set_mesh_rotation_x_degrees);
	ClassDB::bind_method(D_METHOD("get_mesh_rotation_x_degrees"), &SpriteMesh::get_mesh_rotation_x_degrees);
	ClassDB::bind_method(D_METHOD("set_mesh_rotation_y_degrees", "mesh_rotation_x_degrees"), &SpriteMesh::set_mesh_rotation_y_degrees);
	ClassDB::bind_method(D_METHOD("get_mesh_rotation_y_degrees"), &SpriteMesh::get_mesh_rotation_y_degrees);
	ClassDB::bind_method(D_METHOD("set_mesh_rotation_z_degrees", "mesh_rotation_x_degrees"), &SpriteMesh::set_mesh_rotation_z_degrees);
	ClassDB::bind_method(D_METHOD("get_mesh_rotation_z_degrees"), &SpriteMesh::get_mesh_rotation_z_degrees);
	ClassDB::bind_method(D_METHOD("set_mesh_orientation", "mesh_orientation"), &SpriteMesh::set_mesh_orientation);
	ClassDB::bind_method(D_METHOD("get_mesh_orientation"), &SpriteMesh::get_mesh_orientation);

	ClassDB::bind_method(D_METHOD("set_mesh_scale", "mesh_scale"), &SpriteMesh::set_mesh_scale);
	ClassDB::bind_method(D_METHOD("get_mesh_scale"), &SpriteMesh::get_mesh_scale);

	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &SpriteMesh::set_centered);
	ClassDB::bind_method(D_METHOD("is_centered"), &SpriteMesh::is_centered);

	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &SpriteMesh::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &SpriteMesh::get_offset);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mesh_texture", "get_mesh_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_normal_map", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mesh_normal_map", "get_mesh_normal_map");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "select_frame"), "set_selected_frame", "get_selected_frame");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "frames_builder", PROPERTY_HINT_MULTILINE_TEXT, ""), "set_frames_builder", "get_frames_builder");

	ADD_GROUP("Mesh Transform", "mesh_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mesh_centered"), "set_centered", "is_centered");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "mesh_offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BASIS, "mesh_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_mesh_rotation", "get_mesh_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mesh_rotation_x_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_mesh_rotation_x_degrees", "get_mesh_rotation_x_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mesh_rotation_y_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_mesh_rotation_y_degrees", "get_mesh_rotation_y_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mesh_rotation_z_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_mesh_rotation_z_degrees", "get_mesh_rotation_z_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "mesh_scale"), "set_mesh_scale", "get_mesh_scale");

	ADD_SIGNAL(MethodInfo("texture_changed"));
	ADD_SIGNAL(MethodInfo("mesh_changed"));
}

SpriteMesh::SpriteMesh() {

	centered = true;
	offset = Vector2(0, 0);
	mesh_angle = Vector3(0, 0, 0);
	mesh_scale = Vector3(1, 1, 1);
	selected_frame = 0;
	frames_builder = "";
	_mesh_xform = Basis(mesh_angle, mesh_scale);
	_mesh_dirty = false;
	_mesh_xform_dirty = false;
	_mesh_active_surface = 0;
	_mesh_debug = false;
}

SpriteMesh::~SpriteMesh() {
}
