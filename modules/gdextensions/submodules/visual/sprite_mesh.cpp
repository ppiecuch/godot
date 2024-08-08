/**************************************************************************/
/*  sprite_mesh.cpp                                                       */
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

#include <map>
#include <string>
#include <vector>

#include "core/core_string_names.h"
#include "core/io/json.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/variant.h"
#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#include "editor/plugins/canvas_item_editor_plugin.h"
#include "scene/3d/mesh_instance.h"
#include "scene/main/scene_tree.h"
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

#define GIZMO_SCROLLER_SEGS 32.0 // number of vert/horiz ticks
#define GIZMO_RING_WIDTH 0.025 // width of the ring
#define GIZMO_RING_SEGS 128 // number of circle segments
#define GIZMO_RING_TICKS 64 // number of ring ticks

#define get_mesh_surf_info(mesh) \
	((Dictionary)(mesh->has_meta("_mesh_surf_info") ? (Dictionary)mesh->get_meta("_mesh_surf_info") : Dictionary()))

// Reference:
// ----------
// - https://community.khronos.org/t/glsl-multiple-lights/54103/3
// - https://learnopengl.com/Lighting/Multiple-lights
// - https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Multiple_Lights

const char *_light_material_shader = R"(
	shader_type canvas_item;

	uniform vec4 albedo;
	uniform vec4 ambient;
	uniform vec4 specular;

	vec4 light0 () {
		vec4 color;
		vec3 lightDir = vec3(gl_LightSource[0].position);
		vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
		vec4 diffuse = gl_LightSource[0].diffuse * max(dot(normal,lightDir),0.0) * gl_FrontMaterial.diffuse;
		color = ambient + diffuse;
		return color;
	}

	void fragment() {
		vec4 skin = texture(TEXTURE, UV);
		vec4 env = texture(envmap, UV2);

		vec3 color = mix(skin.rgb, env.rgb, map(env.a, 0.5, 1.0, 0.1, 1));
		COLOR *= vec4(color, skin.a);
	}
)";

#ifdef TOOLS_ENABLED
Dictionary SpriteMesh::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["offset"] = offset;
	state["centered"] = centered;
	state["mesh_xform"] = mesh_xform;
	return state;
}

void SpriteMesh::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_offset(p_state["offset"]);
	set_centered(p_state["centered"]);
	set_mesh_orientation(p_state["mesh_xform"]);
}

bool SpriteMesh::_edit_state_changed(const Dictionary &p_state) const {
	return (mesh_xform != p_state["mesh_xform"]) && (offset != p_state["offset"]) && (centered != bool(p_state["centered"]));
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

void SpriteMeshSnapshot::_create() {
	scenario = RID_PRIME(VS::get_singleton()->scenario_create());

	viewport = RID_PRIME(VS::get_singleton()->viewport_create());
	VS::get_singleton()->viewport_set_update_mode(viewport, VS::VIEWPORT_UPDATE_DISABLED);
	VS::get_singleton()->viewport_set_vflip(viewport, true);
	VS::get_singleton()->viewport_set_scenario(viewport, scenario);
	VS::get_singleton()->viewport_set_transparent_background(viewport, true);
	VS::get_singleton()->viewport_set_active(viewport, false);

	viewport_texture = VS::get_singleton()->viewport_get_texture(viewport);

	canvas = RID_PRIME(VS::get_singleton()->canvas_create());
	canvas_item = RID_PRIME(VS::get_singleton()->canvas_item_create());

	VS::get_singleton()->canvas_item_add_mesh_3d(canvas_item, owner->get_mesh()->get_rid());

	VS::get_singleton()->viewport_attach_canvas(viewport, canvas);
	VS::get_singleton()->canvas_item_set_parent(canvas_item, canvas);
}

void SpriteMeshSnapshot::_trigger(const String &filepath) {
	if (!viewport.is_valid()) {
		_create();
	}
	ERR_FAIL_COND(!viewport.is_valid());
	ERR_FAIL_COND(!owner->get_mesh().is_valid());
	// setup mesh
	const AABB &aabb = owner->get_mesh_aabb();
	const Size2 s(aabb.size.x, aabb.size.y);
	Point2 ofs = owner->get_offset();
	if (owner->is_centered()) {
		ofs -= s / 2;
	}
	const Point2 origin = ofs - Point2(aabb.position.x, aabb.position.y);
	const Transform xform(owner->get_mesh_orientation(), { origin.x + s.width / 2, origin.y + s.height / 2, 0 });
	RID texture_rid = owner->get_mesh_texture().is_valid() ? owner->get_mesh_texture()->get_rid() : RID();
	RID normal_map_rid = owner->get_mesh_normal_map().is_valid() ? owner->get_mesh_normal_map()->get_rid() : RID();
	RID mask_rid = owner->get_mesh_mask().is_valid() ? owner->get_mesh_mask()->get_rid() : RID();
	VS::get_singleton()->canvas_item_set_mesh_3d(canvas_item, owner->get_mesh()->get_rid(), xform, owner->get_modulate(), texture_rid, normal_map_rid, mask_rid);
	// setup viewport
	VS::get_singleton()->request_frame_drawn_callback(owner, "_snapshot_done", filepath);
	VS::get_singleton()->viewport_set_size(viewport, s.width, s.height);
	VS::get_singleton()->viewport_set_update_mode(viewport, VS::VIEWPORT_UPDATE_ONCE); // once used for capture
	VS::get_singleton()->viewport_set_active(viewport, true);
}

#define _FREE_RID(var)                  \
	if (var.is_valid()) {               \
		VS::get_singleton()->free(var); \
		var = RID();                    \
	}

void SpriteMeshSnapshot::_destroy() {
	_FREE_RID(canvas_item);
	_FREE_RID(canvas);
	_FREE_RID(viewport);
	_FREE_RID(scenario);
}

Ref<Image> SpriteMeshSnapshot::_get_image() {
	ERR_FAIL_COND_V(!viewport_texture.is_valid(), Ref<Image>());
	Ref<Image> img = VS::get_singleton()->texture_get_data(viewport_texture);
	ERR_FAIL_COND_V(img.is_null(), Ref<Image>());
	img->convert(Image::FORMAT_RGBA8);
	return img;
}

void SpriteMeshSnapshot::done() {
	if (viewport.is_valid()) {
		VS::get_singleton()->viewport_set_active(viewport, true);
	}
}

static _FORCE_INLINE_ Ref<Mesh> _current_submesh(Ref<ArrayMesh> mesh, int index) {
	if (mesh) {
		if (mesh->get_submesh_count()) {
			return mesh->get_submesh_by_index(index);
		}
	}
	return mesh;
}

static Basis flip_x_transform = Basis(-1, 0, 0, 0, 1, 0, 0, 0, 1);
static Basis flip_y_transform = Basis(1, 0, 0, 0, -1, 0, 0, 0, 1);
static Basis flip_z_transform = Basis(1, 0, 0, 0, 1, 0, 0, 0, -1);

void SpriteMesh::_refresh_properties() {
	property_list_changed_notify();
}

SpriteMeshLight *SpriteMesh::_get_light_node(int p_index) {
	ERR_FAIL_INDEX_V(p_index, LIGHTS_NUM, nullptr);
	return lights.get_light(p_index).is_empty() ? nullptr : cast_to<SpriteMeshLight>(get_node(lights.get_light(p_index)));
}

void SpriteMesh::_update_lights() {
	if (!lights.is_empty()) {
		// check for global and local lights ..
		Vector<SpriteMeshLight *> all_lights;
		if (Node *rt = get_tree()->get_current_scene()) {
			for (int i = 0; i < rt->get_child_count(); i++) {
				if (SpriteMeshLight *lt = cast_to<SpriteMeshLight>(rt->get_child(i))) {
					all_lights.push_back(lt);
				}
			}
		}
		for (int i = 0; i < get_child_count(); i++) {
			if (SpriteMeshLight *lt = cast_to<SpriteMeshLight>(get_child(i))) {
				all_lights.push_back(lt);
			}
		}
		property_list_changed_notify();
	}
}

void SpriteMesh::_update_mesh_outline(const PoolVector3Array &p_vertices, const Transform &p_xform, const PoolIntArray &p_triangles) {
	ERR_FAIL_COND_MSG(p_triangles.size() == 0, "Automtic outline only works for indexed meshes");

	// get just the outer edges from the mesh's triangles (ignore or remove any shared edges)
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

	// create edge lookup dictionary
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

void SpriteMesh::_update_mesh_changes() {
	if (mesh.is_valid()) {
		if (auto_collision_shape) {
			if (Ref<ArrayMesh> m = mesh) {
				for (int s = 0; s < m->get_surface_count(); s++) {
					if (m->surface_is_active(s)) {
						const Array &mesh_array = m->surface_get_arrays(s);
						const PoolVector3Array &vertexes = mesh_array[VS::ARRAY_VERTEX];
						_update_mesh_outline(vertexes, mesh_xform, mesh_array[VS::ARRAY_INDEX]); // update outline shape
					}
				}
			}
		}
		item_rect_changed();
	}
}

void SpriteMesh::_update_mesh() {
	if (Ref<Mesh> current = _current_submesh(drawer, submesh_selected)) {
		mesh = current;
	}
	_mesh_dirty = true;
	_update_xform_from_components();
	item_rect_changed();
	update();
	emit_signal("mesh_changed");
	_change_notify("mesh");
}

void SpriteMesh::_update_components_from_xform() {
	_mesh_angle = mesh_xform.get_euler();
	_mesh_scale = mesh_xform.get_scale();
}

void SpriteMesh::_update_xform_from_components() {
	if (flipped) {
		mesh_xform.set_euler_scale(_mesh_angle, _mesh_scale * Vector3(1, -1, 1));
	} else {
		mesh_xform.set_euler_scale(_mesh_angle, _mesh_scale);
	}

	if (!is_inside_tree()) {
		return;
	}

	_mesh_dirty = true;
	_notify_transform();
	update();
}

void SpriteMesh::_snapshot_done(const Variant &p_udata) {
	static int snapshot_count = 0;
	if (Ref<Image> img = snapshot._get_image()) {
		if (img.is_valid()) {
			String file = p_udata;
			if (file.empty()) {
				file = "snapshot" + itos(snapshot_count) + ".png";
				snapshot_count++;
			}
			img->save_png(file);
			emit_signal("snapshot_ready", file);
			print_verbose("Snapshot saved to file: " + file);
		}
	}
	snapshot.done();
}

void SpriteMesh::set_mesh(const Ref<Mesh> &p_mesh) {
	if (drawer == p_mesh) {
		return;
	}

	if (drawer.is_valid()) {
		drawer->disconnect(CoreStringNames::get_singleton()->changed, this, "_mesh_changed");
	}

	drawer = mesh = p_mesh;
	submesh_selected = 0;

	if (drawer.is_valid()) {
		drawer->connect(CoreStringNames::get_singleton()->changed, this, "_mesh_changed");
	}

	if (p_mesh) {
		String mesh_path = p_mesh->get_path();
		if (!mesh_path.empty()) {
			String color_texture = vformat("%s-C.png", mesh_path.trim_suffix("." + mesh_path.get_extension())); // color atlas
			if (ResourceLoader::exists(color_texture)) {
				set_mesh_texture(ResourceLoader::load(color_texture, "Texture"));
			}
			String normal_texture = vformat("%s-N.png", mesh_path.trim_suffix("." + mesh_path.get_extension())); // normal atlas
			if (ResourceLoader::exists(normal_texture)) {
				set_mesh_normal_map(ResourceLoader::load(normal_texture, "Texture"));
			}
		}
	}

	_update_mesh();
}

Ref<Mesh> SpriteMesh::get_mesh() const {
	return drawer;
}

void SpriteMesh::save_snapshot(const String &p_filepath) {
	snapshot._trigger(p_filepath);
}

void SpriteMesh::save_snapshot_mesh(Ref<ArrayMesh> p_mesh) {
	ERR_FAIL_NULL(p_mesh);
	ERR_FAIL_NULL(mesh);
	for (int s = 0; s < mesh->get_surface_count(); s++) {
		Array arrays = mesh->surface_get_arrays(s);

		PoolVector3Array vertexes = arrays[VS::ARRAY_VERTEX];
		PoolVector3Array xform_vertexes;
		ERR_FAIL_COND(xform_vertexes.resize(vertexes.size()) != OK);

		auto w = xform_vertexes.write();
		// build transformed mesh
		for (int v = 0; v < vertexes.size(); ++v) {
			w[v] = mesh_xform.xform(vertexes[v]);
		}
		arrays[VS::ARRAY_VERTEX] = xform_vertexes;
		// Albedo is use in modified renderer.
		//
		// extract albedo color from material, since they are ignored in 2D
		// if (((PoolColorArray)arrays[VS::ARRAY_COLOR]).size() == 0) {
		// 	PoolColorArray colors;
		// 	if (Ref<SpatialMaterial> mat = mesh->surface_get_material(s)) {
		// 		colors.push_multi(xform_vertexes.size(), mat->get_albedo());
		// 	}
		// 	arrays[VS::ARRAY_COLOR] = colors;
		// }
		const int surf_id = p_mesh->get_surface_count();
		p_mesh->add_surface_from_arrays(mesh->surface_get_primitive_type(s), arrays);
		if (Ref<Material> mat = mesh->surface_get_material(s)) {
			p_mesh->surface_set_material(surf_id, mat);
		}
	}
}

void SpriteMesh::set_mesh_rotation(const Vector3 &p_radians) {
	_mesh_angle = p_radians;
	_update_xform_from_components();
}

Vector3 SpriteMesh::get_mesh_rotation() const {
	return _mesh_angle;
}

void SpriteMesh::set_mesh_rotation_x_degrees(float p_degrees) {
	_mesh_angle.x = Math::deg2rad(p_degrees);
	_update_xform_from_components();
}

float SpriteMesh::get_mesh_rotation_x_degrees() const {
	return Math::rad2deg(_mesh_angle.x);
}

void SpriteMesh::set_mesh_rotation_y_degrees(float p_degrees) {
	_mesh_angle.y = Math::deg2rad(p_degrees);
	_update_xform_from_components();
}

float SpriteMesh::get_mesh_rotation_y_degrees() const {
	return Math::rad2deg(_mesh_angle.y);
}

void SpriteMesh::set_mesh_rotation_z_degrees(float p_degrees) {
	_mesh_angle.z = Math::deg2rad(p_degrees);
	_update_xform_from_components();
}

float SpriteMesh::get_mesh_rotation_z_degrees() const {
	return Math::rad2deg(_mesh_angle.z);
}

void SpriteMesh::set_mesh_orientation(const Basis &p_basis) {
	if (mesh_xform != p_basis) {
		mesh_xform = p_basis;
		_update_components_from_xform();

		if (!is_inside_tree()) {
			return;
		}

		_mesh_dirty = true;
		_notify_transform();
		property_list_changed_notify();
		update();
	}
}

Basis SpriteMesh::get_mesh_orientation() const {
	return mesh_xform;
}

void SpriteMesh::set_mesh_scale(const Vector3 &p_scale) {
	_mesh_scale = p_scale;
	// Avoid having 0 scale values, can lead to errors in physics and rendering.
	if (_mesh_scale.x == 0) {
		_mesh_scale.x = CMP_EPSILON;
	}
	if (_mesh_scale.y == 0) {
		_mesh_scale.y = CMP_EPSILON;
	}
	if (_mesh_scale.z == 0) {
		_mesh_scale.z = CMP_EPSILON;
	}
	_update_xform_from_components();
}

Vector3 SpriteMesh::get_mesh_scale() const {
	return _mesh_scale;
}

void SpriteMesh::set_mesh_texture(const Ref<Texture> &p_texture) {
	if (texture == p_texture) {
		return;
	}
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

void SpriteMesh::set_mesh_light(int p_index, const NodePath &p_path) {
	ERR_FAIL_INDEX(p_index, LIGHTS_NUM);
	lights.set_light(p_index, p_path);
	property_list_changed_notify();
}

NodePath SpriteMesh::get_mesh_light(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, LIGHTS_NUM, NodePath());
	return lights.get_light(p_index);
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
							switch (frame["repeat"].get_type()) {
								case Variant::REAL: {
									rep = MAX(rep, int(frame["repeat"]));
								} break;
								default: {
									WARN_PRINT("repeat component can be a number only");
								}
							}
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
													WARN_PRINT(vformat("Rotation expresion parse error at %d", err));
												}
											} break;
											default: {
												WARN_PRINT("Rotation component can be number or String only");
											}
										}
									}
								} else {
									WARN_PRINT("rotate needs to be an Array");
								}
							}
							if (frame.has("scale")) {
								if (frame["scale"].get_type() == Variant::ARRAY) {
									Array scales = frame["scale"];
									for (int c = 0; c < 3; c++) {
										switch (scales[c].get_type()) {
											case Variant::REAL: {
												scaling[c] = real_t(scales[c]);
											} break;
											case Variant::STRING: {
												String expr_str = scales[c];
												te_variable vars[] = { { "repcount", &rep }, { "repcounter", &loop } };
												int err;
												if (te_expr *expr = te_compile(expr_str.utf8().get_data(), vars, 2, &err)) {
													scaling[c] = te_eval(expr);
													te_free(expr);
												} else {
													WARN_PRINT(vformat("Scaling expresion parse error at %d", err));
												}
											} break;
											default: {
												WARN_PRINT("Scaling component can be number or String only");
											}
										}
									}
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

void SpriteMesh::set_flipped_vert(bool p_flipped) {
	flipped = p_flipped;
	update();
	item_rect_changed();
}

bool SpriteMesh::is_flipped_vert() const {
	return flipped;
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
	const AABB &aabb = get_mesh_aabb();

	Size2 s(aabb.size.x, aabb.size.y);

	Point2 ofs = offset;
	if (centered) {
		ofs -= s / 2;
	}
	if (s == Size2(0, 0)) {
		s = Size2(1, 1);
	}
	return Rect2(ofs, s);
}

Rect2 SpriteMesh::get_anchorable_rect() const {
	return get_rect();
}

AABB SpriteMesh::get_mesh_aabb() const {
	ERR_FAIL_COND_V(mesh.is_null(), AABB());
	return Transform(mesh_xform, Vector3()).xform(mesh->get_aabb());
}

void SpriteMesh::_mesh_changed() {
	if (mesh.is_valid()) {
		_update_mesh();
	}
}

bool SpriteMesh::_get(const StringName &p_path, Variant &r_ret) const {
	String path = p_path;

	if (path == "submesh_index") {
		r_ret = submesh_selected;
		return true;
	} else if (!path.begins_with("mesh_lights/")) {
		return false;
	}

	int which = path.get_slicec('/', 1).to_int();
	String what = path.get_slicec('/', 2);

	ERR_FAIL_INDEX_V(which, LIGHTS_NUM, false);

	if (what == "node") {
		r_ret = lights.get_light(which);
		return true;
	} else {
		const NodePath path = lights.get_light(which);
		if (!path.is_empty()) {
			SpriteMeshLight *lt = cast_to<SpriteMeshLight>(get_node(path));
			ERR_FAIL_NULL_V(lt, false);

			if (what == "position") {
				r_ret = lt->get_light_position();
			} else if (what == "radius") {
				r_ret = lt->get_light_radius();
			} else if (what == "ambient") {
				r_ret = lt->get_light_ambient();
			} else if (what == "diffuse") {
				r_ret = lt->get_light_diffuse();
			} else if (what == "specular") {
				r_ret = lt->get_light_specular();
			}
			return true;
		}
	}
	return false;
}

bool SpriteMesh::_set(const StringName &p_path, const Variant &p_value) {
	String path = p_path;

	if (path == "submesh_index") {
		submesh_selected = p_value;
		_update_mesh();
		return true;
	} else if (!path.begins_with("mesh_lights/")) {
		return false;
	}

	int which = path.get_slicec('/', 1).to_int();
	String what = path.get_slicec('/', 2);

	ERR_FAIL_INDEX_V(which, LIGHTS_NUM, false);

	if (what == "node") {
#ifdef TOOLS_ENABLED
		SpriteMeshLight *lt = nullptr;
		NodePath path = p_value;
		if (!path.is_empty()) {
			lt = cast_to<SpriteMeshLight>(get_node(path));
			ERR_FAIL_NULL_V_MSG(lt, false, "Not a SpriteMeshLight type node");
		}
		if (Node *curr = _get_light_node(which)) {
			curr->disconnect("light_config_changed", this, "_refresh_properties");
		}
		if (lt) {
			lt->connect("light_config_changed", this, "_refresh_properties");
		}
#endif
		lights.set_light(which, p_value);
		property_list_changed_notify();
		return true;
	} else {
		SpriteMeshLight *lt = cast_to<SpriteMeshLight>(get_node(lights.get_light(which)));
		ERR_FAIL_NULL_V(lt, false);

		if (lights.has_light(which)) {
			if (what == "position") {
				lt->set_light_position(p_value);
			} else if (what == "radius") {
				lt->set_light_radius(p_value);
			} else if (what == "ambient") {
				lt->set_light_ambient(p_value);
			} else if (what == "diffuse") {
				lt->set_light_diffuse(p_value);
			} else if (what == "specular") {
				lt->set_light_specular(p_value);
			}
			return true;
		}
	}

	return false;
}

void SpriteMesh::_get_property_list(List<PropertyInfo> *p_list) const {
	if (Ref<ArrayMesh> mesh = drawer) {
		if (mesh->get_submesh_count()) {
			p_list->push_back(PropertyInfo(Variant::INT, "submesh_index", PROPERTY_HINT_RANGE, "0," + itos(mesh->get_submesh_count() - 1) + ",1"));
		}
	}
	for (int i = 0; i < LIGHTS_NUM; i++) {
		const String prep = "mesh_lights/" + itos(i) + "/";
		p_list->push_back(PropertyInfo(Variant::NODE_PATH, prep + "node"));
		if (lights.has_light(i)) {
			p_list->push_back(PropertyInfo(Variant::VECTOR3, prep + "position"));
			p_list->push_back(PropertyInfo(Variant::REAL, prep + "radius"));
			p_list->push_back(PropertyInfo(Variant::COLOR, prep + "ambient"));
			p_list->push_back(PropertyInfo(Variant::COLOR, prep + "diffuse"));
			p_list->push_back(PropertyInfo(Variant::COLOR, prep + "specular"));
		}
	}
}

void SpriteMesh::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
#ifdef TOOLS_ENABLED
		set_notify_transform(true);
#endif
	} else if (p_what == NOTIFICATION_ENTER_TREE) {
		// look for local and global lights
		_update_lights();
	} else if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		emit_signal("transform_changed");
	} else if (p_what == NOTIFICATION_DRAW) {
		if (_mesh_dirty) {
			_update_mesh_changes();
			_mesh_dirty = false;
		}
		if (mesh.is_valid()) {
			const AABB &aabb = get_mesh_aabb();

			Size2 s(aabb.size.x, aabb.size.y);

			Point2 ofs = offset;
			if (centered) {
				ofs -= Size2(s) / 2;
			}
			const Point2 origin = ofs - Point2(aabb.position.x, aabb.position.y);
			const Transform xform(mesh_xform, { origin.x, origin.y, 0 });

			draw_mesh_3d(mesh, texture, normal_map, mask, xform);

			if (mesh_debug) {
				draw_rect(Rect2(ofs, s), Color::named("yellow"), false);
				if (mesh_outline.size()) {
					draw_polyline(mesh_outline, Color::named("magenta"));
				}
			}
		}
	}
}

void SpriteMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("save_snapshot", "filepath"), &SpriteMesh::save_snapshot, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("save_snapshot_mesh", "mesh"), &SpriteMesh::save_snapshot_mesh);

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

	ClassDB::bind_method(D_METHOD("set_mesh_light", "light_index", "light_node"), &SpriteMesh::set_mesh_light);
	ClassDB::bind_method(D_METHOD("get_mesh_light", "light_index"), &SpriteMesh::get_mesh_light);

	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &SpriteMesh::set_centered);
	ClassDB::bind_method(D_METHOD("is_centered"), &SpriteMesh::is_centered);
	ClassDB::bind_method(D_METHOD("set_flipped_vert", "flipped"), &SpriteMesh::set_flipped_vert);
	ClassDB::bind_method(D_METHOD("is_flipped_vert"), &SpriteMesh::is_flipped_vert);

	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &SpriteMesh::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &SpriteMesh::get_offset);

	ClassDB::bind_method(D_METHOD("_mesh_changed"), &SpriteMesh::_mesh_changed);
	ClassDB::bind_method(D_METHOD("_snapshot_done"), &SpriteMesh::_snapshot_done);
	ClassDB::bind_method(D_METHOD("_refresh_properties"), &SpriteMesh::_refresh_properties);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mesh_texture", "get_mesh_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_normal_map", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mesh_normal_map", "get_mesh_normal_map");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_mask", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mesh_mask", "get_mesh_mask");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "select_frame"), "set_selected_frame", "get_selected_frame");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "frames_builder", PROPERTY_HINT_MULTILINE_TEXT, ""), "set_frames_builder", "get_frames_builder");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_collision_shape"), "set_auto_collision_shape", "is_auto_collision_shape");

	ADD_GROUP("Mesh Transform", "mesh_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mesh_centered"), "set_centered", "is_centered");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mesh_flipped_vert"), "set_flipped_vert", "is_flipped_vert");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "mesh_offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BASIS, "mesh_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_mesh_rotation", "get_mesh_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mesh_rotation_x_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_mesh_rotation_x_degrees", "get_mesh_rotation_x_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mesh_rotation_y_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_mesh_rotation_y_degrees", "get_mesh_rotation_y_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mesh_rotation_z_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_mesh_rotation_z_degrees", "get_mesh_rotation_z_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "mesh_scale"), "set_mesh_scale", "get_mesh_scale");
	ADD_GROUP("", "");

	ADD_SIGNAL(MethodInfo("texture_changed"));
	ADD_SIGNAL(MethodInfo("mesh_changed"));
	ADD_SIGNAL(MethodInfo("transform_changed"));
	ADD_SIGNAL(MethodInfo("snapshot_ready"));
}

SpriteMesh::SpriteMesh() {
	_mesh_angle = Vector3(0, 0, 0);
	_mesh_scale = Vector3(1, 1, 1);
	_mesh_dirty = false;
	snapshot.owner = this;
	centered = flipped = true;
	offset = Vector2(0, 0);
	auto_collision_shape = false;
	selected_frame = 0;
	frames_builder = "";
	mesh_debug = false;
	mesh_xform = Basis(_mesh_angle, _mesh_scale);
	submesh_selected = 0;
}

/// SpriteMeshEditorPlugin

static void draw_indicator_bar(Control &p_surface, const Rect2 &p_rc, real_t p_fill, const Ref<Texture> p_icon, const Ref<Font> p_font, const String &p_text, const Color &p_color) {
	// Adjust bar size from control height
	const Vector2 surface_size = p_rc.get_size();
	const Vector2 icon_size = p_icon->get_size();
	const real_t h = surface_size.y / 1.5;
	const real_t y = (surface_size.y - (h + icon_size.y + p_font->get_string_size(p_text).y)) / 2.0;

	const Rect2 r(p_rc.get_position().x - 10 * EDSCALE, p_rc.get_position().y + y, 6 * EDSCALE, h);
	const real_t sy = r.size.y * p_fill;

	// Note: because this bar appears over the viewport, it has to stay readable for any background color
	// Draw both neutral dark and bright colors to account this
	p_surface.draw_rect(r, p_color * Color(1, 1, 1, 0.2));
	p_surface.draw_rect(Rect2(r.position.x, r.position.y + r.size.y - sy, r.size.x, sy), p_color * Color(1, 1, 1, 0.6));
	p_surface.draw_rect(r.grow(1), Color(0, 0, 0, 0.7), false, Math::round(EDSCALE));

	const Vector2 icon_pos = Vector2(r.position.x - (icon_size.x - r.size.x) / 2, r.position.y + r.size.y + 2 * EDSCALE);
	p_surface.draw_texture(p_icon, icon_pos, p_color);

	// Draw a shadow for the text to make it easier to read.
	p_surface.draw_string(p_font, Vector2(icon_pos.x + EDSCALE, icon_pos.y + icon_size.y + 17 * EDSCALE), p_text, Color(0, 0, 0));
	// Draw text below the bar.
	p_surface.draw_string(p_font, Vector2(icon_pos.x, icon_pos.y + icon_size.y + 16 * EDSCALE), p_text, p_color);
}

int SpriteMeshEditor::_update_dragging_info(const Point2 &spoint) {
	const CanvasItemEditor::Tool tool = canvas_item_editor->get_current_tool();
	if (tool == CanvasItemEditor::TOOL_ROTATE) {
		// Hover over rotation ring
		auto is_inside_circle = [](const Point2 &center, real_t r, const Point2 &pt) {
			const real_t dist = (pt.x - center.x) * (pt.x - center.x) + (pt.y - center.y) * (pt.y - center.y);
			return dist <= r * r;
		};
		const bool ring_active = is_inside_circle(Point2(), GIZMO_CIRCLE_SIZE + GIZMO_RING_WIDTH * 2, spoint) && !is_inside_circle(Point2(), GIZMO_CIRCLE_SIZE - GIZMO_RING_WIDTH * 2, spoint);
		if (ring_active) {
			return DRAG_ROTATE_Z;
		}
		if (is_inside_circle(Point2(), GIZMO_CIRCLE_SIZE, spoint)) {
			// Hover over x scroller
			const real_t y = spoint.y + GIZMO_CIRCLE_SIZE;
			const real_t ofs1 = Math::sin((Math_PI * y) / (2 * GIZMO_CIRCLE_SIZE));
			const bool scroller_x_active = (spoint.x > -0.05 - 0.1 * ofs1) && (spoint.x < 0.05 + 0.1 * ofs1);
			if (scroller_x_active) {
				return DRAG_ROTATE_X;
			}
			// Hover over y scroller
			const real_t x = spoint.x + GIZMO_CIRCLE_SIZE;
			const real_t ofs2 = Math::sin((Math_PI * x) / (2 * GIZMO_CIRCLE_SIZE));
			const bool scroller_y_active = (spoint.y > -0.05 - 0.1 * ofs2) && (spoint.y < 0.05 + 0.1 * ofs2);
			if (scroller_y_active) {
				return DRAG_ROTATE_Y;
			}
		}
	}
	if (tool == CanvasItemEditor::TOOL_SCALE) {
	}
	return NONE;
}

bool SpriteMeshEditor::forward_gui_input(const Ref<InputEvent> &p_event) {
	if (!node) {
		return false;
	}

	if (!is_visible_in_tree()) {
		return false;
	}

	const CanvasItemEditor::Tool tool = canvas_item_editor->get_current_tool();
	if (tool == CanvasItemEditor::TOOL_SELECT) {
		return false;
	}

	const Transform2D xform = canvas_item_editor->get_canvas_transform() * node->get_global_transform();
	const real_t gizmo_scale = _get_gizmo_scale();
	Transform2D gizmo_xform = Transform2D(0, Size2(gizmo_scale, gizmo_scale), xform.elements[2]);

	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid()) {
		const Vector2 gpoint = mb->get_position();
		const Vector2 spoint = gizmo_xform.affine_inverse().xform(gpoint);
		if (mb->is_pressed()) {
			switch (mb->get_button_index()) {
				case BUTTON_LEFT: {
					if (mb->get_control() || mb->get_shift() || mb->get_alt()) {
						return false;
					}
					// decide what has been selected
					const int _dragging = _update_dragging_info(spoint);
					if (rotate_gizmo[0].mesh.is_valid()) {
						rotate_gizmo[0].mesh->surface_set_active(2, _dragging == DRAG_ROTATE_Z);
					}
					if (rotate_gizmo[1].mesh.is_valid()) {
						rotate_gizmo[1].mesh->surface_set_active(0, _dragging != DRAG_ROTATE_X);
						rotate_gizmo[1].mesh->surface_set_active(1, _dragging != DRAG_ROTATE_X);
						rotate_gizmo[1].mesh->surface_set_active(2, _dragging == DRAG_ROTATE_X);
					}
					if (rotate_gizmo[2].mesh.is_valid()) {
						rotate_gizmo[2].mesh->surface_set_active(0, _dragging != DRAG_ROTATE_Y);
						rotate_gizmo[2].mesh->surface_set_active(1, _dragging != DRAG_ROTATE_Y);
						rotate_gizmo[2].mesh->surface_set_active(2, _dragging == DRAG_ROTATE_Y);
					}
					if (_dragging != NONE) {
						undo_redo_state = node->_edit_get_state(); // save state in case we may need it
						dragging = _dragging;
						mouse_dragging_start = gpoint;
						_dragging_change = 0;
						return true;
					}
				} break;
				case BUTTON_WHEEL_UP:
				case BUTTON_WHEEL_DOWN: {
					if (dragging == NONE) {
						const int _dragging = _update_dragging_info(spoint);
						if (_dragging != NONE) {
							const Basis node_orientation = node->get_mesh_orientation();
							const real_t mult = (mb->get_button_index() == BUTTON_WHEEL_UP ? 1 : -1) * MAX(1, 2 * mb->get_shift()) * MAX(1, 10 * mb->get_alt());
							real_t r = Math::deg2rad(mult * mb->get_factor());
							if (mb->get_control()) { // snap
								r = Math::stepify(r, Math::deg2rad(10.));
							}
							switch (_dragging) {
								case DRAG_ROTATE_Z: {
									VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[0].mesh3d, Transform().rotated(Vector3(0, 0, 1), r), VS::OP_MUL);
									node->set_mesh_orientation(Basis(Vector3(0, 0, 1), r) * node_orientation);
								} break;
								case DRAG_ROTATE_X: {
									const real_t sr = Math_PI / GIZMO_SCROLLER_SEGS; // segment rotation
									VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[1].mesh3d, Transform().rotated(Vector3(1, 0, 0), Math::fmod(r, sr)), VS::OP_SET);
									node->set_mesh_orientation(Basis(Vector3(1, 0, 0), r) * node_orientation);
								} break;
								case DRAG_ROTATE_Y: {
									const real_t sr = Math_PI / GIZMO_SCROLLER_SEGS; // segment rotation
									VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[2].mesh3d, Transform().rotated(Vector3(0, 1, 0), Math::fmod(r, sr)), VS::OP_SET);
									node->set_mesh_orientation(Basis(Vector3(0, 1, 0), r) * node_orientation);
								} break;
							};
							property_list_changed_notify();
							return true;
						}
					}
				} break;
			}
		} else {
			// finish dragging
			if (dragging != NONE) {
				if (node->_edit_state_changed(undo_redo_state)) {
					switch (dragging) {
						case DRAG_ROTATE_X:
						case DRAG_ROTATE_Y:
						case DRAG_ROTATE_Z: {
							const String label[] = { TTR("X-Rotate SpriteMesh"), TTR("Y-Rotate SpriteMesh"), TTR("Z-Rotate SpriteMesh") };
							undo_redo->create_action(label[dragging - DRAG_ROTATE_X]);
						} break;
						case DRAG_SCALE_X:
						case DRAG_SCALE_Y:
						case DRAG_SCALE_Z: {
							const String label[] = { TTR("X-Scale SpriteMesh"), TTR("Y-Scale SpriteMesh"), TTR("Z-Scale SpriteMesh") };
							undo_redo->create_action(label[dragging - DRAG_SCALE_X]);
						} break;
						case DRAG_SCALE_XY:
						case DRAG_SCALE_YZ:
						case DRAG_SCALE_XZ: {
							const String label[] = { TTR("XY-Scale SpriteMesh"), TTR("YZ-Scale SpriteMesh"), TTR("XZ-Scale SpriteMesh") };
							undo_redo->create_action(label[dragging - DRAG_SCALE_XY]);
						} break;
					};
					undo_redo->add_do_method(node, "_edit_set_state", node->_edit_get_state());
					undo_redo->add_undo_method(node, "_edit_set_state", undo_redo_state);
				}
				dragging = NONE;
				plugin->update_overlays();
				return true;
			}
		}
	}

	Ref<InputEventMouseMotion> mm = p_event;

	if (mm.is_valid()) {
		const Vector2 gpoint = mm->get_position();
		const Vector2 spoint = gizmo_xform.affine_inverse().xform(gpoint);
		if (dragging != NONE) {
			mouse_dragging_dist = mouse_dragging_start - gpoint;
			switch (dragging) {
				case DRAG_ROTATE_Z: {
					const real_t r = _dragging_change = -Math_PI * (mouse_dragging_dist.y / gizmo_scale) / GIZMO_CIRCLE_SIZE; // rotation
					VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[0].mesh3d, Transform().rotated(Vector3(0, 0, 1), r), VS::OP_MUL);
					const Basis node_orientation = undo_redo_state["mesh_xform"];
					node->set_mesh_orientation(Basis(Vector3(0, 0, 1), r) * node_orientation);
				} break;
				case DRAG_ROTATE_Y: {
					if (rotate_gizmo[2].item.is_valid()) {
						const real_t sr = Math_PI / GIZMO_SCROLLER_SEGS; // segment rotation
						const real_t sd = 2 * GIZMO_CIRCLE_SIZE / GIZMO_SCROLLER_SEGS; // dist
						const real_t scroller_rotation = sr * Math::fmod(mouse_dragging_dist.x / gizmo_scale, sd) / sd;
						VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[2].mesh3d, Transform().rotated(Vector3(0, 1, 0), scroller_rotation), VS::OP_MUL);
						const Basis node_orientation = undo_redo_state["mesh_xform"];
						_dragging_change = Math_PI * (mouse_dragging_dist.x / gizmo_scale) / GIZMO_CIRCLE_SIZE;
						node->set_mesh_orientation(Basis(Vector3(0, 1, 0), _dragging_change) * node_orientation);
					}
				} break;
				case DRAG_ROTATE_X: {
					if (rotate_gizmo[1].item.is_valid()) {
						const real_t sr = Math_PI / GIZMO_SCROLLER_SEGS; // segment rotation
						const real_t sd = 2 * GIZMO_CIRCLE_SIZE / GIZMO_SCROLLER_SEGS; // dist
						const real_t scroller_rotation = sr * Math::fmod(mouse_dragging_dist.y / gizmo_scale, sd) / sd;
						VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[1].mesh3d, Transform().rotated(Vector3(1, 0, 0), scroller_rotation), VS::OP_MUL);
						const Basis node_orientation = undo_redo_state["mesh_xform"];
						_dragging_change = Math_PI * (mouse_dragging_dist.y / gizmo_scale) / GIZMO_CIRCLE_SIZE;
						node->set_mesh_orientation(Basis(Vector3(1, 0, 0), _dragging_change) * node_orientation);
					}
				} break;
			}
			property_list_changed_notify();
			return true;
		} else {
			const int _dragging = _update_dragging_info(spoint);
			if (rotate_gizmo[0].mesh.is_valid()) {
				rotate_gizmo[0].mesh->surface_set_active(2, _dragging == DRAG_ROTATE_Z);
			}
			if (rotate_gizmo[1].mesh.is_valid()) {
				rotate_gizmo[1].mesh->surface_set_active(0, _dragging != DRAG_ROTATE_X);
				rotate_gizmo[1].mesh->surface_set_active(1, _dragging != DRAG_ROTATE_X);
				rotate_gizmo[1].mesh->surface_set_active(2, _dragging == DRAG_ROTATE_X);
			}
			if (rotate_gizmo[2].mesh.is_valid()) {
				rotate_gizmo[2].mesh->surface_set_active(0, _dragging != DRAG_ROTATE_Y);
				rotate_gizmo[2].mesh->surface_set_active(1, _dragging != DRAG_ROTATE_Y);
				rotate_gizmo[2].mesh->surface_set_active(2, _dragging == DRAG_ROTATE_Y);
			}
		}
	}

	return false;
}

void SpriteMeshEditor::forward_canvas_draw_over_viewport(Control *p_overlay) {
	if (!node) {
		return;
	}

	if (!is_visible_in_tree()) {
		return;
	}

	update_transform_gizmo_view();

	if (dragging != NONE) {
		const Color text_color = Color(0.7, 0.95, 1.0);
		const Ref<Font> text_font = get_font("font", "Label");

		const CanvasItemEditor::Tool tool = canvas_item_editor->get_current_tool();
		const Transform2D xform = canvas_item_editor->get_canvas_transform() * node->get_global_transform();
		Rect2 rc;
		Ref<Texture> icon;
		String label;
		real_t bar = 0;
		if (tool == CanvasItemEditor::TOOL_ROTATE) {
			const real_t gizmo_scale = _get_gizmo_scale();
			const Transform2D gizmo_xform = Transform2D(0, Size2(gizmo_scale, gizmo_scale), xform.elements[2]);
			rc = gizmo_xform.xform(Rect2(Point2(-GIZMO_CIRCLE_SIZE, -GIZMO_CIRCLE_SIZE), Size2(2 * GIZMO_CIRCLE_SIZE, 2 * GIZMO_CIRCLE_SIZE)));
			icon = get_icon("ToolRotate", "EditorIcons");
			real_t dragging_change = Math::fmod(_dragging_change, real_t(Math_TAU));
			label = itos(int(Math::rad2deg(dragging_change)));
			if (dragging_change < 0) {
				bar = Math::map1(dragging_change, -Math_TAU, 0, 0, 1);
			} else {
				bar = Math::map1(dragging_change, 0, Math_TAU, 0, 1);
			}
			const String info = itos(int(Math::rad2deg(node->get_mesh_rotation()[dragging - DRAG_ROTATE_X]))); // ROTATE_.. are in sequence
			if (!info.empty()) {
				p_overlay->draw_string(text_font, gizmo_xform.xform(Point2(0, -GIZMO_CIRCLE_SIZE)) + Point2(EDSCALE, EDSCALE) - text_font->get_string_size(label) / 2, info, Color(0, 0, 0));
				p_overlay->draw_string(text_font, gizmo_xform.xform(Point2(0, -GIZMO_CIRCLE_SIZE)) - text_font->get_string_size(label) / 2, info, text_color);
			}
		} else if (tool == CanvasItemEditor::TOOL_SCALE) {
			icon = get_icon("ToolScale", "EditorIcons");
			label = itos(int(Math::rad2deg(_dragging_change)));
		} else {
			return;
		}
		draw_indicator_bar(
				*p_overlay,
				rc,
				bar,
				icon,
				text_font,
				label,
				text_color);
	}
}

void SpriteMeshEditor::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		get_tree()->connect("node_removed", this, "_node_removed");
	} else if (p_what == NOTIFICATION_VISIBILITY_CHANGED) {
		const bool visible = is_visible_in_tree();
		if (node) {
			if (visible) {
				if (!node->is_connected("transform_changed", this, "update_transform_gizmo_view")) {
					node->connect("transform_changed", this, "update_transform_gizmo_view");
				}
			} else {
				if (node->is_connected("transform_changed", this, "update_transform_gizmo_view")) {
					node->disconnect("transform_changed", this, "update_transform_gizmo_view");
				}
			}
		}
		set_process(visible);
		call_deferred("update_transform_gizmo_view");
	} else if (p_what == NOTIFICATION_RESIZED) {
		call_deferred("update_transform_gizmo_view");
	} else if (p_what == NOTIFICATION_ENTER_TREE) {
		if (!canvas_item_editor) {
			canvas_item_editor = CanvasItemEditor::get_singleton();
		}
		_project_settings_changed(); // Ensure we are up to date with project settings
		ProjectSettings::get_singleton()->connect("project_settings_changed", this, "_project_settings_changed"); // Any further changes to project settings get a signal
		_init_indicators();
		_init_gizmo_instance();
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		ProjectSettings::get_singleton()->disconnect("project_settings_changed", this, "_project_settings_changed");
		_finish_gizmo_instances();
		_finish_indicators();
		canvas_item_editor = nullptr;
	} else if (p_what == NOTIFICATION_PROCESS) {
		if (_project_settings_change_pending) {
			_project_settings_changed();
		}
	} else if (p_what == NOTIFICATION_DRAW) {
		forward_canvas_draw_over_viewport(editor->get_viewport());
	}
}

real_t SpriteMeshEditor::_get_gizmo_scale() const {
	const real_t gizmo_size = EditorSettings::get_singleton()->get("editors/3d/manipulator_gizmo_size");
	// At low viewport heights, multiply the gizmo scale based on the viewport height.
	// This prevents the gizmo from growing very large and going outside the viewport.
	const int viewport_base_height = 400 * MAX(1, EDSCALE);
	const real_t gizmo_scale =
			gizmo_size * MAX(1, EDSCALE) *
			MIN(viewport_base_height, get_viewport()->get_size().height) / viewport_base_height;
	return gizmo_scale;
}

void SpriteMeshEditor::update_transform_gizmo_view() {
	for (int i = 0; i < 3; i++) {
		VS::get_singleton()->canvas_item_set_visible(rotate_gizmo[i].item, false);
		VS::get_singleton()->canvas_item_set_visible(scale_gizmo[i].item, false);
		VS::get_singleton()->canvas_item_set_visible(scale_plane_gizmo[i].item, false);
	}
	VS::get_singleton()->canvas_item_set_visible(origin_indicator.item, false);

	if (!node) {
		return;
	}

	if (!is_visible_in_tree()) {
		return;
	}

	const real_t gizmo_scale = _get_gizmo_scale();

	const Transform2D xform = canvas_item_editor->get_canvas_transform() * node->get_global_transform();
	const Transform2D gizmo_xform = Transform2D(0, Size2(gizmo_scale, gizmo_scale), xform.elements[2]);
	const Transform origin_transform = Transform(Basis(node->get_mesh_rotation(), { gizmo_scale, gizmo_scale, gizmo_scale }), { xform.elements[2].x, xform.elements[2].y, 0 });

	switch (dragging) {
		case DRAG_ROTATE_Z: {
			const real_t r = -Math_PI * (mouse_dragging_dist.y / gizmo_scale) / GIZMO_CIRCLE_SIZE; // rotation
			VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[0].mesh3d, Transform().rotated(Vector3(0, 0, 1), r), VS::OP_SET);
		} break;
		case DRAG_ROTATE_Y: {
			const real_t sr = Math_PI / GIZMO_SCROLLER_SEGS; // segment rotation
			const real_t sd = 2 * GIZMO_CIRCLE_SIZE / GIZMO_SCROLLER_SEGS; // segment dist
			const real_t scroller_rotation = sr * Math::fmod(mouse_dragging_dist.x / gizmo_scale, sd) / sd;
			VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[2].mesh3d, Transform().rotated(Vector3(0, 1, 0), scroller_rotation), VS::OP_SET);
		} break;
		case DRAG_ROTATE_X: {
			const real_t sr = Math_PI / GIZMO_SCROLLER_SEGS; // segment rotation
			const real_t sd = 2 * GIZMO_CIRCLE_SIZE / GIZMO_SCROLLER_SEGS; // segment dist
			const real_t scroller_rotation = sr * Math::fmod(mouse_dragging_dist.y / gizmo_scale, sd) / sd;
			VS::get_singleton()->canvas_item_update_mesh_3d(rotate_gizmo[1].mesh3d, Transform().rotated(Vector3(1, 0, 0), scroller_rotation), VS::OP_SET);
		} break;
	}

	const CanvasItemEditor::Tool tool = canvas_item_editor->get_current_tool();
	for (int i = 0; i < 3; i++) {
		VS::get_singleton()->canvas_item_set_transform(rotate_gizmo[i].item, gizmo_xform);
		VS::get_singleton()->canvas_item_set_visible(rotate_gizmo[i].item, tool == CanvasItemEditor::TOOL_ROTATE);
		VS::get_singleton()->canvas_item_update_mesh_3d(scale_gizmo[i].mesh3d, origin_transform, VS::OP_SET);
		VS::get_singleton()->canvas_item_set_visible(scale_gizmo[i].item, tool == CanvasItemEditor::TOOL_SCALE);
		VS::get_singleton()->canvas_item_update_mesh_3d(scale_plane_gizmo[i].mesh3d, origin_transform, VS::OP_SET);
		VS::get_singleton()->canvas_item_set_visible(scale_plane_gizmo[i].item, tool == CanvasItemEditor::TOOL_SCALE);
	}
	// Origin marker
	VS::get_singleton()->canvas_item_update_mesh_3d(origin_indicator.mesh3d, origin_transform, VS::OP_SET);
	VS::get_singleton()->canvas_item_set_visible(origin_indicator.item, tool != CanvasItemEditor::TOOL_SCALE); // indicator or scale gizmo
}

Color SpriteMeshEditor::_get_color(const StringName &p_name, const StringName &p_theme_type) const {
	return EditorNode::get_singleton()->get_gui_base()->get_color(p_name, p_theme_type);
}

void SpriteMeshEditor::_node_removed(Node *p_node) {
	if (p_node == node) {
		node = nullptr;
	}
}

void SpriteMeshEditor::_init_gizmo_instance() {
	Control *vpc = canvas_item_editor->get_viewport_control();
	for (int i = 0; i < 3; i++) {
		rotate_gizmo[i].item = RID_PRIME(VS::get_singleton()->canvas_item_create());
		VS::get_singleton()->canvas_item_set_parent(rotate_gizmo[i].item, vpc->get_canvas_item());
		rotate_gizmo[i].mesh3d = VS::get_singleton()->canvas_item_create_mesh_3d(rotate_gizmo[i].mesh->get_rid());
		VS::get_singleton()->canvas_item_add_mesh_3d(rotate_gizmo[i].item, rotate_gizmo[i].mesh3d);
		VS::get_singleton()->canvas_item_set_visible(rotate_gizmo[i].item, false);

		scale_gizmo[i].item = RID_PRIME(VS::get_singleton()->canvas_item_create());
		VS::get_singleton()->canvas_item_set_parent(scale_gizmo[i].item, vpc->get_canvas_item());
		scale_gizmo[i].mesh3d = VS::get_singleton()->canvas_item_create_mesh_3d(scale_gizmo[i].mesh->get_rid());
		VS::get_singleton()->canvas_item_add_mesh_3d(scale_gizmo[i].item, scale_gizmo[i].mesh3d);
		VS::get_singleton()->canvas_item_set_visible(scale_gizmo[i].item, false);

		scale_plane_gizmo[i].item = RID_PRIME(VS::get_singleton()->canvas_item_create());
		VS::get_singleton()->canvas_item_set_parent(scale_plane_gizmo[i].item, vpc->get_canvas_item());
		scale_plane_gizmo[i].mesh3d = VS::get_singleton()->canvas_item_create_mesh_3d(scale_plane_gizmo[i].mesh->get_rid());
		VS::get_singleton()->canvas_item_add_mesh_3d(scale_plane_gizmo[i].item, scale_plane_gizmo[i].mesh3d);
		VS::get_singleton()->canvas_item_set_visible(scale_plane_gizmo[i].item, false);
	}
}

void SpriteMeshEditor::_finish_gizmo_instances() {
	for (int i = 0; i < 3; i++) {
		if (rotate_gizmo[i].item.is_valid()) {
			VS::get_singleton()->free(rotate_gizmo[i].mesh3d);
			VS::get_singleton()->free(rotate_gizmo[i].item);
			rotate_gizmo[i].item = rotate_gizmo[i].mesh3d = RID();
		}
		if (scale_gizmo[i].item.is_valid()) {
			VS::get_singleton()->free(scale_gizmo[i].mesh3d);
			VS::get_singleton()->free(scale_gizmo[i].item);
			scale_gizmo[i].item = scale_gizmo[i].mesh3d = RID();
		}
		if (scale_plane_gizmo[i].item.is_valid()) {
			VS::get_singleton()->free(scale_plane_gizmo[i].mesh3d);
			VS::get_singleton()->free(scale_plane_gizmo[i].item);
			scale_plane_gizmo[i].item = scale_plane_gizmo[i].mesh3d = RID();
		}
	}
}

void SpriteMeshEditor::_init_indicators() {
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

		rotate_gizmo[i].mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		scale_gizmo[i].mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		scale_plane_gizmo[i].mesh = Ref<ArrayMesh>(memnew(ArrayMesh));

		const Color col_hl = col.from_hsv(col.get_h(), 0.25, 1, 1);

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
			const int n = 128; // number of circle segments
			const real_t coef = Math_TAU / n;

			Ref<SurfaceTool> surftool = memnew(SurfaceTool);

			if (i == 0) { // z scroller
				Point3 pt0, pt1;
				// Ring
				surftool->begin(Mesh::PRIMITIVE_TRIANGLES);
				surftool->add_color(col);
				for (int i = 0; i <= n; i++) {
					const real_t rads = i * coef;
					const Vector3 norm(Math::sin(rads), Math::cos(rads), 0);
					const Point3 pt2 = GIZMO_CIRCLE_SIZE * norm;
					const Point3 pt3 = (GIZMO_CIRCLE_SIZE - GIZMO_RING_WIDTH) * norm;
					if (i) {
						surftool->add_triangle(pt0, pt2, pt1);
						surftool->add_triangle(pt1, pt2, pt3);
					}
					pt0 = pt2;
					pt1 = pt3;
				}
				surftool->commit(rotate_gizmo[i].mesh); // surf 0

				// White outline
				surftool->begin(Mesh::PRIMITIVE_LINES);
				surftool->add_color(col_hl);
				for (int i = 0; i <= n; i++) {
					const real_t rads = i * coef;
					Point3 pt2 = GIZMO_CIRCLE_SIZE * Point3(Math::cos(rads), Math::sin(rads), 0.5);
					if (i) {
						surftool->add_line(pt1, pt2);
					}
					pt1 = pt2;
				}
				surftool->commit(rotate_gizmo[i].mesh); // surf 1

				// Highlighted ring
				surftool->begin(Mesh::PRIMITIVE_TRIANGLES);
				surftool->add_color(col_hl);
				for (int i = 0; i <= n; i++) {
					const real_t rads = i * coef;
					const Vector3 norm(Math::sin(rads), Math::cos(rads), 1);
					const Point3 pt2 = GIZMO_CIRCLE_SIZE * norm;
					const Point3 pt3 = (GIZMO_CIRCLE_SIZE - GIZMO_RING_WIDTH) * norm;
					if (i) {
						surftool->add_triangle(pt0, pt2, pt1);
						surftool->add_triangle(pt1, pt2, pt3);
					}
					pt0 = pt2;
					pt1 = pt3;
				}
				surftool->commit(rotate_gizmo[i].mesh, false); // surf 2

				// Ticks
				surftool->begin(Mesh::PRIMITIVE_LINES);
				surftool->add_color(col);
				for (int r = 0; r < GIZMO_RING_TICKS; r++) {
					const real_t ang = (Math_TAU / GIZMO_RING_TICKS) * r;
					const Vector3 norm(Math::cos(ang), Math::sin(ang), 0);
					const real_t div = r & 3 ? 0.05 : 0.1;
					const Vector3 pt1 = (GIZMO_CIRCLE_SIZE - GIZMO_RING_WIDTH - div) * norm;
					const Vector3 pt2 = (GIZMO_CIRCLE_SIZE - GIZMO_RING_WIDTH) * norm;
					surftool->add_line(pt1, pt2);
				}
				surftool->commit(rotate_gizmo[i].mesh); // surf 3

				// Highlighted ticks
				surftool->begin(Mesh::PRIMITIVE_LINES);
				surftool->add_color(col_hl);
				for (int r = 0; r < GIZMO_RING_TICKS; r++) {
					const real_t ang = (Math_TAU / GIZMO_RING_TICKS) * r;
					const Vector3 norm(Math::cos(ang), Math::sin(ang), 0);
					const real_t div = (r & 3) ? 0.05 : 0.1;
					const Vector3 pt1 = GIZMO_CIRCLE_SIZE * norm;
					const Vector3 pt2 = (GIZMO_CIRCLE_SIZE + div) * norm;
					surftool->add_line(pt1, pt2);
					const bool quad = (r == 0 || r == GIZMO_RING_TICKS * 0.25 || r == GIZMO_RING_TICKS * 0.5 || r == GIZMO_RING_TICKS * 0.75);
					if (quad) {
						const Vector3 orig = (GIZMO_CIRCLE_SIZE + 0.1) * norm;
						const Vector3 perp = 0.05 * Vector3(norm.y, -norm.x, 0);
						surftool->add_line(orig + perp, orig - perp);
					}
				}
				surftool->commit(rotate_gizmo[i].mesh); // surf 4
			} else if (i == 1) { // x scroller
				for (int p = 0; p < 2; p++) {
					surftool->begin(Mesh::PRIMITIVE_LINES);
					surftool->add_color(p ? col_hl : col);
					for (int r = 1; r < GIZMO_SCROLLER_SEGS; r++) {
						const real_t y = 2 * r * (GIZMO_CIRCLE_SIZE / GIZMO_SCROLLER_SEGS) - GIZMO_CIRCLE_SIZE;
						const real_t z = Math::sin((Math_PI * r) / GIZMO_SCROLLER_SEGS) * GIZMO_CIRCLE_SIZE;
						const real_t ofs = Math::sin((Math_PI * r) / GIZMO_SCROLLER_SEGS);
						if (p) {
							surftool->add_line(Point3(-0.01 - 0.05 * ofs, y, z + 0.25), Point3(0.01 + 0.05 * ofs, y, z + 0.25));
						} else {
							surftool->add_line(Point3(-0.05 - 0.1 * ofs, y, z), Point3(0.05 + 0.1 * ofs, y, z));
						}
					}
					surftool->commit(rotate_gizmo[i].mesh); // surf 0,1
				}
				// Hightlight
				surftool->begin(Mesh::PRIMITIVE_LINES);
				surftool->add_color(col_hl);
				for (int r = 1; r < GIZMO_SCROLLER_SEGS - 1; r++) {
					const real_t y = 2 * r * (GIZMO_CIRCLE_SIZE / GIZMO_SCROLLER_SEGS) - GIZMO_CIRCLE_SIZE;
					const real_t z = Math::sin((Math_PI * r) / GIZMO_SCROLLER_SEGS) * GIZMO_CIRCLE_SIZE;
					const real_t ofs = Math::sin((Math_PI * r) / GIZMO_SCROLLER_SEGS);
					surftool->add_line(Point3(-0.05 - 0.1 * ofs, y, z), Point3(0.05 + 0.1 * ofs, y, z));
				}
				surftool->commit(rotate_gizmo[i].mesh, false); // surf 2
			} else if (i == 2) { // y scroller
				for (int p = 0; p < 2; p++) {
					surftool->begin(Mesh::PRIMITIVE_LINES);
					surftool->add_color(p ? col_hl : col);
					for (int r = 1; r < GIZMO_SCROLLER_SEGS; r++) {
						if (r > (GIZMO_SCROLLER_SEGS / 2) - 2 && r < (GIZMO_SCROLLER_SEGS / 2) + 2) {
							continue; // skip some central,overlapped bars
						}
						const real_t x = 2 * r * (GIZMO_CIRCLE_SIZE / GIZMO_SCROLLER_SEGS) - GIZMO_CIRCLE_SIZE;
						const real_t z = Math::sin((Math_PI * r) / GIZMO_SCROLLER_SEGS) * GIZMO_CIRCLE_SIZE;
						const real_t ofs = Math::sin((Math_PI * r) / GIZMO_SCROLLER_SEGS);
						if (p) {
							surftool->add_line(Point3(x, -0.01 - 0.05 * ofs, z + 0.25), Point3(x, 0.01 + 0.05 * ofs, z + 0.25));
						} else {
							surftool->add_line(Point3(x, -0.05 - 0.1 * ofs, z), Point3(x, 0.05 + 0.1 * ofs, z));
						}
					}
					surftool->commit(rotate_gizmo[i].mesh); // surf 0,1
				}
				// Hightlight
				surftool->begin(Mesh::PRIMITIVE_LINES);
				surftool->add_color(col_hl);
				for (int r = 1; r < GIZMO_SCROLLER_SEGS - 1; r++) {
					const real_t x = 2 * r * (GIZMO_CIRCLE_SIZE / GIZMO_SCROLLER_SEGS) - GIZMO_CIRCLE_SIZE;
					const real_t z = Math::sin((Math_PI * r) / GIZMO_SCROLLER_SEGS) * GIZMO_CIRCLE_SIZE;
					const real_t ofs = Math::sin((Math_PI * r) / GIZMO_SCROLLER_SEGS);
					surftool->add_line(Point3(x, -0.05 - 0.1 * ofs, z), Point3(x, 0.05 + 0.1 * ofs, z));
				}
				surftool->commit(rotate_gizmo[i].mesh, false); // surf 2
			}
		}

		// Scale
		{
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

			const int arrow_sides = 4;

			Ref<SurfaceTool> surftool = memnew(SurfaceTool);
			surftool->begin(Mesh::PRIMITIVE_TRIANGLES);
			for (int k = 0; k < 4; k++) {
				Basis ma(ivec, Math_PI * 2 * real_t(k) / arrow_sides);
				Basis mb(ivec, Math_PI * 2 * real_t(k + 1) / arrow_sides);

				for (int j = 0; j < arrow_points - 1; j++) {
					Vector3 points[4] = {
						ma.xform(arrow[j]),
						mb.xform(arrow[j]),
						mb.xform(arrow[j + 1]),
						ma.xform(arrow[j + 1]),
					};

					surftool->add_color(col.lightened(0.5));
					surftool->add_triangle(points[0], points[1], points[2]);
					surftool->add_triangle(points[0], points[2], points[3]);
				}
			}
			surftool->commit(scale_gizmo[i].mesh);

			for (int k = 0; k < 4; k++) {
				Basis ma(ivec, Math_PI * 2 * real_t(k) / arrow_sides);
				Basis mb(ivec, Math_PI * 2 * real_t(k + 1) / arrow_sides);

				surftool->begin(Mesh::PRIMITIVE_LINE_STRIP);
				for (int j = 0; j < arrow_points - 1; j++) {
					Vector3 points[4] = {
						ma.xform(arrow[j]),
						mb.xform(arrow[j]),
						mb.xform(arrow[j + 1]),
						ma.xform(arrow[j + 1]),
					};

					surftool->add_color(col);
					surftool->add_vertex(points[0]);
					surftool->add_vertex(points[1]);
					surftool->add_vertex(points[2]);
					surftool->add_vertex(points[3]);
					surftool->add_vertex(points[0]);
				}
				surftool->commit(scale_gizmo[i].mesh);
			}
		}

		// Plane Scale
		{
			Vector3 vec = ivec2 - ivec3;
			Vector3 plane[4] = {
				vec * GIZMO_PLANE_DST,
				vec * GIZMO_PLANE_DST + ivec2 * GIZMO_PLANE_SIZE,
				vec * (GIZMO_PLANE_DST + GIZMO_PLANE_SIZE),
				vec * GIZMO_PLANE_DST - ivec3 * GIZMO_PLANE_SIZE
			};
			Vector3 cross[4] = {
				vec * (GIZMO_PLANE_DST + GIZMO_PLANE_SIZE * 0.5) - ivec2 * GIZMO_PLANE_SIZE * 0.75,
				vec * (GIZMO_PLANE_DST + GIZMO_PLANE_SIZE * 0.5) + ivec2 * GIZMO_PLANE_SIZE * 0.75,
				vec * (GIZMO_PLANE_DST + GIZMO_PLANE_SIZE * 0.5) - ivec3 * GIZMO_PLANE_SIZE * 0.75,
				vec * (GIZMO_PLANE_DST + GIZMO_PLANE_SIZE * 0.5) + ivec3 * GIZMO_PLANE_SIZE * 0.75,
			};

			Basis ma(ivec, Math_PI / 2);

			Vector3 points[8] = {
				ma.xform(plane[0]),
				ma.xform(plane[1]),
				ma.xform(plane[2]),
				ma.xform(plane[3]),
				ma.xform(cross[0]),
				ma.xform(cross[1]),
				ma.xform(cross[2]),
				ma.xform(cross[3]),
			};

			Ref<SurfaceTool> surftool = memnew(SurfaceTool);
			surftool->begin(Mesh::PRIMITIVE_TRIANGLES);
			surftool->add_color(col.lightened(0.5));
			surftool->add_triangle(points[0], points[1], points[2]);
			surftool->add_triangle(points[0], points[2], points[3]);
			surftool->commit(scale_plane_gizmo[i].mesh);

			surftool->begin(Mesh::PRIMITIVE_LINE_STRIP);
			surftool->add_color(col);
			surftool->add_vertex(points[0]);
			surftool->add_vertex(points[1]);
			surftool->add_vertex(points[2]);
			surftool->add_vertex(points[3]);
			surftool->add_vertex(points[0]);
			surftool->commit(scale_plane_gizmo[i].mesh);

			surftool->begin(Mesh::PRIMITIVE_LINES);
			surftool->add_color(col_hl);
			surftool->add_line(points[4], points[5]);
			surftool->add_line(points[6], points[7]);
			surftool->commit(scale_plane_gizmo[i].mesh);

			surftool->begin(Mesh::PRIMITIVE_LINE_STRIP);
			surftool->add_color(col_hl);
			surftool->add_vertex(points[0]);
			surftool->add_vertex(points[1]);
			surftool->add_vertex(points[2]);
			surftool->add_vertex(points[3]);
			surftool->add_vertex(points[0]);
			surftool->commit(scale_plane_gizmo[i].mesh, false);
		}
	}

	// Origin indicator
	{
		Vector<Color> origin_colors;
		Vector<Vector3> origin_points;

		for (int i = 0; i < 3; i++) {
			Color origin_color;
			Transform xform;
			xform.scale(Vector3(0.25, 1, 1));
			switch (i) {
				case 0: {
					origin_color = get_color("axis_x_color", "Editor");
					xform.rotate(Vector3(0, 0, 1), Math_PI / 2);
				} break;
				case 1: {
					origin_color = get_color("axis_y_color", "Editor");
					xform.rotate(Vector3(1, 0, 0), Math_PI);
				} break;
				case 2: {
					origin_color = get_color("axis_z_color", "Editor");
					xform.rotate(Vector3(1, 0, 0), -Math_PI / 2);
				} break;
				default:
					origin_color = Color();
					break;
			}

			origin_points.push_back(xform.xform(Point3(0, 0, 0)));
			origin_points.push_back(xform.xform(Point3(-0.3, -0.75, 0)));
			origin_points.push_back(xform.xform(Point3(-0.75, -0.75, 0)));
			origin_points.push_back(xform.xform(Point3(0, -1, 0)));
			origin_points.push_back(xform.xform(Point3(0.75, -0.75, 0)));
			origin_points.push_back(xform.xform(Point3(0.3, -0.75, 0)));
			origin_points.push_back(xform.xform(Point3(0, 0, 0)));

			origin_colors.push_multi(7, origin_color);
		}

		origin_indicator.mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		Array d;
		d.resize(VS::ARRAY_MAX);
		d[VS::ARRAY_VERTEX] = origin_points;
		d[VS::ARRAY_COLOR] = origin_colors;

		origin_indicator.mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINE_STRIP, d);

		// Origin marker
		origin_indicator.item = VS::get_singleton()->canvas_item_create();
		VS::get_singleton()->canvas_item_set_parent(origin_indicator.item, canvas_item_editor->get_viewport_control()->get_canvas_item());
		origin_indicator.mesh3d = VS::get_singleton()->canvas_item_create_mesh_3d(origin_indicator.mesh->get_rid());
		VS::get_singleton()->canvas_item_add_mesh_3d(origin_indicator.item, origin_indicator.mesh3d);
	}
}

void SpriteMeshEditor::_finish_indicators() {
	if (origin_indicator.item.is_valid()) {
		VS::get_singleton()->free(origin_indicator.item);
		origin_indicator.item = RID();
	}
}

void SpriteMeshEditor::_project_settings_changed() {
	if (get_viewport()) {
		_project_settings_change_pending = false;
	} else {
		// Could not update immediately, set a pending update. This may never happen, but is included for safety
		_project_settings_change_pending = true;
	}
}

void SpriteMeshEditor::edit(SpriteMesh *p_node) {
	if (node) {
		if (node->is_connected("transform_changed", this, "update_transform_gizmo_view")) {
			node->disconnect("transform_changed", this, "update_transform_gizmo_view");
		}
	}
	node = p_node;
	if (node) {
		if (!node->is_connected("transform_changed", this, "update_transform_gizmo_view")) {
			node->connect("transform_changed", this, "update_transform_gizmo_view");
		}
	}
	canvas_item_editor->update_viewport();
}

void SpriteMeshEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update_transform_gizmo_view"), &SpriteMeshEditor::update_transform_gizmo_view);
	ClassDB::bind_method(D_METHOD("_project_settings_changed"), &SpriteMeshEditor::_project_settings_changed);
	ClassDB::bind_method(D_METHOD("_node_removed", "node"), &SpriteMeshEditor::_node_removed);
}

SpriteMeshEditor::SpriteMeshEditor(EditorPlugin *p_plugin, EditorNode *p_node) {
	plugin = p_plugin;
	editor = p_node;
	node = nullptr;
	canvas_item_editor = nullptr;
	origin_indicator.enabled = true;
	dragging = NONE;
	undo_redo = EditorNode::get_undo_redo();
	_project_settings_change_pending = false;
	EditorSettings::get_singleton()->connect("settings_changed", this, "update_transform_gizmo_view");
}

SpriteMeshEditor::~SpriteMeshEditor() {
	EditorSettings::get_singleton()->disconnect("settings_changed", this, "update_transform_gizmo_view");
}

void SpriteMeshEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		sprite_mesh_editor->show();
		sprite_mesh_editor->set_process(true);

	} else {
		sprite_mesh_editor->hide();
		sprite_mesh_editor->edit(nullptr);
		sprite_mesh_editor->set_process(false);
	}
}

void SpriteMeshEditorPlugin::edit(Object *p_object) {
	sprite_mesh_editor->edit(cast_to<SpriteMesh>(p_object));
}

bool SpriteMeshEditorPlugin::forward_canvas_gui_input(const Ref<InputEvent> &p_event) {
	return sprite_mesh_editor->forward_gui_input(p_event);
}

void SpriteMeshEditorPlugin::forward_canvas_draw_over_viewport(Control *p_overlay) {
	sprite_mesh_editor->forward_canvas_draw_over_viewport(p_overlay);
}

SpriteMeshEditorPlugin::SpriteMeshEditorPlugin(EditorNode *p_node) {
	sprite_mesh_editor = memnew(SpriteMeshEditor(this, p_node));
	p_node->get_viewport()->add_child(sprite_mesh_editor);
	sprite_mesh_editor->hide();
}

SpriteMeshEditorPlugin::~SpriteMeshEditorPlugin() {
}

/// SpriteMeshLight

#ifdef TOOLS_ENABLED
static bool _is_inside_circle(const Point2 &center, real_t r, const Point2 &pt) {
	const real_t dist = (pt.x - center.x) * (pt.x - center.x) + (pt.y - center.y) * (pt.y - center.y);
	return dist <= r * r;
}

bool SpriteMeshLight::_edit_is_selected_on_click(const Point2 &p_point) const {
	return _is_inside_circle(light_position, light_radius, p_point);
}

Dictionary SpriteMeshLight::_edit_get_state() const {
	Dictionary state;
	state["light_position"] = light_position;
	state["light_radius"] = light_radius;
	state["light_power"] = light_power;
	return state;
}

void SpriteMeshLight::_edit_set_state(const Dictionary &p_state) {
	light_position = p_state["light_position"];
	light_radius = p_state["light_radius"];
	light_power = p_state["light_power"];
	_update();
}

bool SpriteMeshLight::_edit_state_changed(const Dictionary &p_state) const {
	return (light_position != p_state["light_position"]) && (light_radius != real_t(p_state["light_radius"])) && (light_power != real_t(p_state["light_power"]));
}

Transform2D SpriteMeshLight::_edit_get_transform() const {
	return Transform2D();
}

Transform2D SpriteMeshLight::_edit_get_global_transform() const {
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_V(!is_inside_tree(), _edit_get_transform());
#endif
	const CanvasItem *pi = cast_to<CanvasItem>(get_parent());
	if (pi) {
		return pi->get_global_transform() * _edit_get_transform();
	} else {
		return _edit_get_transform();
	}
}
#endif

void SpriteMeshLight::set_active(bool p_state) {
	if (active != p_state) {
		active = p_state;
		_update();
	}
}

bool SpriteMeshLight::is_active() const {
	return active;
}

void SpriteMeshLight::set_visible(bool p_visible) {
	if (visible == p_visible) {
		return;
	}

	visible = p_visible;
	VS::get_singleton()->canvas_item_set_visible(canvas_item, p_visible);

	if (!is_inside_tree()) {
		return;
	}

	_change_notify("visible");
}

bool SpriteMeshLight::is_visible() const {
	return visible;
}

void SpriteMeshLight::set_light_position(const Point2 &p_pos) {
	if (light_position != p_pos) {
		light_position = p_pos;
		_update();
		emit_signal("light_config_changed");
	}
}

Point2 SpriteMeshLight::get_light_position() const {
	return light_position;
}

void SpriteMeshLight::set_light_power(real_t p_power) {
	ERR_FAIL_COND(p_power > 1 || p_power < 0);
	if (light_power != p_power) {
		light_power = p_power;
		_update();
		emit_signal("light_config_changed");
	}
}

real_t SpriteMeshLight::get_light_power() const {
	return light_power;
}

void SpriteMeshLight::set_light_radius(real_t p_radius) {
	ERR_FAIL_COND(p_radius <= 10);
	if (light_radius != p_radius) {
		light_radius = p_radius;
		_update();
		emit_signal("light_config_changed");
	}
}

real_t SpriteMeshLight::get_light_radius() const {
	return light_radius;
}

void SpriteMeshLight::set_light_ambient(Color p_ambient) {
	if (ambient != p_ambient) {
		ambient = p_ambient;
		_update();
		emit_signal("light_config_changed");
	}
}

Color SpriteMeshLight::get_light_ambient() const {
	return ambient;
}

void SpriteMeshLight::set_light_diffuse(Color p_diffuse) {
	if (diffuse != p_diffuse) {
		diffuse = p_diffuse;
		_update();
		emit_signal("light_config_changed");
	}
}

Color SpriteMeshLight::get_light_diffuse() const {
	return diffuse;
}

void SpriteMeshLight::set_light_specular(Color p_specular) {
	if (specular != p_specular) {
		specular = p_specular;
		_update();
		emit_signal("light_config_changed");
	}
}

Color SpriteMeshLight::get_light_specular() const {
	return specular;
}

void SpriteMeshLight::set_debug_outline(bool p_state) {
	if (debug_outline != p_state) {
		debug_outline = p_state;
		_update();
	}
}

bool SpriteMeshLight::is_debug_outline() const {
	return debug_outline;
}

static void _canvas_item_add_circle(const RID &canvas_item, const Vector2 &center, real_t r, const Color &c, int segs = 32) {
	const real_t coef = Math_TAU / segs;
	Vector<Point2> vertices;
	real_t rads = 0;
	for (int i = 0; i <= segs; i++) {
		vertices.push_back(Point2(r * Math::cos(rads) + center.x, r * Math::sin(rads) + center.y));
		rads += coef;
	}
	Vector<Color> colors;
	colors.resize(vertices.size());
	colors.fill(c);
	VS::get_singleton()->canvas_item_add_polyline(canvas_item, vertices, colors);
}

void SpriteMeshLight::_update() {
	if (canvas_item.is_valid() && debug_outline) {
		VS::get_singleton()->canvas_item_clear(canvas_item);
		VS::get_singleton()->canvas_item_set_visible(canvas_item, visible);
		static Color _light_color(1, 1, 0), _inactive_color(0.25, 0.25, 0);
		// light power
		static Vector<Point2> arrow;
		static Vector<Color> arrow_color;
		if (arrow.size() == 0) {
			arrow.resize(7);
			arrow.write[0] = Point2(0, 0);
			arrow.write[1] = Point2(-0.3, -0.75);
			arrow.write[2] = Point2(-0.75, -0.75);
			arrow.write[3] = Point2(0, -1);
			arrow.write[4] = Point2(0.75, -0.75);
			arrow.write[5] = Point2(0.3, -0.75);
			arrow.write[6] = Point2(0, 0);
		}
		if (arrow_color.size() == 0) {
			arrow_color.resize(7);
			arrow_color.fill(_light_color);
		}
		Vector<Point2> arrow_xform;
		for (int s = 0; s < 4; s++) {
			Transform2D xform;
			xform.scale(Size2(MAX(5, light_power * 10), MAX(10, light_radius * light_power)));
			xform.rotate(s * Math_PI / 2);
			arrow_xform.resize(7);
			for (int p = 0; p < arrow.size(); p++) {
				arrow_xform.write[p] = light_position + xform.xform(arrow[p]);
			}
			VS::get_singleton()->canvas_item_add_polyline(canvas_item, arrow_xform, arrow_color);
		}
		// light radius
		const int segs = 16;
		const real_t step = Math_TAU / segs;
		for (int r = 0; r < segs; r++) {
			const real_t ang = step * r;
			const Vector2 norm(Math::sin(ang), Math::cos(ang));
			const int div = r & 1 ? 10 : 5;
			const Vector2 pt1 = light_position + (light_radius - div) * norm;
			const Vector2 pt2 = light_position + light_radius * norm;
			VS::get_singleton()->canvas_item_add_line(canvas_item, pt1, pt2, active ? _light_color : _inactive_color);
		}
		if (CanvasItemEditor *canvas_item_editor = CanvasItemEditor::get_singleton()) {
			const float zoom = canvas_item_editor->get_canvas_zoom() / MAX(1, EDSCALE);
			_canvas_item_add_circle(canvas_item, light_position, light_radius, active ? _light_color : _inactive_color, 32 * zoom);
		} else {
			_canvas_item_add_circle(canvas_item, light_position, light_radius, active ? _light_color : _inactive_color);
		}
		if (EditorNode *editor = EditorNode::get_singleton()) {
			// center icon
			const Ref<Texture> light = editor->get_gui_base()->get_icon("DirectionalLight", "EditorIcons");
			VS::get_singleton()->canvas_item_add_texture_rect(canvas_item, Rect2(light_position - light->get_size() * 0.5, light->get_size()), light->get_rid());
		}
	}
}

String SpriteMeshLight::get_configuration_warning() const {
	String warning = Node::get_configuration_warning();

	if (!cast_to<CanvasItem>(get_parent())) {
		if (warning != String()) {
			warning += "\n\n";
		}
		warning += TTR("This node should be attached to 2D scene.");
	}

	return warning;
}

void SpriteMeshLight::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_ENTER_TREE: {
			if (Engine::get_singleton()->is_editor_hint()) {
				update_configuration_warning(); // update warnings
			}
			canvas_item = RID_PRIME(VS::get_singleton()->canvas_item_create());
			ERR_FAIL_COND(!canvas_item.is_valid());
			if (Node2D *parent = cast_to<Node2D>(get_parent())) {
				VS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas());
			}
			if (CanvasItemEditor *canvas_item_editor = CanvasItemEditor::get_singleton()) {
				canvas_item_editor->connect("canvas_viewport_changed", this, "_update");
			}
			_update();
			emit_signal("light_node_moved");
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (CanvasItemEditor *canvas_item_editor = CanvasItemEditor::get_singleton()) {
				canvas_item_editor->disconnect("canvas_viewport_changed", this, "_update");
			}
			if (canvas_item.is_valid()) {
				VS::get_singleton()->free(canvas_item);
			}
			canvas_item = RID();
			emit_signal("light_node_moved");
		} break;
		case NOTIFICATION_UNPARENTED: {
			emit_signal("light_node_moved");
		} break;
		case NOTIFICATION_PARENTED: {
			emit_signal("light_node_moved");
		} break;
#ifdef TOOLS_ENABLED
		case EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED: {
			_update();
		} break;
#endif
	}
}

void SpriteMeshLight::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_active"), &SpriteMeshLight::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &SpriteMeshLight::is_active);
	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &SpriteMeshLight::set_visible);
	ClassDB::bind_method(D_METHOD("is_visible"), &SpriteMeshLight::is_visible);
	ClassDB::bind_method(D_METHOD("set_light_position"), &SpriteMeshLight::set_light_position);
	ClassDB::bind_method(D_METHOD("get_light_position"), &SpriteMeshLight::get_light_position);
	ClassDB::bind_method(D_METHOD("set_light_power"), &SpriteMeshLight::set_light_power);
	ClassDB::bind_method(D_METHOD("get_light_power"), &SpriteMeshLight::get_light_power);
	ClassDB::bind_method(D_METHOD("set_light_radius"), &SpriteMeshLight::set_light_radius);
	ClassDB::bind_method(D_METHOD("get_light_radius"), &SpriteMeshLight::get_light_radius);
	ClassDB::bind_method(D_METHOD("set_debug_outline"), &SpriteMeshLight::set_debug_outline);
	ClassDB::bind_method(D_METHOD("is_debug_outline"), &SpriteMeshLight::is_debug_outline);

	ClassDB::bind_method(D_METHOD("_update"), &SpriteMeshLight::_update);

#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("_edit_is_selected_on_click"), &SpriteMeshLight::_edit_is_selected_on_click);
	ClassDB::bind_method(D_METHOD("_edit_get_state"), &SpriteMeshLight::_edit_get_state);
	ClassDB::bind_method(D_METHOD("_edit_set_state"), &SpriteMeshLight::_edit_set_state);
	ClassDB::bind_method(D_METHOD("_edit_get_transform"), &SpriteMeshLight::_edit_get_transform);
	ClassDB::bind_method(D_METHOD("_edit_get_global_transform"), &SpriteMeshLight::_edit_get_global_transform);
#endif

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible"), "set_visible", "is_visible");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "light_position"), "set_light_position", "get_light_position");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "light_power", PROPERTY_HINT_RANGE, "0,1,0,0.01"), "set_light_power", "get_light_power");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "light_radius"), "set_light_radius", "get_light_radius");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_outline"), "set_debug_outline", "is_debug_outline");

	ADD_SIGNAL(MethodInfo("light_node_moved"));
	ADD_SIGNAL(MethodInfo("light_config_changed"));
}

SpriteMeshLight::SpriteMeshLight() {
	active = true;
	visible = true;
	light_position = Point2(50, 50);
	light_power = 0.75;
	light_radius = 50;
#ifdef TOOLS_ENABLED
	debug_outline = true;
#else
	debug_outline = false;
#endif
}

SpriteMeshLight::~SpriteMeshLight() {
	if (canvas_item.is_valid()) {
		VS::get_singleton()->free(canvas_item);
	}
}

/// SpriteMeshLightEditor

bool SpriteMeshLightEditor::forward_gui_input(const Ref<InputEvent> &p_event) {
	if (!node) {
		return false;
	}

	const CanvasItemEditor::Tool tool = canvas_item_editor->get_current_tool();
	if (tool != CanvasItemEditor::TOOL_SELECT) {
		return false;
	}

	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid()) {
		Transform2D xform = canvas_item_editor->get_canvas_transform() * node->_edit_get_global_transform();

		const Vector2 gpoint = mb->get_position();
		const Vector2 cpoint = node->_edit_get_global_transform().affine_inverse().xform(node->get_light_position() + canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(gpoint)));

		(void)cpoint;

		if (mb->is_pressed()) {
			if (mb->get_button_index() == BUTTON_LEFT) {
				if (mb->get_control() || mb->get_shift() || mb->get_alt()) {
					return false;
				}
				undo_redo_state = node->_edit_get_state(); // save state in case we may need it
				// moving
				const Ref<Texture> light = get_icon("DirectionalLight", "EditorIcons");
				const real_t dist = 1.1 * (light->get_width() + light->get_height()) / 4;
				Point2 cent = xform.xform(node->get_light_position());
				if (cent.distance_to(gpoint) < dist) {
					dragging = DRAG_MOVE;
					return true;
				} else {
					// resizing
					const Ref<Texture> select_handle = get_icon("EditorHandle", "EditorIcons");
					const real_t dist = 1.1 * (select_handle->get_width() + select_handle->get_height()) / 4;
					Point2 h1 = xform.xform(node->get_light_position() + Point2(node->get_light_radius(), 0)) + select_handle->get_size() * Size2(0.5, 0);
					if (h1.distance_to(gpoint) < dist) {
						dragging = DRAG_RESIZE;
						return true;
					} else {
						Point2 h2 = xform.xform(node->get_light_position() + Point2(-node->get_light_radius(), 0)) - select_handle->get_size() * Size2(0.5, 0);
						if (h2.distance_to(gpoint) < dist) {
							dragging = DRAG_RESIZE;
							return true;
						} else {
							Point2 h3 = xform.xform(node->get_light_position() + Point2(0, node->get_light_radius())) + select_handle->get_size() * Size2(0.5, 0);
							if (h3.distance_to(gpoint) < dist) {
								dragging = DRAG_RESIZE;
								return true;
							} else {
								Point2 h4 = xform.xform(node->get_light_position() + Point2(0, -node->get_light_radius())) - select_handle->get_size() * Size2(0.5, 0);
								if (h4.distance_to(gpoint) < dist) {
									dragging = DRAG_RESIZE;
									return true;
								}
							}
						}
					}
				}
			}
		} else if (dragging != NONE) {
			if (node->_edit_state_changed(undo_redo_state)) {
				switch (dragging) {
					case DRAG_MOVE: {
						undo_redo->create_action(TTR("Move SpriteMeshLight"));
					} break;
					case DRAG_RESIZE: {
						undo_redo->create_action(TTR("Resize SpriteMeshLight"));
					} break;
				}
				undo_redo->add_do_method(node, "_edit_set_state", node->_edit_get_state());
				undo_redo->add_undo_method(node, "_edit_set_state", undo_redo_state);
			}
			dragging = NONE;
			return true;
		}
	}

	Ref<InputEventMouseMotion> mm = p_event;

	if (mm.is_valid()) {
		const Vector2 gpoint = mm->get_position();
		const Vector2 cpoint = node->_edit_get_global_transform().affine_inverse().xform(canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(gpoint)));
		if (dragging != NONE) {
			switch (dragging) {
				case DRAG_MOVE: {
					node->set_light_position(cpoint);
				} break;
				case DRAG_RESIZE: {
					const real_t radius = MAX(1, (cpoint - node->get_light_position()).length());
					node->set_light_radius(radius);
				} break;
			}
			canvas_item_editor->update_viewport();
		}
	}

	return false;
}

void SpriteMeshLightEditor::forward_canvas_draw_over_viewport(Control *p_overlay) {
	if (!node) {
		return;
	}

	CanvasItemEditor::Tool tool = CanvasItemEditor::get_singleton()->get_current_tool();
	if (tool != CanvasItemEditor::TOOL_SELECT) {
		return;
	}

	Transform2D xform = canvas_item_editor->get_canvas_transform() * node->_edit_get_global_transform();
	const Ref<Texture> handle = get_icon("EditorPathSharpHandle", "EditorIcons");
	const Ref<Texture> select_handle = get_icon("EditorHandle", "EditorIcons");

	p_overlay->draw_texture(select_handle, xform.xform(node->get_light_position() + Point2(node->get_light_radius(), 0)) - select_handle->get_size() * Size2(0, 0.5));
	p_overlay->draw_texture(select_handle, xform.xform(node->get_light_position() + Point2(-node->get_light_radius(), 0)) - select_handle->get_size() * Size2(1, 0.5));
	p_overlay->draw_texture(select_handle, xform.xform(node->get_light_position() + Point2(0, node->get_light_radius())) - select_handle->get_size() * Size2(0.5, 0));
	p_overlay->draw_texture(select_handle, xform.xform(node->get_light_position() + Point2(0, -node->get_light_radius())) - select_handle->get_size() * Size2(0.5, 1));
}

void SpriteMeshLightEditor::edit(SpriteMeshLight *p_node) {
	if (!canvas_item_editor) {
		canvas_item_editor = CanvasItemEditor::get_singleton();
	}

	node = p_node;

	canvas_item_editor->update_viewport();
}

SpriteMeshLightEditor::SpriteMeshLightEditor(EditorNode *p_editor) {
	editor = p_editor;
	undo_redo = EditorNode::get_undo_redo();
	dragging = NONE;
}

/// SpriteMeshLightEditorPlugin

bool SpriteMeshLightEditorPlugin::forward_canvas_gui_input(const Ref<InputEvent> &p_event) {
	return node_editor->forward_gui_input(p_event);
}

void SpriteMeshLightEditorPlugin::forward_canvas_draw_over_viewport(Control *p_overlay) {
	node_editor->forward_canvas_draw_over_viewport(p_overlay);
}

void SpriteMeshLightEditorPlugin::edit(Object *p_object) {
	node_editor->edit(cast_to<SpriteMeshLight>(p_object));
}

bool SpriteMeshLightEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("SpriteMeshLight");
}

void SpriteMeshLightEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		node_editor->show();
	} else {
		node_editor->hide();
		node_editor->edit(nullptr);
	}
}

SpriteMeshLightEditorPlugin::SpriteMeshLightEditorPlugin(EditorNode *p_node) {
	node_editor = memnew(SpriteMeshLightEditor(p_node));
	CanvasItemEditor::get_singleton()->add_control_to_menu_panel(node_editor);
	node_editor->hide();
}

SpriteMeshLightEditorPlugin::~SpriteMeshLightEditorPlugin() {
}
