/**************************************************************************/
/*  vector_graphics_editor_plugin.cpp                                     */
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

#include "vector_graphics_editor_plugin.h"
#include "editor/plugins/canvas_item_editor_plugin.h"

bool VGEditorPlugin::forward_canvas_gui_input(const Ref<InputEvent> &p_event) {
	return vg_editor->forward_gui_input(p_event);
}

void VGEditorPlugin::forward_canvas_draw_over_viewport(Control *p_overlay) {
	vg_editor->forward_canvas_draw_over_viewport(p_overlay);
}

void VGEditorPlugin::edit(Object *p_object) {
	vg_editor->edit(Object::cast_to<Node>(p_object));
}

bool VGEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class(klass);
}

void VGEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		vg_editor->show();
	} else {
		vg_editor->hide();
		vg_editor->edit(NULL);
	}
}

VGEditorPlugin::VGEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	vg_editor = memnew(VGEditor(p_node));
	klass = "VGPath";
	CanvasItemEditor::get_singleton()->add_control_to_menu_panel(vg_editor);

	vg_editor->hide();
}

VGEditorPlugin::~VGEditorPlugin() {
}
