/**************************************************************************/
/*  vector_graphics_mesh_renderer.cpp                                     */
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

#include "vector_graphics_mesh_renderer.h"
#include "tove2d/src/cpp/mesh/meshifier.h"
#include "vector_graphics_path.h"

class Renderer {
	int fill_index;
	int line_index;

	tove::MeshRef tove_mesh;
	tove::GraphicsRef root_graphics;

public:
	Renderer(const tove::MeshRef &p_tove_mesh, const tove::GraphicsRef &p_root_graphics) {
		fill_index = 0;
		line_index = 0;
		tove_mesh = p_tove_mesh;
		root_graphics = p_root_graphics;
	}

	void traverse(Node *p_node, const Transform2D &p_transform) {
		const int n = p_node->get_child_count();
		for (int i = 0; i < n; i++) {
			Node *child = p_node->get_child(i);

			Transform2D t;
			if (child->is_class_ptr(CanvasItem::get_class_ptr_static())) {
				t = p_transform * Object::cast_to<CanvasItem>(child)->get_transform();
			} else {
				t = p_transform;
			}

			traverse(child, t);
		}

		if (p_node->is_class_ptr(VGPath::get_class_ptr_static())) {
			VGPath *path = Object::cast_to<VGPath>(p_node);
			Ref<VGRenderer> renderer = path->get_inherited_renderer();
			if (renderer.is_valid() && renderer->is_class_ptr(VGAbstractMeshRenderer::get_class_ptr_static())) {
				Ref<VGAbstractMeshRenderer> meshRenderer = Object::cast_to<VGAbstractMeshRenderer>(renderer.ptr());
				if (meshRenderer.is_valid()) {
					tove::TesselatorRef tesselator = meshRenderer->get_tesselator();
					if (tesselator) {
						const Size2 s = path->is_inside_tree() ? path->get_global_transform().get_scale() : path->get_transform().get_scale();
						tesselator->beginTesselate(root_graphics.get(), MAX(s.width, s.height));

						tesselator->pathToMesh(
								UPDATE_MESH_EVERYTHING,
								new_transformed_path(path->get_tove_path(), p_transform),
								tove_mesh, tove_mesh,
								fill_index, line_index);

						tesselator->endTesselate();
					}
				}
			}
		}
	}
};

Rect2 VGAbstractMeshRenderer::render_mesh(Ref<ArrayMesh> &p_mesh, Ref<Material> &r_material, Ref<Texture> &r_texture, VGPath *p_path, bool p_hq, bool p_spatial) {
	clear_mesh(p_mesh);

	VGPath *root = p_path->get_root_path();
	tove::GraphicsRef subtree_graphics = root->get_subtree_graphics();

	tove::MeshRef tove_mesh;

	if (p_hq && !subtree_graphics->areColorsSolid()) {
		tove_mesh = tove::tove_make_shared<tove::PaintMesh>();
	} else {
		tove_mesh = tove::tove_make_shared<tove::ColorMesh>();
	}

	Renderer r(tove_mesh, subtree_graphics);
	r.traverse(p_path, Transform2D());

	r_material = copy_mesh(p_mesh, tove_mesh, subtree_graphics, r_texture, p_spatial);

	return tove_bounds_to_rect2(p_path->get_tove_path()->getBounds());
}

void VGAbstractMeshRenderer::_bind_methods() {}

VGAbstractMeshRenderer::VGAbstractMeshRenderer() {}
