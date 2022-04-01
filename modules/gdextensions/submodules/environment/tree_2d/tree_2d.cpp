/*************************************************************************/
/*  tree_2d.cpp                                                          */
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

#include "tree_2d.h"
#include "scene/2d/canvas_item.h"

void Tree2D::set_mesh(const Ref<Mesh> &p_mesh) {
	if (_mesh->get_mesh() == p_mesh) {
		return;
	}
	Ref<Mesh> mesh = p_mesh;
	// default materials:
	Ref<CanvasItemMaterial> mat_branch = Ref<CanvasItemMaterial>(memnew(CanvasItemMaterial));
	mat_branch->set_stencil_mode(CanvasItemMaterial::STENCIL_MODE_FILL);
	Ref<CanvasItemMaterial> mat_shadows = Ref<CanvasItemMaterial>(memnew(CanvasItemMaterial));
	mat_shadows->set_stencil_mode(CanvasItemMaterial::STENCIL_MODE_MASK);
	Ref<CanvasItemMaterial> mat_decors = Ref<CanvasItemMaterial>(memnew(CanvasItemMaterial));
	for (int surf = 0; surf < mesh->get_surface_count(); surf++) {
		String n = "n/a";
		if (ArrayMesh *arraymesh = cast_to<ArrayMesh>(*mesh))
			n = arraymesh->surface_get_name(surf);
		Ref<Material> m = mesh->surface_get_material(surf);
		if (n == "branch") {
			mesh->surface_set_material(surf, mat_branch);
		} else if (n == "leaves") {
			mesh->surface_set_material(surf, mat_decors);
		} else if (n == "shadows") {
			mesh->surface_set_material(surf, mat_shadows);
		}
		if (m.is_valid() && m != mesh->surface_get_material(surf))
			WARN_PRINT("Replaced material for surface " + n);
	}
	_mesh->set_mesh(p_mesh);
	emit_signal("mesh_changed");
	_change_notify("mesh");
}

Ref<Mesh> Tree2D::get_mesh() const {
	return _mesh->get_mesh();
}

void Tree2D::set_texture(const Ref<Texture> &p_texture) {
	if (_mesh->get_texture() == p_texture) {
		return;
	}
	_mesh->set_texture(p_texture);
	emit_signal("texture_changed");
	_change_notify("texture");
}

Ref<Texture> Tree2D::get_texture() const {
	return _mesh->get_texture();
}

#ifdef TOOLS_ENABLED
Rect2 Tree2D::_edit_get_rect() const {
	if (_mesh->get_mesh().is_valid()) {
		AABB aabb = _mesh->get_mesh()->get_aabb();
		return Rect2(aabb.position.x, aabb.position.y, aabb.size.x, aabb.size.y);
	}

	return Rect2();
}
#endif

void Tree2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &Tree2D::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &Tree2D::get_mesh);

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &Tree2D::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &Tree2D::get_texture);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");

	ADD_SIGNAL(MethodInfo("texture_changed"));
	ADD_SIGNAL(MethodInfo("mesh_changed"));
}

Tree2D::Tree2D() {
	_mesh = memnew(MeshInstance2D);
	add_child(_mesh);
}

Tree2D::~Tree2D() {
}
