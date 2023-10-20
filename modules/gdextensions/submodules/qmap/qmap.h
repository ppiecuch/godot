/**************************************************************************/
/*  qmap.h                                                                */
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

#ifndef QODOT_QMAP_H
#define QODOT_QMAP_H

#include "core/dictionary.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "libmap/geo_generator.h"
#include "libmap/map_parser.h"
#include "libmap/surface_gatherer.h"

class Qodot : public Reference {
	GDCLASS(Qodot, Reference);

	std::shared_ptr<LMMapData> map_data = std::make_shared<LMMapData>();
	LMMapParser map_parser = LMMapParser(map_data);
	LMGeoGenerator geo_generator = LMGeoGenerator(map_data);
	LMSurfaceGatherer surface_gatherer = LMSurfaceGatherer(map_data);

	void gather_texture_surfaces_internal(const String p_texture_name, const String p_brush_filter_texture, const String p_face_filter_texture, bool p_filter_layers);
	void gather_convex_collision_surfaces(int64_t p_entity_idx, bool p_filter_layers);
	void gather_concave_collision_surfaces(int64_t p_entity_idx, bool p_filter_layers);

protected:
	static void _bind_methods();

public:
	void load_map(const String &map_file_str);
	PoolStringArray get_texture_list();
	void set_entity_definitions(Dictionary p_entity_defs);
	void set_worldspawn_layers(Array p_worldspawn_layers);
	void generate_geometry(Dictionary p_texture_dict);
	Array get_entity_dicts();
	Array get_worldspawn_layer_dicts();
	void gather_texture_surfaces(const String p_texture_name, const String p_brush_filter_texture, const String p_face_filter_texture);
	void gather_worldspawn_layer_surfaces(const String p_texture_name, const String p_brush_filter_texture, const String p_face_filter_texture);
	void gather_entity_convex_collision_surfaces(int64_t p_entity_idx);
	void gather_entity_concave_collision_surfaces(int64_t p_entity_idx);
	void gather_worldspawn_layer_collision_surfaces(int64_t p_entity_idx);
	Array fetch_surfaces(real_t p_inverse_scale_factor);
};

#endif // QODOT_QMAP_H
