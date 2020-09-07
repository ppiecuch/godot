/*************************************************************************/
/*  sprite_mesh.cpp                                                      */
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

#include <string>
#include <vector>

#include "core/math/math_funcs.h"
#include "core/math/vector2.h"

#include "sprite_mesh.h"

// Reference:
// https://stackoverflow.com/questions/6053522/how-to-recalculate-axis-aligned-bounding-box-after-translate-rotate
// https://github.com/libigl/libigl/issues/514 (Projecting mesh onto a plane)
// https://github.com/pyvista/pyvista-support/issues/20 (Project points/surface to a plane)
// https://github.com/patrikhuber/eos/issues/140
// * https://gamedev.stackexchange.com/questions/10261/android-collision-detection-of-a-3d-object-based-on-a-2d-projection

Rect2 _rect_offset(Rect2 rect, real_t dx, real_t dy) {
	rect.position.x += dx;
	rect.position.y += dy;
	return rect;
}

static Rect2 _rect_scale_to_fit(const Rect2 &dest_rect, const Rect2 &src_rect) {
	// Start off with a rectangle the same size as the destination rectangle:
	Rect2 rect = dest_rect;
	// Reduce it by 1 pixel all round to leave a slight margin:
	_rect_offset(rect, -1, -1);
	// find the aspect ratios of the two rectangles:
	const float aspect_scale = src_rect.size.width / src_rect.size.height;
	const float aspect_dest = dest_rect.size.width / dest_rect.size.height;
	// compare the aspect ratios of the two rectangles:
	if (aspect_scale > aspect_dest)
		// if the inside rectangle has the wider shape, reduce its height
		rect.size.height = rect.size.width / aspect_scale;
	else
		// if the inside rectangle has the taller shape, reduce its width:
		rect.size.width = rect.size.height * aspect_scale;
	//now centre the rectangle:
	rect.position.x = dest_rect.size.width + (dest_rect.size.width - rect.size.width) / 2;
	rect.position.y = dest_rect.size.height + (dest_rect.size.height - rect.size.height) / 2;
	return rect;
}

static Rect2 _rect_scale_to_fill(const Rect2 &dest_rect, const Rect2 &src_rect) {
	float aspect_dest = dest_rect.size.width / dest_rect.size.height;
	float aspect_scale = src_rect.size.width / src_rect.size.height;
	Rect2 project_to = Rect2(0, 0, 0, 0);
	// Scale the image so that the aspect ratio is preserved and the
	// dest target size is filled.
	if (aspect_scale < aspect_dest)
		//if the inside rectangle has the taller shape, reduce its width:
		project_to.size.width = project_to.size.height * aspect_scale;
	else
		//if the inside rectangle has the wider shape, reduce its height
		project_to.size.height = project_to.size.width / aspect_scale;
	// now center the rectangle:
	project_to.position.x = dest_rect.size.width + (dest_rect.size.width - project_to.size.width) / 2;
	project_to.position.y = dest_rect.size.height + (dest_rect.size.height - project_to.size.height) / 2;
	return project_to;
}

#ifdef TOOLS_ENABLED
Dictionary SpriteMesh::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["mesh_offset"] = mesh_offset;
	return state;
}

void SpriteMesh::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_mesh_offset(p_state["mesh_offset"]);
}

void SpriteMesh::_edit_set_pivot(const Point2 &p_pivot) {
	set_mesh_offset(get_mesh_offset() - p_pivot);
	set_position(get_transform().xform(p_pivot));
}

Point2 SpriteMesh::_edit_get_pivot() const {
	return Vector2();
}

bool SpriteMesh::_edit_use_pivot() const {
	return true;
}

bool SpriteMesh::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {

	return is_inside_mesh(p_point, p_tolerance);
}

Rect2 SpriteMesh::_edit_get_rect() const {
	return get_rect();
}

bool SpriteMesh::_edit_use_rect() const {
	return texture.is_valid();
}
#endif

void SpriteMesh::_notification(int p_what) {
	if (p_what == NOTIFICATION_DRAW) {
		if (mesh.is_valid()) {
			draw_mesh(mesh, texture, normal_map, _mesh_xform);
			draw_rect(get_rect(), Color::named("yellow"), false);
		}
	}
}

void SpriteMesh::set_mesh(const Ref<Mesh> &p_mesh) {
	if (mesh == p_mesh)
		return;
	mesh = p_mesh;
	mesh_aabb = p_mesh->get_aabb();
	update();
	emit_signal("mesh_changed");
	_change_notify("mesh");
}

Ref<Mesh> SpriteMesh::get_mesh() const {
	return mesh;
}

void SpriteMesh::_update_xform_values() {

	mesh_angle = _mesh_xform.get_rotation();
	mesh_scale = _mesh_xform.get_scale();
	_mesh_xform_dirty = false;
}

void SpriteMesh::_update_transform() {

	_mesh_xform.set_rotation_and_scale(mesh_angle, mesh_scale);
	_mesh_xform.elements[2] = get_rect().position + mesh_offset;

	VisualServer::get_singleton()->canvas_item_set_transform(get_canvas_item(), _mesh_xform);

	if (!is_inside_tree())
		return;

	_notify_transform();
}

void SpriteMesh::set_mesh_rotation(float p_radians) {

	if (_mesh_xform_dirty)
		_update_xform_values();
	mesh_angle = p_radians;
	_update_transform();
}

void SpriteMesh::set_mesh_rotation_degrees(float p_degrees) {

	set_mesh_rotation(Math::deg2rad(p_degrees));
}

void SpriteMesh::set_mesh_scale(const Size2 &p_scale) {

	if (_mesh_xform_dirty)
		_update_xform_values();
	mesh_scale = p_scale;
	// Avoid having 0 scale values, can lead to errors in physics and rendering.
	if (mesh_scale.x == 0)
		mesh_scale.x = CMP_EPSILON;
	if (mesh_scale.y == 0)
		mesh_scale.y = CMP_EPSILON;
	_update_transform();
}

float SpriteMesh::get_mesh_rotation() const {
	if (_mesh_xform_dirty)
		((SpriteMesh *)this)->_update_xform_values();

	return mesh_angle;
}

float SpriteMesh::get_mesh_rotation_degrees() const {

	return Math::rad2deg(get_mesh_rotation());
}

Size2 SpriteMesh::get_mesh_scale() const {
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

	normal_map = p_texture;
	update();
}

Ref<Texture> SpriteMesh::get_mesh_normal_map() const {

	return normal_map;
}

void SpriteMesh::set_mesh_centered(bool p_mesh_center) {

	mesh_centered = p_mesh_center;
	update();
	item_rect_changed();
}

bool SpriteMesh::is_mesh_centered() const {

	return mesh_centered;
}

void SpriteMesh::set_mesh_offset(const Point2 &p_mesh_offset) {

	mesh_offset = p_mesh_offset;
	update();
	item_rect_changed();
	_change_notify("offset");
}
Point2 SpriteMesh::get_mesh_offset() const {

	return mesh_offset;
}

bool SpriteMesh::is_inside_mesh(const Point2 &p_point, double p_tolerance) const {
	// TODO
	return true;
}

void SpriteMesh::set_mesh_flip_h(bool p_mesh_flip) {

	mesh_hflip = p_mesh_flip;
	update();
}
bool SpriteMesh::is_mesh_flipped_h() const {

	return mesh_hflip;
}

void SpriteMesh::set_mesh_flip_v(bool p_mesh_flip) {

	mesh_vflip = p_mesh_flip;
	update();
}
bool SpriteMesh::is_mesh_flipped_v() const {

	return mesh_vflip;
}

Rect2 SpriteMesh::get_rect() const {
	return Rect2(
			mesh_aabb.get_position().x, mesh_aabb.get_position().y,
			mesh_aabb.get_size().x, mesh_aabb.get_size().y);
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

	ClassDB::bind_method(D_METHOD("set_mesh_rotation", "mesh_rotation"), &SpriteMesh::set_mesh_rotation);
	ClassDB::bind_method(D_METHOD("get_mesh_rotation"), &SpriteMesh::get_mesh_rotation);

	ClassDB::bind_method(D_METHOD("set_mesh_rotation_degrees", "mesh_rotation_degrees"), &SpriteMesh::set_mesh_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_mesh_rotation_degrees"), &SpriteMesh::get_mesh_rotation_degrees);

	ClassDB::bind_method(D_METHOD("set_mesh_scale", "mesh_scale"), &SpriteMesh::set_mesh_scale);
	ClassDB::bind_method(D_METHOD("get_mesh_scale"), &SpriteMesh::get_mesh_scale);

	ClassDB::bind_method(D_METHOD("set_mesh_flip_h", "mesh_fiph_h"), &SpriteMesh::set_mesh_flip_h);
	ClassDB::bind_method(D_METHOD("is_mesh_flipped_h"), &SpriteMesh::is_mesh_flipped_h);

	ClassDB::bind_method(D_METHOD("set_mesh_flip_v", "mesh_fiph_v"), &SpriteMesh::set_mesh_flip_v);
	ClassDB::bind_method(D_METHOD("is_mesh_flipped_v"), &SpriteMesh::is_mesh_flipped_v);

	ClassDB::bind_method(D_METHOD("set_mesh_texture", "mesh_texture"), &SpriteMesh::set_mesh_texture);
	ClassDB::bind_method(D_METHOD("get_mesh_texture"), &SpriteMesh::get_mesh_texture);

	ClassDB::bind_method(D_METHOD("set_mesh_normal_map", "mesh_normal_map"), &SpriteMesh::set_mesh_normal_map);
	ClassDB::bind_method(D_METHOD("get_mesh_normal_map"), &SpriteMesh::get_mesh_normal_map);

	ClassDB::bind_method(D_METHOD("set_mesh_centered", "centered"), &SpriteMesh::set_mesh_centered);
	ClassDB::bind_method(D_METHOD("is_mesh_centered"), &SpriteMesh::is_mesh_centered);

	ClassDB::bind_method(D_METHOD("set_mesh_offset", "offset"), &SpriteMesh::set_mesh_offset);
	ClassDB::bind_method(D_METHOD("get_mesh_offset"), &SpriteMesh::get_mesh_offset);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_GROUP("Mesh Transform", "mesh_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mesh_centered"), "set_mesh_centered", "is_mesh_centered");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "mesh_offset"), "set_mesh_offset", "get_mesh_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mesh_flip_h"), "set_mesh_flip_h", "is_mesh_flipped_h");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mesh_flip_v"), "set_mesh_flip_v", "is_mesh_flipped_v");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mesh_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_mesh_rotation", "get_mesh_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mesh_rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_mesh_rotation_degrees", "get_mesh_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "mesh_scale"), "set_mesh_scale", "get_mesh_scale");
	ADD_GROUP("", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mesh_texture", "get_mesh_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_normal_map", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mesh_normal_map", "get_mesh_normal_map");

	ADD_SIGNAL(MethodInfo("texture_changed"));
	ADD_SIGNAL(MethodInfo("mesh_changed"));
}

SpriteMesh::SpriteMesh() {
	mesh_hflip = false;
	mesh_vflip = false;
	mesh_centered = true;
	mesh_offset = Vector2(0, 0);
	mesh_angle = 0;
	mesh_scale = Vector2(1, 1);
}

SpriteMesh::~SpriteMesh() {
}
