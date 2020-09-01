/*************************************************************************/
/*  sprite_mesh.cpp                                                   */
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
			draw_mesh(mesh, texture, 0);
		}
	}
}

void SpriteMesh::set_mesh(const Ref<Mesh> &p_mesh) {
	if (mesh == p_mesh)
		return;
	mesh = p_mesh;
	update();
	emit_signal("mesh_changed");
	_change_notify("mesh");
}

Ref<Mesh> SpriteMesh::get_mesh() const {
	return mesh;
}

void SpriteMesh::set_texture(const Ref<Texture> &p_texture) {
	if (texture == p_texture)
		return;
	texture = p_texture;
	update();
	emit_signal("texture_changed");
	_change_notify("texture");
}

Ref<Texture> SpriteMesh::get_texture() const {
	return texture;
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

bool SpriteMesh::is_inside_mesh(const Point2 &p_point, double p_tolerance) const {
	// TODO
	return true;
}

Rect2 SpriteMesh::get_rect() const {
	// TODO
	// Transform mesh AABB base on its current scale and roatation.
	return Rect2();
}

Rect2 SpriteMesh::get_anchorable_rect() const {
	return get_rect();
}

void SpriteMesh::_validate_property(PropertyInfo &property) const {

}

void SpriteMesh::_texture_changed() {

	// Changes to the texture need to trigger an update to make
	// the editor redraw the sprite with the updated texture.
	if (texture.is_valid()) {
		update();
	}
}

void SpriteMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &SpriteMesh::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &SpriteMesh::get_mesh);

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &SpriteMesh::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &SpriteMesh::get_texture);

	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &Sprite::set_centered);
	ClassDB::bind_method(D_METHOD("is_centered"), &Sprite::is_centered);

	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &Sprite::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &Sprite::get_offset);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");

	ADD_SIGNAL(MethodInfo("texture_changed"));
	ADD_SIGNAL(MethodInfo("mesh_changed"));
}

SpriteMesh::SpriteMesh() {
	centered = true;
}

SpriteMesh::~SpriteMesh() {
}
