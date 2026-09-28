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

#include "core/image.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "editor/editor_settings.h"
#include "environment/proc_rocks/generators/procrockgen/procrockgen.h"
#include "proc_rocks_baked_textures.h"
#include "scene/3d/light.h"
#include "scene/gui/viewport_container.h"
#include "scene/resources/environment.h"
#include "scene/resources/material.h"
#include "scene/resources/primitive_meshes.h"
#include "scene/resources/surface_tool.h"
#include "scene/resources/world.h"

namespace {

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
			// Orbit around the mesh's own AABB center rather than a hardcoded world
			// origin -- flatten_base_offset (and RockCluster's own always-at-y=0
			// clip+cap) can shift the visible geometry well off-center vertically, and
			// orbiting a fixed point above/below the actual mesh made the floor grid
			// barely visible in frame at most angles ("bottom of the rock seems...
			// perpendicular to grid floor" -- really just poor framing, not a
			// geometry bug: see proc_rocks memo.md item 32).
			//
			// The flatten_base cap being visible at all from above (at any elevation) WAS a
			// real defect -- clip_and_cap()'s cap-fan winding, fixed separately (see
			// generators/shared/plane_flatten.cpp and memo.md's "Bugs Fixed"). This steeper
			// elevation (was ~27°, i.e. +1.5) is a separate, additional UX improvement on
			// top of that fix, per the user's own suggestion ("I should look at the rock
			// from above") -- a properly grounded rock's underside shouldn't be a normal
			// part of the view anyway, cap-winding bug or not.
			AABB aabb = preview_mesh_instance->get_aabb();
			Vector3 center = aabb.has_no_area() ? Vector3() : aabb.position + aabb.size * 0.5;
			preview_camera->set_translation(Vector3(
					center.x + Math::sin(camera_orbit_angle) * dist,
					center.y + 3.5,
					center.z + Math::cos(camera_orbit_angle) * dist));
			preview_camera->look_at(center, Vector3(0, 1, 0));
		} break;
		case NOTIFICATION_POPUP_HIDE: {
			// Persist across editor sessions -- same "dialog_bounds" project-metadata
			// mechanism the engine's own CreateDialog/EditorHelpSearch use for exactly
			// this. Restored in ProcRockEditorPlugin::_open_dialog().
			EditorSettings::get_singleton()->set_project_metadata("dialog_bounds", "proc_rock", get_rect());
		} break;
	}
}

void ProcRockDialog::_on_generator_changed(int p_idx) {
	rock_mesh->set_generator(p_idx);
	generate();
}

void ProcRockDialog::generate() {
	if (!properties_inspector->get_edited_object()) {
		properties_inspector->edit(rock_mesh.ptr());
	}

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
	_update_preview_material();
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
	_update_preview_material(); // a JSON-driven regenerate wipes surface 0's material too
	_update_preview();
	_update_info();
}

void ProcRockDialog::_on_preset_selected(int p_idx) {
	if (p_idx <= 0 || p_idx - 1 >= preset_paths.size()) {
		return; // "None"
	}
	_on_load_json_file_selected(preset_paths[p_idx - 1]);
}

// ProcRockMesh's own texture_source property (see proc_rocks.h, applies to every
// generator, Gravel/Mossy/Rock included — applied entirely in memory) drives real
// per-surface materials, so this dock needs no demo-texture-specific handling here:
// when texture_source is None (or nothing generated yet), fall back to a plain
// cosmetic preview material rather than showing Godot's flat engine default, matching
// this dock's original look.
void ProcRockDialog::_update_preview_material() {
	if (rock_mesh->get_surface_count() > 0 && rock_mesh->surface_get_material(0).is_valid()) {
		preview_mesh_instance->set_material_override(Ref<Material>());
	} else {
		preview_mesh_instance->set_material_override(default_preview_material);
	}
}

void ProcRockDialog::_update_preview() {
	preview_mesh_instance->set_mesh(rock_mesh);
	_update_grid_position();
}

// Keep the floor grid just below the mesh's own lowest point rather than fixed at a
// hardcoded height -- see the constructor's comment above the grid's construction for
// why a fixed y=0 visibly intersected methods 0-3's default geometry. Called both after
// a full regenerate (_update_preview()) and after a direct property edit (e.g.
// flatten_base_offset itself), since the latter mutates rock_mesh in place without
// going through _update_preview().
void ProcRockDialog::_update_grid_position() {
	AABB aabb = preview_mesh_instance->get_aabb();
	real_t grid_y = aabb.has_no_area() ? 0.0 : aabb.position.y - 0.05;
	Transform t = grid_instance->get_transform();
	t.origin.y = grid_y;
	grid_instance->set_transform(t);
	Transform ft = floor_instance->get_transform();
	ft.origin.y = grid_y;
	floor_instance->set_transform(ft);
}

void ProcRockDialog::_on_property_edited(const StringName &p_prop) {
	// The mesh geometry/material itself updates in place — preview_mesh_instance
	// holds the same rock_mesh Ref, and _rebuild() (triggered by the inspector's own
	// _set() call, via ProcRockMesh's existing auto_refresh handling) mutates it
	// directly. Only the preview's material_override (see _update_preview_material()),
	// the floor grid's position, and the vertex/surface count label can go stale after
	// a direct property edit.
	_update_preview_material();
	_update_grid_position();
	_update_info();
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
	ClassDB::bind_method(D_METHOD("_on_property_edited", "prop"), &ProcRockDialog::_on_property_edited);
}

ProcRockDialog::ProcRockDialog() {
	camera_orbit_angle = 0;
	rock_mesh.instance();

	set_title("Procedural Rock Generator");
	set_resizable(true);

	VBoxContainer *vbox = memnew(VBoxContainer);
	vbox->set_anchors_and_margins_preset(PRESET_WIDE, PRESET_MODE_MINSIZE, 8);
	add_child(vbox);

	HBoxContainer *main_hbox = memnew(HBoxContainer);
	main_hbox->set_v_size_flags(SIZE_EXPAND_FILL);
	vbox->add_child(main_hbox);

	VBoxContainer *left_col = memnew(VBoxContainer);
	left_col->set_h_size_flags(SIZE_EXPAND_FILL);
	main_hbox->add_child(left_col);

	// --- 3D Preview viewport ---
	preview_viewport = memnew(Viewport);
	preview_viewport->set_size(Vector2(500, 300));
	preview_viewport->set_transparent_background(false);
	preview_viewport->set_update_mode(Viewport::UPDATE_ALWAYS);
	preview_viewport->set_msaa(Viewport::MSAA_4X);

	Ref<World> world;
	world.instance();
	// A bare World has no Environment, so this Viewport had zero ambient light — any
	// surface not directly facing the single DirectionalLight rendered pure black,
	// reading as an inverted-normals bug even though the generated geometry's winding
	// is independently verified correct. Ambient here is deliberately fairly high
	// (0.4) — a camera-aligned key light (tried and reverted, see below) proved that
	// *removing* the angled shadowing removes the shape-reading contrast entirely, so
	// the balance that actually works is: keep the key light off-axis for real facet
	// contrast, and raise the ambient floor so its shadow side doesn't read as a big
	// dark hole, rather than flattening the light itself.
	Ref<Environment> env;
	env.instance();
	env->set_background(Environment::BG_COLOR);
	env->set_bg_color(Color(0.2, 0.2, 0.24));
	env->set_ambient_light_color(Color(1, 1, 1));
	env->set_ambient_light_energy(0.4);
	world->set_environment(env);
	preview_viewport->set_world(world);

	preview_camera = memnew(Camera);
	preview_camera->set_translation(Vector3(0, 1.5, 3));
	preview_camera->set_perspective(45, 0.1, 100);
	preview_viewport->add_child(preview_camera);
	preview_camera->look_at(Vector3(0, 0, 0), Vector3(0, 1, 0));
	preview_camera->set_current(true);

	// Both lights are children of the VIEWPORT (world space), not the camera. This used
	// to be the other way around (a camera-attached "headlamp" rig) specifically to avoid
	// the rock's lit/unlit regions appearing to rotate with the rock as the view orbits.
	// That reasoning predates the floor grid (see below): with a camera-attached light,
	// the rock's shading pattern never changes as the camera orbits around it (always lit
	// from the same relative angle, like a flashlight strapped to the viewer's own head),
	// while the grid -- correctly fixed in world space -- visibly pans/turns as the camera
	// moves. That mismatch (object's shading looks static; the floor visibly rotates) is
	// exactly what one user report described as "I have even feeling that they rotate in
	// different sides", plus a bright, contrast-washed near-camera patch that looked like
	// a flat, wrongly-oriented cap ("flat area in front of me... perpendicular to floor")
	// -- the headlamp always lighting roughly the same camera-facing region into an
	// overexposed patch, regardless of true orbit angle. Switched to world-space so the
	// rock's own shading changes naturally as the camera orbits, agreeing with the grid's
	// motion instead of contradicting it — see memo.md's "Bugs Fixed" for the investigation.
	//
	// Key light: off-axis (not aimed straight down the camera's forward direction) for
	// real facet contrast — a camera-aligned key was tried and rejected early on (see
	// memo.md) since it collapses the tonal range into one flat, textureless gray (the
	// textbook "on-camera flashlight" problem: no shadow means no perceptible shape).
	// Being world-space now (not just off-axis) means the shadow side genuinely rotates
	// into view as the camera orbits, rather than staying fixed relative to the viewer.
	// Casts a real shadow (see the solid floor quad below) -- the single most direct,
	// unambiguous cue that an object rests ON a surface, which no amount of camera
	// framing or grid-position math can substitute for.
	DirectionalLight *light = memnew(DirectionalLight);
	light->set_rotation_degrees(Vector3(-35, 30, 0));
	light->set_param(Light::PARAM_ENERGY, 1.2);
	light->set_shadow(true);
	preview_viewport->add_child(light);

	// Fill light: dimmer, cooler, offset to a different angle than the key — fills in
	// some of the key light's shadow side so it isn't uniformly flat, without directly
	// opposing the key (which would cancel out the contrast this whole setup is for).
	DirectionalLight *fill_light = memnew(DirectionalLight);
	fill_light->set_rotation_degrees(Vector3(25, -40, 0));
	fill_light->set_color(Color(0.8, 0.85, 1.0));
	fill_light->set_param(Light::PARAM_ENERGY, 0.5);
	preview_viewport->add_child(fill_light);

	preview_mesh_instance = memnew(MeshInstance);
	default_preview_material.instance();
	default_preview_material->set_albedo(Color(0.7, 0.65, 0.6));
	default_preview_material->set_roughness(0.8);
	default_preview_material->set_cull_mode(SpatialMaterial::CULL_BACK); // explicit, not relying on the class default
	preview_viewport->add_child(preview_mesh_instance);
	// _update_preview_material() (called from generate(), including the initial
	// generate() in ProcRockEditorPlugin::_open_dialog()) applies default_preview_material
	// as a fallback only when texture_source hasn't produced a real surface material.

	// Subtle ground-plane grid. Added because the preview's slow automatic orbit (see
	// NOTIFICATION_PROCESS below) gives no other fixed reference: without a horizon,
	// an asymmetric rock viewed from a middling angle gives no visual cue for which
	// side is "up," so a viewer can't always tell top from bottom at a glance. Kept
	// intentionally faint (unshaded, close in value to the dock's own background
	// color) so it reads as a spatial reference without competing with the rock
	// itself for attention.
	//
	// Y position is NOT fixed at 0 -- it's repositioned in _update_preview() to sit
	// just below the current mesh's own AABB every time the mesh regenerates.
	// A fixed y=0 plane looked like it was "inside" the rock (bug report: "I see
	// grid inside the rock"), because it is: methods 0-3's `flatten_base_offset`
	// defaults to -0.3 and their unflattened geometry already spans roughly y in
	// [-1, 1], so y=0 sits well inside the solid volume, not below it. RockCluster
	// (method 4) cells are clipped+capped at y=0 specifically, so for that one
	// generator y=0 does coincide with the true base -- but relying on that only for
	// method 4 while every other method silently intersects would be worse than a
	// single generic rule that always sits below whatever mesh is actually loaded.
	{
		SurfaceTool st;
		st.begin(Mesh::PRIMITIVE_LINES);
		const real_t extent = 3.0;
		const real_t step = 0.5;
		int line_count = (int)Math::round(extent / step);
		for (int i = -line_count; i <= line_count; i++) {
			real_t coord = i * step;
			st.add_vertex(Vector3(coord, 0, -extent));
			st.add_vertex(Vector3(coord, 0, extent));
			st.add_vertex(Vector3(-extent, 0, coord));
			st.add_vertex(Vector3(extent, 0, coord));
		}
		Ref<ArrayMesh> grid_mesh = st.commit();

		grid_instance = memnew(MeshInstance);
		grid_instance->set_mesh(grid_mesh);

		Ref<SpatialMaterial> grid_material;
		grid_material.instance();
		grid_material->set_flag(SpatialMaterial::FLAG_UNSHADED, true);
		// Deliberately close in value to the viewport's own background (0.2,0.2,0.24,
		// set above) -- opaque rather than alpha-blended, so it's always faint without
		// depending on draw order against the rock mesh.
		grid_material->set_albedo(Color(0.32, 0.32, 0.37));
		grid_instance->set_material_override(grid_material);
		preview_viewport->add_child(grid_instance);

		// Solid, shadow-receiving floor beneath the grid lines. The grid alone gives an
		// orientation reference, but a wireframe can't show the one cue that actually
		// reads as "this object rests on this surface": a cast shadow. FLAG_UNSHADED
		// (used for the grid lines above, deliberately, to keep them a constant faint
		// value regardless of lighting) also means a material can never receive a
		// shadow at all -- this quad uses a normal shaded, matte material instead, kept
		// close to the background color so it's still unobtrusive when unshadowed.
		Ref<PlaneMesh> floor_mesh;
		floor_mesh.instance();
		floor_mesh->set_size(Size2(extent * 2, extent * 2));
		floor_instance = memnew(MeshInstance);
		floor_instance->set_mesh(floor_mesh);
		Ref<SpatialMaterial> floor_material;
		floor_material.instance();
		floor_material->set_albedo(Color(0.22, 0.22, 0.26));
		floor_material->set_roughness(1.0);
		floor_material->set_metallic(0.0);
		floor_instance->set_material_override(floor_material);
		preview_viewport->add_child(floor_instance);
	}

	ViewportContainer *viewport_container = memnew(ViewportContainer);
	viewport_container->set_stretch(true);
	viewport_container->set_custom_minimum_size(Size2(500, 300));
	viewport_container->set_v_size_flags(SIZE_EXPAND_FILL);
	viewport_container->add_child(preview_viewport);
	left_col->add_child(viewport_container);

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
	generator_option->add_item("RockCluster", 4);
	generator_option->set_h_size_flags(SIZE_EXPAND_FILL);
	generator_option->connect("item_selected", this, "_on_generator_changed");
	top_bar->add_child(generator_option);

	left_col->add_child(top_bar);

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

	left_col->add_child(action_bar);

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

	left_col->add_child(preset_bar);

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

	left_col->add_child(btn_bar);

	// Info
	info_label = memnew(Label);
	info_label->set_text("Ready");
	info_label->set_align(Label::ALIGN_CENTER);
	left_col->add_child(info_label);

	// --- Live per-generator property panel ---
	VBoxContainer *right_col = memnew(VBoxContainer);
	right_col->set_h_size_flags(SIZE_EXPAND_FILL);
	main_hbox->add_child(right_col);

	Label *properties_label = memnew(Label);
	properties_label->set_text("Properties:");
	right_col->add_child(properties_label);

	properties_inspector = memnew(EditorInspector);
	properties_inspector->set_h_size_flags(SIZE_EXPAND_FILL);
	properties_inspector->set_v_size_flags(SIZE_EXPAND_FILL);
	properties_inspector->connect("property_edited", this, "_on_property_edited");
	right_col->add_child(properties_inspector);
	// edit() is deferred to generate() (first real call happens from
	// ProcRockEditorPlugin::_open_dialog(), never at construction time) rather than
	// called here: this class, like any GDCLASS-registered type, also gets
	// instantiated by Godot's own doc-generation pass (DocData::generate(), called
	// from EditorNode::EditorNode() itself) purely to read default property values.
	// EditorInspector::edit()'s update_tree() reaches into EditorNode::get_singleton()
	// for some property hints, which crashes when called from inside that
	// still-under-construction, throwaway instance.

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
	// Restore the size/position saved on last close (see ProcRockDialog::_notification()'s
	// NOTIFICATION_POPUP_HIDE), falling back to the original fixed default the first time
	// the dock is ever opened in a given project.
	Rect2 saved_bounds = EditorSettings::get_singleton()->get_project_metadata("dialog_bounds", "proc_rock", Rect2());
	if (saved_bounds != Rect2()) {
		dialog->popup(saved_bounds);
	} else {
		dialog->popup_centered(Size2(950, 600));
	}
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
		CHECK(gravel.albedo->get_width() == 256);
		CHECK(gravel.albedo->get_height() == 256);
		// These two are single-channel (grayscale) source JPEGs — the ones that actually
		// failed to decode (jpgd chokes on their unusual 2x2 luma sampling factor) until
		// the embedded assets were re-encoded with standard 1x1 sampling.
		CHECK(gravel.roughness->get_width() == 256);
		CHECK(gravel.roughness->get_height() == 256);
		CHECK(gravel.ambient_occlusion->get_width() == 256);
		Ref<Image> gravel_roughness_img = gravel.roughness->get_data();
		CHECK(gravel_roughness_img.is_valid());
		CHECK(gravel_roughness_img->get_width() == 256);

		ProcRockPipelineTextures mossy = load_baked_textures(PROCROCK_BAKED_MOSSY);
		CHECK(mossy.albedo.is_valid());
		CHECK(mossy.albedo->get_width() == 256);
		CHECK(mossy.roughness->get_width() == 256);
		CHECK(mossy.ambient_occlusion->get_width() == 256);

		ProcRockPipelineTextures rock = load_baked_textures(PROCROCK_BAKED_ROCK);
		CHECK(rock.albedo.is_valid());
		CHECK(rock.albedo->get_width() == 256);
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
