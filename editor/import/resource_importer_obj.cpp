/**************************************************************************/
/*  resource_importer_obj.cpp                                             */
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

#include "resource_importer_obj.h"

#include "core/io/resource_saver.h"
#include "core/os/file_access.h"
#include "editor/plugins/material_editor_plugin.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/spatial.h"
#include "scene/resources/mesh.h"
#include "scene/resources/surface_tool.h"

#include "common/gd_pack.h"

// Reference:
// 1. https://github.com/godotengine/godot/issues/57401
// 2. https://github.com/mattdesl/texture-region

uint32_t EditorOBJImporter::get_import_flags() const {
	return IMPORT_SCENE;
}

static Ref<ArrayMesh> _build_mesh_for_atlas(const Ref<ArrayMesh> &p_mesh, String p_path, int p_max_side, List<String> *r_paths = nullptr) {
	ERR_FAIL_NULL_V(p_mesh, Ref<Mesh>());
	ERR_FAIL_COND_V(p_max_side < 0, Ref<Mesh>());

	enum {
		texture_albedo,
		texture_normal,
		texture_metallic,
		texture_roughness,
		texture_max,
	};

	static const struct {
		const char *prefix;
		SpatialMaterial::TextureParam texture;
	} info[] = {
		{ "C", SpatialMaterial::TEXTURE_ALBEDO },
		{ "N", SpatialMaterial::TEXTURE_NORMAL },
		{ "M", SpatialMaterial::TEXTURE_METALLIC },
		{ "R", SpatialMaterial::TEXTURE_ROUGHNESS },
	};

	Ref<ArrayMesh> r_mesh = memnew(ArrayMesh);
	r_mesh->set_name(p_mesh->get_name());
	r_mesh->set_comment(p_mesh->get_comment());

	for (int t = 0; t < texture_max; t++) {
		Vector<Ref<Image>> images;
		Vector<String> paths;
		// collect texture information
		for (int s = 0; s < p_mesh->get_surface_count(); s++) {
			if (Ref<SpatialMaterial> mat = p_mesh->surface_get_material(s)) {
				if (Ref<Texture> texture = mat->get_texture(info[t].texture)) {
					String image_path = texture->get_path().replace_first("res://", "");
					int index = paths.find(image_path);
					if (index < 0) {
						Ref<Image> img = memnew(Image);
						if (img->load(image_path) == OK) {
							paths.push_back(image_path);
							images.push_back(img);
						}
					}
				}
			}
		}
		if (images.size() > 1) {
			ImageMergeOptions opts;
			if (p_max_side > 1) {
				opts.set_max_size(p_max_side);
			}
			Dictionary atlas_info = merge_images(images, opts);
			ERR_FAIL_COND_V(atlas_info.empty(), Ref<Mesh>());

			Array pages = atlas_info["_generated_images"];
			ERR_FAIL_COND_V(pages.size() > 1, Ref<Mesh>());

			Ref<Image> atlas_image = pages[0];

			ERR_FAIL_NULL_V(atlas_image, Ref<Mesh>());

			String path = vformat("%s-%s.png", p_path.trim_suffix("." + p_path.get_extension()), info[t].prefix);
			ERR_FAIL_COND_V(atlas_image->save_png(path) != OK, Ref<Mesh>());
			print_verbose(vformat("OBJ: Save atlas texture: %s", path));
			if (r_paths) {
				r_paths->push_back(path);
			}

			Array atlas_rects = atlas_info["_rects"];
			for (int s = 0; s < p_mesh->get_surface_count(); s++) {
				if (Ref<SpatialMaterial> mat = p_mesh->surface_get_material(s)) {
					if (Ref<Texture> texture = mat->get_texture(info[t].texture)) {
						String image_path = texture->get_path().replace_first("res://", "");
						int image = paths.find(image_path);
						if (image < 0) {
							WARN_PRINT("Missing image " + image_path);
						} else {
							if (t == texture_albedo) { // recalculate only once
								Array mesh_array = p_mesh->surface_get_arrays(s);
								Dictionary entry = atlas_rects[image];
								ERR_CONTINUE_MSG(entry.empty(), "Empty atlas entry, Skipping!");

								PoolVector2Array uvs = mesh_array[VS::ARRAY_TEX_UV];
								PoolVector2Array xform_uvs;
								ERR_FAIL_COND_V(xform_uvs.resize(uvs.size()) != OK, Ref<ArrayMesh>());

								auto w = xform_uvs.write();
								Rect2 rc = entry["rrect"];
								for (int v = 0; v < uvs.size(); ++v) {
									w[v] = uvs[v] * rc.size + rc.position; // build atlas transformed coords
								}
								w.release();

								mesh_array[VS::ARRAY_TEX_UV] = xform_uvs;

								// save new surface:
								const int surf_id = r_mesh->get_surface_count();
								r_mesh->add_surface_from_arrays(p_mesh->surface_get_primitive_type(s), mesh_array);
								if (Ref<Material> mat = p_mesh->surface_get_material(s)) {
									r_mesh->surface_set_material(surf_id, mat);
								}
								r_mesh->surface_set_name(surf_id, p_mesh->surface_get_name(s));
							}
						}
					}
				}
			}
			// replace original texture with new atlas
			for (int s = 0; s < p_mesh->get_surface_count(); s++) {
				if (Ref<SpatialMaterial> mat = p_mesh->surface_get_material(s)) {
					if (Ref<Texture> texture = mat->get_texture(info[t].texture)) {
						mat->set_texture(info[t].texture, ResourceLoader::load(path));
					}
				}
			}
		}
	}

	return r_mesh;
}

static Error _parse_material_library(const String &p_path, Map<String, Ref<SpatialMaterial>> &p_material_map, List<String> *r_missing_deps) {
	FileAccessRef f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!f, ERR_CANT_OPEN, vformat("Couldn't open MTL file '%s', it may not exist or not be readable.", p_path));

	Ref<SpatialMaterial> current;
	String current_name;
	String base_path = p_path.get_base_dir();
	Dictionary meta;

	auto save_meta = [&]() {
		if (current.is_valid()) {
			current->set_meta("mtl", meta);
			meta = Dictionary();
		}
	};

	auto validate_texture_path = [&](const String &p) {
		String path = p;
		if (!ResourceLoader::exists(path)) {
			if (ResourceLoader::exists(base_path.plus_file(path))) {
				return base_path.plus_file(path);
			} else if (ResourceLoader::exists(path.get_file())) {
				return path.get_file();
			} else if (path.get_extension() == "bmp") {
				path = path.trim_suffix(path.get_extension()) + "png";
				if (ResourceLoader::exists(path)) {
					return path;
				} else if (ResourceLoader::exists(base_path.plus_file(path))) {
					return base_path.plus_file(path);
				} else if (ResourceLoader::exists(path.get_file())) {
					return path.get_file();
				}
			}
		}
		return p;
	};

	while (true) {
		String l = f->get_line().strip_edges();

		if (l.begins_with("newmtl ")) {
			//newmtl

			save_meta();

			current_name = l.replace("newmtl", "").strip_edges();
			current.instance();
			current->set_name(current_name);
			p_material_map[current_name] = current;
		} else if (l.begins_with("Ka ")) {
			//ambient
			WARN_PRINT("OBJ: Ambient light for material '" + current_name + "' is ignored in PBR");

			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_INVALID_DATA);
			meta["Ka"] = Color(v[1].to_float(), v[2].to_float(), v[3].to_float());
		} else if (l.begins_with("Kd ")) {
			//albedo
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_INVALID_DATA);
			Color c = current->get_albedo();
			c.r = v[1].to_float();
			c.g = v[2].to_float();
			c.b = v[3].to_float();
			current->set_albedo(c);
			meta["Kd"] = c;
		} else if (l.begins_with("Ks ")) {
			//metalness
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_INVALID_DATA);
			const float r = v[1].to_float();
			const float g = v[2].to_float();
			const float b = v[3].to_float();
			const float metalness = MAX(r, MAX(g, b));
			current->set_metallic(metalness);
			meta["Ks"] = metalness;
		} else if (l.begins_with("Ns ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			const float s = v[1].to_float();
			const float m = (1000.0 - s) / 1000.0;
			current->set_metallic(m);
			meta["Ns"] = m;
		} else if (l.begins_with("d ")) {
			//transparency
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			const float d = v[1].to_float();
			Color c = current->get_albedo();
			c.a = d;
			current->set_albedo(c);
			if (c.a < 0.99) {
				current->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
			}
			meta["Kd"] = c;
			meta["d"] = d;
		} else if (l.begins_with("Tr ")) {
			//transparency
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			const float d = v[1].to_float();
			Color c = current->get_albedo();
			c.a = 1.0 - d;
			current->set_albedo(c);
			if (c.a < 0.99) {
				current->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
			}
			meta["Kd"] = c;
			meta["Tr"] = d;
		} else if (l.begins_with("map_Ka ")) {
			//ambient texture
			WARN_PRINT("OBJ: Ambient light texture for material '" + current_name + "' is ignored in PBR");

		} else if (l.begins_with("map_Kd ")) {
			//albedo texture
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Kd", "").replace("\\", "/").strip_edges();
			String path = validate_texture_path(p);

			Ref<Texture> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(SpatialMaterial::TEXTURE_ALBEDO, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}

		} else if (l.begins_with("map_Ks ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Ks", "").replace("\\", "/").strip_edges();
			String path = validate_texture_path(p);

			Ref<Texture> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(SpatialMaterial::TEXTURE_METALLIC, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}

		} else if (l.begins_with("map_Ns ")) {
			//roughness texture
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Ns", "").replace("\\", "/").strip_edges();
			String path = validate_texture_path(p);

			Ref<Texture> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(SpatialMaterial::TEXTURE_ROUGHNESS, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}
		} else if (l.begins_with("map_bump ")) {
			//normalmap texture
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_bump", "").replace("\\", "/").strip_edges();
			String path = validate_texture_path(p);

			Ref<Texture> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_feature(SpatialMaterial::FEATURE_NORMAL_MAPPING, true);
				current->set_texture(SpatialMaterial::TEXTURE_NORMAL, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}
		} else if (f->eof_reached()) {
			break;
		}
	}

	// save outstanding meta:
	save_meta();

	return OK;
}

struct _parse_opt {
	bool single_mesh;
	bool generate_tangents;
	bool to_shadermaterial;
	bool fix_names;
	bool build_atlas_texture;
	int max_atlas_size;
	uint32_t compress_flags;
	Vector3 scale_mesh;
	Vector3 offset_mesh;
};

static Error _parse_obj(const String &p_path, List<Ref<Mesh>> &r_meshes, const _parse_opt &p_opts, List<String> *r_missing_deps) {
	FileAccessRef f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!f, ERR_CANT_OPEN, vformat("Couldn't open OBJ file '%s', it may not exist or not be readable.", p_path));

	Ref<ArrayMesh> mesh = memnew(ArrayMesh);

	Vector3 scale_mesh = p_opts.scale_mesh;
	Vector3 offset_mesh = p_opts.offset_mesh;

	LocalVector<String> instances_names;
	int unique_instances = 0;

	Vector<Vector3> vertices;
	Vector<Vector3> normals;
	Vector<Vector2> uvs;
	Vector<Color> colors;
	String name;

	Ref<SpatialMaterialConversionPlugin> spatial_mat_convert = memnew(SpatialMaterialConversionPlugin);

	Map<String, Ref<ShaderMaterial>> material_map_conv;
	Map<String, Map<String, Ref<SpatialMaterial>>> material_map;

	Ref<SurfaceTool> surf_tool = memnew(SurfaceTool);
	surf_tool->begin(Mesh::PRIMITIVE_TRIANGLES);

	const static String Found[2] = { "not found", "found" };

	String current_material_library, current_material;
	String current_group;
	String name;
	String last_name, last_material;
	int current_object_faces = 0;
	String comment;

	while (true) {
		String l = f->get_line().strip_edges();
		while (l.length() && l[l.length() - 1] == '\\') {
			String add = f->get_line().strip_edges();
			l += add;
			if (add == String()) {
				break;
			}
		}

		if (l.begins_with("#")) {
			comment += l.substr(1) + "\n";
		} else if (l.begins_with("v ")) {
			//vertex
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);
			Vector3 vtx;
			vtx.x = v[1].to_float() * scale_mesh.x + offset_mesh.x;
			vtx.y = v[2].to_float() * scale_mesh.y + offset_mesh.y;
			vtx.z = v[3].to_float() * scale_mesh.z + offset_mesh.z;
			vertices.push_back(vtx);
			//vertex colors
			if (v.size() >= 7) {
				while (colors.size() < vertices.size() - 1) {
					colors.push_back(Color(1.0, 1.0, 1.0));
				}
				Color c;
				c.r = v[4].to_float();
				c.g = v[5].to_float();
				c.b = v[6].to_float();
				colors.push_back(c);
			} else if (!colors.empty()) {
				colors.push_back(Color(1.0, 1.0, 1.0));
			}
		} else if (l.begins_with("vt ")) {
			//uv
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 3, ERR_FILE_CORRUPT);
			Vector2 uv;
			uv.x = v[1].to_float();
			uv.y = 1.0 - v[2].to_float();
			uvs.push_back(uv);

		} else if (l.begins_with("vn ")) {
			//normal
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);
			Vector3 nrm;
			nrm.x = v[1].to_float();
			nrm.y = v[2].to_float();
			nrm.z = v[3].to_float();
			normals.push_back(nrm);
		} else if (l.begins_with("f ")) {
			//vertex

			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);

			//not very fast, could be speed up

			Vector<String> face[3];
			face[0] = v[1].split("/");
			face[1] = v[2].split("/");

			ERR_FAIL_COND_V(face[0].size() == 0, ERR_FILE_CORRUPT);
			ERR_FAIL_COND_V(face[0].size() != face[1].size(), ERR_FILE_CORRUPT);

			for (int i = 2; i < v.size() - 1; i++) {
				face[2] = v[i + 1].split("/");

				ERR_FAIL_COND_V(face[0].size() != face[2].size(), ERR_FILE_CORRUPT);
				for (int j = 0; j < 3; j++) {
					int idx = j;

					if (idx < 2) {
						idx = 1 ^ idx;
					}

					if (face[idx].size() == 3) {
						int norm = face[idx][2].to_int() - 1;
						if (norm < 0) {
							norm += normals.size() + 1;
						}
						ERR_FAIL_INDEX_V(norm, normals.size(), ERR_FILE_CORRUPT);
						surf_tool->add_normal(normals[norm]);
					} else if (normals.size() && normals.size() == vertices.size()) {
						// Assume one normal per vertex
						int norm = face[idx][0].to_int() - 1;
						if (norm < 0) {
							norm += normals.size() + 1;
						}
						surf_tool->add_normal(normals[norm]);
					}

					if (face[idx].size() >= 2 && face[idx][1] != String()) {
						int uv = face[idx][1].to_int() - 1;
						if (uv < 0) {
							uv += uvs.size() + 1;
						}
						ERR_FAIL_INDEX_V(uv, uvs.size(), ERR_FILE_CORRUPT);
						surf_tool->add_uv(uvs[uv]);
					} else if (uvs.size() && uvs.size() == normals.size()) {
						// Assume one uv per vertex
						int uv = face[idx][0].to_int() - 1;
						if (uv < 0) {
							uv += uvs.size() + 1;
						}
						ERR_FAIL_INDEX_V(uv, uvs.size(), ERR_FILE_CORRUPT);
						surf_tool->add_uv(uvs[uv]);
					}

					int vtx = face[idx][0].to_int() - 1;
					if (vtx < 0) {
						vtx += vertices.size() + 1;
					}
					ERR_FAIL_INDEX_V(vtx, vertices.size(), ERR_FILE_CORRUPT);

					Vector3 vertex = vertices[vtx];
					if (!colors.empty()) {
						surf_tool->add_color(colors[vtx]);
					}
					//if (weld_vertices)
					//	vertex.snap(Vector3(weld_tolerance, weld_tolerance, weld_tolerance));
					surf_tool->add_vertex(vertex);
				}

				face[1] = face[2];
			}

			++current_object_faces;
		} else if (l.begins_with("s ")) { //smoothing
			String what = l.substr(2, l.length()).strip_edges();
			if (what == "off") {
				surf_tool->add_smooth_group(false);
			} else {
				surf_tool->add_smooth_group(true);
			}
		} else if (/*l.begins_with("g ") ||*/ l.begins_with("usemtl ") || (l.begins_with("o ") || f->eof_reached())) { //commit group to mesh
			//groups are too annoying
			if (current_object_faces || f->eof_reached()) { //another group going on or end, commit it
				if (normals.size() == 0) {
					surf_tool->generate_normals();
				}

				if (p_opts.generate_tangents && uvs.size()) {
					surf_tool->generate_tangents();
				}

				if (p_opts.compress_flags & VS::ARRAY_FLAG_USE_OCTAHEDRAL_COMPRESSION) {
					print_verbose("OBJ: Validating compression flags");
					const List<SurfaceTool::Vertex> &verts = surf_tool->get_vertex_array();
					if (surf_tool->get_array_format() & Mesh::ARRAY_FORMAT_NORMAL) {
						for (const SurfaceTool::Vertex &v : verts) {
							const float L1Norm = Math::absf(v.normal.x) + Math::absf(v.normal.y) + Math::absf(v.normal.z);
							if (Math::is_zero_approx(L1Norm)) {
								WARN_PRINT_ONCE("OBJ: Octahedral compression cannot be used to compress a zero-length normal vector");
								break;
							}
						}
					}
					if (surf_tool->get_array_format() & Mesh::ARRAY_FORMAT_TANGENT) {
						for (const SurfaceTool::Vertex &v : verts) {
							const float L1Norm = Math::absf(v.tangent.x) + Math::absf(v.tangent.y) + Math::absf(v.tangent.z);
							if (Math::is_zero_approx(L1Norm)) {
								WARN_PRINT_ONCE("OBJ: Octahedral compression cannot be used to compress a zero-length tangent vector");
								break;
							}
						}
					}
				}

				surf_tool->index();

				if (!current_material_library.empty()) {
					print_verbose("OBJ: Current material library " + current_material_library + " is " + Found[material_map.has(current_material_library)]);
					print_verbose("OBJ: Current material " + current_material + " is " + Found[material_map.has(current_material_library) && material_map[current_material_library].has(current_material)]);
				}

				if (material_map.has(current_material_library) && material_map[current_material_library].has(current_material)) {
					if (p_opts.to_shadermaterial) {
						String key = current_material_library + ":" + current_material;
						if (!material_map_conv.has(key)) {
							material_map[current_material_library][current_material]->flush_changes(); // materialize shader
							material_map_conv[key] = spatial_mat_convert->convert(material_map[current_material_library][current_material]);
							if (material_map[current_material_library][current_material]->has_meta("mtl")) {
								material_map_conv[key]->set_meta("mtl", material_map[current_material_library][current_material]->get_meta("mtl"));
							}
						}
						surf_tool->set_material(material_map_conv[key]);
					} else {
						Ref<SpatialMaterial> &material = material_map[current_material_library][current_material];
						if (!colors.empty()) {
							material->set_flag(SpatialMaterial::FLAG_SRGB_VERTEX_COLOR, true);
						}
						surf_tool->set_material(material);
					}
				}

				const int surf_id = mesh->get_surface_count();
				mesh = surf_tool->commit(mesh, p_opts.compress_flags);

				if (current_material != String()) {
					mesh->surface_set_name(surf_id, current_material);
				} else if (current_group != String()) {
					mesh->surface_set_name(surf_id, current_group);
				} else if (!current_material_library.empty()) {
					mesh->surface_set_name(surf_id, current_material_library.get_basename());
				}

				print_verbose("OBJ: Added surface: '" + mesh->surface_get_name(surf_id) + "' with " + itos(current_object_faces) + " faces to object: '" + name + "'");

				surf_tool->clear();
				surf_tool->begin(Mesh::PRIMITIVE_TRIANGLES);

				if (l.begins_with("o ") || f->eof_reached()) {
					if (p_opts.single_mesh) {
						if (instances_names.empty() || !instances_names.has(name)) {
							unique_instances++;
						}
						instances_names.push_back(name);
						last_name = name;
						last_material = current_material;
					} else {
						if (last_name != name || f->eof_reached()) {
							mesh->set_name(name);
							mesh->set_comment(comment);
							r_meshes.push_back(mesh);
							mesh.instance();
							name = "";
							last_name = name;
							last_material = current_material;
							current_group = "";
							current_material = "";
						}
					}
				}

				current_object_faces = 0;
			}

			if (l.begins_with("o ")) {
				name = l.substr(2, l.length()).strip_edges();
				if (name.empty()) {
					WARN_PRINT("Instance name is empty in: " + l);
				}
			}

			if (f->eof_reached()) {
				break;
			}

			if (l.begins_with("usemtl ")) {
				current_material = l.replace("usemtl", "").strip_edges();
				if (current_material.empty()) {
					WARN_PRINT("OBJ: Material name is empty in: " + l);
				}
			}

			if (l.begins_with("g ")) {
				current_group = l.substr(2, l.length()).strip_edges();
				if (current_group.empty()) {
					WARN_PRINT("OBJ: Group name is empty in: " + l);
				}
			}
		} else if (l.begins_with("mtllib ")) { //parse material
			current_material_library = l.replace("mtllib", "").strip_edges();
			if (!material_map.has(current_material_library)) {
				Map<String, Ref<SpatialMaterial>> lib;
				String lib_path = current_material_library;
				if (lib_path.is_rel_path()) {
					lib_path = p_path.get_base_dir().plus_file(current_material_library);
				}
				Error err = _parse_material_library(lib_path, lib, r_missing_deps);
				if (err != OK && FileAccess::exists(current_material_library)) {
					err = _parse_material_library(current_material_library, lib, r_missing_deps);
				}
				if (err == OK) {
					material_map[current_material_library] = lib;
				}
			}
		}
	}

	if (p_opts.single_mesh) {
		// final mesh name
		if (unique_instances == 1) {
			mesh->set_name(name.empty() ? p_path.get_basename() : name); // use last name value
		} else {
			mesh->set_name(p_path.get_basename());
			ERR_FAIL_COND_V(instances_names.size() != mesh->get_surface_count(), ERR_PARSE_ERROR);
			if (unique_instances != instances_names.size()) {
				if (p_opts.fix_names) { // check and rename duplicates per material
					Map<String, LocalVector<int>> dups;
					for (int s = 0; s < mesh->get_surface_count(); s++) {
						dups[instances_names[s] + "@" + mesh->surface_get_name(s)].push_back(s);
					}
					for (const auto *E = dups.front(); E; E = E->next()) {
						if (E->value().size() > 1) {
							print_verbose("OBJ: Rename surface " + E->key());
							for (int n = 0; n < E->value().size(); n++) {
								String new_name = instances_names[E->value()[n]];
								if (new_name.empty()) {
									new_name = "Mesh ";
								} else if (new_name.substr(-1).is_numeric()) {
									new_name += "_";
								} else {
									new_name += " ";
								}
								new_name += itos(n);
								instances_names[E->value()[n]] = new_name;
							}
						}
					}
				}
			}
			print_verbose("OBJ: New names of surfaces:");
			for (int s = 0; s < mesh->get_surface_count(); s++) { // rename surfaces
				print_verbose(" - " + instances_names[s]);
				mesh->surface_set_name(s, instances_names[s]);
			}
		}
		mesh->set_comment(comment);
		r_meshes.push_back(mesh); // final mesh
	}
	return OK;
}

Node *EditorOBJImporter::import_scene(const String &p_path, uint32_t p_flags, int p_bake_fps, uint32_t p_compress_flags, List<String> *r_missing_deps, Error *r_err) {
	List<Ref<Mesh>> meshes;

	const _parse_opt opts = { false, bool(p_flags & IMPORT_GENERATE_TANGENT_ARRAYS), false, false, 0, false, p_compress_flags, Vector3(1, 1, 1), Vector3(0, 0, 0) };
	Error err = _parse_obj(p_path, meshes, opts, r_missing_deps);

	if (err != OK) {
		if (r_err) {
			*r_err = err;
		}
		return nullptr;
	}

	Spatial *scene = memnew(Spatial);

	for (List<Ref<Mesh>>::Element *E = meshes.front(); E; E = E->next()) {
		MeshInstance *mi = memnew(MeshInstance);
		mi->set_mesh(E->get());
		mi->set_name(E->get()->get_name());
		scene->add_child(mi);
		mi->set_owner(scene);
	}

	if (r_err) {
		*r_err = OK;
	}

	return scene;
}
Ref<Animation> EditorOBJImporter::import_animation(const String &p_path, uint32_t p_flags, int p_bake_fps) {
	return Ref<Animation>();
}

void EditorOBJImporter::get_extensions(List<String> *r_extensions) const {
	r_extensions->push_back("obj");
}

EditorOBJImporter::EditorOBJImporter() {
}
////////////////////////////////////////////////////

static List<Ref<Mesh>> _array_to_list(const Array &p_array) {
	List<Ref<Mesh>> ret;
	for (int i = 0; i < p_array.size(); i++) {
		ret.push_back(p_array[i]);
	}
	return ret;
}

static Array _list_to_array(const List<Ref<Mesh>> &p_list) {
	Array ret;
	for (const List<Ref<Mesh>>::Element *E = p_list.front(); E; E = E->next()) {
		ret.append(E->get());
	}
	return ret;
}

String ResourceImporterOBJ::get_importer_name() const {
	return "wavefront_obj";
}
String ResourceImporterOBJ::get_visible_name() const {
	return "OBJ As Mesh";
}
void ResourceImporterOBJ::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("obj");
}
String ResourceImporterOBJ::get_save_extension() const {
	return "mesh";
}
String ResourceImporterOBJ::get_resource_type() const {
	return "Mesh";
}

int ResourceImporterOBJ::get_preset_count() const {
	return 0;
}
String ResourceImporterOBJ::get_preset_name(int p_idx) const {
	return "";
}

void ResourceImporterOBJ::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "generate_tangents"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::VECTOR3, "scale_mesh"), Vector3(1, 1, 1)));
	r_options->push_back(ImportOption(PropertyInfo(Variant::VECTOR3, "offset_mesh"), Vector3(0, 0, 0)));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "octahedral_compression"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "fix_submesh_names"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "merge_textures"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "max_atlas_size"), "2048"));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "optimize_mesh_flags", PROPERTY_HINT_FLAGS, "Vertex,Normal,Tangent,Color,TexUV,TexUV2,Bones,Weights,Index"), VS::ARRAY_COMPRESS_DEFAULT >> VS::ARRAY_COMPRESS_BASE));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "convert_to_shadermaterial"), false));

	List<String> script_extentions;
	ResourceLoader::get_recognized_extensions_for_type("Script", &script_extentions);

	String script_ext_hint;

	for (List<String>::Element *E = script_extentions.front(); E; E = E->next()) {
		if (script_ext_hint != "") {
			script_ext_hint += ",";
		}
		script_ext_hint += "*." + E->get();
	}

	r_options->push_back(ImportOption(PropertyInfo(Variant::STRING, "custom_script", PROPERTY_HINT_FILE, script_ext_hint), ""));
}
bool ResourceImporterOBJ::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return true;
}

Error ResourceImporterOBJ::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	List<Ref<Mesh>> meshes;

	uint32_t compress_flags = int(p_options["optimize_mesh_flags"]) << VS::ARRAY_COMPRESS_BASE;
	if (bool(p_options["octahedral_compression"])) {
		compress_flags |= VS::ARRAY_FLAG_USE_OCTAHEDRAL_COMPRESSION;
	}
	const _parse_opt opts = {
		true,
		p_options["generate_tangents"],
		p_options["convert_to_shadermaterial"],
		p_options["merge_textures"],
		p_options["fix_submesh_names"],
		p_options["max_atlas_size"],
		compress_flags,
		p_options["scale_mesh"],
		p_options["offset_mesh"],
	};
	Error err = _parse_obj(p_source_file, meshes, opts, nullptr);

	ERR_FAIL_COND_V(err != OK, err);
	ERR_FAIL_COND_V(meshes.size() != 1, ERR_BUG);

	String post_import_script_path = p_options["custom_script"];
	Ref<EditorOBJPostImport> post_import_script;

	if (post_import_script_path != "") {
		print_verbose("OBJ: Running Custom Script...");

		Ref<Script> scr = ResourceLoader::load(post_import_script_path);
		if (!scr.is_valid()) {
			print_verbose(vformat("OBJ: Couldn't load post-import script: %s", post_import_script_path));
		} else {
			post_import_script = Ref<EditorOBJPostImport>(memnew(EditorOBJPostImport));
			post_import_script->set_script(scr.get_ref_ptr());
			if (!post_import_script->get_script_instance()) {
				print_verbose(vformat("OBJ: Invalid/broken script for post-import: %s", post_import_script_path));
				post_import_script.unref();
				return ERR_CANT_CREATE;
			}
		}
	}

	if (post_import_script.is_valid()) {
		String base_path = p_save_path.get_base_dir();
		post_import_script->init(base_path, p_source_file);
		meshes = post_import_script->post_import(meshes);
	}

	ERR_FAIL_COND_V(meshes.size() != 1, ERR_BUG);

	Ref<Mesh> mesh = meshes.front()->get();
	if (opts.build_atlas_texture) {
		if (Ref<Mesh> _mesh = _build_mesh_for_atlas(meshes.front()->get(), p_source_file, opts.max_atlas_size, r_gen_files)) {
			mesh = _mesh; // new mesh with atlased texture
		}
	}

	String save_path = p_save_path + ".mesh";
	err = ResourceSaver::save(save_path, mesh);
	ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot save Mesh to file '" + save_path + "'.");

	r_gen_files->push_back(save_path);

	return OK;
}

ResourceImporterOBJ::ResourceImporterOBJ() {
}

/////////////////////////////////

void EditorOBJPostImport::_bind_methods() {
	BIND_VMETHOD(MethodInfo(Variant::ARRAY, "post_import", PropertyInfo(Variant::ARRAY, "meshes")));

	ClassDB::bind_method(D_METHOD("get_source_folder"), &EditorOBJPostImport::get_source_folder);
	ClassDB::bind_method(D_METHOD("get_source_file"), &EditorOBJPostImport::get_source_file);
}

List<Ref<Mesh>> EditorOBJPostImport::post_import(List<Ref<Mesh>> p_meshes) {
	if (get_script_instance()) {
		Variant::CallError ce;
		ce.error = Variant::CallError::CALL_OK;
		Variant vref = _list_to_array(p_meshes);
		const Variant *refp[] = { &vref };
		Variant ret = get_script_instance()->call("post_import", refp, 1, ce);

		if (ret.get_type() != Variant::ARRAY || ce.error != Variant::CallError::CALL_OK) {
			// script failed
			return p_meshes;
		}

		return _array_to_list(ret);
	}

	return p_meshes;
}

String EditorOBJPostImport::get_source_folder() const {
	return source_folder;
}

String EditorOBJPostImport::get_source_file() const {
	return source_file;
}

void EditorOBJPostImport::init(const String &p_source_folder, const String &p_source_file) {
	source_folder = p_source_folder;
	source_file = p_source_file;
}

EditorOBJPostImport::EditorOBJPostImport() {
}
