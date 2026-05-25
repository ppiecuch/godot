/**************************************************************************/
/*  atlas_info.h                                                          */
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

#ifndef ATLAS_INFO_H
#define ATLAS_INFO_H

#include "core/resource.h"
#include "scene/resources/texture.h"

class AtlasInfo : public Resource {
	GDCLASS(AtlasInfo, Resource);

public:
	enum Algorithm {
		ALGO_BSP = 0,
		ALGO_GUILLOTINE = 1,
		ALGO_MAXRECTS = 2,
	};

	enum BorderMode {
		BORDER_EMPTY = 0,
		BORDER_MIRROR = 1,
		BORDER_BLUR = 2,
	};

	struct Item {
		String name;
		Rect2 rect;
		Rect2 rrect;
		int atlas_page = 0;
		bool flipped = false;
		int trim_l = 0, trim_r = 0, trim_t = 0, trim_b = 0;
		PoolVector2Array hull;
		PoolIntArray hull_indices;
	};

private:
	Vector<Item> items;
	Ref<Texture> default_texture;
	Array generated_textures;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();

public:
	int get_item_count() const;
	void set_item_count(int p_count);

	String get_item_name(int p_index) const;
	void set_item_name(int p_index, const String &p_name);

	Rect2 get_item_rect(int p_index) const;
	void set_item_rect(int p_index, const Rect2 &p_rect);

	Rect2 get_item_rrect(int p_index) const;
	void set_item_rrect(int p_index, const Rect2 &p_rrect);

	int get_item_atlas_page(int p_index) const;
	void set_item_atlas_page(int p_index, int p_page);

	bool get_item_flipped(int p_index) const;
	void set_item_flipped(int p_index, bool p_flipped);

	void set_item_trim(int p_index, int l, int r, int t, int b);
	int get_item_trim_l(int p_index) const;
	int get_item_trim_r(int p_index) const;
	int get_item_trim_t(int p_index) const;
	int get_item_trim_b(int p_index) const;

	PoolVector2Array get_item_hull(int p_index) const;
	void set_item_hull(int p_index, const PoolVector2Array &p_hull);
	PoolIntArray get_item_hull_indices(int p_index) const;
	void set_item_hull_indices(int p_index, const PoolIntArray &p_indices);
	bool item_has_hull(int p_index) const;

	void set_default_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_default_texture() const;

	int find_item(const String &p_name) const;

	void load_from_merge_result(const Dictionary &p_result, const Vector<String> &p_names);

	// Pack images into atlas. Options Dictionary keys (all optional):
	//   algorithm: int (ALGO_BSP/ALGO_GUILLOTINE/ALGO_MAXRECTS)
	//   margin: int, max_atlas_size: int, force_single_page: bool
	//   allow_rotation: bool, power_of_two: bool, square_atlas: bool
	//   trim_alpha: bool, trim_alpha_threshold: int
	//   trim_color: bool, trim_color_threshold: float
	//   fix_halo: bool, border_mode: int (BORDER_*)
	//   hull_compute: bool, hull_vertex_count: int, hull_alpha_threshold: int
	//   hull_max_size: int, hull_sub_pixel: int
	//   separate_alpha: bool
	//   debug_borders: bool, debug_hull_outline: bool, debug_hull_triangulation: bool
	Error pack(const Array &p_images, const PoolStringArray &p_names, const Dictionary &p_options = Dictionary());

	int get_page_count() const;
	Ref<Texture> get_page_texture(int p_index) const;
	Array get_page_textures() const;

	AtlasInfo() {}
};

VARIANT_ENUM_CAST(AtlasInfo::Algorithm);
VARIANT_ENUM_CAST(AtlasInfo::BorderMode);

class AtlasInfoTexture : public Texture {
	GDCLASS(AtlasInfoTexture, Texture);

	Ref<AtlasInfo> atlas_info;
	String item_name;

protected:
	static void _bind_methods();

public:
	void set_atlas_info(const Ref<AtlasInfo> &p_info);
	Ref<AtlasInfo> get_atlas_info() const;

	void set_item_name(const String &p_name);
	String get_item_name() const;

	void set_flags(uint32_t p_flags) override;
	uint32_t get_flags() const override;

	int get_width() const override;
	int get_height() const override;
	RID get_rid() const override;
	bool has_alpha() const override;

	void draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, const Ref<Texture> &p_normal_map = Ref<Texture>(), const Ref<Texture> &p_mask = Ref<Texture>()) const override;
	void draw_rect(RID p_canvas_item, const Rect2 &p_rect, bool p_tile = false, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, const Ref<Texture> &p_normal_map = Ref<Texture>(), const Ref<Texture> &p_mask = Ref<Texture>()) const override;
	void draw_rect_region(RID p_canvas_item, const Rect2 &p_rect, const Rect2 &p_src_rect, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, const Ref<Texture> &p_normal_map = Ref<Texture>(), const Ref<Texture> &p_mask = Ref<Texture>(), bool p_clip_uv = true) const override;

	Rect2 get_region() const;
	PoolVector2Array get_hull() const;
	PoolIntArray get_hull_indices() const;

	AtlasInfoTexture() {}
};

#endif // ATLAS_INFO_H
