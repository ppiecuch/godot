/**************************************************************************/
/*  vat_multi_mesh_instance.h                                             */
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

#ifndef VAT_MULTI_MESH_INSTANCE_H
#define VAT_MULTI_MESH_INSTANCE_H

#include "scene/3d/multimesh_instance.h"
#include "scene/resources/mesh.h"
#include "scene/resources/multimesh.h"

#include "vat_animation_track.h"

class VATMultiMeshInstance : public MultiMeshInstance {
	GDCLASS(VATMultiMeshInstance, MultiMeshInstance);

	Ref<ArrayMesh> exported_mesh;
	int instance_count;
	bool rand_anim_offset;
	String openvat_json_config_file;
	Vector3 min_values;
	Vector3 max_values;

	Vector<Ref<VATAnimationTrack>> animation_tracks;
	int frames;

	void _ensure_multimesh();
	void _push_shader_param(const StringName &p_param, const Variant &p_value);
	void _parse_json_data(const Dictionary &data);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_exported_mesh(const Ref<ArrayMesh> &p_mesh);
	Ref<ArrayMesh> get_exported_mesh() const;

	void set_instance_count(int p_count);
	int get_instance_count() const;

	void set_rand_anim_offset(bool p_rand);
	bool get_rand_anim_offset() const;

	void set_openvat_json_config_file(const String &p_path);
	String get_openvat_json_config_file() const;

	void set_min_values(const Vector3 &p_min);
	Vector3 get_min_values() const;

	void set_max_values(const Vector3 &p_max);
	Vector3 get_max_values() const;

	// Instance manipulation
	void update_instance_animation_offset(int p_instance_id, float p_offset);
	void update_instance_track(int p_instance_id, int p_track_number);
	void update_instance_alpha(int p_instance_id, float p_alpha);
	void update_instance(int p_instance_id, float p_offset, int p_track_number, float p_alpha);
	void update_all_instances(float p_offset, int p_track_number, float p_alpha);

	void play_next_track_instance(int p_instance_id);
	void play_next_track_all_instances();

	// Track queries
	Ref<VATAnimationTrack> get_animation_from_instance(int p_instance_id) const;
	int get_track_number_from_animation(const Ref<VATAnimationTrack> &p_track) const;
	int get_track_number_from_instance(int p_instance_id) const;
	int get_track_count() const;
	Ref<VATAnimationTrack> get_animation_track(int p_index) const;

	// JSON import
	void import_json();
	void import_json_string(const String &p_json_content);

	virtual String get_configuration_warning() const;

	VATMultiMeshInstance();
};

#endif // VAT_MULTI_MESH_INSTANCE_H
