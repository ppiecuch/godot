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

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "proc_rocks.h"
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

class ProcRockDialog : public WindowDialog {
	GDCLASS(ProcRockDialog, WindowDialog);

	Ref<ProcRockMesh> rock_mesh;

	// 3D preview
	Viewport *preview_viewport;
	MeshInstance *preview_mesh_instance;
	Camera *preview_camera;
	real_t camera_orbit_angle;

	// Controls
	OptionButton *generator_option;
	SpinBox *seed_spin;
	Button *generate_btn;
	Button *randomize_btn;
	Button *export_btn;
	Label *info_label;
	FileDialog *export_dialog;

	void _on_generator_changed(int p_idx);
	void _on_randomize_pressed();
	void _on_export_pressed();
	void _on_export_file_selected(const String &p_path);
	void _update_preview();
	void _update_info();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void generate();
	ProcRockDialog();
};

class ProcRockEditorPlugin : public EditorPlugin {
	GDCLASS(ProcRockEditorPlugin, EditorPlugin);

	ProcRockDialog *dialog;

	void _open_dialog();

protected:
	static void _bind_methods();

public:
	virtual String get_name() const { return "ProcRock"; }

	ProcRockEditorPlugin(EditorNode *p_node);
	~ProcRockEditorPlugin();
};

#endif // TOOLS_ENABLED

#endif // PROC_ROCKS_EDITOR_PLUGIN_H
