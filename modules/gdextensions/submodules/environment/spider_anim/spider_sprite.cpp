/**************************************************************************/
/*  spider_sprite.cpp                                                     */
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

#include "core/core_string_names.h"
#include "core/os/os.h"
#include "scene/main/viewport.h"
#include "scene/scene_string_names.h"

#include "spider_res.h"
#include "spider_sprite.h"

#include "common/gd_core.h"
#include "common/gd_pack.h"

#ifdef DEBUG_ENABLED
static void _draw_circle(CanvasItem *canvas, const Vector2 &center, real_t r, const Color &c, int segs) {
	const real_t coef = Math_TAU / segs;
	Vector<Point2> vertices;
	real_t rads = 0;
	for (int i = 0; i <= segs; i++) {
		vertices.push_back(Point2(r * Math::cos(rads) + center.x, r * Math::sin(rads) + center.y));
		rads += coef;
	}
	canvas->draw_polyline(vertices, c);
}
#endif

static Ref<SpiderThemeInfo> &get_theme_mgr_instance() {
	static Ref<SpiderThemeInfo> instance;
	if (!instance) {
		instance = newref(SpiderThemeInfo);
		_register_global_ref(instance);
	};
	return instance;
}

void SpiderThemeInfo::_cache_themes() {
	auto get_image_format = [](int channels) {
		switch (channels) {
			case 4:
				return Image::FORMAT_RGBA8;
			case 3:
				return Image::FORMAT_RGB8;
			case 2:
				return Image::FORMAT_LA8;
			default:
				return Image::FORMAT_MAX;
		}
	};

	// build texture atlas from resources
	Vector<Ref<Image>> images;
	Vector<String> names;
	std::vector<EmbedImageItem> embed(embed_spider_res, embed_spider_res + embed_spider_res_count);
	for (const auto &r : embed) {
		ERR_CONTINUE_MSG(get_image_format(r.channels) == Image::FORMAT_MAX, "Image format is not supported, Skipping!");

		Ref<Image> image = newref(Image);
		PoolByteArray data;
		data.resize(r.size);
		std::memcpy(data.write().ptr(), r.pixels, r.size);
		image->create(r.width, r.height, false, get_image_format(r.channels), data);
		images.push_back(image);
		names.push_back(String(r.image).get_basename());
	}

	Dictionary atlas_info = merge_images(images, names);

	ERR_FAIL_COND(atlas_info.empty());

	Array pages = atlas_info["_generated_images"];
	Array textures;
	for (int a = 0; a < pages.size(); ++a) {
		Ref<Image> atlas_image = pages[a];

		ERR_CONTINUE_MSG(!atlas_image.is_valid(), "Atlas image is not valid, Skipping!");

		Ref<ImageTexture> atlas_page = newref(ImageTexture);
		atlas_page->create_from_image(atlas_image);
		textures.push_back(atlas_page);
	}

	Dictionary atlas_rects = atlas_info["_rects"];
	for (int j = 0; j < names.size(); ++j) {
		String name = names[j];
		Dictionary entry = atlas_rects[name];

		ERR_CONTINUE_MSG(entry.empty(), "Empty atlas entry, Skipping!");

		Ref<AtlasTexture> at = newref(AtlasTexture);
		at->set_atlas(textures[entry["atlas_page"]]);
		at->set_region(entry["rect"]);
#ifdef DEBUG_ENABLED
		at->set_name(name);
#endif

		_cache[name] = at;
	}
}

Ref<Texture> SpiderThemeInfo::get_part(int p_theme_nr, int p_part_nr) {
	ERR_FAIL_INDEX_V(p_theme_nr, AvailableThemes, Ref<Texture>());
	ERR_FAIL_INDEX_V(p_part_nr, SpiderSpritePartsNum, Ref<Texture>());

	if (p_part_nr == SpiderSpriteGroupNode) {
		return Ref<Texture>();
	}

	if (_cache.empty()) {
		_cache_themes();
	}

	static const struct {
		int theme_nr;
		String theme_name;
	} _theme_map[] = {
		{ Theme1, "0" },
		{ Theme2, "1" },
		{ Theme3, "2" },
		{ Theme4, "3" },
		{ Theme5, "4" },
		{ Theme6, "5" },
		{ -1, "" }
	};

	static const struct {
		int part_nr;
		String part_name;
	} _parts_map[] = {
		{ SpiderLegTop, "B" },
		{ SpiderLegMiddle, "A" },
		{ SpiderLegBottom, "H" },
		{ SpiderLegTopShadow, "E" },
		{ SpiderLegBottomShadow, "F" },
		{ SpiderBodyBack, "I" },
		{ SpiderBodyBackTop, "D" },
		{ SpiderBodyFront, "C" },
		{ SpiderBodyShadow, "G" },
		{ SpiderBodyPed, "J" },
		{ -1, "" }
	};
	String texture_name = vformat("%s-%s", _parts_map[p_part_nr].part_name, _theme_map[p_theme_nr].theme_name);

	return _cache[texture_name];
}

Rect2 SpiderSprite::get_anchorable_rect() const {
	return get_rect();
}

Rect2 SpiderSprite::_get_dest_rect() const {
	Point2 dest_offset = offset;
	if (Engine::get_singleton()->get_use_gpu_pixel_snap()) {
		dest_offset = dest_offset.floor();
	}

	Rect2 dest_rect = Rect2(dest_offset, base_size);

	if (hflip) {
		dest_rect.size.x = -dest_rect.size.x;
	}
	if (vflip) {
		dest_rect.size.y = -dest_rect.size.y;
	}
	return dest_rect;
}

void SpiderSprite::_texture_changed() {
	// Changes to the texture need to trigger an update to make
	// the editor redraw the sprite with the updated texture.
	if (texture.is_valid()) {
		update();
	}
}

void SpiderSprite::set_skin_theme(int p_skin) {
	ERR_FAIL_INDEX(p_skin, AvailableThemes);
	skin = (SpiderTheme)p_skin;
	set_texture(theme[part]);
}

void SpiderSprite::set_texture(const Ref<Texture> &p_texture) {
	if (p_texture == texture) {
		return;
	}
	if (texture.is_valid()) {
		texture->disconnect(CoreStringNames::get_singleton()->changed, this, "_texture_changed");
	}
	texture = p_texture;
	if (texture.is_valid()) {
		texture->connect(CoreStringNames::get_singleton()->changed, this, "_texture_changed");
	}
	update();
	emit_signal("texture_changed");
	item_rect_changed();
	_change_notify("texture");
}

Ref<Texture> SpiderSprite::get_texture() const {
	return texture;
}

void SpiderSprite::set_size(const Size2 &p_size) {
	base_size = p_size;
	update();
	item_rect_changed();
}

Size2 SpiderSprite::get_size() const {
	return base_size;
}

void SpiderSprite::set_offset(const Point2 &p_offset) {
	offset = p_offset;
	update();
	item_rect_changed();
	_change_notify("offset");
}

Point2 SpiderSprite::get_offset() const {
	return offset;
}

void SpiderSprite::set_flip_h(bool p_flip) {
	hflip = p_flip;
	update();
}
void SpiderSprite::toggle_flip_h() {
	hflip = !hflip;
	update();
}
bool SpiderSprite::is_flipped_h() const {
	return hflip;
}

void SpiderSprite::set_flip_v(bool p_flip) {
	vflip = p_flip;
	update();
}
void SpiderSprite::toggle_flip_v() {
	vflip = !vflip;
	update();
}
bool SpiderSprite::is_flipped_v() const {
	return vflip;
}

Rect2 SpiderSprite::get_rect() const {
	Size2i s = base_size;

	Point2 ofs = offset;
	if (Engine::get_singleton()->get_use_gpu_pixel_snap()) {
		ofs = ofs.floor();
	}

	if (s == Size2(0, 0))
		s = Size2(1, 1);

	return Rect2(ofs, s);
}

void SpiderSprite::set_debug_draw(bool p_state) {
	_debug_draw = p_state;
	update();
}

bool SpiderSprite::get_debug_draw() const {
	return _debug_draw;
}

void SpiderSprite::init_geometry(const Ref<Texture> &p_face, Rect2 p_bbox, real_t p_scale) {
	set_position(Point2());
	set_size(p_bbox.size);
	set_offset(p_bbox.position);
	set_scale(p_scale);
	set_texture(p_face);
}

#define _draw_debug(rc, color)                       \
	{                                                \
		draw_rect(rc, color, false);                 \
		draw_circle(Point2(), 2, Color(1, 1, 0, 1)); \
	}

void SpiderSprite::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (texture.is_null()) {
				set_texture(theme[part]); // default
			}
		} break;
		case NOTIFICATION_DRAW: {
			if (texture.is_null()) {
				if (_debug_draw) {
					_draw_debug(Rect2(offset, base_size), Color(1, 0, 1, 0.3));
				}
				return;
			}
			RID ci = get_canvas_item();
			Rect2 dst_rect = _get_dest_rect();
			texture->draw_rect(ci, dst_rect);
			if (_debug_draw) {
				_draw_debug(Rect2(offset, dst_rect.abs().size), Color(1, 1, 0, 0.3));
			}
		} break;
	}
}

void SpiderSprite::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_set_debug_draw", "state"), &SpiderSprite::set_debug_draw);
	ClassDB::bind_method(D_METHOD("_set_skin_theme", "skin"), &SpiderSprite::set_skin_theme);
}

SpiderSprite::SpiderSprite(SpiderSpriteParts spider_part) :
		part(spider_part), theme({ this }) {
	_debug_draw = false;

	skin = Theme1;
	base_size = Size2(1, 1);
	offset = Point2(0, 0);
	hflip = false;
	vflip = false;

	theme_mgr = get_theme_mgr_instance();

	set_name(vformat("part_%d", spider_part));
}

SpiderSprite *make_sprite_spider(SpiderSpriteParts p_spider_part, Rect2 p_bbox, Point2 p_pos, real_t p_scale) {
	SpiderSprite *s = memnew(SpiderSprite(p_spider_part));
	s->set_position(p_pos);
	s->set_size(p_bbox.size);
	s->set_offset(p_bbox.position);
	s->set_scale(p_scale);

	return s;
}

SpiderSprite *make_sprite_spider(SpiderSpriteParts p_spider_part, Rect2 p_bbox, real_t p_scale) {
	SpiderSprite *s = memnew(SpiderSprite(p_spider_part));
	s->set_position(Point2());
	s->set_size(p_bbox.size);
	s->set_offset(p_bbox.position);
	s->set_scale(p_scale);

	return s;
}

SpiderSprite *make_sprite_spider(SpiderSpriteParts p_spider_part, real_t p_xbox, real_t p_ybox, real_t p_wbox, real_t p_hbox, real_t p_scale) {
	SpiderSprite *s = memnew(SpiderSprite(p_spider_part));
	s->set_position(Point2());
	s->set_size(Size2(p_wbox, p_hbox));
	s->set_offset(Point2(p_xbox, p_ybox));
	s->set_scale(p_scale);

	return s;
}
