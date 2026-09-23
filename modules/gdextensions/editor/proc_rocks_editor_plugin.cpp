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

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "editor/editor_settings.h"
#include "environment/proc_rocks/generators/procrockgen/procrockgen.h"
#include "scene/3d/light.h"
#include "scene/gui/viewport_container.h"
#include "scene/resources/material.h"
#include "scene/resources/world.h"

#include "proc_rocks_demo/baked_textures.h"

#include <cstring>

// =========================================================================
// Baked demo texture packs (ProcRock dock "Demo Texture" picker)
// =========================================================================

namespace {

enum ProcRockBakedTexturePack {
	PROCROCK_BAKED_GRAVEL,
	PROCROCK_BAKED_MOSSY,
	PROCROCK_BAKED_ROCK,
};

// Same relative path as procrockgen.cpp's doctest-only PRESETS_DIR (kept as a
// separate copy since that one is DOCTEST+anonymous-namespace-local, not linkable
// from here) and as load_json_dialog's default directory below.
const char *const PRESETS_DIR = "modules/gdextensions/editor/proc_rocks_demo/presets";

// Presets have no verified rock-type identity (memo.md's "Presets: real extracted
// values") — just numbered filenames (plus one hand-named one) — so this only
// orders "1..12" numerically before any non-numeric name, alphabetically.
bool _preset_name_less(const String &p_a, const String &p_b) {
	String a = p_a.get_basename(), b = p_b.get_basename();
	bool a_num = a.is_valid_integer(), b_num = b.is_valid_integer();
	if (a_num && b_num) {
		return a.to_int() < b.to_int();
	}
	if (a_num != b_num) {
		return a_num;
	}
	return a < b;
}

// Scans PRESETS_DIR for *.json files, sorted per _preset_name_less. Returns an
// empty Vector if the directory can't be opened (e.g. running from an install
// without the bundled demo presets) — callers treat that as "no presets found".
Vector<String> _scan_bundled_presets() {
	Vector<String> names;
	DirAccessRef dir = DirAccess::open(PRESETS_DIR);
	if (!dir) {
		return names;
	}
	dir->list_dir_begin();
	for (String entry = dir->get_next(); !entry.empty(); entry = dir->get_next()) {
		if (!dir->current_is_dir() && entry.get_extension().to_lower() == "json") {
			names.push_back(entry);
		}
	}
	dir->list_dir_end();

	for (int i = 1; i < names.size(); i++) {
		String key = names[i];
		int j = i - 1;
		while (j >= 0 && _preset_name_less(key, names[j])) {
			names.write[j + 1] = names[j];
			j--;
		}
		names.write[j + 1] = key;
	}
	return names;
}

Ref<Image> load_embedded_jpg(const uint8_t *p_data, unsigned int p_size) {
	PoolByteArray buf;
	buf.resize(p_size);
	{
		PoolByteArray::Write w = buf.write();
		memcpy(w.ptr(), p_data, p_size);
	}
	Ref<Image> img;
	img.instance();
	img->load_jpg_from_buffer(buf);
	return img;
}

Ref<Image> make_constant_image(int p_size, const Color &p_color) {
	Ref<Image> img;
	img.instance();
	img->create(p_size, p_size, false, Image::FORMAT_RGB8);
	img->lock();
	for (int y = 0; y < p_size; y++) {
		for (int x = 0; x < p_size; x++) {
			img->set_pixel(x, y, p_color);
		}
	}
	img->unlock();
	return img;
}

Ref<ImageTexture> to_texture(const Ref<Image> &p_image) {
	Ref<ImageTexture> tex;
	tex.instance();
	tex->create_from_image(p_image);
	return tex;
}

ProcRockPipelineTextures load_baked_textures(ProcRockBakedTexturePack p_pack) {
	ProcRockPipelineTextures textures;
	// Baked sets have no metalness map (rock isn't metallic) — a constant black texture
	// is equivalent to metallic=0 without needing a dedicated scalar path.
	Ref<ImageTexture> non_metal = to_texture(make_constant_image(8, Color(0, 0, 0)));

	switch (p_pack) {
		case PROCROCK_BAKED_GRAVEL:
			textures.albedo = to_texture(load_embedded_jpg(gravel_albedo_jpg_data, gravel_albedo_jpg_size));
			textures.normal = to_texture(load_embedded_jpg(gravel_normals_jpg_data, gravel_normals_jpg_size));
			textures.roughness = to_texture(load_embedded_jpg(gravel_roughness_jpg_data, gravel_roughness_jpg_size));
			textures.ambient_occlusion = to_texture(load_embedded_jpg(gravel_ambientOcc_jpg_data, gravel_ambientOcc_jpg_size));
			textures.metalness = non_metal;
			break;
		case PROCROCK_BAKED_MOSSY:
			textures.albedo = to_texture(load_embedded_jpg(moss_albedo_jpg_data, moss_albedo_jpg_size));
			textures.normal = to_texture(load_embedded_jpg(moss_normals_jpg_data, moss_normals_jpg_size));
			textures.roughness = to_texture(load_embedded_jpg(moss_roughness_jpg_data, moss_roughness_jpg_size));
			textures.ambient_occlusion = to_texture(load_embedded_jpg(moss_ambientOcc_jpg_data, moss_ambientOcc_jpg_size));
			textures.metalness = non_metal;
			break;
		case PROCROCK_BAKED_ROCK:
		default:
			// Only a single combined albedo texture ships for this pack.
			textures.albedo = to_texture(load_embedded_jpg(rock_jpg_data, rock_jpg_size));
			textures.normal = to_texture(make_constant_image(8, Color(0.5, 0.5, 1.0)));
			textures.roughness = to_texture(make_constant_image(8, Color(0.6, 0.6, 0.6)));
			textures.ambient_occlusion = to_texture(make_constant_image(8, Color(1, 1, 1)));
			textures.metalness = non_metal;
			break;
	}

	return textures;
}

} // namespace

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
	_apply_demo_texture();
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

void ProcRockDialog::_on_load_json_pressed() {
	load_json_dialog->popup_centered_ratio(0.5);
}

void ProcRockDialog::_on_load_json_file_selected(const String &p_path) {
	Error err = rock_mesh->load_from_file(p_path);
	if (err != OK) {
		ERR_PRINT("ProcRock: Failed to load pipeline JSON from " + p_path);
		return;
	}
	print_line("ProcRock: Loaded pipeline JSON from " + p_path);
	_apply_demo_texture(); // a JSON-driven regenerate wipes surface 0's material too
	_update_preview();
	_update_info();
}

void ProcRockDialog::_on_preset_selected(int p_idx) {
	if (p_idx <= 0 || p_idx - 1 >= preset_paths.size()) {
		return; // "None"
	}
	_on_load_json_file_selected(preset_paths[p_idx - 1]);
}

void ProcRockDialog::_on_demo_texture_changed(int p_idx) {
	demo_texture_pack = p_idx - 1; // item 0 is "None"
	_apply_demo_texture();
	_update_preview();
}

void ProcRockDialog::_apply_demo_texture() {
	if (demo_texture_pack < 0) {
		preview_mesh_instance->set_material_override(default_preview_material);
		if (rock_mesh->get_surface_count() > 0) {
			rock_mesh->surface_set_material(0, Ref<Material>());
		}
		return;
	}
	if (rock_mesh->get_surface_count() == 0) {
		return;
	}
	// Clear the override so the mesh's own (demo-textured) surface material is visible.
	preview_mesh_instance->set_material_override(Ref<Material>());
	ProcRockPipelineTextures textures = load_baked_textures((ProcRockBakedTexturePack)demo_texture_pack);
	rock_mesh->surface_set_material(0, rock_pipeline_make_material(textures));
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
	ClassDB::bind_method(D_METHOD("_on_load_json_pressed"), &ProcRockDialog::_on_load_json_pressed);
	ClassDB::bind_method(D_METHOD("_on_load_json_file_selected", "path"), &ProcRockDialog::_on_load_json_file_selected);
	ClassDB::bind_method(D_METHOD("_on_preset_selected", "idx"), &ProcRockDialog::_on_preset_selected);
	ClassDB::bind_method(D_METHOD("_on_demo_texture_changed", "idx"), &ProcRockDialog::_on_demo_texture_changed);
}

ProcRockDialog::ProcRockDialog() {
	camera_orbit_angle = 0;
	demo_texture_pack = -1;
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
	default_preview_material.instance();
	default_preview_material->set_albedo(Color(0.7, 0.65, 0.6));
	default_preview_material->set_roughness(0.8);
	preview_mesh_instance->set_material_override(default_preview_material);
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

	// Demo texture picker (editor-only baked PBR packs)
	HBoxContainer *texture_bar = memnew(HBoxContainer);

	Label *demo_texture_label = memnew(Label);
	demo_texture_label->set_text("Demo Texture:");
	texture_bar->add_child(demo_texture_label);

	demo_texture_option = memnew(OptionButton);
	demo_texture_option->add_item("None", 0);
	demo_texture_option->add_item("Gravel", 1);
	demo_texture_option->add_item("Mossy", 2);
	demo_texture_option->add_item("Rock", 3);
	demo_texture_option->set_h_size_flags(SIZE_EXPAND_FILL);
	demo_texture_option->connect("item_selected", this, "_on_demo_texture_changed");
	texture_bar->add_child(demo_texture_option);

	vbox->add_child(texture_bar);

	// Bundled pipeline preset browser (memo.md's "Full JSON round-trip UX" item) —
	// quick-select for the presets shipped with this fork; "Load Preset JSON..."
	// below still covers arbitrary external files.
	HBoxContainer *preset_bar = memnew(HBoxContainer);

	Label *preset_label = memnew(Label);
	preset_label->set_text("Bundled Preset:");
	preset_bar->add_child(preset_label);

	preset_option = memnew(OptionButton);
	preset_option->add_item("None", 0);
	preset_paths.clear();
	Vector<String> preset_names = _scan_bundled_presets();
	for (int i = 0; i < preset_names.size(); i++) {
		preset_option->add_item(preset_names[i].get_basename(), i + 1);
		preset_paths.push_back(String(PRESETS_DIR).plus_file(preset_names[i]));
	}
	preset_option->set_h_size_flags(SIZE_EXPAND_FILL);
	preset_option->connect("item_selected", this, "_on_preset_selected");
	preset_bar->add_child(preset_option);

	vbox->add_child(preset_bar);

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

	load_json_btn = memnew(Button);
	load_json_btn->set_text("Load Preset JSON...");
	load_json_btn->connect("pressed", this, "_on_load_json_pressed");
	btn_bar->add_child(load_json_btn);

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

	// Load Preset JSON dialog — browses the real OS filesystem (not res://), since these
	// pipeline JSON files (procrocklib's real preset format, see memo.md's "JSON pipeline
	// reader" section) aren't necessarily part of the edited project. Defaults to this
	// engine fork's own bundled demo presets as a starting point, if that path resolves.
	load_json_dialog = memnew(FileDialog);
	load_json_dialog->set_mode(FileDialog::MODE_OPEN_FILE);
	load_json_dialog->set_access(FileDialog::ACCESS_FILESYSTEM);
	load_json_dialog->set_title("Load ProcRock Pipeline JSON");
	load_json_dialog->add_filter("*.json ; JSON Pipeline Preset");
	load_json_dialog->set_current_dir("modules/gdextensions/editor/proc_rocks_demo/presets");
	load_json_dialog->connect("file_selected", this, "_on_load_json_file_selected");
	add_child(load_json_dialog);

	set_process(true);
}

// =========================================================================
// ProcRockExportPlugin
// =========================================================================

void ProcRockExportPlugin::_export_file(const String &p_path, const String &p_type, const Set<String> &p_features) {
	if (p_type != "ProcRockMesh") {
		return;
	}
	Ref<ProcRockMesh> mesh = ResourceLoader::load(p_path);
	if (mesh.is_null() || mesh->get_baked()) {
		return; // not ours to touch, or already frozen — nothing to do
	}
	Error err = mesh->bake();
	if (err != OK) {
		return; // e.g. nothing generated yet — export the original untouched
	}

	String tmp_path = EditorSettings::get_singleton()->get_cache_dir().plus_file("procrock_bake_tmp.res");
	ResourceSaver::save(tmp_path, mesh);
	Vector<uint8_t> data = FileAccess::get_file_as_array(tmp_path);
	DirAccess::remove_file_or_error(tmp_path);
	if (data.size() > 0) {
		add_file(p_path + ".baked.res", data, true); // remap=true swaps every reference transparently
	}
}

// =========================================================================
// ProcRockEditorPlugin
// =========================================================================

void ProcRockEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_open_dialog"), &ProcRockEditorPlugin::_open_dialog);
}

void ProcRockEditorPlugin::_open_dialog(Variant p_ud) {
	dialog->popup_centered(Size2(550, 450));
	// Generate initial rock on first open
	dialog->generate();
}

ProcRockEditorPlugin::ProcRockEditorPlugin(EditorNode *p_node) {
	dialog = memnew(ProcRockDialog);
	p_node->get_gui_base()->add_child(dialog);

	add_tool_menu_item("Procedural Rock Generator...", this, "_open_dialog");

	export_plugin.instance();
	add_export_plugin(export_plugin);
}

ProcRockEditorPlugin::~ProcRockEditorPlugin() {
	remove_tool_menu_item("Procedural Rock Generator...");
	remove_export_plugin(export_plugin);
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

TEST_SUITE("[[proc_rocks]] Baked demo textures") {
	TEST_CASE("[proc_rocks] load_baked_textures returns valid editor-only PBR sets") {
		ProcRockPipelineTextures gravel = load_baked_textures(PROCROCK_BAKED_GRAVEL);
		CHECK(gravel.albedo.is_valid());
		CHECK(gravel.normal.is_valid());
		CHECK(gravel.roughness.is_valid());
		CHECK(gravel.ambient_occlusion.is_valid());
		CHECK(gravel.metalness.is_valid());
		CHECK(gravel.albedo->get_width() == 512);
		CHECK(gravel.albedo->get_height() == 512);
		// These two are single-channel (grayscale) source JPEGs — the ones that actually
		// failed to decode (jpgd chokes on their unusual 2x2 luma sampling factor) until
		// the embedded assets were re-encoded with standard 1x1 sampling.
		CHECK(gravel.roughness->get_width() == 512);
		CHECK(gravel.roughness->get_height() == 512);
		CHECK(gravel.ambient_occlusion->get_width() == 512);
		Ref<Image> gravel_roughness_img = gravel.roughness->get_data();
		CHECK(gravel_roughness_img.is_valid());
		CHECK(gravel_roughness_img->get_width() == 512);

		ProcRockPipelineTextures mossy = load_baked_textures(PROCROCK_BAKED_MOSSY);
		CHECK(mossy.albedo.is_valid());
		CHECK(mossy.albedo->get_width() == 512);
		CHECK(mossy.roughness->get_width() == 512);
		CHECK(mossy.ambient_occlusion->get_width() == 512);

		ProcRockPipelineTextures rock = load_baked_textures(PROCROCK_BAKED_ROCK);
		CHECK(rock.albedo.is_valid());
		CHECK(rock.albedo->get_width() == 512);
		CHECK(rock.normal.is_valid()); // synthesized flat-up normal, since only albedo ships for this pack

		Ref<SpatialMaterial> material = rock_pipeline_make_material(gravel);
		CHECK(material.is_valid());
		CHECK(material->get_texture(SpatialMaterial::TEXTURE_ALBEDO).is_valid());
	}

	TEST_CASE("[proc_rocks] _scan_bundled_presets finds and numerically sorts the real preset files") {
		Vector<String> names = _scan_bundled_presets();
		REQUIRE(names.size() == 13);
		CHECK(names[0] == "1.json");
		CHECK(names[1] == "2.json");
		CHECK(names[9] == "10.json");
		CHECK(names[10] == "11.json");
		CHECK(names[11] == "12.json");
		CHECK(names[12] == "granite_custom.json"); // only non-numeric name, sorts last
	}
}

#endif // DOCTEST

#endif // TOOLS_ENABLED
