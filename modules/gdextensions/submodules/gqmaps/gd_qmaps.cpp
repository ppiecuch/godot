/**************************************************************************/
/*  gd_qmaps.cpp                                                         */
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

#include "gd_qmaps.h"

#include "doctest/doctest.h"

#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "core/os/os.h"
#include "scene/resources/mesh.h"

#include "libmap/geo_generator.h"
#include "libmap/map_data.h"
#include "libmap/map_parser.h"
#include "libmap/surface_gatherer.h"

extern "C" {
#include "aa2map/aa2map.h"
#include "aa2map/aa2map_defines.h"
#include "aa2map/aa2map_misc.h"
#include "aa2map/aa2map_parse.h"
#include "aa2map/aa2map_write.h"
}

// C-callable logging functions for aa2map → Godot error system
extern "C" {

void aa2map_godot_log_error(const char *fmt, ...) {
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	// Strip trailing newlines
	int len = strlen(buf);
	while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
		buf[--len] = '\0';
	}
	ERR_PRINT(String::utf8(buf));
}

void aa2map_godot_log_info(const char *fmt, ...) {
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	int len = strlen(buf);
	while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
		buf[--len] = '\0';
	}
	print_line(String::utf8(buf));
}

} // extern "C"

// Reference:
// ----------
// 1. https://github.com/bnoordhuis/bspc
// 2. https://github.com/id-tech-3-tools/map-compiler
// 3. https://github.com/qbism/q2tools-220/tree/master

// === libmap API ===

void GdQMaps::load_map(const String &map_file_str) {
	const CharString map_file = map_file_str.utf8();
	map_parser.map_parser_load(map_file);
}

PoolStringArray GdQMaps::get_texture_list() {
	PoolStringArray g_textures;
	int tex_count = map_data->map_data_get_texture_count();
	LMTextureData *textures = map_data->map_data_get_textures();

	g_textures.resize(tex_count);

	for (int i = 0; i < tex_count; i++) {
		LMTextureData *texture = &textures[i];
		String g_name;
		g_name.parse_utf8(texture->name);
		g_textures.set(i, g_name);
	}

	return g_textures;
}

void GdQMaps::set_entity_definitions(Dictionary p_entity_defs) {
	for (int i = 0; i < p_entity_defs.size(); i++) {
		String key = p_entity_defs.get_key_at_index(i);
		int value = p_entity_defs.get_value_at_index(i).get("spawn_type");
		const CharString key_cs = key.utf8();
		map_parser.map_data->map_data_set_spawn_type_by_classname(key_cs, (ENTITY_SPAWN_TYPE)value);
	}
}

void GdQMaps::set_worldspawn_layers(Array p_worldspawn_layers) {
	for (int i = 0; i < p_worldspawn_layers.size(); i++) {
		Dictionary worldspawn_layer = p_worldspawn_layers.get(i);

		bool build_visuals = false;
		CharString texture = NULL;

		for (int k = 0; k < worldspawn_layer.size(); k++) {
			String key = worldspawn_layer.get_key_at_index(k);
			if (key == "texture") {
				String value = worldspawn_layer.get_value_at_index(k);
				texture = value.utf8();
			} else if (key == "build_visuals") {
				build_visuals = worldspawn_layer.get_value_at_index(k);
			}
		}
		map_data->map_data_register_worldspawn_layer(texture, build_visuals);
	}
}

void GdQMaps::generate_geometry(Dictionary p_texture_dict) {
	Array texture_keys = p_texture_dict.keys();
	for (int i = 0; i < texture_keys.size(); i++) {
		String texture_key = texture_keys.get(i);
		Vector2 value = p_texture_dict.get_value_at_index(i);

		const int width = value.x;
		const int height = value.y;

		CharString texture_key_cs = texture_key.utf8();

		map_data->map_data_set_texture_size(texture_key_cs, width, height);
	}
	geo_generator.geo_generator_run();
}

Array GdQMaps::get_entity_dicts() {
	int ent_count = map_data->map_data_get_entity_count();
	const LMEntity *ents = map_data->map_data_get_entities();

	Array ent_dicts;

	for (int i = 0; i < ent_count; i++) {
		const LMEntity *ent = &ents[i];
		Dictionary entity_dict;

		entity_dict["brush_count"] = ent->brush_count;

		PoolIntArray brush_indices;

		for (int b = 0; b < ent->brush_count; b++) {
			const LMBrush *brush = &ent->brushes[b];
			bool is_worldspawn_layer_brush = false;

			for (int f = 0; f < brush->face_count; f++) {
				const face *face = &brush->faces[f];

				if (map_data->map_data_find_worldspawn_layer(face->texture_idx) != -1) {
					is_worldspawn_layer_brush = true;
					break;
				}
			}
			if (!is_worldspawn_layer_brush) {
				brush_indices.append(b);
			}
		}

		entity_dict["brush_indices"] = brush_indices;

		entity_dict["center"] = Vector3(ent->center.y, ent->center.z, ent->center.x);

		Dictionary entity_properties;

		for (int p = 0; p < ent->property_count; p++) {
			LMProperty *prop = &ent->properties[p];
			entity_properties[String::utf8(prop->key)] = String::utf8(prop->value);
		}

		entity_dict["properties"] = entity_properties;

		ent_dicts.append(entity_dict);
	}

	return ent_dicts;
}

Array GdQMaps::get_worldspawn_layer_dicts() {
	const LMEntity *ents = map_data->map_data_get_entities();
	const LMEntity *worldspawn_entity = &ents[0];

	Array worldspawn_layer_dicts;

	if (worldspawn_entity == NULL) {
		return worldspawn_layer_dicts;
	}

	int layer_count = map_data->map_data_get_worldspawn_layer_count();
	const LMWorldspawnLayer *layers = map_data->map_data_get_worldspawn_layers();

	for (int l = 0; l < layer_count; l++) {
		const LMWorldspawnLayer *worldspawn_layer = &layers[l];

		Dictionary layer_dict;

		LMTextureData *tex_data = map_data->map_data_get_texture(worldspawn_layer->texture_idx);
		if (tex_data == NULL) {
			continue;
		}

		layer_dict["texture"] = String::utf8(tex_data->name);

		PoolIntArray brush_indices;

		for (int b = 0; b < worldspawn_entity->brush_count; ++b) {
			const LMBrush *brush = &worldspawn_entity->brushes[b];
			bool is_layer_brush = false;

			for (int f = 0; f < brush->face_count; ++f) {
				const face *face = &brush->faces[f];

				if (face->texture_idx == worldspawn_layer->texture_idx) {
					is_layer_brush = true;
					break;
				}
			}

			if (is_layer_brush) {
				brush_indices.append(b);
			}
		}

		layer_dict["brush_indices"] = brush_indices;

		worldspawn_layer_dicts.append(layer_dict);
	}

	return worldspawn_layer_dicts;
}

void GdQMaps::gather_texture_surfaces(const String p_texture_name, const String p_brush_filter_texture, const String p_face_filter_texture) {
	gather_texture_surfaces_internal(p_texture_name, p_brush_filter_texture, p_face_filter_texture, true);
}

void GdQMaps::gather_worldspawn_layer_surfaces(const String p_texture_name, const String p_brush_filter_texture, const String p_face_filter_texture) {
	gather_texture_surfaces_internal(p_texture_name, p_brush_filter_texture, p_face_filter_texture, false);
}

void GdQMaps::gather_texture_surfaces_internal(const String p_texture_name, const String p_brush_filter_texture, const String p_face_filter_texture, bool p_filter_layers) {
	CharString texture_name = p_texture_name.utf8();
	CharString brush_filter_texture = p_brush_filter_texture.utf8();
	CharString face_filter_texture = p_face_filter_texture.utf8();

	surface_gatherer.surface_gatherer_reset_params();
	surface_gatherer.surface_gatherer_set_split_type(SST_ENTITY);
	surface_gatherer.surface_gatherer_set_texture_filter(texture_name);
	surface_gatherer.surface_gatherer_set_brush_filter_texture(brush_filter_texture);
	surface_gatherer.surface_gatherer_set_face_filter_texture(face_filter_texture);
	surface_gatherer.surface_gatherer_set_worldspawn_layer_filter(p_filter_layers);

	surface_gatherer.surface_gatherer_run();
}

void GdQMaps::gather_entity_convex_collision_surfaces(int64_t p_entity_idx) {
	gather_convex_collision_surfaces(p_entity_idx, true);
}

void GdQMaps::gather_entity_concave_collision_surfaces(int64_t p_entity_idx) {
	gather_concave_collision_surfaces(p_entity_idx, true);
}

void GdQMaps::gather_worldspawn_layer_collision_surfaces(int64_t p_entity_idx) {
	gather_convex_collision_surfaces(p_entity_idx, false);
}

void GdQMaps::gather_convex_collision_surfaces(int64_t p_entity_idx, bool p_filter_layers) {
	surface_gatherer.surface_gatherer_reset_params();
	surface_gatherer.surface_gatherer_set_split_type(SST_BRUSH);
	surface_gatherer.surface_gatherer_set_entity_index_filter((int)p_entity_idx);
	surface_gatherer.surface_gatherer_set_worldspawn_layer_filter(p_filter_layers);

	surface_gatherer.surface_gatherer_run();
}

void GdQMaps::gather_concave_collision_surfaces(int64_t p_entity_idx, bool p_filter_layers) {
	surface_gatherer.surface_gatherer_reset_params();
	surface_gatherer.surface_gatherer_set_split_type(SST_NONE);
	surface_gatherer.surface_gatherer_set_entity_index_filter((int)p_entity_idx);
	surface_gatherer.surface_gatherer_set_worldspawn_layer_filter(p_filter_layers);

	surface_gatherer.surface_gatherer_run();
}

Array GdQMaps::fetch_surfaces(real_t p_inverse_scale_factor) {
	const LMSurfaces *surfs = surface_gatherer.surface_gatherer_fetch();

	Array surf_array;
	Variant v_nil;

	Vector3 gv3;
	Vector2 gv2;

	for (int s = 0; s < surfs->surface_count; ++s) {
		LMSurface *surf = &surfs->surfaces[s];

		if (surf->vertex_count == 0) {
			surf_array.append(v_nil);
			continue;
		}

		PoolVector3Array vertices;
		for (int v = 0; v < surf->vertex_count; ++v) {
			gv3 = Vector3(surf->vertices[v].vertex.y, surf->vertices[v].vertex.z, surf->vertices[v].vertex.x);
			gv3 = gv3 / p_inverse_scale_factor;
			vertices.append(gv3);
		}

		PoolVector3Array normals;
		for (int v = 0; v < surf->vertex_count; ++v) {
			gv3 = Vector3(surf->vertices[v].normal.y, surf->vertices[v].normal.z, surf->vertices[v].normal.x);
			normals.append(gv3);
		}

		PoolRealArray tangents;
		for (int v = 0; v < surf->vertex_count; v++) {
			tangents.append(surf->vertices[v].tangent.y);
			tangents.append(surf->vertices[v].tangent.z);
			tangents.append(surf->vertices[v].tangent.x);
			tangents.append(surf->vertices[v].tangent.w);
		}

		PoolVector2Array uvs;

		for (int v = 0; v < surf->vertex_count; v++) {
			gv2 = Vector2(surf->vertices[v].uv.u, surf->vertices[v].uv.v);
			uvs.append(gv2);
		}

		PoolIntArray indices;
		for (int i = 0; i < surf->index_count; i++) {
			indices.append(surf->indices[i]);
		}

		Array brush_array;
		brush_array.resize(Mesh::ArrayType::ARRAY_MAX);
		brush_array.fill(v_nil);

		brush_array[Mesh::ArrayType::ARRAY_VERTEX] = vertices;
		brush_array[Mesh::ArrayType::ARRAY_NORMAL] = normals;
		brush_array[Mesh::ArrayType::ARRAY_TANGENT] = tangents;
		brush_array[Mesh::ArrayType::ARRAY_TEX_UV] = uvs;
		brush_array[Mesh::ArrayType::ARRAY_INDEX] = indices;

		surf_array.append(brush_array);
	}
	return surf_array;
}

// === aa2map API ===

static void _aa2map_init_defaults(st_aa2map_t *aa2map) {
	memset(aa2map, 0, sizeof(st_aa2map_t));
	aa2map->map_file = NULL;
	aa2map->shader_file = NULL;
	aa2map->xscale = aa2map->yscale = aa2map->zscale = 128;
	aa2map->type = AA2MAP_IDTECH3;
	strcpy(aa2map->path, AA2MAP_PATH_DEFAULT);
	strncpy(aa2map->ascii_chars, AA2MAP_DEFAULT_ASCII_CHARS_S, AA2MAP_MAX_ASCII_CHARS);
	aa2map->ascii_chars[AA2MAP_MAX_ASCII_CHARS - 1] = 0;
	aa2map->xmaze = 10;
	aa2map->ymaze = 20;
	aa2map->pads = 10;
	aa2map->gap_initial = 220;
	aa2map->gap_increment = 40;
	aa2map->run_len = 20000;
	aa2map->checkpoints = 6;
}

static void _aa2map_cleanup(st_aa2map_t *aa2map) {
	if (aa2map->map_file && aa2map->map_file != stdout) {
		fclose(aa2map->map_file);
		aa2map->map_file = NULL;
	}
	if (aa2map->shader_file && aa2map->shader_file != stdout) {
		fclose(aa2map->shader_file);
		aa2map->shader_file = NULL;
	}
}

static void _apply_aa2map_options(st_aa2map_t *aa2map, const Dictionary &p_options) {
	Array keys = p_options.keys();
	for (int i = 0; i < keys.size(); i++) {
		String key = keys[i];

		if (key == "scale") {
			Vector3 scale = p_options[key];
			aa2map->xscale = (int)scale.x;
			aa2map->yscale = (int)scale.y;
			aa2map->zscale = (int)scale.z;
		} else if (key == "path") {
			String path = p_options[key];
			CharString cs = path.utf8();
			strncpy(aa2map->path, cs.get_data(), FILENAME_MAX);
			aa2map->path[FILENAME_MAX - 1] = 0;
		} else if (key == "mirror") {
			String mirror = p_options[key];
			if (mirror.length() > 0) {
				char c = (char)mirror.ord_at(0);
				if (c == 'n' || c == 'e' || c == 's' || c == 'w') {
					aa2map->mirror = c;
				}
			}
		} else if (key == "hflip") {
			bool hflip = p_options[key];
			aa2map->hflip = hflip ? 1 : 0;
		} else if (key == "vflip") {
			bool vflip = p_options[key];
			aa2map->vflip = vflip ? 1 : 0;
		} else if (key == "gravity") {
			aa2map->gravity = (int)p_options[key];
		} else if (key == "fog") {
			Color fog = p_options[key];
			aa2map->fog = ((unsigned long)(fog.r * 255) << 24) |
					((unsigned long)(fog.g * 255) << 16) |
					((unsigned long)(fog.b * 255) << 8) |
					((unsigned long)(fog.a * 255));
		} else if (key == "light") {
			aa2map->light = (int)p_options[key];
		} else if (key == "message") {
			String message = p_options[key];
			CharString cs = message.utf8();
			strncpy(aa2map->message, cs.get_data(), MAXBUFSIZE);
			aa2map->message[MAXBUFSIZE - 1] = 0;
		}
	}
}

String GdQMaps::generate_map_from_ascii(Array p_ascii_layers, Dictionary p_options) {
	if (p_ascii_layers.empty()) {
		ERR_PRINT("GdQMaps::generate_map_from_ascii: empty ascii_layers array");
		return String();
	}

	String temp_dir = OS::get_singleton()->get_cache_path();

	// Write each ASCII layer to a temp file
	Vector<String> temp_input_paths;
	for (int i = 0; i < p_ascii_layers.size(); i++) {
		String ascii_text = p_ascii_layers[i];
		String temp_path = temp_dir.plus_file(vformat("gd_qmaps_layer_%d.txt", i));

		FileAccess *f = FileAccess::open(temp_path, FileAccess::WRITE);
		if (!f) {
			ERR_PRINT("GdQMaps: could not create temp file: " + temp_path);
			for (int j = 0; j < temp_input_paths.size(); j++) {
				DirAccess::remove_file_or_error(temp_input_paths[j]);
			}
			return String();
		}
		f->store_string(ascii_text);
		f->close();
		memdelete(f);
		temp_input_paths.push_back(temp_path);
	}

	String output_path = temp_dir.plus_file("gd_qmaps_output.map");

	st_aa2map_t aa2map;
	_aa2map_init_defaults(&aa2map);
	_apply_aa2map_options(&aa2map, p_options);

	// Set input files
	Vector<CharString> input_cs;
	for (int i = 0; i < temp_input_paths.size() && i < ARGS_MAX; i++) {
		input_cs.push_back(temp_input_paths[i].utf8());
	}
	for (int i = 0; i < input_cs.size(); i++) {
		aa2map.input_file[i] = input_cs[i].get_data();
	}

	// Set output file
	CharString output_cs = output_path.utf8();
	strncpy(aa2map.map_file_s, output_cs.get_data(), FILENAME_MAX);
	aa2map.map_file_s[FILENAME_MAX - 1] = 0;
	aa2map.map_file = fopen(aa2map.map_file_s, "w");
	if (!aa2map.map_file) {
		ERR_PRINT("GdQMaps: could not open output file: " + output_path);
		for (int i = 0; i < temp_input_paths.size(); i++) {
			DirAccess::remove_file_or_error(temp_input_paths[i]);
		}
		return String();
	}

	if (!aa2map.message[0]) {
		strncpy(aa2map.message, "Generated by GdQMaps", MAXBUFSIZE);
	}

	st_aa2map_parse_t *parsed = aa2map_parse(&aa2map);
	if (parsed) {
		aa2map_map_write(&aa2map, parsed);
		free(parsed);
	}
	_aa2map_cleanup(&aa2map);

	// Cleanup temp input files
	for (int i = 0; i < temp_input_paths.size(); i++) {
		DirAccess::remove_file_or_error(temp_input_paths[i]);
	}

	if (!parsed) {
		ERR_PRINT("GdQMaps: aa2map_parse() failed");
		return String();
	}

	return output_path;
}

void GdQMaps::load_ascii_map(Array p_ascii_layers, Dictionary p_options) {
	String map_path = generate_map_from_ascii(p_ascii_layers, p_options);
	if (map_path.empty()) {
		return;
	}
	load_map(map_path);
	DirAccess::remove_file_or_error(map_path);
}

void GdQMaps::load_maze(int p_width, int p_height, Dictionary p_options) {
	if (p_width < 2 || p_height < 2) {
		ERR_PRINT("GdQMaps::load_maze: width and height must be >= 2");
		return;
	}

	String temp_dir = OS::get_singleton()->get_cache_path();
	String maze_ascii_path = temp_dir.plus_file("gd_qmaps_maze.txt");
	String output_path = temp_dir.plus_file("gd_qmaps_maze.map");

	st_aa2map_t aa2map;
	_aa2map_init_defaults(&aa2map);
	_apply_aa2map_options(&aa2map, p_options);

	// mazegen writes an ASCII file to map_file
	CharString maze_cs = maze_ascii_path.utf8();
	strncpy(aa2map.map_file_s, maze_cs.get_data(), FILENAME_MAX);
	aa2map.map_file_s[FILENAME_MAX - 1] = 0;
	aa2map.map_file = fopen(aa2map.map_file_s, "w");
	if (!aa2map.map_file) {
		ERR_PRINT("GdQMaps: could not open maze temp file");
		return;
	}

	aa2map_mazegen(&aa2map, p_width, p_height);
	_aa2map_cleanup(&aa2map);

	// Now parse the generated ASCII and produce a .map
	st_aa2map_t aa2map2;
	_aa2map_init_defaults(&aa2map2);
	_apply_aa2map_options(&aa2map2, p_options);

	CharString maze_ascii_cs = maze_ascii_path.utf8();
	aa2map2.input_file[0] = maze_ascii_cs.get_data();

	CharString output_cs = output_path.utf8();
	strncpy(aa2map2.map_file_s, output_cs.get_data(), FILENAME_MAX);
	aa2map2.map_file_s[FILENAME_MAX - 1] = 0;
	aa2map2.map_file = fopen(aa2map2.map_file_s, "w");
	if (!aa2map2.map_file) {
		ERR_PRINT("GdQMaps: could not open maze output file");
		DirAccess::remove_file_or_error(maze_ascii_path);
		return;
	}

	if (!aa2map2.message[0]) {
		strncpy(aa2map2.message, "Maze generated by GdQMaps", MAXBUFSIZE);
	}

	st_aa2map_parse_t *parsed = aa2map_parse(&aa2map2);
	if (parsed) {
		aa2map_map_write(&aa2map2, parsed);
		free(parsed);
	}
	_aa2map_cleanup(&aa2map2);
	DirAccess::remove_file_or_error(maze_ascii_path);

	if (FileAccess::exists(output_path)) {
		load_map(output_path);
		DirAccess::remove_file_or_error(output_path);
	}
}

void GdQMaps::load_demo(Dictionary p_options) {
	// Built-in demo level — a small arena with various entities.
	// Each line is one row of the ASCII art (no newlines in the original data),
	// so we join them with newlines for aa2map.
	static const char *demo_ascii =
			"LLLLLLLLLLLLLLLLLLLLLWLLLWLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLWWLLWLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLWLWLWLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLWLLWWLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLWLLLWLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLL                                    LLLLL\n"
			"WLLLW 0123 !@# k .:; b   T J _a  /   W m WWWWW\n"
			"WLLLW  456 $%^ x efs r X     il /W/      WLLLL\n"
			"WLWLW  789 &*    hHM n        d L/L  g ~ WWWWL\n"
			"WWLWW  ABC       +IQ          D LLL      WLLLL\n"
			"WLLLW            R       t j    LLL      WWWWW\n"
			"LLLLL                           LLL      LLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLWWWWWLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLWLLLLLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLWWWWWLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLLLLLWLLLLLLLLLLLLLLLLLLLL\n"
			"LLLLLLLLLLLLLLLLLLLLLWWWWWLLLLLLLLLLLLLLLLLLLL\n";
	Array layers;
	layers.push_back(String::utf8(demo_ascii));
	load_ascii_map(layers, p_options);
}

void GdQMaps::_bind_methods() {
	// libmap API
	ClassDB::bind_method(D_METHOD("load_map", "map_file"), &GdQMaps::load_map);
	ClassDB::bind_method(D_METHOD("get_texture_list"), &GdQMaps::get_texture_list);
	ClassDB::bind_method(D_METHOD("set_entity_definitions", "entity_defs"), &GdQMaps::set_entity_definitions);
	ClassDB::bind_method(D_METHOD("set_worldspawn_layers", "worldspawn_layers"), &GdQMaps::set_worldspawn_layers);
	ClassDB::bind_method(D_METHOD("generate_geometry", "texture_size_dict"), &GdQMaps::generate_geometry);
	ClassDB::bind_method(D_METHOD("get_entity_dicts"), &GdQMaps::get_entity_dicts);
	ClassDB::bind_method(D_METHOD("get_worldspawn_layer_dicts"), &GdQMaps::get_worldspawn_layer_dicts);
	ClassDB::bind_method(D_METHOD("gather_texture_surfaces", "texture_name", "brush_filter_texture", "face_filter_texture"), &GdQMaps::gather_texture_surfaces);
	ClassDB::bind_method(D_METHOD("gather_worldspawn_layer_surfaces", "texture_name", "brush_filter_texture", "face_filter_texture"), &GdQMaps::gather_worldspawn_layer_surfaces);
	ClassDB::bind_method(D_METHOD("gather_entity_convex_collision_surfaces", "entity_idx"), &GdQMaps::gather_entity_convex_collision_surfaces);
	ClassDB::bind_method(D_METHOD("gather_entity_concave_collision_surfaces", "entity_idx"), &GdQMaps::gather_entity_concave_collision_surfaces);
	ClassDB::bind_method(D_METHOD("gather_worldspawn_layer_collision_surfaces", "entity_idx"), &GdQMaps::gather_worldspawn_layer_collision_surfaces);
	ClassDB::bind_method(D_METHOD("fetch_surfaces", "inverse_scale_factor"), &GdQMaps::fetch_surfaces);

	// aa2map API
	ClassDB::bind_method(D_METHOD("generate_map_from_ascii", "ascii_layers", "options"), &GdQMaps::generate_map_from_ascii, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("load_ascii_map", "ascii_layers", "options"), &GdQMaps::load_ascii_map, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("load_maze", "width", "height", "options"), &GdQMaps::load_maze, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("load_demo", "options"), &GdQMaps::load_demo, DEFVAL(Dictionary()));
}

// === Doctest Tests ===

#ifdef DOCTEST

static const char *SIMPLE_BOX_MAP =
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"{\n"
		"( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) TEXTURE 0 0 0 1 1\n"
		"( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) TEXTURE 0 0 0 1 1\n"
		"( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) TEXTURE 0 0 0 1 1\n"
		"( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) TEXTURE 0 0 0 1 1\n"
		"( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) TEXTURE 0 0 0 1 1\n"
		"( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) TEXTURE 0 0 0 1 1\n"
		"}\n"
		"}\n";

static const char *TWO_ENTITY_MAP =
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"{\n"
		"( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) TEXTURE 0 0 0 1 1\n"
		"( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) TEXTURE 0 0 0 1 1\n"
		"( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) TEXTURE 0 0 0 1 1\n"
		"( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) TEXTURE 0 0 0 1 1\n"
		"( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) TEXTURE 0 0 0 1 1\n"
		"( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) TEXTURE 0 0 0 1 1\n"
		"}\n"
		"}\n"
		"{\n"
		"\"classname\" \"info_player_start\"\n"
		"\"origin\" \"0 0 0\"\n"
		"}\n";

static const char *TWO_TEXTURE_MAP =
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"{\n"
		"( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) TEX_A 0 0 0 1 1\n"
		"( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) TEX_A 0 0 0 1 1\n"
		"( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) TEX_B 0 0 0 1 1\n"
		"( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) TEX_B 0 0 0 1 1\n"
		"( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) TEX_A 0 0 0 1 1\n"
		"( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) TEX_B 0 0 0 1 1\n"
		"}\n"
		"}\n";

static const char *EXAMPLE_ASCII =
		"LLLLLLLLLLLLLLLLLLLLLWLLLWLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLWWLLWLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLWLWLWLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLWLLWWLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLWLLLWLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLL                                    LLLLL\n"
		"WLLLW 0123 !@# k .:; b   T J _a  /   W m WWWWW\n"
		"WLLLW  456 $%^ x efs r X     il /W/      WLLLL\n"
		"WLWLW  789 &*    hHM n        d L/L  g ~ WWWWL\n"
		"WWLWW  ABC       +IQ          D LLL      WLLLL\n"
		"WLLLW            R       t j    LLL      WWWWW\n"
		"LLLLL                           LLL      LLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLWWWWWLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLWLLLLLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLWWWWWLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLLLLLWLLLLLLLLLLLLLLLLLLLL\n"
		"LLLLLLLLLLLLLLLLLLLLLWWWWWLLLLLLLLLLLLLLLLLLLL\n";

static String _write_temp_map(const char *content) {
	String temp_dir = OS::get_singleton()->get_cache_path();
	String temp_path = temp_dir.plus_file("gd_qmaps_test.map");
	FileAccess *f = FileAccess::open(temp_path, FileAccess::WRITE);
	REQUIRE(f != nullptr);
	f->store_string(String::utf8(content));
	f->close();
	memdelete(f);
	return temp_path;
}

TEST_SUITE("[[gqmaps]] GdQMaps") {
	// --- libmap tests ---

	TEST_CASE("[gqmaps] default state") {
		Ref<GdQMaps> q;
		q.instance();
		REQUIRE(q.is_valid());
		CHECK(q->get_texture_list().size() == 0);
		CHECK(q->get_entity_dicts().size() == 0);
	}

	TEST_CASE("[gqmaps] load invalid file") {
		Ref<GdQMaps> q;
		q.instance();
		q->load_map("/tmp/gd_qmaps_nonexistent_test_12345.map");
		CHECK(q->get_texture_list().size() == 0);
		CHECK(q->get_entity_dicts().size() == 0);
	}

	TEST_CASE("[gqmaps] load simple box map") {
		String path = _write_temp_map(SIMPLE_BOX_MAP);
		Ref<GdQMaps> q;
		q.instance();
		q->load_map(path);
		DirAccess::remove_file_or_error(path);

		PoolStringArray textures = q->get_texture_list();
		CHECK(textures.size() == 1);
		if (textures.size() > 0) {
			CHECK(textures[0] == "TEXTURE");
		}

		Array ents = q->get_entity_dicts();
		CHECK(ents.size() == 1);
		if (ents.size() > 0) {
			Dictionary ent = ents[0];
			CHECK((int)ent["brush_count"] == 1);
			Dictionary props = ent["properties"];
			CHECK(String(props["classname"]) == "worldspawn");
		}
	}

	TEST_CASE("[gqmaps] generate geometry from box") {
		String path = _write_temp_map(SIMPLE_BOX_MAP);
		Ref<GdQMaps> q;
		q.instance();
		q->load_map(path);
		DirAccess::remove_file_or_error(path);

		Dictionary tex_dict;
		tex_dict["TEXTURE"] = Vector2(64, 64);
		q->generate_geometry(tex_dict);

		q->gather_texture_surfaces("TEXTURE", "", "");
		Array surfaces = q->fetch_surfaces(1.0);
		CHECK(surfaces.size() > 0);

		if (surfaces.size() > 0 && surfaces[0].get_type() == Variant::ARRAY) {
			Array brush_array = surfaces[0];
			CHECK(brush_array.size() == Mesh::ArrayType::ARRAY_MAX);
			PoolVector3Array verts = brush_array[Mesh::ArrayType::ARRAY_VERTEX];
			CHECK(verts.size() > 0);
			PoolVector3Array norms = brush_array[Mesh::ArrayType::ARRAY_NORMAL];
			CHECK(norms.size() > 0);
			PoolVector2Array uvs = brush_array[Mesh::ArrayType::ARRAY_TEX_UV];
			CHECK(uvs.size() > 0);
			PoolIntArray indices = brush_array[Mesh::ArrayType::ARRAY_INDEX];
			CHECK(indices.size() > 0);
		}
	}

	TEST_CASE("[gqmaps] multiple entities") {
		String path = _write_temp_map(TWO_ENTITY_MAP);
		Ref<GdQMaps> q;
		q.instance();
		q->load_map(path);
		DirAccess::remove_file_or_error(path);

		Array ents = q->get_entity_dicts();
		CHECK(ents.size() == 2);
		if (ents.size() >= 2) {
			Dictionary ent1 = ents[1];
			Dictionary props = ent1["properties"];
			CHECK(String(props["classname"]) == "info_player_start");
		}
	}

	TEST_CASE("[gqmaps] multiple textures") {
		String path = _write_temp_map(TWO_TEXTURE_MAP);
		Ref<GdQMaps> q;
		q.instance();
		q->load_map(path);
		DirAccess::remove_file_or_error(path);

		PoolStringArray textures = q->get_texture_list();
		CHECK(textures.size() == 2);
		bool has_a = false, has_b = false;
		for (int i = 0; i < textures.size(); i++) {
			if (textures[i] == "TEX_A") {
				has_a = true;
			}
			if (textures[i] == "TEX_B") {
				has_b = true;
			}
		}
		CHECK(has_a);
		CHECK(has_b);
	}

	TEST_CASE("[gqmaps] convex collision surfaces") {
		String path = _write_temp_map(SIMPLE_BOX_MAP);
		Ref<GdQMaps> q;
		q.instance();
		q->load_map(path);
		DirAccess::remove_file_or_error(path);

		Dictionary tex_dict;
		tex_dict["TEXTURE"] = Vector2(64, 64);
		q->generate_geometry(tex_dict);

		q->gather_entity_convex_collision_surfaces(0);
		Array surfaces = q->fetch_surfaces(1.0);
		CHECK(surfaces.size() > 0);
	}

	TEST_CASE("[gqmaps] concave collision surfaces") {
		String path = _write_temp_map(SIMPLE_BOX_MAP);
		Ref<GdQMaps> q;
		q.instance();
		q->load_map(path);
		DirAccess::remove_file_or_error(path);

		Dictionary tex_dict;
		tex_dict["TEXTURE"] = Vector2(64, 64);
		q->generate_geometry(tex_dict);

		q->gather_entity_concave_collision_surfaces(0);
		Array surfaces = q->fetch_surfaces(1.0);
		CHECK(surfaces.size() > 0);
	}

	TEST_CASE("[gqmaps] inverse scale factor") {
		String path = _write_temp_map(SIMPLE_BOX_MAP);
		Ref<GdQMaps> q;
		q.instance();
		q->load_map(path);
		DirAccess::remove_file_or_error(path);

		Dictionary tex_dict;
		tex_dict["TEXTURE"] = Vector2(64, 64);
		q->generate_geometry(tex_dict);

		q->gather_texture_surfaces("TEXTURE", "", "");
		Array surfaces_1x = q->fetch_surfaces(1.0);

		q->gather_texture_surfaces("TEXTURE", "", "");
		Array surfaces_2x = q->fetch_surfaces(2.0);

		if (surfaces_1x.size() > 0 && surfaces_1x[0].get_type() == Variant::ARRAY &&
				surfaces_2x.size() > 0 && surfaces_2x[0].get_type() == Variant::ARRAY) {
			Array arr_1x = surfaces_1x[0];
			Array arr_2x = surfaces_2x[0];
			PoolVector3Array verts_1x = arr_1x[Mesh::ArrayType::ARRAY_VERTEX];
			PoolVector3Array verts_2x = arr_2x[Mesh::ArrayType::ARRAY_VERTEX];
			REQUIRE(verts_1x.size() > 0);
			REQUIRE(verts_2x.size() > 0);
			// Find a non-zero vertex to compare scale ratios
			for (int i = 0; i < verts_1x.size() && i < verts_2x.size(); i++) {
				if (verts_1x[i].length() > 0.1f) {
					CHECK(verts_1x[i].x == doctest::Approx(verts_2x[i].x * 2.0f).epsilon(0.01));
					CHECK(verts_1x[i].y == doctest::Approx(verts_2x[i].y * 2.0f).epsilon(0.01));
					CHECK(verts_1x[i].z == doctest::Approx(verts_2x[i].z * 2.0f).epsilon(0.01));
					break;
				}
			}
		}
	}

	// --- aa2map tests ---

	TEST_CASE("[gqmaps] generate_map_from_ascii basic") {
		Ref<GdQMaps> q;
		q.instance();

		Array layers;
		layers.push_back(String("XXX\nX X\nXXX\n"));

		String result = q->generate_map_from_ascii(layers, Dictionary());
		CHECK(!result.empty());
		if (!result.empty()) {
			CHECK(FileAccess::exists(result));
			DirAccess::remove_file_or_error(result);
		}
	}

	TEST_CASE("[gqmaps] generate_map_from_ascii empty") {
		Ref<GdQMaps> q;
		q.instance();

		Array empty_layers;
		String result = q->generate_map_from_ascii(empty_layers, Dictionary());
		CHECK(result.empty());
	}

	TEST_CASE("[gqmaps] load_ascii_map full pipeline") {
		Ref<GdQMaps> q;
		q.instance();

		Array layers;
		layers.push_back(String("XXXXX\nX   X\nX   X\nX   X\nXXXXX\n"));

		q->load_ascii_map(layers, Dictionary());
		Array ents = q->get_entity_dicts();
		CHECK(ents.size() >= 1);
	}

	TEST_CASE("[gqmaps] aa2map options scale") {
		Ref<GdQMaps> q;
		q.instance();

		Array layers;
		layers.push_back(String("XXX\nX X\nXXX\n"));

		Dictionary opts;
		opts["scale"] = Vector3(64, 64, 64);
		opts["path"] = "test/texture";

		String result = q->generate_map_from_ascii(layers, opts);
		CHECK(!result.empty());
		if (!result.empty()) {
			DirAccess::remove_file_or_error(result);
		}
	}

	TEST_CASE("[gqmaps] aa2map example level") {
		Ref<GdQMaps> q;
		q.instance();

		Array layers;
		layers.push_back(String::utf8(EXAMPLE_ASCII));

		q->load_ascii_map(layers, Dictionary());
		Array ents = q->get_entity_dicts();
		CHECK(ents.size() >= 1);
	}
}

#endif // DOCTEST
