/**************************************************************************/
/*  proc_rocks_editor_plugin.h                                            */
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

#ifndef PROC_ROCKS_EDITOR_PLUGIN_H
#define PROC_ROCKS_EDITOR_PLUGIN_H

#ifdef TOOLS_ENABLED

#include "editor/editor_export.h"
#include "editor/editor_inspector.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "environment/proc_rocks/proc_rocks.h"
#include "scene/3d/camera.h"
#include "scene/3d/mesh_instance.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/label.h"
#include "scene/gui/option_button.h"
#include "scene/gui/spin_box.h"
#include "scene/main/viewport.h"
#include "scene/resources/material.h"

class ProcRockDialog : public WindowDialog {
	GDCLASS(ProcRockDialog, WindowDialog);

	Ref<ProcRockMesh> rock_mesh;

	// 3D preview
	Viewport *preview_viewport;
	MeshInstance *preview_mesh_instance;
	MeshInstance *grid_instance;
	MeshInstance *floor_instance;
	Camera *preview_camera;
	Ref<SpatialMaterial> default_preview_material;
	real_t camera_orbit_angle;

	// Controls
	OptionButton *generator_option;
	SpinBox *seed_spin;
	Button *generate_btn;
	Button *randomize_btn;
	Button *export_btn;
	Button *load_json_btn;
	Label *info_label;
	FileDialog *export_dialog;
	FileDialog *load_json_dialog;

	// Bundled pipeline preset browser (memo.md's "Full JSON round-trip UX" item) —
	// lists every *.json found in proc_rocks_demo/presets/ at dock-construction time,
	// selecting one immediately loads+previews it via the same path as the
	// "Load Preset JSON..." FileDialog. Presets have no verified rock-type identity
	// (see memo.md's "Presets: real extracted values"), so entries are just filenames.
	OptionButton *preset_option;
	Vector<String> preset_paths; // preset_option item (idx-1) -> full path; index 0 is "None"

	// Live per-generator property panel (memo.md's "per-generator parameter sliders"
	// item) — a real EditorInspector edit()ing rock_mesh directly, so every dynamic
	// property ProcRockMesh::_get_property_list() exposes for the current generator
	// shows up automatically (sliders, color pickers, enum dropdowns, dependent-field
	// show/hide), without duplicating that logic in hand-rolled dock controls.
	EditorInspector *properties_inspector;

	void _on_generator_changed(int p_idx);
	void _on_randomize_pressed();
	void _on_export_pressed();
	void _on_export_file_selected(const String &p_path);
	void _on_load_json_pressed();
	void _on_load_json_file_selected(const String &p_path);
	void _on_preset_selected(int p_idx);
	void _on_property_edited(const StringName &p_prop);
	void _update_preview_material();
	void _update_preview();
	void _update_grid_position();
	void _update_info();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void generate();
	ProcRockDialog();
};

// Compiles every ProcRockMesh resource referenced by the project into a static,
// baked mesh at export time (see proc_rocks.h's set_baked()/bake() and memo.md's
// "Baking" section) — the exported game never needs the (tools=no-excluded)
// generation code. Modeled directly on the engine's own
// EditorExportTextSceneToBinaryPlugin (editor/editor_export.cpp), which uses the
// exact same _export_file()+add_file(remap=true) pattern for a different purpose.
class ProcRockExportPlugin : public EditorExportPlugin {
	GDCLASS(ProcRockExportPlugin, EditorExportPlugin);

public:
	virtual void _export_file(const String &p_path, const String &p_type, const Set<String> &p_features);
};

class ProcRockEditorPlugin : public EditorPlugin {
	GDCLASS(ProcRockEditorPlugin, EditorPlugin);

	ProcRockDialog *dialog;
	Ref<ProcRockExportPlugin> export_plugin;

	// add_tool_menu_item()'s dispatcher (EditorNode::_tool_menu_option) always calls
	// the handler with exactly 1 argument (the p_ud passed to add_tool_menu_item,
	// Variant() here) — this must take a parameter even though it's unused, or the
	// call fails with "Method expected 0 arguments, but called with 1" and the menu
	// item silently does nothing.
	void _open_dialog(Variant p_ud);

protected:
	static void _bind_methods();

public:
	virtual String get_name() const { return "ProcRock"; }

	ProcRockEditorPlugin(EditorNode *p_node);
	~ProcRockEditorPlugin();
};

#endif // TOOLS_ENABLED

#endif // PROC_ROCKS_EDITOR_PLUGIN_H
