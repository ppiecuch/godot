/**************************************************************************/
/*  virtual_joystick_editor_plugin.cpp                                    */
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

#include "virtual_joystick_editor_plugin.h"

#include "canvas_item_editor_plugin.h"
#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#include "scene/gui/virtual_joystick.h"

void VirtualJoystickEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			get_tree()->connect("node_removed", this, "_node_removed");
		} break;

		case NOTIFICATION_EXIT_TREE: {
			get_tree()->disconnect("node_removed", this, "_node_removed");
		} break;
	}
}

void VirtualJoystickEditor::_node_removed(Node *p_node) {
	if (p_node == node) {
		node = nullptr;
	}
}

void VirtualJoystickEditor::forward_canvas_draw_over_viewport(Control *p_overlay) {
	if (!node || !node->is_visible_in_tree()) {
		return;
	}

	if (!canvas_item_editor) {
		canvas_item_editor = CanvasItemEditor::get_singleton();
	}

	Transform2D xform = canvas_item_editor->get_canvas_transform() * node->get_global_transform_with_canvas();

	Vector2 center = node->get_joystick_position();
	float base_radius = node->get_joystick_size() * 0.5f;

	float clampzone_radius = base_radius * node->get_clampzone_ratio();
	float deadzone_radius = clampzone_radius * node->get_deadzone_ratio();

	Color zone_color = Color(1, 1, 0.25, 0.63);

	int width = Math::round(1.0f * EDSCALE);

	// Draw clampzone circle.
	Vector2 xformed_center = xform.xform(center);
	float scale = xform.get_scale().x;

	p_overlay->draw_arc(xformed_center, clampzone_radius * scale, 0, Math_TAU, 64, zone_color, width, true);

	// Draw deadzone circle.
	if (deadzone_radius > 0) {
		p_overlay->draw_arc(xformed_center, deadzone_radius * scale, 0, Math_TAU, 64, zone_color, width, true);
	}
}

void VirtualJoystickEditor::edit(Node *p_node) {
	if (!canvas_item_editor) {
		canvas_item_editor = CanvasItemEditor::get_singleton();
	}

	if (node) {
		node->disconnect("draw", canvas_item_editor, "update_viewport");
	}

	if (p_node) {
		node = Object::cast_to<VirtualJoystick>(p_node);
	} else {
		node = nullptr;
	}

	if (node) {
		node->connect("draw", canvas_item_editor, "update_viewport");
	}

	canvas_item_editor->update_viewport();
}

void VirtualJoystickEditor::_bind_methods() {
	ClassDB::bind_method("_node_removed", &VirtualJoystickEditor::_node_removed);
}

VirtualJoystickEditor::VirtualJoystickEditor() {
	canvas_item_editor = nullptr;
	node = nullptr;
}

///////////////////////

void VirtualJoystickEditorPlugin::edit(Object *p_object) {
	virtual_joystick_editor->edit(Object::cast_to<VirtualJoystick>(p_object));
}

bool VirtualJoystickEditorPlugin::handles(Object *p_object) const {
	return Object::cast_to<VirtualJoystick>(p_object) != nullptr;
}

void VirtualJoystickEditorPlugin::make_visible(bool p_visible) {
	if (!p_visible) {
		edit(nullptr);
	}
}

VirtualJoystickEditorPlugin::VirtualJoystickEditorPlugin(EditorNode *p_editor) {
	virtual_joystick_editor = memnew(VirtualJoystickEditor);
	p_editor->get_gui_base()->add_child(virtual_joystick_editor);
}
