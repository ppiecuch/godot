/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"
#include "image_loader_svg.h"
#include "vector_graphics_adaptive_renderer.h"
#include "vector_graphics_color.h"
#include "vector_graphics_gradient.h"
#include "vector_graphics_linear_gradient.h"
#include "vector_graphics_paint.h"
#include "vector_graphics_path.h"
#include "vector_graphics_radial_gradient.h"
#include "vector_graphics_renderer.h"
#include "vector_graphics_texture_renderer.h"

#ifdef TOOLS_ENABLED
#include "vector_graphics_editor_plugin.h"

static void editor_init_callback() {
	EditorNode *editor = EditorNode::get_singleton();
	editor->add_editor_plugin(memnew(VGEditorPlugin(editor)));
}
#endif // TOOLS_ENABLED

void register_svg_mesh_types() {
	ClassDB::register_class<VGPath>();
	ClassDB::register_virtual_class<VGPaint>();
	ClassDB::register_class<VGColor>();
	ClassDB::register_class<VGGradient>();
	ClassDB::register_class<VGLinearGradient>();
	ClassDB::register_class<VGRadialGradient>();

	ClassDB::register_virtual_class<VGRenderer>();
	ClassDB::register_class<VGSpriteRenderer>();
	ClassDB::register_class<VGMeshRenderer>();

	Ref<ResourceImporterSVGSpatial> svg_spatial_loader;
	svg_spatial_loader.instance();
	ResourceFormatImporter::get_singleton()->add_importer(svg_spatial_loader);

	Ref<ResourceImporterSVGNode2D> svg_node_2d_loader;
	svg_node_2d_loader.instance();
	ResourceFormatImporter::get_singleton()->add_importer(svg_node_2d_loader);

	Ref<ResourceImporterSVGVGPath> svg_vg_path_loader;
	svg_vg_path_loader.instance();
	ResourceFormatImporter::get_singleton()->add_importer(svg_vg_path_loader);

#ifdef TOOLS_ENABLED
	EditorNode::add_init_callback(editor_init_callback);
#endif
}

void unregister_svg_mesh_types() {
}
