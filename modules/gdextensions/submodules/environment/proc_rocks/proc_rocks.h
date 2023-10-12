/**************************************************************************/
/*  proc_rocks.h                                                          */
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

#ifndef PROC_ROCK_MESH_H
#define PROC_ROCK_MESH_H

#include "core/io/resource_importer.h"
#include "scene/main/timer.h"
#include "scene/resources/mesh.h"

namespace procrock {
class Pipeline;
}

class ProcRockMesh : public ArrayMesh {
	GDCLASS(ProcRockMesh, ArrayMesh)

	struct {
		int depth;
		int randseed;
		real_t smoothness;
		bool smoothed;
	} rockgen;

	struct {
		Vector3 dimensions;
		uint32_t steps;
		Vector2 rand_angle_range;
		real_t rand_offset_percent;
		real_t rand_shift;
		Vector2i plane_verts_range;
		uint32_t max_planes;
	} rockgeneration;

	struct {
		procrock::Pipeline *pipeline;
	} procrock;

	bool auto_refresh;
	int method;

	bool _dirty;
	void _rebuild();

protected:
	static void _bind_methods();

	bool _is_generated() const { return true; }

	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _set(const StringName &p_path, const Variant &p_value);
	bool _get(const StringName &p_path, Variant &r_ret) const;

public:
	void set_auto_refresh(bool p_refresh);
	bool get_auto_refresh() const;
	void set_generator(int p_mode);
	int get_generator() const;

	// Gen. method 1
	void set_rockgen_depth(int p_depth);
	int get_rockgen_depth() const;
	void set_rockgen_randseed(int p_randseed);
	int get_rockgen_randseed() const;
	void set_rockgen_smoothness(real_t p_smoothness);
	real_t get_rockgen_smoothness() const;
	void set_rockgen_smoothed(bool p_smoothed);
	bool get_rockgen_smoothed() const;

	// Gen. method 2
	void set_rockgeneration_steps(uint32_t p_steps);
	uint32_t get_rockgeneration_steps() const;
	void set_rockgeneration_width(real_t p_width);
	real_t get_rockgeneration_width() const;
	void set_rockgeneration_height(real_t p_height);
	real_t get_rockgeneration_height() const;
	void set_rockgeneration_depth(real_t p_depth);
	real_t get_rockgeneration_depth() const;
	void set_rockgeneration_max_planes(uint32_t p_planes);
	uint32_t get_rockgeneration_max_planes() const;

	// Gen. method 3
	void set_procrock_generator(uint32_t p_gen);
	uint32_t get_procrock_generator() const;

	Error load_from_file(const String p_path);

	ProcRockMesh();
};

#ifdef TOOLS_ENABLED
class ResourceImporterProcRock : public ResourceImporter {
	GDCLASS(ResourceImporterProcRock, ResourceImporter);
	OBJ_SAVE_TYPE(Mesh);

public:
	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;

	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;
	virtual bool get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const;
	virtual Error import(const String &p_source_file, const String &p_save_path,
			const Map<StringName, Variant> &p_options,
			List<String> *r_platform_variants,
			List<String> *r_gen_files = NULL,
			Variant *r_metadata = NULL);

	ResourceImporterProcRock() {}
	~ResourceImporterProcRock() {}
};
#endif

#endif // PROC_ROCK_MESH_H
