/**************************************************************************/
/*  gd_ldr.h                                                              */
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

#ifndef GD_LDR_H
#define GD_LDR_H

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/reference.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/spatial.h"
#include "scene/gui/control.h"
#include "scene/resources/mesh.h"

#ifdef TOOLS_ENABLED
#include "core/io/resource_importer.h"
#endif

namespace ldraw {
class model_multipart;
class reader;
class part_library;
} // namespace ldraw

class GdLdrModel : public Reference {
	GDCLASS(GdLdrModel, Reference);

	ldraw::model_multipart *m_model = nullptr;
	bool m_owns_model = false;
	bool m_geometry_dirty = true;

	PoolVector3Array m_tri_vertices;
	PoolVector3Array m_tri_normals;
	PoolColorArray m_tri_colors;
	PoolVector3Array m_quad_vertices;
	PoolVector3Array m_quad_normals;
	PoolColorArray m_quad_colors;
	PoolVector3Array m_line_vertices;
	PoolColorArray m_line_colors;

	void _export_geometry();
	static void _ensure_init();

protected:
	static void _bind_methods();

public:
	bool load_file(const String &path);
	bool load_from_string(const String &ldr_text);

	String get_description() const;
	String get_author() const;
	int get_element_count() const;
	int get_submodel_count() const;
	PoolStringArray get_submodel_names() const;
	Dictionary get_bounding_box();

	Dictionary get_color_info(int color_id);

	PoolVector3Array get_triangle_vertices();
	PoolVector3Array get_triangle_normals();
	PoolColorArray get_triangle_colors();
	PoolVector3Array get_line_vertices();
	PoolColorArray get_line_colors();

	Ref<ArrayMesh> generate_mesh();

	bool is_loaded() const;

#ifdef TOOLS_ENABLED
	PoolStringArray get_demo_model_names() const;
	bool load_demo_model(const String &name);
#endif

	GdLdrModel();
	~GdLdrModel();
};

class LdrView3D : public Spatial {
	GDCLASS(LdrView3D, Spatial);

	Ref<GdLdrModel> m_model;
	MeshInstance *m_mesh_instance = nullptr;
	float m_scale_factor = 0.05f;
	bool m_show_edges = true;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_model(Ref<GdLdrModel> model);
	Ref<GdLdrModel> get_model() const;
	void set_scale_factor(float f);
	float get_scale_factor() const;
	void set_show_edges(bool show);
	bool get_show_edges() const;
	void rebuild_mesh();
};

class LdrView2D : public Control {
	GDCLASS(LdrView2D, Control);

	Ref<GdLdrModel> m_model;
	float m_scale_factor = 0.4f;
	float m_rotation_x = -30.0f;
	float m_rotation_y = 45.0f;
	bool m_show_edges = true;
	Color m_bg_color = Color(0.2f, 0.2f, 0.25f);

protected:
	static void _bind_methods();

public:
	void set_model(Ref<GdLdrModel> model);
	Ref<GdLdrModel> get_model() const;
	void set_rotation_degrees_2d(Vector2 rot);
	Vector2 get_rotation_degrees_2d() const;
	void set_scale_factor(float f);
	float get_scale_factor() const;
	void set_show_edges(bool show);
	bool get_show_edges() const;
	void set_bg_color(Color c);
	Color get_bg_color() const;
	void _draw();
};

// ============================================================================
// ResourceFormatLoader — loads .ldr/.mpd/.dat directly as ArrayMesh
// ============================================================================

class ResourceFormatLoaderLDR : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_no_subresource_cache = false);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_resource_type(const String &p_path) const;
	virtual bool handles_type(const String &p_type) const;
};

// ============================================================================
// ResourceImporterLDR — editor import: .ldr/.mpd/.dat → .mesh
// ============================================================================

#ifdef TOOLS_ENABLED
class ResourceImporterLDR : public ResourceImporter {
	GDCLASS(ResourceImporterLDR, ResourceImporter);

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
			List<String> *r_gen_files = nullptr,
			Variant *r_metadata = nullptr);
};
#endif

#endif // GD_LDR_H
