#include "tree_2d.h"
#include "scene/2d/canvas_item.h"

void Tree2D::_notification(int p_what) {
	if (p_what == NOTIFICATION_DRAW) {
		if (mesh.is_valid()) {
			draw_mesh(mesh, texture, 0);
		}
	}
}

void Tree2D::set_mesh(const Ref<Mesh> &p_mesh) {
	if (mesh == p_mesh)
		return;
	mesh = p_mesh;
	// default materials:
	Ref<CanvasItemMaterial> mat_branch = Ref<CanvasItemMaterial>(memnew(CanvasItemMaterial));
    mat_branch->set_stencil_mode(CanvasItemMaterial::STENCIL_MODE_FILL);
	Ref<CanvasItemMaterial> mat_shadows = Ref<CanvasItemMaterial>(memnew(CanvasItemMaterial));
    mat_shadows->set_stencil_mode(CanvasItemMaterial::STENCIL_MODE_MASK);
	Ref<CanvasItemMaterial> mat_decors = Ref<CanvasItemMaterial>(memnew(CanvasItemMaterial));
	for(int surf=0; surf<mesh->get_surface_count(); surf++) {
		String n = "n/a";
		if (ArrayMesh *arraymesh = Object::cast_to<ArrayMesh>(*mesh))
			n = arraymesh->surface_get_name(surf);
		Ref<Material> m = mesh->surface_get_material(surf);
		if (n == "branch") {
			mesh->surface_set_material(surf, mat_branch);
		} else if (n == "leaves") {
			mesh->surface_set_material(surf, mat_decors);
		} else if (n == "shadows") {
			mesh->surface_set_material(surf, mat_shadows);
		}
		if (m.is_valid() &&  m != mesh->surface_get_material(surf))
			WARN_PRINTS("Replaced material for surface " + n);
	}
	update();
	emit_signal("mesh_changed");
	_change_notify("mesh");
}

Ref<Mesh> Tree2D::get_mesh() const {
	return mesh;
}

void Tree2D::set_texture(const Ref<Texture> &p_texture) {
	if (texture == p_texture)
		return;
	texture = p_texture;
	update();
	emit_signal("texture_changed");
	_change_notify("texture");
}

Ref<Texture> Tree2D::get_texture() const {
	return texture;
}

Tree2D::~Tree2D() {
}

Tree2D::Tree2D() {
}

#ifdef TOOLS_ENABLED
Rect2 Tree2D::_edit_get_rect() const {

	if (mesh.is_valid()) {
		AABB aabb = mesh->get_aabb();
		return Rect2(aabb.position.x, aabb.position.y, aabb.size.x, aabb.size.y);
	}

	return Node2D::_edit_get_rect();
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
