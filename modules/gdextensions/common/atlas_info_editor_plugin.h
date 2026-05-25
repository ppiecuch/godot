/**************************************************************************/
/*  atlas_info_editor_plugin.h                                            */
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

#ifndef ATLAS_INFO_EDITOR_PLUGIN_H
#define ATLAS_INFO_EDITOR_PLUGIN_H

#ifdef TOOLS_ENABLED

#include "atlas_info.h"
#include "editor/editor_inspector.h"
#include "editor/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/check_button.h"
#include "scene/gui/option_button.h"

class AtlasInfoPreview : public VBoxContainer {
	GDCLASS(AtlasInfoPreview, VBoxContainer);

	Ref<AtlasInfo> atlas_info;

	OptionButton *item_combo;
	CheckButton *show_hull_outline;
	CheckButton *show_hull_triangulation;
	Control *preview_canvas;
	Ref<Texture> override_texture;

	int selected_item;

	void _on_item_selected(int p_index);
	void _on_hull_toggled(bool p_pressed);
	void _on_tri_toggled(bool p_pressed);
	void _draw_preview();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void edit(const Ref<AtlasInfo> &p_info);
	AtlasInfoPreview();
};

class EditorInspectorPluginAtlasInfo : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginAtlasInfo, EditorInspectorPlugin);

public:
	bool can_handle(Object *p_object) override;
	void parse_begin(Object *p_object) override;
};

class AtlasInfoEditorPlugin : public EditorPlugin {
	GDCLASS(AtlasInfoEditorPlugin, EditorPlugin);

	Ref<EditorInspectorPluginAtlasInfo> inspector_plugin;

public:
	AtlasInfoEditorPlugin(EditorNode *p_node);
};

#endif // TOOLS_ENABLED
#endif // ATLAS_INFO_EDITOR_PLUGIN_H
