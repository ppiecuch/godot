/**************************************************************************/
/*  vat_multi_mesh_instance.cpp                                           */
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

#include "vat_multi_mesh_instance.h"

#include "core/io/json.h"
#include "core/math/math_funcs.h"
#include "core/os/file_access.h"
#include "scene/resources/material.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

void VATMultiMeshInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_exported_mesh", "mesh"), &VATMultiMeshInstance::set_exported_mesh);
	ClassDB::bind_method(D_METHOD("get_exported_mesh"), &VATMultiMeshInstance::get_exported_mesh);

	ClassDB::bind_method(D_METHOD("set_instance_count", "count"), &VATMultiMeshInstance::set_instance_count);
	ClassDB::bind_method(D_METHOD("get_instance_count"), &VATMultiMeshInstance::get_instance_count);

	ClassDB::bind_method(D_METHOD("set_rand_anim_offset", "rand"), &VATMultiMeshInstance::set_rand_anim_offset);
	ClassDB::bind_method(D_METHOD("get_rand_anim_offset"), &VATMultiMeshInstance::get_rand_anim_offset);

	ClassDB::bind_method(D_METHOD("set_openvat_json_config_file", "path"), &VATMultiMeshInstance::set_openvat_json_config_file);
	ClassDB::bind_method(D_METHOD("get_openvat_json_config_file"), &VATMultiMeshInstance::get_openvat_json_config_file);

	ClassDB::bind_method(D_METHOD("set_min_values", "min"), &VATMultiMeshInstance::set_min_values);
	ClassDB::bind_method(D_METHOD("get_min_values"), &VATMultiMeshInstance::get_min_values);

	ClassDB::bind_method(D_METHOD("set_max_values", "max"), &VATMultiMeshInstance::set_max_values);
	ClassDB::bind_method(D_METHOD("get_max_values"), &VATMultiMeshInstance::get_max_values);

	ClassDB::bind_method(D_METHOD("import_json"), &VATMultiMeshInstance::import_json);
	ClassDB::bind_method(D_METHOD("import_json_string", "json_content"), &VATMultiMeshInstance::import_json_string);

	ClassDB::bind_method(D_METHOD("update_instance_animation_offset", "instance_id", "offset"), &VATMultiMeshInstance::update_instance_animation_offset);
	ClassDB::bind_method(D_METHOD("update_instance_track", "instance_id", "track_number"), &VATMultiMeshInstance::update_instance_track);
	ClassDB::bind_method(D_METHOD("update_instance_alpha", "instance_id", "alpha"), &VATMultiMeshInstance::update_instance_alpha);
	ClassDB::bind_method(D_METHOD("update_instance", "instance_id", "offset", "track_number", "alpha"), &VATMultiMeshInstance::update_instance);
	ClassDB::bind_method(D_METHOD("update_all_instances", "offset", "track_number", "alpha"), &VATMultiMeshInstance::update_all_instances);

	ClassDB::bind_method(D_METHOD("play_next_track_instance", "instance_id"), &VATMultiMeshInstance::play_next_track_instance);
	ClassDB::bind_method(D_METHOD("play_next_track_all_instances"), &VATMultiMeshInstance::play_next_track_all_instances);

	ClassDB::bind_method(D_METHOD("get_animation_from_instance", "instance_id"), &VATMultiMeshInstance::get_animation_from_instance);
	ClassDB::bind_method(D_METHOD("get_track_number_from_animation", "track"), &VATMultiMeshInstance::get_track_number_from_animation);
	ClassDB::bind_method(D_METHOD("get_track_number_from_instance", "instance_id"), &VATMultiMeshInstance::get_track_number_from_instance);
	ClassDB::bind_method(D_METHOD("get_track_count"), &VATMultiMeshInstance::get_track_count);
	ClassDB::bind_method(D_METHOD("get_animation_track", "index"), &VATMultiMeshInstance::get_animation_track);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "exported_mesh", PROPERTY_HINT_RESOURCE_TYPE, "ArrayMesh"), "set_exported_mesh", "get_exported_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "instance_count", PROPERTY_HINT_RANGE, "1,10000,1"), "set_instance_count", "get_instance_count");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rand_anim_offset"), "set_rand_anim_offset", "get_rand_anim_offset");

	ADD_GROUP("OpenVAT Config", "openvat_");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "openvat_json_config_file", PROPERTY_HINT_FILE, "*.json"), "set_openvat_json_config_file", "get_openvat_json_config_file");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "openvat_min_values"), "set_min_values", "get_min_values");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "openvat_max_values"), "set_max_values", "get_max_values");
}

void VATMultiMeshInstance::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_ensure_multimesh();

			Ref<MultiMesh> mm = get_multimesh();
			if (mm.is_valid()) {
				mm->set_instance_count(0);
				mm->set_transform_format(MultiMesh::TRANSFORM_3D);
				mm->set_custom_data_format(MultiMesh::CUSTOM_DATA_FLOAT);
				mm->set_instance_count(instance_count);
			}

			if (animation_tracks.size() == 0) {
				import_json();
			}
		} break;
	}
}

void VATMultiMeshInstance::_ensure_multimesh() {
	if (get_multimesh().is_null()) {
		Ref<MultiMesh> mm;
		mm.instance();
		mm->set_instance_count(0);
		mm->set_transform_format(MultiMesh::TRANSFORM_3D);
		mm->set_custom_data_format(MultiMesh::CUSTOM_DATA_FLOAT);
		set_multimesh(mm);
	}
}

void VATMultiMeshInstance::_push_shader_param(const StringName &p_param, const Variant &p_value) {
	Ref<MultiMesh> mm = get_multimesh();
	if (mm.is_null() || mm->get_mesh().is_null()) {
		return;
	}
	Ref<Mesh> mesh = mm->get_mesh();
	if (mesh->get_surface_count() == 0) {
		return;
	}
	Ref<Material> mat = mesh->surface_get_material(0);
	if (mat.is_null()) {
		return;
	}
	ShaderMaterial *shader_mat = Object::cast_to<ShaderMaterial>(mat.ptr());
	if (shader_mat) {
		shader_mat->set_shader_param(p_param, p_value);
	}
}

// Property setters/getters

void VATMultiMeshInstance::set_exported_mesh(const Ref<ArrayMesh> &p_mesh) {
	exported_mesh = p_mesh;
	_ensure_multimesh();
	get_multimesh()->set_mesh(p_mesh);
}

Ref<ArrayMesh> VATMultiMeshInstance::get_exported_mesh() const {
	return exported_mesh;
}

void VATMultiMeshInstance::set_instance_count(int p_count) {
	instance_count = p_count;
}

int VATMultiMeshInstance::get_instance_count() const {
	return instance_count;
}

void VATMultiMeshInstance::set_rand_anim_offset(bool p_rand) {
	rand_anim_offset = p_rand;
}

bool VATMultiMeshInstance::get_rand_anim_offset() const {
	return rand_anim_offset;
}

void VATMultiMeshInstance::set_openvat_json_config_file(const String &p_path) {
	openvat_json_config_file = p_path;
}

String VATMultiMeshInstance::get_openvat_json_config_file() const {
	return openvat_json_config_file;
}

void VATMultiMeshInstance::set_min_values(const Vector3 &p_min) {
	min_values = p_min;
	_push_shader_param("min_values", p_min);
}

Vector3 VATMultiMeshInstance::get_min_values() const {
	return min_values;
}

void VATMultiMeshInstance::set_max_values(const Vector3 &p_max) {
	max_values = p_max;
	_push_shader_param("max_values", p_max);
}

Vector3 VATMultiMeshInstance::get_max_values() const {
	return max_values;
}

// Instance manipulation

void VATMultiMeshInstance::update_instance_animation_offset(int p_instance_id, float p_offset) {
	Ref<MultiMesh> mm = get_multimesh();
	ERR_FAIL_COND(mm.is_null());
	ERR_FAIL_INDEX(p_instance_id, mm->get_instance_count());

	p_offset = CLAMP(p_offset, 0.0f, 1.0f);
	Color custom_data = mm->get_instance_custom_data(p_instance_id);
	custom_data.r = rand_anim_offset ? p_offset : 0.0f;
	mm->set_instance_custom_data(p_instance_id, custom_data);
}

void VATMultiMeshInstance::update_instance_track(int p_instance_id, int p_track_number) {
	Ref<MultiMesh> mm = get_multimesh();
	ERR_FAIL_COND(mm.is_null());
	ERR_FAIL_INDEX(p_instance_id, mm->get_instance_count());
	ERR_FAIL_INDEX(p_track_number, animation_tracks.size());

	Color custom_data = mm->get_instance_custom_data(p_instance_id);
	custom_data.g = (float)animation_tracks[p_track_number]->get_start_frame();
	custom_data.b = (float)animation_tracks[p_track_number]->get_end_frame();
	mm->set_instance_custom_data(p_instance_id, custom_data);
}

void VATMultiMeshInstance::update_instance_alpha(int p_instance_id, float p_alpha) {
	Ref<MultiMesh> mm = get_multimesh();
	ERR_FAIL_COND(mm.is_null());
	ERR_FAIL_INDEX(p_instance_id, mm->get_instance_count());

	p_alpha = CLAMP(p_alpha, 0.0f, 1.0f);
	Color custom_data = mm->get_instance_custom_data(p_instance_id);
	custom_data.a = p_alpha;
	mm->set_instance_custom_data(p_instance_id, custom_data);
}

void VATMultiMeshInstance::update_instance(int p_instance_id, float p_offset, int p_track_number, float p_alpha) {
	update_instance_animation_offset(p_instance_id, p_offset);
	update_instance_track(p_instance_id, p_track_number);
	update_instance_alpha(p_instance_id, p_alpha);
}

void VATMultiMeshInstance::update_all_instances(float p_offset, int p_track_number, float p_alpha) {
	Ref<MultiMesh> mm = get_multimesh();
	ERR_FAIL_COND(mm.is_null());

	for (int i = 0; i < mm->get_instance_count(); i++) {
		update_instance_animation_offset(i, p_offset);
		update_instance_track(i, p_track_number);
		update_instance_alpha(i, p_alpha);
	}
}

void VATMultiMeshInstance::play_next_track_instance(int p_instance_id) {
	int track_number = get_track_number_from_instance(p_instance_id);
	track_number += 1;
	if (track_number > animation_tracks.size() - 1) {
		track_number = 0;
	}
	update_instance_track(p_instance_id, track_number);
}

void VATMultiMeshInstance::play_next_track_all_instances() {
	Ref<MultiMesh> mm = get_multimesh();
	ERR_FAIL_COND(mm.is_null());

	for (int i = 0; i < mm->get_instance_count(); i++) {
		play_next_track_instance(i);
	}
}

// Track queries

Ref<VATAnimationTrack> VATMultiMeshInstance::get_animation_from_instance(int p_instance_id) const {
	Ref<MultiMesh> mm = get_multimesh();
	ERR_FAIL_COND_V(mm.is_null(), Ref<VATAnimationTrack>());
	ERR_FAIL_INDEX_V(p_instance_id, mm->get_instance_count(), Ref<VATAnimationTrack>());

	Color custom_data = mm->get_instance_custom_data(p_instance_id);

	for (int i = 0; i < animation_tracks.size(); i++) {
		if (Math::is_equal_approx(custom_data.g, (float)animation_tracks[i]->get_start_frame()) &&
				Math::is_equal_approx(custom_data.b, (float)animation_tracks[i]->get_end_frame())) {
			return animation_tracks[i];
		}
	}

	return Ref<VATAnimationTrack>();
}

int VATMultiMeshInstance::get_track_number_from_animation(const Ref<VATAnimationTrack> &p_track) const {
	if (p_track.is_null()) {
		return -1;
	}
	for (int i = 0; i < animation_tracks.size(); i++) {
		if (animation_tracks[i] == p_track) {
			return i;
		}
	}
	return -1;
}

int VATMultiMeshInstance::get_track_number_from_instance(int p_instance_id) const {
	return get_track_number_from_animation(get_animation_from_instance(p_instance_id));
}

int VATMultiMeshInstance::get_track_count() const {
	return animation_tracks.size();
}

Ref<VATAnimationTrack> VATMultiMeshInstance::get_animation_track(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, animation_tracks.size(), Ref<VATAnimationTrack>());
	return animation_tracks[p_index];
}

// JSON import — shared parsing logic

void VATMultiMeshInstance::_parse_json_data(const Dictionary &data) {
	ERR_FAIL_COND_MSG(!data.has("os-remap"), "VATMultiMeshInstance: JSON missing 'os-remap' key.");
	Dictionary os_remap = data["os-remap"];

	// Min/Max vectors
	ERR_FAIL_COND_MSG(!os_remap.has("Min"), "VATMultiMeshInstance: JSON missing 'Min' in os-remap.");
	Array min_array = os_remap["Min"];
	set_min_values(Vector3(min_array[0], min_array[1], min_array[2]));
	print_line("VATMultiMeshInstance: Min values: " + String(min_values));

	ERR_FAIL_COND_MSG(!os_remap.has("Max"), "VATMultiMeshInstance: JSON missing 'Max' in os-remap.");
	Array max_array = os_remap["Max"];
	set_max_values(Vector3(max_array[0], max_array[1], max_array[2]));
	print_line("VATMultiMeshInstance: Max values: " + String(max_values));

	frames = int(os_remap["Frames"]);

	// Animations
	animation_tracks.clear();

	if (data.has("animations")) {
		Dictionary anim_dict = data["animations"];

		if (anim_dict.empty()) {
			Ref<VATAnimationTrack> track;
			track.instance();
			track->set_track("Default", 0, frames - 1, 24, true);
			animation_tracks.push_back(track);
			print_line("VATMultiMeshInstance: No animation metadata found. Created default track with " + itos(frames) + " frames.");
		} else {
			List<Variant> keys;
			anim_dict.get_key_list(&keys);
			int i = 0;
			for (List<Variant>::Element *E = keys.front(); E; E = E->next()) {
				String key = E->get();
				Dictionary anim_data = anim_dict[key];

				Ref<VATAnimationTrack> track;
				track.instance();
				track->set_track(
						key,
						int(anim_data["startFrame"]) - 1,
						int(anim_data["endFrame"]) - 1,
						int(anim_data["framerate"]),
						bool(int(anim_data["looping"])));

				print_line("VATMultiMeshInstance: Track " + itos(i) + ": " + key +
						" frames " + itos(track->get_start_frame()) + "-" + itos(track->get_end_frame()));

				animation_tracks.push_back(track);
				i++;
			}
			print_line("VATMultiMeshInstance: Total animation tracks: " + itos(animation_tracks.size()));
		}
	} else {
		Ref<VATAnimationTrack> track;
		track.instance();
		track->set_track("Default", 0, frames - 1, 24, true);
		animation_tracks.push_back(track);
		print_line("VATMultiMeshInstance: No 'animations' key. Created default track with " + itos(frames) + " frames.");
	}

	print_line("VATMultiMeshInstance: Frames: " + itos(frames));
	print_line("VATMultiMeshInstance: OpenVAT import completed.");
}

void VATMultiMeshInstance::import_json() {
	_ensure_multimesh();

	if (!exported_mesh.is_valid()) {
		ERR_PRINT("VATMultiMeshInstance: No exported mesh assigned.");
		return;
	}

	if (openvat_json_config_file.empty()) {
		ERR_PRINT("VATMultiMeshInstance: No JSON config file set.");
		return;
	}

	print_line("VATMultiMeshInstance: Beginning OpenVAT JSON config file import...");

	FileAccess *file = FileAccess::open(openvat_json_config_file, FileAccess::READ);
	if (!file) {
		ERR_PRINT("VATMultiMeshInstance: Could not open JSON file: " + openvat_json_config_file);
		return;
	}

	String content = file->get_as_utf8_string();
	memdelete(file);

	Variant result;
	String error_string;
	int error_line;
	Error err = JSON::parse(content, result, error_string, error_line);
	if (err != OK) {
		ERR_PRINT("VATMultiMeshInstance: JSON parse error: " + error_string + " at line " + itos(error_line));
		return;
	}

	Dictionary data = result;
	print_line("VATMultiMeshInstance: JSON file: " + openvat_json_config_file);
	_parse_json_data(data);
}

void VATMultiMeshInstance::import_json_string(const String &p_json_content) {
	_ensure_multimesh();

	Variant result;
	String error_string;
	int error_line;
	Error err = JSON::parse(p_json_content, result, error_string, error_line);
	if (err != OK) {
		ERR_PRINT("VATMultiMeshInstance: JSON parse error: " + error_string + " at line " + itos(error_line));
		return;
	}

	Dictionary data = result;
	_parse_json_data(data);
}

String VATMultiMeshInstance::get_configuration_warning() const {
	String warning = MultiMeshInstance::get_configuration_warning();

	if (!exported_mesh.is_valid()) {
		if (!warning.empty()) {
			warning += "\n\n";
		}
		warning += TTR("No exported mesh assigned.");
	}

	if (openvat_json_config_file.empty()) {
		if (!warning.empty()) {
			warning += "\n\n";
		}
		warning += TTR("No OpenVAT JSON config file assigned.");
	}

	return warning;
}

VATMultiMeshInstance::VATMultiMeshInstance() {
	instance_count = 10;
	rand_anim_offset = true;
	frames = 0;
}

#ifdef DOCTEST

// Helper: create a VATMultiMeshInstance with a MultiMesh ready for testing
static VATMultiMeshInstance *_create_test_vat(int p_instance_count = 5) {
	VATMultiMeshInstance *vat = memnew(VATMultiMeshInstance);

	Ref<MultiMesh> mm;
	mm.instance();
	mm->set_instance_count(0);
	mm->set_transform_format(MultiMesh::TRANSFORM_3D);
	mm->set_custom_data_format(MultiMesh::CUSTOM_DATA_FLOAT);
	mm->set_instance_count(p_instance_count);
	vat->set_multimesh(mm);

	return vat;
}

// Sample JSON matching OpenVAT remap_info.json format
static const char *TEST_JSON_MULTI_TRACK =
		"{"
		"  \"os-remap\": {"
		"    \"Min\": [-1.5, -0.2, -1.5],"
		"    \"Max\": [1.5, 3.0, 1.5],"
		"    \"Frames\": 90"
		"  },"
		"  \"animations\": {"
		"    \"Walk\": { \"startFrame\": 1, \"endFrame\": 30, \"framerate\": 24, \"looping\": 1 },"
		"    \"Run\":  { \"startFrame\": 31, \"endFrame\": 60, \"framerate\": 30, \"looping\": 1 },"
		"    \"Die\":  { \"startFrame\": 61, \"endFrame\": 90, \"framerate\": 24, \"looping\": 0 }"
		"  }"
		"}";

static const char *TEST_JSON_NO_ANIMS =
		"{"
		"  \"os-remap\": {"
		"    \"Min\": [-2.0, -1.0, -2.0],"
		"    \"Max\": [2.0, 4.0, 2.0],"
		"    \"Frames\": 48"
		"  },"
		"  \"animations\": {}"
		"}";

static const char *TEST_JSON_NO_ANIM_KEY =
		"{"
		"  \"os-remap\": {"
		"    \"Min\": [0.0, 0.0, 0.0],"
		"    \"Max\": [1.0, 1.0, 1.0],"
		"    \"Frames\": 10"
		"  }"
		"}";

TEST_CASE("[VATMultiMeshInstance] default constructor") {
	VATMultiMeshInstance *vat = memnew(VATMultiMeshInstance);
	CHECK(vat->get_instance_count() == 10);
	CHECK(vat->get_rand_anim_offset() == true);
	CHECK(vat->get_track_count() == 0);
	CHECK(vat->get_min_values() == Vector3());
	CHECK(vat->get_max_values() == Vector3());
	CHECK(vat->get_exported_mesh().is_null());
	CHECK(vat->get_openvat_json_config_file() == "");
	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] property setters and getters") {
	VATMultiMeshInstance *vat = memnew(VATMultiMeshInstance);

	SUBCASE("instance_count") {
		vat->set_instance_count(100);
		CHECK(vat->get_instance_count() == 100);
		vat->set_instance_count(1);
		CHECK(vat->get_instance_count() == 1);
	}

	SUBCASE("rand_anim_offset") {
		vat->set_rand_anim_offset(false);
		CHECK(vat->get_rand_anim_offset() == false);
		vat->set_rand_anim_offset(true);
		CHECK(vat->get_rand_anim_offset() == true);
	}

	SUBCASE("openvat_json_config_file") {
		vat->set_openvat_json_config_file("res://test.json");
		CHECK(vat->get_openvat_json_config_file() == "res://test.json");
	}

	SUBCASE("min_values") {
		Vector3 min(-1, -2, -3);
		vat->set_min_values(min);
		CHECK(vat->get_min_values() == min);
	}

	SUBCASE("max_values") {
		Vector3 max(5, 10, 15);
		vat->set_max_values(max);
		CHECK(vat->get_max_values() == max);
	}

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] import_json_string with multiple tracks") {
	VATMultiMeshInstance *vat = _create_test_vat(5);

	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	REQUIRE(vat->get_track_count() == 3);

	// Check min/max were parsed
	CHECK(vat->get_min_values().is_equal_approx(Vector3(-1.5, -0.2, -1.5)));
	CHECK(vat->get_max_values().is_equal_approx(Vector3(1.5, 3.0, 1.5)));

	// Check tracks (startFrame/endFrame are 1-indexed in JSON, stored as 0-indexed)
	Ref<VATAnimationTrack> walk = vat->get_animation_track(0);
	REQUIRE(walk.is_valid());
	CHECK(walk->get_track_name() == "Walk");
	CHECK(walk->get_start_frame() == 0);
	CHECK(walk->get_end_frame() == 29);
	CHECK(walk->get_framerate() == 24);
	CHECK(walk->get_is_looping() == true);

	Ref<VATAnimationTrack> run = vat->get_animation_track(1);
	REQUIRE(run.is_valid());
	CHECK(run->get_track_name() == "Run");
	CHECK(run->get_start_frame() == 30);
	CHECK(run->get_end_frame() == 59);
	CHECK(run->get_framerate() == 30);
	CHECK(run->get_is_looping() == true);

	Ref<VATAnimationTrack> die = vat->get_animation_track(2);
	REQUIRE(die.is_valid());
	CHECK(die->get_track_name() == "Die");
	CHECK(die->get_start_frame() == 60);
	CHECK(die->get_end_frame() == 89);
	CHECK(die->get_framerate() == 24);
	CHECK(die->get_is_looping() == false);

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] import_json_string with empty animations") {
	VATMultiMeshInstance *vat = _create_test_vat(3);

	vat->import_json_string(TEST_JSON_NO_ANIMS);

	// Should create a single default track
	REQUIRE(vat->get_track_count() == 1);
	Ref<VATAnimationTrack> track = vat->get_animation_track(0);
	CHECK(track->get_track_name() == "Default");
	CHECK(track->get_start_frame() == 0);
	CHECK(track->get_end_frame() == 47); // Frames=48, so 0..47
	CHECK(track->get_framerate() == 24);
	CHECK(track->get_is_looping() == true);

	CHECK(vat->get_min_values().is_equal_approx(Vector3(-2, -1, -2)));
	CHECK(vat->get_max_values().is_equal_approx(Vector3(2, 4, 2)));

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] import_json_string with no animations key") {
	VATMultiMeshInstance *vat = _create_test_vat(2);

	vat->import_json_string(TEST_JSON_NO_ANIM_KEY);

	REQUIRE(vat->get_track_count() == 1);
	Ref<VATAnimationTrack> track = vat->get_animation_track(0);
	CHECK(track->get_track_name() == "Default");
	CHECK(track->get_start_frame() == 0);
	CHECK(track->get_end_frame() == 9); // Frames=10, so 0..9

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] reimport clears previous tracks") {
	VATMultiMeshInstance *vat = _create_test_vat(3);

	vat->import_json_string(TEST_JSON_MULTI_TRACK);
	REQUIRE(vat->get_track_count() == 3);

	// Reimport with different data
	vat->import_json_string(TEST_JSON_NO_ANIM_KEY);
	REQUIRE(vat->get_track_count() == 1);
	CHECK(vat->get_animation_track(0)->get_track_name() == "Default");

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] update_instance_track and get_track_number_from_instance") {
	VATMultiMeshInstance *vat = _create_test_vat(5);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	// Assign track 0 (Walk) to instance 0
	vat->update_instance_track(0, 0);
	CHECK(vat->get_track_number_from_instance(0) == 0);

	// Assign track 1 (Run) to instance 1
	vat->update_instance_track(1, 1);
	CHECK(vat->get_track_number_from_instance(1) == 1);

	// Assign track 2 (Die) to instance 2
	vat->update_instance_track(2, 2);
	CHECK(vat->get_track_number_from_instance(2) == 2);

	// Verify get_animation_from_instance returns correct track
	Ref<VATAnimationTrack> anim = vat->get_animation_from_instance(1);
	REQUIRE(anim.is_valid());
	CHECK(anim->get_track_name() == "Run");

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] update_instance_animation_offset") {
	VATMultiMeshInstance *vat = _create_test_vat(3);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	SUBCASE("with rand_anim_offset enabled") {
		vat->set_rand_anim_offset(true);
		vat->update_instance_animation_offset(0, 0.75);
		Color cd = vat->get_multimesh()->get_instance_custom_data(0);
		CHECK(Math::is_equal_approx(cd.r, 0.75f));
	}

	SUBCASE("with rand_anim_offset disabled") {
		vat->set_rand_anim_offset(false);
		vat->update_instance_animation_offset(0, 0.75);
		Color cd = vat->get_multimesh()->get_instance_custom_data(0);
		CHECK(Math::is_equal_approx(cd.r, 0.0f));
	}

	SUBCASE("offset clamped to 0..1") {
		vat->set_rand_anim_offset(true);
		vat->update_instance_animation_offset(0, 2.5);
		Color cd = vat->get_multimesh()->get_instance_custom_data(0);
		CHECK(Math::is_equal_approx(cd.r, 1.0f));

		vat->update_instance_animation_offset(0, -0.5);
		cd = vat->get_multimesh()->get_instance_custom_data(0);
		CHECK(Math::is_equal_approx(cd.r, 0.0f));
	}

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] update_instance_alpha") {
	VATMultiMeshInstance *vat = _create_test_vat(3);

	vat->update_instance_alpha(0, 1.0);
	Color cd = vat->get_multimesh()->get_instance_custom_data(0);
	CHECK(Math::is_equal_approx(cd.a, 1.0f));

	vat->update_instance_alpha(0, 0.5);
	cd = vat->get_multimesh()->get_instance_custom_data(0);
	CHECK(Math::is_equal_approx(cd.a, 0.5f));

	vat->update_instance_alpha(0, 0.0);
	cd = vat->get_multimesh()->get_instance_custom_data(0);
	CHECK(Math::is_equal_approx(cd.a, 0.0f));

	SUBCASE("alpha clamped to 0..1") {
		vat->update_instance_alpha(0, 5.0);
		cd = vat->get_multimesh()->get_instance_custom_data(0);
		CHECK(Math::is_equal_approx(cd.a, 1.0f));

		vat->update_instance_alpha(0, -1.0);
		cd = vat->get_multimesh()->get_instance_custom_data(0);
		CHECK(Math::is_equal_approx(cd.a, 0.0f));
	}

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] update_instance (combined)") {
	VATMultiMeshInstance *vat = _create_test_vat(3);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	vat->update_instance(0, 0.5, 1, 0.8);

	Color cd = vat->get_multimesh()->get_instance_custom_data(0);
	CHECK(Math::is_equal_approx(cd.r, 0.5f)); // offset
	CHECK(Math::is_equal_approx(cd.g, 30.0f)); // Run start frame
	CHECK(Math::is_equal_approx(cd.b, 59.0f)); // Run end frame
	CHECK(Math::is_equal_approx(cd.a, 0.8f)); // alpha

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] update_all_instances") {
	VATMultiMeshInstance *vat = _create_test_vat(4);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	vat->update_all_instances(0.25, 2, 0.9);

	for (int i = 0; i < 4; i++) {
		Color cd = vat->get_multimesh()->get_instance_custom_data(i);
		CHECK(Math::is_equal_approx(cd.r, 0.25f));
		CHECK(Math::is_equal_approx(cd.g, 60.0f)); // Die start
		CHECK(Math::is_equal_approx(cd.b, 89.0f)); // Die end
		CHECK(Math::is_equal_approx(cd.a, 0.9f));
	}

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] play_next_track_instance") {
	VATMultiMeshInstance *vat = _create_test_vat(3);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	// Start at track 0
	vat->update_instance_track(0, 0);
	CHECK(vat->get_track_number_from_instance(0) == 0);

	// Next -> track 1
	vat->play_next_track_instance(0);
	CHECK(vat->get_track_number_from_instance(0) == 1);

	// Next -> track 2
	vat->play_next_track_instance(0);
	CHECK(vat->get_track_number_from_instance(0) == 2);

	// Next -> wraps to track 0
	vat->play_next_track_instance(0);
	CHECK(vat->get_track_number_from_instance(0) == 0);

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] play_next_track_all_instances") {
	VATMultiMeshInstance *vat = _create_test_vat(3);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	// Assign different tracks to each instance
	vat->update_instance_track(0, 0); // Walk
	vat->update_instance_track(1, 1); // Run
	vat->update_instance_track(2, 2); // Die

	vat->play_next_track_all_instances();

	// Each should advance by 1
	CHECK(vat->get_track_number_from_instance(0) == 1); // Walk -> Run
	CHECK(vat->get_track_number_from_instance(1) == 2); // Run -> Die
	CHECK(vat->get_track_number_from_instance(2) == 0); // Die -> Walk (wrap)

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] get_track_number_from_animation") {
	VATMultiMeshInstance *vat = _create_test_vat(3);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	Ref<VATAnimationTrack> walk = vat->get_animation_track(0);
	Ref<VATAnimationTrack> run = vat->get_animation_track(1);
	Ref<VATAnimationTrack> die = vat->get_animation_track(2);

	CHECK(vat->get_track_number_from_animation(walk) == 0);
	CHECK(vat->get_track_number_from_animation(run) == 1);
	CHECK(vat->get_track_number_from_animation(die) == 2);

	SUBCASE("null track returns -1") {
		Ref<VATAnimationTrack> null_track;
		CHECK(vat->get_track_number_from_animation(null_track) == -1);
	}

	SUBCASE("unknown track returns -1") {
		Ref<VATAnimationTrack> unknown;
		unknown.instance();
		unknown->set_track("Unknown", 100, 200, 24, true);
		CHECK(vat->get_track_number_from_animation(unknown) == -1);
	}

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] get_animation_track bounds checking") {
	VATMultiMeshInstance *vat = _create_test_vat(2);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);
	REQUIRE(vat->get_track_count() == 3);

	// Valid indices
	CHECK(vat->get_animation_track(0).is_valid());
	CHECK(vat->get_animation_track(1).is_valid());
	CHECK(vat->get_animation_track(2).is_valid());

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] custom data channel layout") {
	// Verify the RGBA custom data channel mapping:
	// R=offset, G=start_frame, B=end_frame, A=alpha
	VATMultiMeshInstance *vat = _create_test_vat(1);
	vat->import_json_string(TEST_JSON_MULTI_TRACK);

	vat->update_instance_animation_offset(0, 0.33);
	vat->update_instance_track(0, 1); // Run: start=30, end=59
	vat->update_instance_alpha(0, 0.77);

	Color cd = vat->get_multimesh()->get_instance_custom_data(0);
	CHECK(Math::is_equal_approx(cd.r, 0.33f)); // animation offset
	CHECK(Math::is_equal_approx(cd.g, 30.0f)); // start frame
	CHECK(Math::is_equal_approx(cd.b, 59.0f)); // end frame
	CHECK(Math::is_equal_approx(cd.a, 0.77f)); // alpha

	memdelete(vat);
}

TEST_CASE("[VATMultiMeshInstance] configuration_warning") {
	VATMultiMeshInstance *vat = memnew(VATMultiMeshInstance);

	SUBCASE("warns when no mesh and no json") {
		String warning = vat->get_configuration_warning();
		CHECK(warning.find("mesh") != -1);
		CHECK(warning.find("JSON") != -1);
	}

	SUBCASE("warns only about json when mesh is set") {
		Ref<ArrayMesh> mesh;
		mesh.instance();
		vat->set_exported_mesh(mesh);
		String warning = vat->get_configuration_warning();
		CHECK(warning.find("mesh") == -1);
		CHECK(warning.find("JSON") != -1);
	}

	SUBCASE("warns only about mesh when json is set") {
		vat->set_openvat_json_config_file("res://test.json");
		String warning = vat->get_configuration_warning();
		CHECK(warning.find("mesh") != -1);
		CHECK(warning.find("JSON") == -1);
	}

	memdelete(vat);
}

#endif // DOCTEST
