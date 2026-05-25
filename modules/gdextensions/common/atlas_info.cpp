/**************************************************************************/
/*  atlas_info.cpp                                                        */
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

#include "atlas_info.h"

#include "servers/visual_server.h"

// ---------------------------------------------------------------------------
// AtlasInfo
// ---------------------------------------------------------------------------

bool AtlasInfo::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;

	if (name == "item_count") {
		set_item_count(p_value);
		return true;
	}

	if (name.begins_with("items/")) {
		int idx = name.get_slicec('/', 1).to_int();
		String prop = name.get_slicec('/', 2);

		if (idx >= items.size())
			items.resize(idx + 1);

		if (prop == "name") {
			items.write[idx].name = p_value;
		} else if (prop == "rect") {
			items.write[idx].rect = p_value;
		} else if (prop == "rrect") {
			items.write[idx].rrect = p_value;
		} else if (prop == "atlas_page") {
			items.write[idx].atlas_page = p_value;
		} else if (prop == "flipped") {
			items.write[idx].flipped = p_value;
		} else if (prop == "trim_l") {
			items.write[idx].trim_l = p_value;
		} else if (prop == "trim_r") {
			items.write[idx].trim_r = p_value;
		} else if (prop == "trim_t") {
			items.write[idx].trim_t = p_value;
		} else if (prop == "trim_b") {
			items.write[idx].trim_b = p_value;
		} else if (prop == "hull") {
			items.write[idx].hull = p_value;
		} else if (prop == "hull_indices") {
			items.write[idx].hull_indices = p_value;
		} else {
			return false;
		}
		return true;
	}

	return false;
}

bool AtlasInfo::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;

	if (name == "item_count") {
		r_ret = items.size();
		return true;
	}

	if (name.begins_with("items/")) {
		int idx = name.get_slicec('/', 1).to_int();
		String prop = name.get_slicec('/', 2);

		ERR_FAIL_INDEX_V(idx, items.size(), false);

		if (prop == "name") {
			r_ret = items[idx].name;
		} else if (prop == "rect") {
			r_ret = items[idx].rect;
		} else if (prop == "rrect") {
			r_ret = items[idx].rrect;
		} else if (prop == "atlas_page") {
			r_ret = items[idx].atlas_page;
		} else if (prop == "flipped") {
			r_ret = items[idx].flipped;
		} else if (prop == "trim_l") {
			r_ret = items[idx].trim_l;
		} else if (prop == "trim_r") {
			r_ret = items[idx].trim_r;
		} else if (prop == "trim_t") {
			r_ret = items[idx].trim_t;
		} else if (prop == "trim_b") {
			r_ret = items[idx].trim_b;
		} else if (prop == "hull") {
			r_ret = items[idx].hull;
		} else if (prop == "hull_indices") {
			r_ret = items[idx].hull_indices;
		} else {
			return false;
		}
		return true;
	}

	return false;
}

void AtlasInfo::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "item_count", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));

	for (int i = 0; i < items.size(); ++i) {
		String prefix = "items/" + itos(i) + "/";
		int usage = PROPERTY_USAGE_STORAGE;
		p_list->push_back(PropertyInfo(Variant::STRING, prefix + "name", PROPERTY_HINT_NONE, "", usage));
		p_list->push_back(PropertyInfo(Variant::RECT2, prefix + "rect", PROPERTY_HINT_NONE, "", usage));
		p_list->push_back(PropertyInfo(Variant::RECT2, prefix + "rrect", PROPERTY_HINT_NONE, "", usage));
		p_list->push_back(PropertyInfo(Variant::INT, prefix + "atlas_page", PROPERTY_HINT_NONE, "", usage));
		p_list->push_back(PropertyInfo(Variant::BOOL, prefix + "flipped", PROPERTY_HINT_NONE, "", usage));
		p_list->push_back(PropertyInfo(Variant::INT, prefix + "trim_l", PROPERTY_HINT_NONE, "", usage));
		p_list->push_back(PropertyInfo(Variant::INT, prefix + "trim_r", PROPERTY_HINT_NONE, "", usage));
		p_list->push_back(PropertyInfo(Variant::INT, prefix + "trim_t", PROPERTY_HINT_NONE, "", usage));
		p_list->push_back(PropertyInfo(Variant::INT, prefix + "trim_b", PROPERTY_HINT_NONE, "", usage));
		if (items[i].hull.size() > 0) {
			p_list->push_back(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, prefix + "hull", PROPERTY_HINT_NONE, "", usage));
			p_list->push_back(PropertyInfo(Variant::POOL_INT_ARRAY, prefix + "hull_indices", PROPERTY_HINT_NONE, "", usage));
		}
	}
}

void AtlasInfo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_item_count"), &AtlasInfo::get_item_count);
	ClassDB::bind_method(D_METHOD("set_item_count", "count"), &AtlasInfo::set_item_count);

	ClassDB::bind_method(D_METHOD("get_item_name", "index"), &AtlasInfo::get_item_name);
	ClassDB::bind_method(D_METHOD("set_item_name", "index", "name"), &AtlasInfo::set_item_name);

	ClassDB::bind_method(D_METHOD("get_item_rect", "index"), &AtlasInfo::get_item_rect);
	ClassDB::bind_method(D_METHOD("set_item_rect", "index", "rect"), &AtlasInfo::set_item_rect);

	ClassDB::bind_method(D_METHOD("get_item_rrect", "index"), &AtlasInfo::get_item_rrect);
	ClassDB::bind_method(D_METHOD("set_item_rrect", "index", "rrect"), &AtlasInfo::set_item_rrect);

	ClassDB::bind_method(D_METHOD("get_item_atlas_page", "index"), &AtlasInfo::get_item_atlas_page);
	ClassDB::bind_method(D_METHOD("set_item_atlas_page", "index", "page"), &AtlasInfo::set_item_atlas_page);

	ClassDB::bind_method(D_METHOD("get_item_flipped", "index"), &AtlasInfo::get_item_flipped);
	ClassDB::bind_method(D_METHOD("set_item_flipped", "index", "flipped"), &AtlasInfo::set_item_flipped);

	ClassDB::bind_method(D_METHOD("get_item_hull", "index"), &AtlasInfo::get_item_hull);
	ClassDB::bind_method(D_METHOD("set_item_hull", "index", "hull"), &AtlasInfo::set_item_hull);
	ClassDB::bind_method(D_METHOD("get_item_hull_indices", "index"), &AtlasInfo::get_item_hull_indices);
	ClassDB::bind_method(D_METHOD("set_item_hull_indices", "index", "indices"), &AtlasInfo::set_item_hull_indices);
	ClassDB::bind_method(D_METHOD("item_has_hull", "index"), &AtlasInfo::item_has_hull);

	ClassDB::bind_method(D_METHOD("set_default_texture", "texture"), &AtlasInfo::set_default_texture);
	ClassDB::bind_method(D_METHOD("get_default_texture"), &AtlasInfo::get_default_texture);

	ClassDB::bind_method(D_METHOD("find_item", "name"), &AtlasInfo::find_item);
	ClassDB::bind_method(D_METHOD("load_from_merge_result", "result", "names"), &AtlasInfo::load_from_merge_result);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "default_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_default_texture", "get_default_texture");
}

int AtlasInfo::get_item_count() const {
	return items.size();
}

void AtlasInfo::set_item_count(int p_count) {
	items.resize(MAX(0, p_count));
	emit_changed();
}

String AtlasInfo::get_item_name(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), String());
	return items[p_index].name;
}

void AtlasInfo::set_item_name(int p_index, const String &p_name) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.write[p_index].name = p_name;
	emit_changed();
}

Rect2 AtlasInfo::get_item_rect(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), Rect2());
	return items[p_index].rect;
}

void AtlasInfo::set_item_rect(int p_index, const Rect2 &p_rect) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.write[p_index].rect = p_rect;
	emit_changed();
}

Rect2 AtlasInfo::get_item_rrect(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), Rect2());
	return items[p_index].rrect;
}

void AtlasInfo::set_item_rrect(int p_index, const Rect2 &p_rrect) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.write[p_index].rrect = p_rrect;
	emit_changed();
}

int AtlasInfo::get_item_atlas_page(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), 0);
	return items[p_index].atlas_page;
}

void AtlasInfo::set_item_atlas_page(int p_index, int p_page) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.write[p_index].atlas_page = p_page;
	emit_changed();
}

bool AtlasInfo::get_item_flipped(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), false);
	return items[p_index].flipped;
}

void AtlasInfo::set_item_flipped(int p_index, bool p_flipped) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.write[p_index].flipped = p_flipped;
	emit_changed();
}

void AtlasInfo::set_item_trim(int p_index, int l, int r, int t, int b) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.write[p_index].trim_l = l;
	items.write[p_index].trim_r = r;
	items.write[p_index].trim_t = t;
	items.write[p_index].trim_b = b;
	emit_changed();
}

int AtlasInfo::get_item_trim_l(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), 0);
	return items[p_index].trim_l;
}
int AtlasInfo::get_item_trim_r(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), 0);
	return items[p_index].trim_r;
}
int AtlasInfo::get_item_trim_t(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), 0);
	return items[p_index].trim_t;
}
int AtlasInfo::get_item_trim_b(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), 0);
	return items[p_index].trim_b;
}

PoolVector2Array AtlasInfo::get_item_hull(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), PoolVector2Array());
	return items[p_index].hull;
}

void AtlasInfo::set_item_hull(int p_index, const PoolVector2Array &p_hull) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.write[p_index].hull = p_hull;
	emit_changed();
}

PoolIntArray AtlasInfo::get_item_hull_indices(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), PoolIntArray());
	return items[p_index].hull_indices;
}

void AtlasInfo::set_item_hull_indices(int p_index, const PoolIntArray &p_indices) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.write[p_index].hull_indices = p_indices;
	emit_changed();
}

bool AtlasInfo::item_has_hull(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, items.size(), false);
	return items[p_index].hull.size() >= 3;
}

void AtlasInfo::set_default_texture(const Ref<Texture> &p_texture) {
	default_texture = p_texture;
	emit_changed();
}

Ref<Texture> AtlasInfo::get_default_texture() const {
	return default_texture;
}

int AtlasInfo::find_item(const String &p_name) const {
	for (int i = 0; i < items.size(); ++i) {
		if (items[i].name == p_name)
			return i;
	}
	return -1;
}

void AtlasInfo::load_from_merge_result(const Dictionary &p_result, const Vector<String> &p_names) {
	items.clear();

	if (!p_result.has("_rects"))
		return;

	Array rects = p_result["_rects"];
	int count = rects.size();
	items.resize(count);

	for (int i = 0; i < count; ++i) {
		Dictionary entry = rects[i];
		Item &item = items.write[i];

		item.name = (i < p_names.size()) ? p_names[i] : ("item_" + itos(i));

		if (entry.has("rect"))
			item.rect = entry["rect"];
		if (entry.has("rrect"))
			item.rrect = entry["rrect"];
		if (entry.has("atlas_page"))
			item.atlas_page = entry["atlas_page"];
		if (entry.has("flipped"))
			item.flipped = entry["flipped"];
		if (entry.has("trim_l"))
			item.trim_l = entry["trim_l"];
		if (entry.has("trim_r"))
			item.trim_r = entry["trim_r"];
		if (entry.has("trim_t"))
			item.trim_t = entry["trim_t"];
		if (entry.has("trim_b"))
			item.trim_b = entry["trim_b"];
		if (entry.has("hull"))
			item.hull = entry["hull"];
		if (entry.has("hull_indices"))
			item.hull_indices = entry["hull_indices"];
	}

	emit_changed();
}

// ---------------------------------------------------------------------------
// AtlasInfoTexture
// ---------------------------------------------------------------------------

void AtlasInfoTexture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_atlas_info", "info"), &AtlasInfoTexture::set_atlas_info);
	ClassDB::bind_method(D_METHOD("get_atlas_info"), &AtlasInfoTexture::get_atlas_info);

	ClassDB::bind_method(D_METHOD("set_item_name", "name"), &AtlasInfoTexture::set_item_name);
	ClassDB::bind_method(D_METHOD("get_item_name"), &AtlasInfoTexture::get_item_name);

	ClassDB::bind_method(D_METHOD("get_region"), &AtlasInfoTexture::get_region);
	ClassDB::bind_method(D_METHOD("get_hull"), &AtlasInfoTexture::get_hull);
	ClassDB::bind_method(D_METHOD("get_hull_indices"), &AtlasInfoTexture::get_hull_indices);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "atlas_info", PROPERTY_HINT_RESOURCE_TYPE, "AtlasInfo"), "set_atlas_info", "get_atlas_info");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "item_name"), "set_item_name", "get_item_name");
}

void AtlasInfoTexture::set_atlas_info(const Ref<AtlasInfo> &p_info) {
	atlas_info = p_info;
	emit_changed();
}

Ref<AtlasInfo> AtlasInfoTexture::get_atlas_info() const {
	return atlas_info;
}

void AtlasInfoTexture::set_item_name(const String &p_name) {
	item_name = p_name;
	emit_changed();
}

String AtlasInfoTexture::get_item_name() const {
	return item_name;
}

Rect2 AtlasInfoTexture::get_region() const {
	if (atlas_info.is_null())
		return Rect2();
	int idx = atlas_info->find_item(item_name);
	if (idx < 0)
		return Rect2();
	return atlas_info->get_item_rect(idx);
}

PoolVector2Array AtlasInfoTexture::get_hull() const {
	if (atlas_info.is_null())
		return PoolVector2Array();
	int idx = atlas_info->find_item(item_name);
	if (idx < 0)
		return PoolVector2Array();
	return atlas_info->get_item_hull(idx);
}

PoolIntArray AtlasInfoTexture::get_hull_indices() const {
	if (atlas_info.is_null())
		return PoolIntArray();
	int idx = atlas_info->find_item(item_name);
	if (idx < 0)
		return PoolIntArray();
	return atlas_info->get_item_hull_indices(idx);
}

void AtlasInfoTexture::set_flags(uint32_t p_flags) {
	if (atlas_info.is_valid()) {
		Ref<Texture> tex = atlas_info->get_default_texture();
		if (tex.is_valid())
			tex->set_flags(p_flags);
	}
}

uint32_t AtlasInfoTexture::get_flags() const {
	if (atlas_info.is_valid()) {
		Ref<Texture> tex = atlas_info->get_default_texture();
		if (tex.is_valid())
			return tex->get_flags();
	}
	return 0;
}

int AtlasInfoTexture::get_width() const {
	Rect2 r = get_region();
	return (int)r.size.width;
}

int AtlasInfoTexture::get_height() const {
	Rect2 r = get_region();
	return (int)r.size.height;
}

RID AtlasInfoTexture::get_rid() const {
	if (atlas_info.is_valid()) {
		Ref<Texture> tex = atlas_info->get_default_texture();
		if (tex.is_valid())
			return tex->get_rid();
	}
	return RID();
}

bool AtlasInfoTexture::has_alpha() const {
	if (atlas_info.is_valid()) {
		Ref<Texture> tex = atlas_info->get_default_texture();
		if (tex.is_valid())
			return tex->has_alpha();
	}
	return true;
}

void AtlasInfoTexture::draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate, bool p_transpose, const Ref<Texture> &p_normal_map, const Ref<Texture> &p_mask) const {
	if (atlas_info.is_null())
		return;
	Ref<Texture> tex = atlas_info->get_default_texture();
	if (tex.is_null())
		return;

	Rect2 region = get_region();
	if (region.size.x <= 0 || region.size.y <= 0)
		return;

	Rect2 dr(p_pos, region.size);
	VisualServer::get_singleton()->canvas_item_add_texture_rect_region(
			p_canvas_item, dr, tex->get_rid(), region, p_modulate, p_transpose,
			p_normal_map.is_valid() ? p_normal_map->get_rid() : RID(),
			p_mask.is_valid() ? p_mask->get_rid() : RID(), false);
}

void AtlasInfoTexture::draw_rect(RID p_canvas_item, const Rect2 &p_rect, bool p_tile, const Color &p_modulate, bool p_transpose, const Ref<Texture> &p_normal_map, const Ref<Texture> &p_mask) const {
	if (atlas_info.is_null())
		return;
	Ref<Texture> tex = atlas_info->get_default_texture();
	if (tex.is_null())
		return;

	Rect2 region = get_region();
	if (region.size.x <= 0 || region.size.y <= 0)
		return;

	VisualServer::get_singleton()->canvas_item_add_texture_rect_region(
			p_canvas_item, p_rect, tex->get_rid(), region, p_modulate, p_transpose,
			p_normal_map.is_valid() ? p_normal_map->get_rid() : RID(),
			p_mask.is_valid() ? p_mask->get_rid() : RID(), false);
}

void AtlasInfoTexture::draw_rect_region(RID p_canvas_item, const Rect2 &p_rect, const Rect2 &p_src_rect, const Color &p_modulate, bool p_transpose, const Ref<Texture> &p_normal_map, const Ref<Texture> &p_mask, bool p_clip_uv) const {
	if (atlas_info.is_null())
		return;
	Ref<Texture> tex = atlas_info->get_default_texture();
	if (tex.is_null())
		return;

	Rect2 region = get_region();
	if (region.size.x <= 0 || region.size.y <= 0)
		return;

	Rect2 src = Rect2(region.position + p_src_rect.position, p_src_rect.size);
	VisualServer::get_singleton()->canvas_item_add_texture_rect_region(
			p_canvas_item, p_rect, tex->get_rid(), src, p_modulate, p_transpose,
			p_normal_map.is_valid() ? p_normal_map->get_rid() : RID(),
			p_mask.is_valid() ? p_mask->get_rid() : RID(), p_clip_uv);
}
