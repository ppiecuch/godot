/*************************************************************************/
/*  spider_sprite.h                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef GD_SPIDER_SPRITE_H
#define GD_SPIDER_SPRITE_H

#include "core/math/rect2.h"
#include "core/reference.h"
#include "scene/2d/node_2d.h"

// -- skins and theme configuration for skins:
enum SpiderSpriteParts {
	SpiderLegTop = 0,
	SpiderLegMiddle, // 1
	SpiderLegBottom, // 2
	SpiderLegTopShadow, // 3
	SpiderLegBottomShadow, // 4
	SpiderBodyBack, // 5
	SpiderBodyBackTop, // 6
	SpiderBodyFront, // 7
	SpiderBodyShadow, // 8
	SpiderBodyPed, // 9
	SpiderSpriteGroupNode, // 10
	SpiderSpritePartsNum, // 11
};

enum SpiderTheme {
	Theme1 = 0,
	Theme2, // = 1
	Theme3, // = 2
	Theme4, // = 3
	Theme5, // = 4
	Theme6, // = 5
	AvailableThemes // = 6
};

class SpiderThemeInfo : public Reference {
	GDCLASS(SpiderThemeInfo, Reference)

public:
	Dictionary _cache;
	void _cache_themes();
	Ref<Texture> get_part(int theme_nr, int part_nr);
};
// -- end.

class SpiderSprite : public Node2D {
	GDCLASS(SpiderSprite, Node2D)

	Ref<Texture> texture;
	Size2 base_size;
	Point2 offset;

	bool hflip;
	bool vflip;

	Rect2 _get_dest_rect() const;
	void _texture_changed();

	bool _debug_draw;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void init_geometry(const Ref<Texture> &face, Rect2 bbox, real_t scale = 1.0);

public:
	bool operator<(const SpiderSprite &s1) const { return (get_z_index() < s1.get_z_index()); };
	bool operator==(const SpiderSprite &s1) const { return (&s1 == this); }

	void set_skin_theme(int p_skin);

	void set_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_texture() const;

	void set_size(const Size2 &p_size);
	Size2 get_size() const;

	void set_offset(const Point2 &p_offset);
	Point2 get_offset() const;

	void set_flip_h(bool p_flip);
	void toggle_flip_h();
	bool is_flipped_h() const;

	void set_flip_v(bool p_flip);
	void toggle_flip_v();
	bool is_flipped_v() const;

	Rect2 get_rect() const;
	virtual Rect2 get_anchorable_rect() const;

	void set_position(const Point2 &p_pos) { Node2D::set_position(p_pos); }
	void set_position(real_t p_x, real_t p_y) { Node2D::set_position(Point2(p_x, p_y)); }
	void set_position_y(real_t p_pos) { Node2D::set_position(Point2(get_position().x, p_pos)); }

	void set_scale(real_t p_scale) { Node2D::set_scale(Size2(p_scale, p_scale)); }
	void set_scale(real_t p_scale_x, real_t p_scale_y) { Node2D::set_scale(Size2(p_scale_x, p_scale_y)); }
	void set_scale_x(real_t p_scale) { Node2D::set_scale(Size2(p_scale, get_scale_y())); }
	real_t get_scale_x() const { return get_scale().x; }
	void set_scale_y(real_t p_scale) { Node2D::set_scale(Size2(get_scale_x(), p_scale)); }
	real_t get_scale_y() const { return get_scale().y; }

	void set_color(Color p_color) { set_modulate(p_color); }
	void set_alpha(float p_alpha) { set_modulate(get_modulate().with_alpha(p_alpha)); }

	void set_debug_draw(bool p_state);
	bool get_debug_draw() const;

	SpiderSprite(SpiderSpriteParts spider_part);

public:
	SpiderTheme skin; // spider's skin
	const SpiderSpriteParts part; // part of the skin

	struct {
		SpiderSprite *owner;
		Ref<Texture> operator[](int part) const {
			ERR_FAIL_INDEX_V(part, SpiderSpritePartsNum, Ref<Texture>());
			return owner->theme_mgr->get_part(owner->skin, owner->part);
		}
	} theme;
	Ref<SpiderThemeInfo> theme_mgr; // themes manager
};

SpiderSprite *make_sprite_spider(SpiderSpriteParts p_spider_part, Rect2 p_bbox, real_t p_scale = 1.0);
SpiderSprite *make_sprite_spider(SpiderSpriteParts p_spider_part, Rect2 p_bbox, Point2 p_pos, real_t p_scale = 1.0);
SpiderSprite *make_sprite_spider(SpiderSpriteParts p_spider_part, real_t p_xbox, real_t p_ybox, real_t p_wbox, real_t p_hbox, real_t p_scale = 1.0);

#endif // GD_SPIDER_SPRITE_H
