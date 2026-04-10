/**************************************************************************/
/*  proc_rocks_editor_plugin.cpp                                          */
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

#ifdef TOOLS_ENABLED

#include "proc_rocks_editor_plugin.h"

#include "core/io/resource_saver.h"
#include "scene/3d/light.h"
#include "scene/gui/viewport_container.h"
#include "scene/resources/material.h"
#include "scene/resources/world.h"

// =========================================================================
// ProcRockDialog
// =========================================================================

void ProcRockDialog::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			if (!is_visible_in_tree() || !preview_camera || !preview_camera->is_inside_tree()) {
				return;
			}
			camera_orbit_angle += get_process_delta_time() * 0.3;
			real_t dist = 3.0;
			preview_camera->set_translation(Vector3(
					Math::sin(camera_orbit_angle) * dist,
					1.5,
					Math::cos(camera_orbit_angle) * dist));
			preview_camera->look_at(Vector3(0, 0, 0), Vector3(0, 1, 0));
		} break;
	}
}

void ProcRockDialog::_on_generator_changed(int p_idx) {
	rock_mesh->set_generator(p_idx);
	generate();
}

void ProcRockDialog::generate() {
	rock_mesh->set_auto_refresh(false);

	int seed = (int)seed_spin->get_value();
	switch (rock_mesh->get_generator()) {
		case 0:
			rock_mesh->set_rockgen_randseed(seed);
			break;
		case 2:
			rock_mesh->set_rockstudio_randseed(seed);
			break;
	}

	rock_mesh->set_auto_refresh(true);
	_update_preview();
	_update_info();
}

void ProcRockDialog::_on_randomize_pressed() {
	seed_spin->set_value(Math::rand() % 99999);
	generate();
}

void ProcRockDialog::_on_export_pressed() {
	export_dialog->popup_centered_ratio(0.5);
}

void ProcRockDialog::_on_export_file_selected(const String &p_path) {
	Error err = ResourceSaver::save(p_path, rock_mesh);
	if (err == OK) {
		print_line("ProcRock: Exported mesh to " + p_path);
	} else {
		ERR_PRINT("ProcRock: Failed to export mesh to " + p_path);
	}
}

void ProcRockDialog::_update_preview() {
	preview_mesh_instance->set_mesh(rock_mesh);
}

void ProcRockDialog::_update_info() {
	int surface_count = rock_mesh->get_surface_count();
	int vertex_count = 0;
	for (int i = 0; i < surface_count; i++) {
		vertex_count += rock_mesh->surface_get_array_len(i);
	}
	static const char *gen_names[] = { "RockGen", "IcoRock", "RockStudio", "ProcRock" };
	info_label->set_text(vformat("%s | %d vertices | %d surfaces",
			gen_names[rock_mesh->get_generator()],
			vertex_count, surface_count));
}

void ProcRockDialog::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_generator_changed", "idx"), &ProcRockDialog::_on_generator_changed);
	ClassDB::bind_method(D_METHOD("generate"), &ProcRockDialog::generate);
	ClassDB::bind_method(D_METHOD("_on_randomize_pressed"), &ProcRockDialog::_on_randomize_pressed);
	ClassDB::bind_method(D_METHOD("_on_export_pressed"), &ProcRockDialog::_on_export_pressed);
	ClassDB::bind_method(D_METHOD("_on_export_file_selected", "path"), &ProcRockDialog::_on_export_file_selected);
}

ProcRockDialog::ProcRockDialog() {
	camera_orbit_angle = 0;
	rock_mesh.instance();

	set_title("Procedural Rock Generator");
	set_resizable(true);

	VBoxContainer *vbox = memnew(VBoxContainer);
	vbox->set_anchors_and_margins_preset(PRESET_WIDE, PRESET_MODE_MINSIZE, 8);
	add_child(vbox);

	// --- 3D Preview viewport ---
	preview_viewport = memnew(Viewport);
	preview_viewport->set_size(Vector2(500, 300));
	preview_viewport->set_transparent_background(false);
	preview_viewport->set_update_mode(Viewport::UPDATE_ALWAYS);
	preview_viewport->set_msaa(Viewport::MSAA_4X);

	Ref<World> world;
	world.instance();
	preview_viewport->set_world(world);

	preview_camera = memnew(Camera);
	preview_camera->set_translation(Vector3(0, 1.5, 3));
	preview_camera->set_perspective(45, 0.1, 100);
	preview_viewport->add_child(preview_camera);
	preview_camera->look_at(Vector3(0, 0, 0), Vector3(0, 1, 0));
	preview_camera->set_current(true);

	DirectionalLight *light = memnew(DirectionalLight);
	light->set_rotation_degrees(Vector3(-45, 30, 0));
	preview_viewport->add_child(light);

	preview_mesh_instance = memnew(MeshInstance);
	Ref<SpatialMaterial> mat;
	mat.instance();
	mat->set_albedo(Color(0.7, 0.65, 0.6));
	mat->set_roughness(0.8);
	preview_mesh_instance->set_material_override(mat);
	preview_viewport->add_child(preview_mesh_instance);

	ViewportContainer *viewport_container = memnew(ViewportContainer);
	viewport_container->set_stretch(true);
	viewport_container->set_custom_minimum_size(Size2(500, 300));
	viewport_container->set_v_size_flags(SIZE_EXPAND_FILL);
	viewport_container->add_child(preview_viewport);
	vbox->add_child(viewport_container);

	// --- Controls ---
	HBoxContainer *top_bar = memnew(HBoxContainer);

	Label *gen_label = memnew(Label);
	gen_label->set_text("Generator:");
	top_bar->add_child(gen_label);

	generator_option = memnew(OptionButton);
	generator_option->add_item("RockGen", 0);
	generator_option->add_item("IcoRock", 1);
	generator_option->add_item("RockStudio", 2);
	generator_option->add_item("ProcRock", 3);
	generator_option->set_h_size_flags(SIZE_EXPAND_FILL);
	generator_option->connect("item_selected", this, "_on_generator_changed");
	top_bar->add_child(generator_option);

	vbox->add_child(top_bar);

	// Seed + buttons
	HBoxContainer *action_bar = memnew(HBoxContainer);

	Label *seed_label = memnew(Label);
	seed_label->set_text("Seed:");
	action_bar->add_child(seed_label);

	seed_spin = memnew(SpinBox);
	seed_spin->set_min(0);
	seed_spin->set_max(99999);
	seed_spin->set_step(1);
	seed_spin->set_value(42);
	seed_spin->set_h_size_flags(SIZE_EXPAND_FILL);
	action_bar->add_child(seed_spin);

	randomize_btn = memnew(Button);
	randomize_btn->set_text("Randomize");
	randomize_btn->connect("pressed", this, "_on_randomize_pressed");
	action_bar->add_child(randomize_btn);

	vbox->add_child(action_bar);

	// Generate + Export
	HBoxContainer *btn_bar = memnew(HBoxContainer);

	generate_btn = memnew(Button);
	generate_btn->set_text("Generate");
	generate_btn->set_h_size_flags(SIZE_EXPAND_FILL);
	generate_btn->connect("pressed", this, "generate");
	btn_bar->add_child(generate_btn);

	export_btn = memnew(Button);
	export_btn->set_text("Export Mesh...");
	export_btn->connect("pressed", this, "_on_export_pressed");
	btn_bar->add_child(export_btn);

	vbox->add_child(btn_bar);

	// Info
	info_label = memnew(Label);
	info_label->set_text("Ready");
	info_label->set_align(Label::ALIGN_CENTER);
	vbox->add_child(info_label);

	// Export dialog
	export_dialog = memnew(FileDialog);
	export_dialog->set_mode(FileDialog::MODE_SAVE_FILE);
	export_dialog->set_title("Export ProcRock Mesh");
	export_dialog->add_filter("*.tres ; Godot Resource");
	export_dialog->add_filter("*.res ; Binary Resource");
	export_dialog->connect("file_selected", this, "_on_export_file_selected");
	add_child(export_dialog);

	set_process(true);
}

// =========================================================================
// ProcRockEditorPlugin
// =========================================================================

void ProcRockEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_open_dialog"), &ProcRockEditorPlugin::_open_dialog);
}

void ProcRockEditorPlugin::_open_dialog() {
	dialog->popup_centered(Size2(550, 450));
	// Generate initial rock on first open
	dialog->generate();
}

ProcRockEditorPlugin::ProcRockEditorPlugin(EditorNode *p_node) {
	dialog = memnew(ProcRockDialog);
	p_node->get_gui_base()->add_child(dialog);

	add_tool_menu_item("Procedural Rock Generator...", this, "_open_dialog");
}

ProcRockEditorPlugin::~ProcRockEditorPlugin() {
	remove_tool_menu_item("Procedural Rock Generator...");
}

#endif // TOOLS_ENABLED
