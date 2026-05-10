/**************************************************************************/
/*  goost_editor_plugin.cpp                                               */
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

#include "goost_editor_plugin.h"

#include "scene/2d/debug_2d.h"

#include "classes_enabled.gen.h"

void GoostEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_editor_scene_changed"), &GoostEditorPlugin::_on_editor_scene_changed);
}

void GoostEditorPlugin::_on_editor_scene_changed(Node *p_scene_root) {
#if defined(GOOST_SCENE_ENABLED) && defined(GOOST_GEOMETRY_ENABLED) && defined(GOOST_Debug2D)
	if (Debug2D::get_singleton()) {
		Debug2D::get_singleton()->clear();
	}
#endif
}

GoostEditorPlugin::GoostEditorPlugin(EditorNode *p_editor) {
	// Signals are added during `EditorPlugin::_bind_methods()`, so calling deferred.
	call_deferred("connect", "scene_changed", this, "_on_editor_scene_changed");
}
