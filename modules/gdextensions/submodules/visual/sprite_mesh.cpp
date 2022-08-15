/*************************************************************************/
/*  sprite_mesh.cpp                                                      */
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

#include <map>
#include <string>
#include <vector>

#include "core/io/json.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/variant.h"
#include "editor/editor_node.h"
#include "scene/3d/mesh_instance.h"
#include "scene/resources/mesh.h"
#include "scene/resources/mesh_data_tool.h"
#include "scene/resources/surface_tool.h"

#include "sprite_mesh.h"
#include "thumb_wheel.h"
#include "tinyexpr.h"

// References:
// -----------
// https://stackoverflow.com/questions/6053522/how-to-recalculate-axis-aligned-bounding-box-after-translate-rotate
// https://github.com/libigl/libigl/issues/514 (Projecting mesh onto a plane)
// https://github.com/pyvista/pyvista-support/issues/20 (Project points/surface to a plane)
// https://github.com/patrikhuber/eos/issues/140
// * https://gamedev.stackexchange.com/questions/10261/android-collision-detection-of-a-3d-object-based-on-a-2d-projection
// https://github.com/Huangtingting93/Trace_outline/blob/main/trace_outline.py
// * https://www.h3xed.com/programming/create-2d-mesh-outline-in-unity-silhouette
// https://stackoverflow.com/questions/62748136/how-to-get-outer-edges-of-a-mesh-edges-which-are-part-of-only-one-triangle

#define GIZMO_ARROW_SIZE 0.35
#define GIZMO_RING_HALF_WIDTH 0.1
#define GIZMO_SCALE_DEFAULT 0.15
#define GIZMO_PLANE_SIZE 0.2
#define GIZMO_PLANE_DST 0.3
#define GIZMO_CIRCLE_SIZE 1.1
#define GIZMO_SCALE_OFFSET (GIZMO_CIRCLE_SIZE + 0.3)
#define GIZMO_ARROW_OFFSET (GIZMO_CIRCLE_SIZE + 0.3)

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

void SpriteMesh::_update_mesh_outline(const PoolVector3Array &p_vertices, const PoolIntArray &p_triangles) {
	ERR_FAIL_COND_MSG(p_triangles.size() == 0, "Automtic outline only works for indexed meshes");
	// Get just the outer edges from the mesh's triangles (ignore or remove any shared edges)
	typedef std::pair<int, int> index_pair_t;
	std::map<index_pair_t, index_pair_t> edges;
	for (int i = 0; i < p_triangles.size() - 1; i += 3) {
		for (int e = 0; e < 3; e++) {
			const int vert1 = p_triangles[i + e];
			const int vert2 = p_triangles[i + e + 1 > i + 2 ? i : i + e + 1];
			const auto &edge = index_pair_t(MIN(vert1, vert2), MAX(vert1, vert2));
			if (edges.count(edge)) {
				edges.erase(edge); // shared edge
			} else {
				edges[edge] = index_pair_t(vert1, vert2);
			}
		}
	}

	// Create edge lookup Dictionary
	std::map<int, int> lookup;
	for (const auto &edge : edges) {
		// const auto &key = edge.first;
		const auto &value = edge.second;
		if (lookup.count(value.first) == 0) {
			lookup[value.first] = value.second;
		}
	}

	mesh_outline.clear();

	int start_vert = 0;
	int next_vert = start_vert;
	int highest_vert = start_vert;
	while (true) { // loop through edge vertices in order
		mesh_outline.push_back(Vector2(p_vertices[next_vert].x, p_vertices[next_vert].y));
		next_vert = lookup[next_vert]; // get next vertex
		if (next_vert > highest_vert) { // store highest vertex (to know what shape to move to next)
			highest_vert = next_vert;
		}
		if (next_vert == start_vert) { // shape complete
			mesh_outline.push_back(Vector2(p_vertices[next_vert].x, p_vertices[next_vert].y)); // finish this shape's line
			if (lookup.count(highest_vert + 1)) { // go to next shape if one exists
				start_vert = highest_vert + 1; // set starting and next vertices
				next_vert = start_vert;
				continue; // continue to next loop
			}
			break; // no more verts
		}
	}
}

void SpriteMesh::_update_mesh_xform() {
	if (_mesh_ref.is_valid()) {
		if (!mesh.is_valid()) {
			mesh.instance();
		}
		mesh->clear_mesh();
		for (int s = 0; s < _mesh_ref->get_surface_count(); s++) {
			Array mesh_array = _mesh_ref->surface_get_arrays(s);

			PoolVector3Array vertexes = mesh_array[VS::ARRAY_VERTEX];
			PoolVector3Array xform_vertexes;
			ERR_FAIL_COND(xform_vertexes.resize(vertexes.size()) != OK);

			auto w = xform_vertexes.write();
			for (int v = 0; v < vertexes.size(); ++v) {
				w[v] = _mesh_xform.xform(vertexes[v]);
			}
			// build transformed mesh
			mesh_array[VS::ARRAY_VERTEX] = xform_vertexes;
			if (((PoolColorArray)mesh_array[VS::ARRAY_COLOR]).size() == 0) {
				// extract albedo color from material, since they are ignored in 2D
				PoolColorArray colors;
				Ref<SpatialMaterial> mat = _mesh_ref->surface_get_material(s);
				if (mat.is_valid()) {
					Color alb = mat->get_albedo();
					colors.push_multi(xform_vertexes.size(), alb);
				}
				mesh_array[VS::ARRAY_COLOR] = colors;
			}
			mesh->add_surface_from_arrays(_mesh_ref->surface_get_primitive_type(s), mesh_array, Array());

			// update outline shape
			if (auto_collision_shape) {
				_update_mesh_outline(xform_vertexes, mesh_array[VS::ARRAY_INDEX]);
			}
		}
		item_rect_changed();
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
			draw_mesh(mesh, texture, normal_map, mask, xform);
			if (_mesh_debug) {
				draw_rect(Rect2(ofs, s), Color::named("yellow"), false);
				if (mesh_outline.size()) {
					draw_polyline(mesh_outline, Color::named("magenta"));
				}
			}
		}
	}
}

void SpriteMesh::set_mesh(const Ref<Mesh> &p_mesh) {
	if (_mesh_ref == p_mesh) {
		return;
	}

	if (*p_mesh && cast_to<ArrayMesh>(*p_mesh) == nullptr) {
		WARN_PRINT("Not an ArrayMesh object: do not know how to transform this.");
		return;
	}

	_mesh_ref = p_mesh;
	_mesh_dirty = true;
	_update_transform();
	item_rect_changed();
	update();
	emit_signal("mesh_changed");
	_change_notify("mesh");
}

Ref<Mesh> SpriteMesh::get_mesh() const {
	return _mesh_ref;
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

void SpriteMesh::set_mesh_mask(const Ref<Texture> &p_texture) {
	if (mask != p_texture) {
		mask = p_texture;
		update();
	}
}

Ref<Texture> SpriteMesh::get_mesh_mask() const {
	return mask;
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
							_frames.push_back({ Basis(Vector3(rotation), Vector3(scaling)) });
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

void SpriteMesh::set_auto_collision_shape(bool state) {
	auto_collision_shape = state;
}

bool SpriteMesh::is_auto_collision_shape() {
	return auto_collision_shape;
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
	if (mesh.is_null()) {
		return Rect2(0, 0, 1, 1);
	}
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
	ClassDB::bind_method(D_METHOD("set_mesh_texture", "texture"), &SpriteMesh::set_mesh_texture);
	ClassDB::bind_method(D_METHOD("get_mesh_texture"), &SpriteMesh::get_mesh_texture);
	ClassDB::bind_method(D_METHOD("set_mesh_normal_map", "normal_map"), &SpriteMesh::set_mesh_normal_map);
	ClassDB::bind_method(D_METHOD("get_mesh_normal_map"), &SpriteMesh::get_mesh_normal_map);
	ClassDB::bind_method(D_METHOD("set_mesh_mask", "mask"), &SpriteMesh::set_mesh_mask);
	ClassDB::bind_method(D_METHOD("get_mesh_mask"), &SpriteMesh::get_mesh_mask);

	ClassDB::bind_method(D_METHOD("set_auto_collision_shape", "mode"), &SpriteMesh::set_auto_collision_shape);
	ClassDB::bind_method(D_METHOD("is_auto_collision_shape"), &SpriteMesh::is_auto_collision_shape);

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
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_mask", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mesh_mask", "get_mesh_mask");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "select_frame"), "set_selected_frame", "get_selected_frame");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "frames_builder", PROPERTY_HINT_MULTILINE_TEXT, ""), "set_frames_builder", "get_frames_builder");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_collision_shape"), "set_auto_collision_shape", "is_auto_collision_shape");

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
	ADD_SIGNAL(MethodInfo("transform_changed"));
}

SpriteMesh::SpriteMesh() {
	centered = true;
	offset = Vector2(0, 0);
	mesh_angle = Vector3(0, 0, 0);
	mesh_scale = Vector3(1, 1, 1);
	auto_collision_shape = false;
	selected_frame = 0;
	frames_builder = "";
	_mesh_xform = Basis(mesh_angle, mesh_scale);
	_mesh_dirty = false;
	_mesh_xform_dirty = false;
	_mesh_active_surface = 0;
	_mesh_debug = false;
}

/// SpriteMeshEditorPlugin

Color SpriteMeshEditor::_get_color(const StringName &p_name, const StringName &p_theme_type) const {
	return EditorNode::get_singleton()->get_gui_base()->get_color(p_name, p_theme_type);
}

void SpriteMeshEditor::_init_indicators() {
	{
		for (int i = 0; i < 3; i++) {
			Color col;
			switch (i) {
				case 0:
					col = _get_color("axis_x_color", "Editor");
					break;
				case 1:
					col = _get_color("axis_y_color", "Editor");
					break;
				case 2:
					col = _get_color("axis_z_color", "Editor");
					break;
				default:
					col = Color();
					break;
			}

			col.a = EditorSettings::get_singleton()->get("editors/3d/manipulator_gizmo_opacity");

			rotate_gizmo[i] = Ref<ArrayMesh>(memnew(ArrayMesh));
			scale_gizmo[i] = Ref<ArrayMesh>(memnew(ArrayMesh));
			scale_plane_gizmo[i] = Ref<ArrayMesh>(memnew(ArrayMesh));

			Ref<SpatialMaterial> mat = memnew(SpatialMaterial);
			mat->set_flag(SpatialMaterial::FLAG_UNSHADED, true);
			mat->set_on_top_of_alpha();
			mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
			mat->set_albedo(col);
			gizmo_color[i] = mat;

			Ref<SpatialMaterial> mat_hl = mat->duplicate();
			const Color albedo = col.from_hsv(col.get_h(), 0.25, 1.0, 1);
			mat_hl->set_albedo(albedo);
			gizmo_color_hl[i] = mat_hl;

			Vector3 ivec;
			ivec[i] = 1;
			Vector3 nivec;
			nivec[(i + 1) % 3] = 1;
			nivec[(i + 2) % 3] = 1;
			Vector3 ivec2;
			ivec2[(i + 1) % 3] = 1;
			Vector3 ivec3;
			ivec3[(i + 2) % 3] = 1;

			// Rotate
			{
				Ref<SurfaceTool> surftool = memnew(SurfaceTool);
				surftool->begin(Mesh::PRIMITIVE_TRIANGLES);

				int n = 128; // number of circle segments
				int m = 3; // number of thickness segments

				for (int j = 0; j < n; ++j) {
					Basis basis = Basis(ivec, (Math_PI * 2.0f * j) / n);
					Vector3 vertex = basis.xform(ivec2 * GIZMO_CIRCLE_SIZE);
					for (int k = 0; k < m; ++k) {
						Vector2 ofs = Vector2(Math::cos((Math_PI * 2.0 * k) / m), Math::sin((Math_PI * 2.0 * k) / m));
						Vector3 normal = ivec * ofs.x + ivec2 * ofs.y;
						surftool->add_normal(basis.xform(normal));
						surftool->add_vertex(vertex);
					}
				}

				for (int j = 0; j < n; ++j) {
					for (int k = 0; k < m; ++k) {
						int current_ring = j * m;
						int next_ring = ((j + 1) % n) * m;
						int current_segment = k;
						int next_segment = (k + 1) % m;

						surftool->add_index(current_ring + next_segment);
						surftool->add_index(current_ring + current_segment);
						surftool->add_index(next_ring + current_segment);

						surftool->add_index(next_ring + current_segment);
						surftool->add_index(next_ring + next_segment);
						surftool->add_index(current_ring + next_segment);
					}
				}

				Ref<Shader> rotate_shader = memnew(Shader);

				rotate_shader->set_code(
						"\n"
						"shader_type spatial; \n"
						"render_mode unshaded, depth_test_disable; \n"
						"uniform vec4 albedo; \n"
						"\n"
						"mat3 orthonormalize(mat3 m) { \n"
						"	vec3 x = normalize(m[0]); \n"
						"	vec3 y = normalize(m[1] - x * dot(x, m[1])); \n"
						"	vec3 z = m[2] - x * dot(x, m[2]); \n"
						"	z = normalize(z - y * (dot(y,m[2]))); \n"
						"	return mat3(x,y,z); \n"
						"} \n"
						"\n"
						"void vertex() { \n"
						"	mat3 mv = orthonormalize(mat3(MODELVIEW_MATRIX)); \n"
						"	vec3 n = mv * VERTEX; \n"
						"	float orientation = dot(vec3(0,0,-1),n); \n"
						"	if (orientation <= 0.005) { \n"
						"		VERTEX += NORMAL*0.02; \n"
						"	} \n"
						"} \n"
						"\n"
						"void fragment() { \n"
						"	ALBEDO = albedo.rgb; \n"
						"	ALPHA = albedo.a; \n"
						"}");

				Ref<ShaderMaterial> rotate_mat = memnew(ShaderMaterial);
				rotate_mat->set_render_priority(Material::RENDER_PRIORITY_MAX);
				rotate_mat->set_shader(rotate_shader);
				rotate_mat->set_shader_param("albedo", col);
				rotate_gizmo_color[i] = rotate_mat;

				Array arrays = surftool->commit_to_arrays();
				rotate_gizmo[i]->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
				rotate_gizmo[i]->surface_set_material(0, rotate_mat);

				Ref<ShaderMaterial> rotate_mat_hl = rotate_mat->duplicate();
				rotate_mat_hl->set_shader_param("albedo", albedo);
				rotate_gizmo_color_hl[i] = rotate_mat_hl;

				if (i == 2) { // Rotation white outline
					Ref<ShaderMaterial> border_mat = rotate_mat->duplicate();

					Ref<Shader> border_shader = memnew(Shader);
					border_shader->set_code(
							"\n"
							"shader_type spatial; \n"
							"render_mode unshaded, depth_test_disable; \n"
							"uniform vec4 albedo; \n"
							"\n"
							"mat3 orthonormalize(mat3 m) { \n"
							"	vec3 x = normalize(m[0]); \n"
							"	vec3 y = normalize(m[1] - x * dot(x, m[1])); \n"
							"	vec3 z = m[2] - x * dot(x, m[2]); \n"
							"	z = normalize(z - y * (dot(y,m[2]))); \n"
							"	return mat3(x,y,z); \n"
							"} \n"
							"\n"
							"void vertex() { \n"
							"	mat3 mv = orthonormalize(mat3(MODELVIEW_MATRIX)); \n"
							"	mv = inverse(mv); \n"
							"	VERTEX += NORMAL*0.008; \n"
							"	vec3 camera_dir_local = mv * vec3(0,0,1); \n"
							"	vec3 camera_up_local = mv * vec3(0,1,0); \n"
							"	mat3 rotation_matrix = mat3(cross(camera_dir_local, camera_up_local), camera_up_local, camera_dir_local); \n"
							"	VERTEX = rotation_matrix * VERTEX; \n"
							"} \n"
							"\n"
							"void fragment() { \n"
							"	ALBEDO = albedo.rgb; \n"
							"	ALPHA = albedo.a; \n"
							"}");

					border_mat->set_shader(border_shader);
					border_mat->set_shader_param("albedo", Color(0.75, 0.75, 0.75, col.a / 3.0));

					rotate_gizmo[3] = Ref<ArrayMesh>(memnew(ArrayMesh));
					rotate_gizmo[3]->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
					rotate_gizmo[3]->surface_set_material(0, border_mat);
				}
			}

			// Scale
			{
				Ref<SurfaceTool> surftool = memnew(SurfaceTool);
				surftool->begin(Mesh::PRIMITIVE_TRIANGLES);

				// Cube arrow profile
				const int arrow_points = 6;
				Vector3 arrow[6] = {
					nivec * 0.0 + ivec * 0.0,
					nivec * 0.01 + ivec * 0.0,
					nivec * 0.01 + ivec * 1.0 * GIZMO_SCALE_OFFSET,
					nivec * 0.07 + ivec * 1.0 * GIZMO_SCALE_OFFSET,
					nivec * 0.07 + ivec * 1.11 * GIZMO_SCALE_OFFSET,
					nivec * 0.0 + ivec * 1.11 * GIZMO_SCALE_OFFSET,
				};

				int arrow_sides = 4;

				for (int k = 0; k < 4; k++) {
					Basis ma(ivec, Math_PI * 2 * float(k) / arrow_sides);
					Basis mb(ivec, Math_PI * 2 * float(k + 1) / arrow_sides);

					for (int j = 0; j < arrow_points - 1; j++) {
						Vector3 points[4] = {
							ma.xform(arrow[j]),
							mb.xform(arrow[j]),
							mb.xform(arrow[j + 1]),
							ma.xform(arrow[j + 1]),
						};
						surftool->add_vertex(points[0]);
						surftool->add_vertex(points[1]);
						surftool->add_vertex(points[2]);

						surftool->add_vertex(points[0]);
						surftool->add_vertex(points[2]);
						surftool->add_vertex(points[3]);
					}
				}

				surftool->set_material(mat);
				surftool->commit(scale_gizmo[i]);
			}

			// Plane Scale
			{
				Ref<SurfaceTool> surftool = memnew(SurfaceTool);
				surftool->begin(Mesh::PRIMITIVE_TRIANGLES);

				Vector3 vec = ivec2 - ivec3;
				Vector3 plane[4] = {
					vec * GIZMO_PLANE_DST,
					vec * GIZMO_PLANE_DST + ivec2 * GIZMO_PLANE_SIZE,
					vec * (GIZMO_PLANE_DST + GIZMO_PLANE_SIZE),
					vec * GIZMO_PLANE_DST - ivec3 * GIZMO_PLANE_SIZE
				};

				Basis ma(ivec, Math_PI / 2);

				Vector3 points[4] = {
					ma.xform(plane[0]),
					ma.xform(plane[1]),
					ma.xform(plane[2]),
					ma.xform(plane[3]),
				};
				surftool->add_vertex(points[0]);
				surftool->add_vertex(points[1]);
				surftool->add_vertex(points[2]);

				surftool->add_vertex(points[0]);
				surftool->add_vertex(points[2]);
				surftool->add_vertex(points[3]);

				Ref<SpatialMaterial> plane_mat = memnew(SpatialMaterial);
				plane_mat->set_flag(SpatialMaterial::FLAG_UNSHADED, true);
				plane_mat->set_on_top_of_alpha();
				plane_mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
				plane_mat->set_cull_mode(SpatialMaterial::CULL_DISABLED);
				plane_mat->set_albedo(col);
				plane_gizmo_color[i] = plane_mat; // needed, so we can draw planes from both sides
				surftool->set_material(plane_mat);
				surftool->commit(scale_plane_gizmo[i]);

				Ref<SpatialMaterial> plane_mat_hl = plane_mat->duplicate();
				plane_mat_hl->set_albedo(col.from_hsv(col.get_h(), 0.25, 1.0, 1));
				plane_gizmo_color_hl[i] = plane_mat_hl; // needed, so we can draw planes from both sides
			}
		}
	}

	_generate_selection_boxes();
}

void SpriteMeshEditor::_generate_selection_boxes() {
	// Use two AABBs to create the illusion of a slightly thicker line.
	AABB aabb(Vector3(), Vector3(1, 1, 1));

	// Create a x-ray (visible through solid surfaces) and standard version of the selection box.
	// Both will be drawn at the same position, but with different opacity.
	// This lets the user see where the selection is while still having a sense of depth.
	Ref<SurfaceTool> st = memnew(SurfaceTool);
	Ref<SurfaceTool> st_xray = memnew(SurfaceTool);

	st->begin(Mesh::PRIMITIVE_LINES);
	st_xray->begin(Mesh::PRIMITIVE_LINES);
	for (int i = 0; i < 12; i++) {
		Vector3 a, b;
		aabb.get_edge(i, a, b);

		st->add_vertex(a);
		st->add_vertex(b);
		st_xray->add_vertex(a);
		st_xray->add_vertex(b);
	}

	Ref<SpatialMaterial> mat = memnew(SpatialMaterial);
	mat->set_flag(SpatialMaterial::FLAG_UNSHADED, true);
	const Color selection_box_color = EDITOR_GET("editors/3d/selection_box_color");
	mat->set_albedo(selection_box_color);
	mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
	st->set_material(mat);
	selection_box = st->commit();

	Ref<SpatialMaterial> mat_xray = memnew(SpatialMaterial);
	mat_xray->set_flag(SpatialMaterial::FLAG_UNSHADED, true);
	mat_xray->set_flag(SpatialMaterial::FLAG_DISABLE_DEPTH_TEST, true);
	mat_xray->set_albedo(selection_box_color * Color(1, 1, 1, 0.15));
	mat_xray->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
	st_xray->set_material(mat_xray);
	selection_box_xray = st_xray->commit();
}

SpriteMeshEditor::SpriteMeshEditor(EditorNode *p_node) {
	editor = p_node;
}

SpriteMeshEditor::~SpriteMeshEditor() {
}

void SpriteMeshEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		sprite_mesh_editor->show();
		sprite_mesh_editor->set_process(true);

	} else {
		sprite_mesh_editor->hide();
		sprite_mesh_editor->set_process(false);
	}
}

void SpriteMeshEditorPlugin::edit(Object *p_object) {
	node = cast_to<SpriteMesh>(p_object);
}

bool SpriteMeshEditorPlugin::forward_canvas_gui_input(const Ref<InputEvent> &p_event) {
	return false;
}

void SpriteMeshEditorPlugin::forward_canvas_draw_over_viewport(Control *p_overlay) {
	if (!node || !node->is_visible_in_tree()) {
		return;
	}
}

SpriteMeshEditorPlugin::SpriteMeshEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	sprite_mesh_editor = memnew(SpriteMeshEditor(p_node));
	editor->get_viewport()->add_child(sprite_mesh_editor);

	sprite_mesh_editor->hide();
}

SpriteMeshEditorPlugin::~SpriteMeshEditorPlugin() {
}
