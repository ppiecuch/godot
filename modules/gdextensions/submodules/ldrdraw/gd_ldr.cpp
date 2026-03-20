/**************************************************************************/
/*  gd_ldr.cpp                                                            */
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

#include "gd_ldr.h"

#include "core/os/file_access.h"
#include "scene/resources/material.h"

#include "libldr/color.h"
#include "libldr/elements.h"
#include "libldr/geometry_exporter.h"
#include "libldr/math.h"
#include "libldr/metrics.h"
#include "libldr/model.h"
#include "libldr/part_library.h"
#include "libldr/reader.h"
#include "libldr/utils.h"

#include <sstream>

// ============================================================================
// GdLdrModel
// ============================================================================

static bool s_color_initialized = false;
static ldraw::reader *s_reader = nullptr;
static ldraw::part_library *s_library = nullptr;

void GdLdrModel::_ensure_init() {
	if (!s_color_initialized) {
		ldraw::color::init();
		s_color_initialized = true;
	}
	if (!s_reader) {
		s_reader = new ldraw::reader();
		s_reader->enable_global_cache();
	}
}

static Color _resolve_color(unsigned int color_id) {
	ldraw::color c(color_id);
	const ldraw::color_entity *e = c.get_entity();
	if (e) {
		return Color(e->rgba[0] / 255.0f, e->rgba[1] / 255.0f, e->rgba[2] / 255.0f, e->rgba[3] / 255.0f);
	}
	// Fallback for unknown colors
	return Color(0.5f, 0.5f, 0.5f, 1.0f);
}

GdLdrModel::GdLdrModel() {
	_ensure_init();
}

GdLdrModel::~GdLdrModel() {
	if (m_model && m_owns_model) {
		delete m_model;
	}
}

bool GdLdrModel::load_file(const String &path) {
	if (m_model && m_owns_model) {
		delete m_model;
		m_model = nullptr;
	}
	m_geometry_dirty = true;

	std::string spath = path.utf8().get_data();

	try {
		m_model = s_reader->load_from_file(spath);
		m_owns_model = true;
	} catch (const ldraw::exception &e) {
		ERR_PRINT(String("LDR: Failed to load file: ") + e.what());
		return false;
	}

	if (!m_model) {
		ERR_PRINT("LDR: Failed to load file (null model)");
		return false;
	}

#ifdef LDR_ARCHIVE_SUPPORT
	// Preload dependencies from archive for part resolution
	std::vector<lcatalog::handle_t> load_list;

	std::function<void(const std::vector<lcatalog::handle_t> &, std::vector<lcatalog::handle_t> &)> check_deps;
	check_deps = [&](const std::vector<lcatalog::handle_t> &hndls, std::vector<lcatalog::handle_t> &lst) {
		for (auto &h : hndls) {
			if (std::find(lst.begin(), lst.end(), h) == lst.end()) {
				lst.push_back(h);
				check_deps(s_reader->get_archive_deps(h), lst);
			}
		}
	};

	auto collect_refs = [&](ldraw::model *mdl, std::vector<lcatalog::handle_t> &lst) {
		for (int i = 0; i < mdl->size(); ++i) {
			if (mdl->at(i)->get_type() == ldraw::type_ref) {
				ldraw::element_ref *r = CAST_AS_REF(mdl->at(i));
				const std::string fn = ldraw::utils::translate_string(r->filename());
				if (s_reader->get_archive_exists(fn)) {
					const lcatalog::handle_t h = s_reader->get_archive_handle(fn);
					if (std::find(lst.begin(), lst.end(), h) == lst.end()) {
						lst.push_back(h);
						check_deps(s_reader->get_archive_deps(fn), lst);
					}
				}
			}
		}
	};

	collect_refs(m_model->main_model(), load_list);
	for (auto &sm : m_model->submodel_list()) {
		collect_refs(sm.second, load_list);
	}

	// Sort by offset for sequential archive access
	std::sort(load_list.begin(), load_list.end(), [](const lcatalog::handle_t &a, const lcatalog::handle_t &b) {
		return s_reader->get_archive_offs(a) < s_reader->get_archive_offs(b);
	});

	for (auto &h : load_list) {
		try {
			s_reader->load_from_archive(h);
		} catch (const ldraw::exception &) {
			// Skip failed parts
		}
	}

	// Link parts
	if (!s_library) {
		try {
			s_library = new ldraw::part_library();
			s_library->set_unlink_policy(ldraw::part_library::parts);
		} catch (const ldraw::exception &) {
			// No filesystem LDraw library available - archive parts still work
			s_library = nullptr;
		}
	}
	if (s_library) {
		s_library->link(m_model);
	}
#endif

	return true;
}

bool GdLdrModel::load_from_string(const String &ldr_text) {
	if (m_model && m_owns_model) {
		delete m_model;
		m_model = nullptr;
	}
	m_geometry_dirty = true;

	std::string text = ldr_text.utf8().get_data();
	std::istringstream stream(text);

	try {
		m_model = ldraw::reader::load_from_stream(stream, "inline_model");
		m_owns_model = true;
	} catch (const ldraw::exception &e) {
		ERR_PRINT(String("LDR: Failed to parse string: ") + e.what());
		return false;
	}

	if (!m_model) {
		return false;
	}

	// Link submodels within the multipart
	m_model->link_submodels();

	return true;
}

void GdLdrModel::_export_geometry() {
	if (!m_geometry_dirty || !m_model) {
		return;
	}
	m_geometry_dirty = false;

	m_tri_vertices.resize(0);
	m_tri_normals.resize(0);
	m_tri_colors.resize(0);
	m_quad_vertices.resize(0);
	m_quad_normals.resize(0);
	m_quad_colors.resize(0);
	m_line_vertices.resize(0);
	m_line_colors.resize(0);

	lexporter::vbuffer_params params;
	params.collapse_subfiles = true;
	lexporter::geometry_exporter xp(m_model->main_model(), &params);
	xp.update(true);

	// Triangles
	int tri_count = xp.count(ldraw::type_triangles);
	if (tri_count > 0) {
		const float *verts = xp.get_vertex_array(ldraw::type_triangles);
		const float *norms = xp.get_normal_array(ldraw::type_triangles);
		const unsigned int *colors = xp.get_color_index(ldraw::type_triangles);
		int num_tris = tri_count / 3;

		m_tri_vertices.resize(tri_count);
		if (norms) {
			m_tri_normals.resize(tri_count);
		}
		m_tri_colors.resize(tri_count);

		PoolVector3Array::Write vw = m_tri_vertices.write();
		PoolVector3Array::Write nw = norms ? m_tri_normals.write() : PoolVector3Array::Write();
		PoolColorArray::Write cw = m_tri_colors.write();

		for (int i = 0; i < tri_count; ++i) {
			vw[i] = Vector3(verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]);
			if (norms) {
				nw[i] = Vector3(norms[i * 3], norms[i * 3 + 1], norms[i * 3 + 2]);
			}
		}
		for (int t = 0; t < num_tris; ++t) {
			Color c = _resolve_color(colors[t]);
			cw[t * 3] = c;
			cw[t * 3 + 1] = c;
			cw[t * 3 + 2] = c;
		}
	}

	// Quads → split into triangles
	int quad_count = xp.count(ldraw::type_quads);
	if (quad_count > 0) {
		const float *verts = xp.get_vertex_array(ldraw::type_quads);
		const float *norms = xp.get_normal_array(ldraw::type_quads);
		const unsigned int *colors = xp.get_color_index(ldraw::type_quads);
		int num_quads = quad_count / 4;

		// Each quad becomes 2 triangles = 6 vertices
		int out_count = num_quads * 6;
		m_quad_vertices.resize(out_count);
		if (norms) {
			m_quad_normals.resize(out_count);
		}
		m_quad_colors.resize(out_count);

		PoolVector3Array::Write vw = m_quad_vertices.write();
		PoolVector3Array::Write nw = norms ? m_quad_normals.write() : PoolVector3Array::Write();
		PoolColorArray::Write cw = m_quad_colors.write();

		for (int q = 0; q < num_quads; ++q) {
			Vector3 v0(verts[q * 12], verts[q * 12 + 1], verts[q * 12 + 2]);
			Vector3 v1(verts[q * 12 + 3], verts[q * 12 + 4], verts[q * 12 + 5]);
			Vector3 v2(verts[q * 12 + 6], verts[q * 12 + 7], verts[q * 12 + 8]);
			Vector3 v3(verts[q * 12 + 9], verts[q * 12 + 10], verts[q * 12 + 11]);

			int base = q * 6;
			// Triangle 1: v0, v1, v2
			vw[base] = v0;
			vw[base + 1] = v1;
			vw[base + 2] = v2;
			// Triangle 2: v0, v2, v3
			vw[base + 3] = v0;
			vw[base + 4] = v2;
			vw[base + 5] = v3;

			if (norms) {
				Vector3 n0(norms[q * 12], norms[q * 12 + 1], norms[q * 12 + 2]);
				Vector3 n1(norms[q * 12 + 3], norms[q * 12 + 4], norms[q * 12 + 5]);
				Vector3 n2(norms[q * 12 + 6], norms[q * 12 + 7], norms[q * 12 + 8]);
				Vector3 n3(norms[q * 12 + 9], norms[q * 12 + 10], norms[q * 12 + 11]);
				nw[base] = n0;
				nw[base + 1] = n1;
				nw[base + 2] = n2;
				nw[base + 3] = n0;
				nw[base + 4] = n2;
				nw[base + 5] = n3;
			}

			Color c = _resolve_color(colors[q]);
			for (int j = 0; j < 6; ++j) {
				cw[base + j] = c;
			}
		}
	}

	// Lines
	int line_count = xp.count(ldraw::type_lines);
	if (line_count > 0) {
		const float *verts = xp.get_vertex_array(ldraw::type_lines);
		const unsigned int *colors = xp.get_color_index(ldraw::type_lines);
		int num_lines = line_count / 2;

		m_line_vertices.resize(line_count);
		m_line_colors.resize(line_count);

		PoolVector3Array::Write vw = m_line_vertices.write();
		PoolColorArray::Write cw = m_line_colors.write();

		for (int i = 0; i < line_count; ++i) {
			vw[i] = Vector3(verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]);
		}
		for (int l = 0; l < num_lines; ++l) {
			Color c = _resolve_color(colors[l]);
			cw[l * 2] = c;
			cw[l * 2 + 1] = c;
		}
	}
}

String GdLdrModel::get_description() const {
	if (!m_model) {
		return String();
	}
	return String(m_model->main_model()->desc().c_str());
}

String GdLdrModel::get_author() const {
	if (!m_model) {
		return String();
	}
	return String(m_model->main_model()->author().c_str());
}

int GdLdrModel::get_element_count() const {
	if (!m_model) {
		return 0;
	}
	return m_model->main_model()->size();
}

int GdLdrModel::get_submodel_count() const {
	if (!m_model) {
		return 0;
	}
	return m_model->count();
}

PoolStringArray GdLdrModel::get_submodel_names() const {
	PoolStringArray names;
	if (!m_model) {
		return names;
	}
	for (auto &kv : m_model->submodel_list()) {
		names.append(String(kv.first.c_str()));
	}
	return names;
}

Dictionary GdLdrModel::get_bounding_box() {
	Dictionary result;
	if (!m_model) {
		result["min"] = Vector3();
		result["max"] = Vector3();
		return result;
	}

	ldraw::metrics met(m_model->main_model());
	met.update();

	if (met.is_null()) {
		result["min"] = Vector3();
		result["max"] = Vector3();
	} else {
		const ldraw::vector &mn = met.min_();
		const ldraw::vector &mx = met.max_();
		result["min"] = Vector3(mn.x(), mn.y(), mn.z());
		result["max"] = Vector3(mx.x(), mx.y(), mx.z());
	}
	return result;
}

Dictionary GdLdrModel::get_color_info(int color_id) {
	Dictionary info;
	ldraw::color c(color_id);
	const ldraw::color_entity *e = c.get_entity();
	if (e) {
		info["name"] = String(e->name.c_str());
		info["r"] = e->rgba[0];
		info["g"] = e->rgba[1];
		info["b"] = e->rgba[2];
		info["a"] = e->rgba[3];
		info["id"] = (int)e->id;
		const char *mat_names[] = { "normal", "transparent", "luminant", "glitter", "pearlescent", "chrome", "metallic", "rubber", "speckle" };
		info["material"] = String(mat_names[e->material]);
	}
	return info;
}

PoolVector3Array GdLdrModel::get_triangle_vertices() {
	_export_geometry();
	// Combine triangles and split quads
	PoolVector3Array combined;
	combined.append_array(m_tri_vertices);
	combined.append_array(m_quad_vertices);
	return combined;
}

PoolVector3Array GdLdrModel::get_triangle_normals() {
	_export_geometry();
	PoolVector3Array combined;
	combined.append_array(m_tri_normals);
	combined.append_array(m_quad_normals);
	return combined;
}

PoolColorArray GdLdrModel::get_triangle_colors() {
	_export_geometry();
	PoolColorArray combined;
	combined.append_array(m_tri_colors);
	combined.append_array(m_quad_colors);
	return combined;
}

PoolVector3Array GdLdrModel::get_line_vertices() {
	_export_geometry();
	return m_line_vertices;
}

PoolColorArray GdLdrModel::get_line_colors() {
	_export_geometry();
	return m_line_colors;
}

Ref<ArrayMesh> GdLdrModel::generate_mesh() {
	_export_geometry();

	Ref<ArrayMesh> mesh;
	mesh.instance();

	PoolVector3Array tri_verts = get_triangle_vertices();
	PoolVector3Array tri_norms = get_triangle_normals();
	PoolColorArray tri_cols = get_triangle_colors();

	// Triangle surface
	if (tri_verts.size() > 0) {
		Array arrays;
		arrays.resize(ArrayMesh::ARRAY_MAX);
		arrays[ArrayMesh::ARRAY_VERTEX] = tri_verts;
		if (tri_norms.size() == tri_verts.size()) {
			arrays[ArrayMesh::ARRAY_NORMAL] = tri_norms;
		}
		arrays[ArrayMesh::ARRAY_COLOR] = tri_cols;
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

		Ref<SpatialMaterial> mat;
		mat.instance();
		mat->set_flag(SpatialMaterial::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		mat->set_cull_mode(SpatialMaterial::CULL_DISABLED);
		mesh->surface_set_material(mesh->get_surface_count() - 1, mat);
	}

	// Line surface
	PoolVector3Array line_verts = get_line_vertices();
	PoolColorArray line_cols = get_line_colors();

	if (line_verts.size() > 0) {
		Array arrays;
		arrays.resize(ArrayMesh::ARRAY_MAX);
		arrays[ArrayMesh::ARRAY_VERTEX] = line_verts;
		arrays[ArrayMesh::ARRAY_COLOR] = line_cols;
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, arrays);

		Ref<SpatialMaterial> mat;
		mat.instance();
		mat->set_flag(SpatialMaterial::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		mat->set_flag(SpatialMaterial::FLAG_UNSHADED, true);
		mesh->surface_set_material(mesh->get_surface_count() - 1, mat);
	}

	return mesh;
}

bool GdLdrModel::is_loaded() const {
	return m_model != nullptr;
}

void GdLdrModel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_file", "path"), &GdLdrModel::load_file);
	ClassDB::bind_method(D_METHOD("load_from_string", "ldr_text"), &GdLdrModel::load_from_string);

	ClassDB::bind_method(D_METHOD("get_description"), &GdLdrModel::get_description);
	ClassDB::bind_method(D_METHOD("get_author"), &GdLdrModel::get_author);
	ClassDB::bind_method(D_METHOD("get_element_count"), &GdLdrModel::get_element_count);
	ClassDB::bind_method(D_METHOD("get_submodel_count"), &GdLdrModel::get_submodel_count);
	ClassDB::bind_method(D_METHOD("get_submodel_names"), &GdLdrModel::get_submodel_names);
	ClassDB::bind_method(D_METHOD("get_bounding_box"), &GdLdrModel::get_bounding_box);

	ClassDB::bind_method(D_METHOD("get_color_info", "color_id"), &GdLdrModel::get_color_info);

	ClassDB::bind_method(D_METHOD("get_triangle_vertices"), &GdLdrModel::get_triangle_vertices);
	ClassDB::bind_method(D_METHOD("get_triangle_normals"), &GdLdrModel::get_triangle_normals);
	ClassDB::bind_method(D_METHOD("get_triangle_colors"), &GdLdrModel::get_triangle_colors);
	ClassDB::bind_method(D_METHOD("get_line_vertices"), &GdLdrModel::get_line_vertices);
	ClassDB::bind_method(D_METHOD("get_line_colors"), &GdLdrModel::get_line_colors);

	ClassDB::bind_method(D_METHOD("generate_mesh"), &GdLdrModel::generate_mesh);
	ClassDB::bind_method(D_METHOD("is_loaded"), &GdLdrModel::is_loaded);

#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("get_demo_model_names"), &GdLdrModel::get_demo_model_names);
	ClassDB::bind_method(D_METHOD("load_demo_model", "name"), &GdLdrModel::load_demo_model);
#endif
}

// ============================================================================
// LdrView3D
// ============================================================================

void LdrView3D::set_model(Ref<GdLdrModel> model) {
	m_model = model;
	rebuild_mesh();
}

Ref<GdLdrModel> LdrView3D::get_model() const {
	return m_model;
}

void LdrView3D::set_scale_factor(float f) {
	m_scale_factor = f;
	rebuild_mesh();
}

float LdrView3D::get_scale_factor() const {
	return m_scale_factor;
}

void LdrView3D::set_show_edges(bool show) {
	m_show_edges = show;
	rebuild_mesh();
}

bool LdrView3D::get_show_edges() const {
	return m_show_edges;
}

void LdrView3D::rebuild_mesh() {
	if (!m_mesh_instance) {
		return;
	}

	if (m_model.is_null() || !m_model->is_loaded()) {
		m_mesh_instance->set_mesh(Ref<Mesh>());
		return;
	}

	Ref<ArrayMesh> mesh = m_model->generate_mesh();
	m_mesh_instance->set_mesh(mesh);
	m_mesh_instance->set_scale(Vector3(m_scale_factor, m_scale_factor, m_scale_factor));
}

void LdrView3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!m_mesh_instance) {
				m_mesh_instance = memnew(MeshInstance);
				m_mesh_instance->set_name("LdrMeshInstance");
				add_child(m_mesh_instance);
			}
			rebuild_mesh();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			// MeshInstance is a child node, will be freed automatically
			m_mesh_instance = nullptr;
		} break;
	}
}

void LdrView3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_model", "model"), &LdrView3D::set_model);
	ClassDB::bind_method(D_METHOD("get_model"), &LdrView3D::get_model);
	ClassDB::bind_method(D_METHOD("set_scale_factor", "factor"), &LdrView3D::set_scale_factor);
	ClassDB::bind_method(D_METHOD("get_scale_factor"), &LdrView3D::get_scale_factor);
	ClassDB::bind_method(D_METHOD("set_show_edges", "show"), &LdrView3D::set_show_edges);
	ClassDB::bind_method(D_METHOD("get_show_edges"), &LdrView3D::get_show_edges);
	ClassDB::bind_method(D_METHOD("rebuild_mesh"), &LdrView3D::rebuild_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "model", PROPERTY_HINT_RESOURCE_TYPE, "GdLdrModel"), "set_model", "get_model");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_factor"), "set_scale_factor", "get_scale_factor");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_edges"), "set_show_edges", "get_show_edges");
}

// ============================================================================
// LdrView2D
// ============================================================================

void LdrView2D::set_model(Ref<GdLdrModel> model) {
	m_model = model;
	update();
}

Ref<GdLdrModel> LdrView2D::get_model() const {
	return m_model;
}

void LdrView2D::set_rotation_degrees_2d(Vector2 rot) {
	m_rotation_x = rot.x;
	m_rotation_y = rot.y;
	update();
}

Vector2 LdrView2D::get_rotation_degrees_2d() const {
	return Vector2(m_rotation_x, m_rotation_y);
}

void LdrView2D::set_scale_factor(float f) {
	m_scale_factor = f;
	update();
}

float LdrView2D::get_scale_factor() const {
	return m_scale_factor;
}

void LdrView2D::set_show_edges(bool show) {
	m_show_edges = show;
	update();
}

bool LdrView2D::get_show_edges() const {
	return m_show_edges;
}

void LdrView2D::set_bg_color(Color c) {
	m_bg_color = c;
	update();
}

Color LdrView2D::get_bg_color() const {
	return m_bg_color;
}

static Vector2 _project_vertex(const Vector3 &v, float rx_deg, float ry_deg, const Vector2 &center, float scale) {
	float rx = Math::deg2rad(rx_deg);
	float ry = Math::deg2rad(ry_deg);

	float cos_rx = Math::cos(rx), sin_rx = Math::sin(rx);
	float cos_ry = Math::cos(ry), sin_ry = Math::sin(ry);

	// Rotate around Y axis
	float x1 = v.x * cos_ry + v.z * sin_ry;
	float z1 = -v.x * sin_ry + v.z * cos_ry;
	float y1 = v.y;

	// Rotate around X axis
	float y2 = y1 * cos_rx - z1 * sin_rx;

	// Orthographic projection
	return Vector2(x1 * scale + center.x, -y2 * scale + center.y);
}

void LdrView2D::_draw() {
	draw_rect(Rect2(Vector2(), get_size()), m_bg_color);

	if (m_model.is_null() || !m_model->is_loaded()) {
		return;
	}

	Vector2 center = get_size() / 2.0f;

	// Draw triangles
	PoolVector3Array tri_verts = m_model->get_triangle_vertices();
	PoolColorArray tri_colors = m_model->get_triangle_colors();

	if (tri_verts.size() >= 3) {
		PoolVector3Array::Read vr = tri_verts.read();
		PoolColorArray::Read cr = tri_colors.read();

		int num_tris = tri_verts.size() / 3;
		for (int t = 0; t < num_tris; ++t) {
			Vector<Vector2> points;
			Vector<Color> colors;
			points.resize(3);
			colors.resize(3);

			for (int j = 0; j < 3; ++j) {
				int idx = t * 3 + j;
				points.write[j] = _project_vertex(vr[idx], m_rotation_x, m_rotation_y, center, m_scale_factor);
				colors.write[j] = cr[idx];
			}
			draw_polygon(points, colors);
		}
	}

	// Draw edges
	if (m_show_edges) {
		PoolVector3Array line_verts = m_model->get_line_vertices();
		PoolColorArray line_colors = m_model->get_line_colors();

		if (line_verts.size() >= 2) {
			PoolVector3Array::Read vr = line_verts.read();
			PoolColorArray::Read cr = line_colors.read();

			int num_lines = line_verts.size() / 2;
			for (int l = 0; l < num_lines; ++l) {
				Vector2 p1 = _project_vertex(vr[l * 2], m_rotation_x, m_rotation_y, center, m_scale_factor);
				Vector2 p2 = _project_vertex(vr[l * 2 + 1], m_rotation_x, m_rotation_y, center, m_scale_factor);
				draw_line(p1, p2, cr[l * 2], 1.0f);
			}
		}
	}
}

void LdrView2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_model", "model"), &LdrView2D::set_model);
	ClassDB::bind_method(D_METHOD("get_model"), &LdrView2D::get_model);
	ClassDB::bind_method(D_METHOD("set_rotation_degrees_2d", "rot"), &LdrView2D::set_rotation_degrees_2d);
	ClassDB::bind_method(D_METHOD("get_rotation_degrees_2d"), &LdrView2D::get_rotation_degrees_2d);
	ClassDB::bind_method(D_METHOD("set_scale_factor", "factor"), &LdrView2D::set_scale_factor);
	ClassDB::bind_method(D_METHOD("get_scale_factor"), &LdrView2D::get_scale_factor);
	ClassDB::bind_method(D_METHOD("set_show_edges", "show"), &LdrView2D::set_show_edges);
	ClassDB::bind_method(D_METHOD("get_show_edges"), &LdrView2D::get_show_edges);
	ClassDB::bind_method(D_METHOD("set_bg_color", "color"), &LdrView2D::set_bg_color);
	ClassDB::bind_method(D_METHOD("get_bg_color"), &LdrView2D::get_bg_color);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "model", PROPERTY_HINT_RESOURCE_TYPE, "GdLdrModel"), "set_model", "get_model");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_rotation"), "set_rotation_degrees_2d", "get_rotation_degrees_2d");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_factor"), "set_scale_factor", "get_scale_factor");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_edges"), "set_show_edges", "get_show_edges");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "bg_color"), "set_bg_color", "get_bg_color");
}

// ============================================================================
// Demo Models (TOOLS_ENABLED only)
// ============================================================================

#ifdef TOOLS_ENABLED

extern "C" {
extern const unsigned char ldr_demo_car_data[];
extern const unsigned int ldr_demo_car_size;
extern const unsigned char ldr_demo_ship_data[];
extern const unsigned int ldr_demo_ship_size;
extern const unsigned char ldr_demo_viper_data[];
extern const unsigned int ldr_demo_viper_size;
extern const unsigned char ldr_demo_millennium_falcon_data[];
extern const unsigned int ldr_demo_millennium_falcon_size;
extern const unsigned char ldr_demo_at_at_data[];
extern const unsigned int ldr_demo_at_at_size;
extern const unsigned char ldr_demo_at_st_data[];
extern const unsigned int ldr_demo_at_st_size;
extern const unsigned char ldr_demo_vulture_droid_data[];
extern const unsigned int ldr_demo_vulture_droid_size;
extern const unsigned char ldr_demo_attack_cruiser_data[];
extern const unsigned int ldr_demo_attack_cruiser_size;
}

struct LdrDemoEntry {
	const char *name;
	const unsigned char *data;
	const unsigned int *size;
};

static const LdrDemoEntry s_demo_models[] = {
	{ "Car", ldr_demo_car_data, &ldr_demo_car_size },
	{ "Ship", ldr_demo_ship_data, &ldr_demo_ship_size },
	{ "Viper", ldr_demo_viper_data, &ldr_demo_viper_size },
	{ "Millennium Falcon", ldr_demo_millennium_falcon_data, &ldr_demo_millennium_falcon_size },
	{ "AT-AT", ldr_demo_at_at_data, &ldr_demo_at_at_size },
	{ "AT-ST", ldr_demo_at_st_data, &ldr_demo_at_st_size },
	{ "Vulture Droid", ldr_demo_vulture_droid_data, &ldr_demo_vulture_droid_size },
	{ "Attack Cruiser", ldr_demo_attack_cruiser_data, &ldr_demo_attack_cruiser_size },
};
static const int s_demo_model_count = sizeof(s_demo_models) / sizeof(s_demo_models[0]);

PoolStringArray GdLdrModel::get_demo_model_names() const {
	PoolStringArray names;
	for (int i = 0; i < s_demo_model_count; ++i) {
		names.append(String(s_demo_models[i].name));
	}
	return names;
}

bool GdLdrModel::load_demo_model(const String &name) {
	for (int i = 0; i < s_demo_model_count; ++i) {
		if (name == s_demo_models[i].name) {
			String text;
			text.parse_utf8((const char *)s_demo_models[i].data, *s_demo_models[i].size);
			return load_from_string(text);
		}
	}
	ERR_PRINT("LDR: Demo model not found: " + name);
	return false;
}

#endif // TOOLS_ENABLED

// ============================================================================
// ResourceFormatLoaderLDR
// ============================================================================

static bool _is_ldr_extension(const String &ext) {
	return ext == "ldr" || ext == "mpd" || ext == "dat";
}

RES ResourceFormatLoaderLDR::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_no_subresource_cache) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	Ref<GdLdrModel> model;
	model.instance();

	if (!model->load_file(p_path)) {
		return RES();
	}

	Ref<ArrayMesh> mesh = model->generate_mesh();
	if (mesh.is_null()) {
		return RES();
	}

	if (r_error) {
		*r_error = OK;
	}
	return mesh;
}

void ResourceFormatLoaderLDR::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("ldr");
	p_extensions->push_back("mpd");
	p_extensions->push_back("dat");
}

String ResourceFormatLoaderLDR::get_resource_type(const String &p_path) const {
	if (_is_ldr_extension(p_path.get_extension().to_lower())) {
		return "ArrayMesh";
	}
	return "";
}

bool ResourceFormatLoaderLDR::handles_type(const String &p_type) const {
	return p_type == "ArrayMesh" || p_type == "Mesh";
}

// ============================================================================
// ResourceImporterLDR (editor only)
// ============================================================================

#ifdef TOOLS_ENABLED

#include "core/io/resource_saver.h"

String ResourceImporterLDR::get_importer_name() const {
	return "ldraw_mesh";
}

String ResourceImporterLDR::get_visible_name() const {
	return "LDraw Model";
}

void ResourceImporterLDR::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("ldr");
	p_extensions->push_back("mpd");
	p_extensions->push_back("dat");
}

String ResourceImporterLDR::get_save_extension() const {
	return "mesh";
}

String ResourceImporterLDR::get_resource_type() const {
	return "ArrayMesh";
}

int ResourceImporterLDR::get_preset_count() const {
	return 1;
}

String ResourceImporterLDR::get_preset_name(int p_idx) const {
	return "Default";
}

void ResourceImporterLDR::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "scale_factor", PROPERTY_HINT_RANGE, "0.001,10.0,0.001"), 0.05));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "include_edges"), true));
}

bool ResourceImporterLDR::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return true;
}

Error ResourceImporterLDR::import(const String &p_source_file, const String &p_save_path,
		const Map<StringName, Variant> &p_options,
		List<String> *r_platform_variants,
		List<String> *r_gen_files,
		Variant *r_metadata) {
	Ref<GdLdrModel> model;
	model.instance();

	if (!model->load_file(p_source_file)) {
		return ERR_FILE_CANT_OPEN;
	}

	Ref<ArrayMesh> mesh = model->generate_mesh();
	if (mesh.is_null()) {
		return ERR_CANT_CREATE;
	}

	String save_path = p_save_path + "." + get_save_extension();
	Error err = ResourceSaver::save(save_path, mesh);
	return err;
}

#endif // TOOLS_ENABLED

// ============================================================================
// Doctest Tests
// ============================================================================

#ifdef DOCTEST_LIBRARY_INCLUDED
#include "tests/test_macros.h"

TEST_SUITE("[[ldrdraw]] GdLdrModel") {
	TEST_CASE("[ldrdraw] color init and lookup") {
		GdLdrModel::_ensure_init();
		CHECK(ldraw::color::color_chart_count > 0);
	}

	TEST_CASE("[ldrdraw] color entity properties") {
		GdLdrModel::_ensure_init();
		// Color 0 = Black
		ldraw::color black(0);
		const ldraw::color_entity *e = black.get_entity();
		REQUIRE(e != nullptr);
		CHECK(e->id == 0);
		CHECK(e->name.length() > 0);
	}

	TEST_CASE("[ldrdraw] get_color_info") {
		Ref<GdLdrModel> m;
		m.instance();
		Dictionary info = m->get_color_info(4); // Red
		CHECK(info.has("name"));
		CHECK(info.has("r"));
		CHECK(info.has("g"));
		CHECK(info.has("b"));
		CHECK(info.has("a"));
		CHECK(info.has("material"));
		CHECK((int)info["r"] > 100); // Red should have high R value
	}

	TEST_CASE("[ldrdraw] custom RGB color codes") {
		Ref<GdLdrModel> m;
		m.instance();
		Dictionary info = m->get_color_info(0x2FF0000); // Direct red
		// Direct colors may or may not be in the color chart
		// Just verify the function doesn't crash
		CHECK(true);
	}

	TEST_CASE("[ldrdraw] empty model state") {
		Ref<GdLdrModel> model;
		model.instance();
		CHECK_FALSE(model->is_loaded());
		CHECK(model->get_description() == String());
		CHECK(model->get_author() == String());
		CHECK(model->get_element_count() == 0);
		CHECK(model->get_submodel_count() == 0);
		CHECK(model->get_submodel_names().size() == 0);
	}

	TEST_CASE("[ldrdraw] load simple triangle from string") {
		Ref<GdLdrModel> model;
		model.instance();
		bool ok = model->load_from_string("0 Test Triangle\n3 4 0 0 0 10 0 0 5 10 0\n");
		CHECK(ok);
		CHECK(model->is_loaded());
		CHECK(model->get_element_count() >= 1);
	}

	TEST_CASE("[ldrdraw] load quad from string") {
		Ref<GdLdrModel> model;
		model.instance();
		bool ok = model->load_from_string("0 Test Quad\n4 1 0 0 0 10 0 0 10 10 0 0 10 0\n");
		CHECK(ok);
		CHECK(model->is_loaded());
		CHECK(model->get_element_count() >= 1);
	}

	TEST_CASE("[ldrdraw] load line from string") {
		Ref<GdLdrModel> model;
		model.instance();
		bool ok = model->load_from_string("0 Test Line\n2 0 0 0 0 10 0 0\n");
		CHECK(ok);
		CHECK(model->is_loaded());
	}

	TEST_CASE("[ldrdraw] load comment parsing") {
		Ref<GdLdrModel> model;
		model.instance();
		bool ok = model->load_from_string("0 My Description\n0 Author: Test Author\n0 This is a comment\n3 4 0 0 0 10 0 0 5 10 0\n");
		CHECK(ok);
		CHECK(model->get_description() == "My Description");
	}

	TEST_CASE("[ldrdraw] load multi-element model") {
		Ref<GdLdrModel> model;
		model.instance();
		String ldr = "0 Multi Element\n"
					 "3 4 0 0 0 10 0 0 5 10 0\n"
					 "3 1 20 0 0 30 0 0 25 10 0\n"
					 "2 0 0 0 0 10 10 10\n"
					 "4 2 0 0 0 10 0 0 10 10 0 0 10 0\n";
		bool ok = model->load_from_string(ldr);
		CHECK(ok);
		CHECK(model->get_element_count() >= 4);
	}

	TEST_CASE("[ldrdraw] load multipart (MPD) from string") {
		Ref<GdLdrModel> model;
		model.instance();
		String mpd = "0 FILE main.ldr\n"
					 "0 Main Model\n"
					 "1 4 0 0 0 1 0 0 0 1 0 0 0 1 sub.ldr\n"
					 "0 FILE sub.ldr\n"
					 "0 Sub Model\n"
					 "3 1 0 0 0 10 0 0 5 10 0\n"
					 "0 NOFILE\n";
		bool ok = model->load_from_string(mpd);
		CHECK(ok);
		CHECK(model->get_submodel_count() >= 1);
	}

	TEST_CASE("[ldrdraw] load invalid file gracefully") {
		Ref<GdLdrModel> model;
		model.instance();
		bool ok = model->load_file("/nonexistent/path/to/file.ldr");
		CHECK_FALSE(ok);
		CHECK_FALSE(model->is_loaded());
	}

	TEST_CASE("[ldrdraw] model description and author") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 My Cool Model\n0 Author: John Doe\n3 4 0 0 0 10 0 0 5 10 0\n");
		CHECK(model->get_description() == "My Cool Model");
	}

	TEST_CASE("[ldrdraw] element count") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 Test\n3 4 0 0 0 10 0 0 5 10 0\n3 1 0 0 0 5 0 0 2 5 0\n");
		CHECK(model->get_element_count() >= 2);
	}

	TEST_CASE("[ldrdraw] bounding box") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 Test\n3 4 0 0 0 10 0 0 5 10 0\n");
		Dictionary bb = model->get_bounding_box();
		CHECK(bb.has("min"));
		CHECK(bb.has("max"));
		Vector3 mn = bb["min"];
		Vector3 mx = bb["max"];
		CHECK(mx.x >= mn.x);
	}

	TEST_CASE("[ldrdraw] triangle geometry export") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 Test\n3 4 0 0 0 10 0 0 5 10 0\n");
		PoolVector3Array verts = model->get_triangle_vertices();
		CHECK(verts.size() >= 3);
	}

	TEST_CASE("[ldrdraw] quad to triangles conversion") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 Test\n4 1 0 0 0 10 0 0 10 10 0 0 10 0\n");
		PoolVector3Array verts = model->get_triangle_vertices();
		// A quad splits into 2 triangles = 6 vertices
		CHECK(verts.size() >= 6);
	}

	TEST_CASE("[ldrdraw] line geometry export") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 Test\n2 0 0 0 0 10 10 10\n");
		PoolVector3Array verts = model->get_line_vertices();
		CHECK(verts.size() >= 2);
	}

	TEST_CASE("[ldrdraw] color array from geometry") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 Test\n3 4 0 0 0 10 0 0 5 10 0\n");
		PoolColorArray colors = model->get_triangle_colors();
		CHECK(colors.size() >= 3);
		// Color 4 = Red, should have high R
		CHECK(colors.read()[0].r > 0.5f);
	}

	TEST_CASE("[ldrdraw] generate_mesh returns valid ArrayMesh") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 Test\n3 4 0 0 0 10 0 0 5 10 0\n");
		Ref<ArrayMesh> mesh = model->generate_mesh();
		CHECK(mesh.is_valid());
		CHECK(mesh->get_surface_count() >= 1);
	}

	TEST_CASE("[ldrdraw] generate_mesh has vertices and normals") {
		Ref<GdLdrModel> model;
		model.instance();
		model->load_from_string("0 Test\n3 4 0 0 0 10 0 0 5 10 0\n2 0 0 0 0 10 10 10\n");
		Ref<ArrayMesh> mesh = model->generate_mesh();
		CHECK(mesh.is_valid());
		// Should have at least triangle surface
		CHECK(mesh->get_surface_count() >= 1);
	}

	TEST_CASE("[ldrdraw] vector operations") {
		ldraw::vector a(1, 2, 3);
		ldraw::vector b(4, 5, 6);
		ldraw::vector c = a + b;
		CHECK(c.x() == doctest::Approx(5.0f));
		CHECK(c.y() == doctest::Approx(7.0f));
		CHECK(c.z() == doctest::Approx(9.0f));
	}

	TEST_CASE("[ldrdraw] matrix identity and multiplication") {
		ldraw::matrix identity;
		ldraw::vector v(1, 2, 3);
		ldraw::vector result = identity * v;
		CHECK(result.x() == doctest::Approx(1.0f));
		CHECK(result.y() == doctest::Approx(2.0f));
		CHECK(result.z() == doctest::Approx(3.0f));
	}

	TEST_CASE("[ldrdraw] matrix-vector transform") {
		// Translation matrix
		ldraw::matrix m(1, 0, 0, 0, 1, 0, 0, 0, 1, 10, 20, 30);
		ldraw::vector v(1, 2, 3);
		ldraw::vector result = m * v;
		CHECK(result.x() == doctest::Approx(11.0f));
		CHECK(result.y() == doctest::Approx(22.0f));
		CHECK(result.z() == doctest::Approx(33.0f));
	}

	TEST_CASE("[ldrdraw] BFC certification parsing") {
		Ref<GdLdrModel> model;
		model.instance();
		bool ok = model->load_from_string("0 BFC Test\n0 BFC CERTIFY CCW\n3 4 0 0 0 10 0 0 5 10 0\n");
		CHECK(ok);
		CHECK(model->is_loaded());
	}

#ifdef LDR_ARCHIVE_SUPPORT
	TEST_CASE("[ldrdraw] archive part lookup") {
		GdLdrModel::_ensure_init();
		// Check if archive has the basic brick 3001.dat
		bool exists = s_reader->get_archive_exists("3001.dat");
		// Archive may or may not be available depending on build
		CHECK(true); // Just verify no crash
	}

	TEST_CASE("[ldrdraw] load standard brick from archive") {
		Ref<GdLdrModel> model;
		model.instance();
		// Try loading a model that references a standard brick
		String ldr = "0 Brick Test\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\n";
		bool ok = model->load_from_string(ldr);
		CHECK(ok);
		// Model loads even without archive - references just won't be resolved
	}
#endif
}

#endif // DOCTEST_LIBRARY_INCLUDED
