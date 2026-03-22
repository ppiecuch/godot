/**************************************************************************/
/*  gd_tilengine.h                                                        */
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

#ifndef GD_TILENGINE_H
#define GD_TILENGINE_H

#include "core/image.h"
#include "core/reference.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

#include <Tilengine.h>

// ---------------------------------------------------------------------------
// Resource wrappers (Reference)
// ---------------------------------------------------------------------------

class TLNPalette : public Reference {
	GDCLASS(TLNPalette, Reference);

	TLN_Palette palette;
	bool owned;

protected:
	static void _bind_methods();

public:
	void set_from(TLN_Palette p, bool p_owned);
	TLN_Palette get_handle() const { return palette; }

	void create(int entries);
	void load(const String &file);
	Ref<TLNPalette> duplicate();

	void set_color(int idx, Color c);
	Color get_color(int idx);
	int get_num_colors();

	void mix(Ref<TLNPalette> src1, Ref<TLNPalette> src2, int factor);
	void add_color(Color c, int start, int num);
	void sub_color(Color c, int start, int num);
	void mod_color(Color c, int start, int num);

	TLNPalette();
	~TLNPalette();
};

class TLNBitmap : public Reference {
	GDCLASS(TLNBitmap, Reference);

	TLN_Bitmap bitmap;
	bool owned;

protected:
	static void _bind_methods();

public:
	void set_from(TLN_Bitmap b, bool p_owned);
	TLN_Bitmap get_handle() const { return bitmap; }

	void create(int w, int h, int bpp);
	void load(const String &file);
	Ref<TLNBitmap> duplicate();

	int get_width();
	int get_height();
	int get_depth();
	int get_pitch();

	Ref<TLNPalette> get_palette();
	void set_palette(Ref<TLNPalette> pal);

	Ref<Image> to_image();

	TLNBitmap();
	~TLNBitmap();
};

class TLNTileset : public Reference {
	GDCLASS(TLNTileset, Reference);

	TLN_Tileset tileset;
	bool owned;

protected:
	static void _bind_methods();

public:
	void set_from(TLN_Tileset t, bool p_owned);
	TLN_Tileset get_handle() const { return tileset; }

	void create(int numtiles, int width, int height, Ref<TLNPalette> pal);
	void load(const String &file);
	Ref<TLNTileset> duplicate();

	int get_tile_width();
	int get_tile_height();
	int get_num_tiles();
	Ref<TLNPalette> get_palette();

	TLNTileset();
	~TLNTileset();
};

class TLNTilemap : public Reference {
	GDCLASS(TLNTilemap, Reference);

	TLN_Tilemap tilemap;
	bool owned;

protected:
	static void _bind_methods();

public:
	void set_from(TLN_Tilemap t, bool p_owned);
	TLN_Tilemap get_handle() const { return tilemap; }

	void create(int rows, int cols, Ref<TLNTileset> ts);
	void load(const String &file, const String &layername);
	Ref<TLNTilemap> duplicate();

	int get_rows();
	int get_cols();

	Dictionary get_tile(int row, int col);
	void set_tile(int row, int col, int index, int flags);

	Ref<TLNTileset> get_tileset();
	void set_tileset(Ref<TLNTileset> ts);

	TLNTilemap();
	~TLNTilemap();
};

class TLNSpriteset : public Reference {
	GDCLASS(TLNSpriteset, Reference);

	TLN_Spriteset spriteset;
	bool owned;

protected:
	static void _bind_methods();

public:
	void set_from(TLN_Spriteset s, bool p_owned);
	TLN_Spriteset get_handle() const { return spriteset; }

	void load(const String &name);
	Ref<TLNSpriteset> duplicate();

	Vector2 get_sprite_size(int entry);
	int find_sprite(const String &name);
	Ref<TLNPalette> get_palette();

	TLNSpriteset();
	~TLNSpriteset();
};

class TLNSequence : public Reference {
	GDCLASS(TLNSequence, Reference);

	TLN_Sequence sequence;
	bool owned;

protected:
	static void _bind_methods();

public:
	void set_from(TLN_Sequence s, bool p_owned);
	TLN_Sequence get_handle() const { return sequence; }

	void create_frame_sequence(const String &name, int target, Array frames);
	void create_cycle(const String &name, Array strips);
	void create_sprite_sequence(const String &name, Ref<TLNSpriteset> ss, const String &basename, int delay);
	Ref<TLNSequence> duplicate();

	String get_name();
	int get_num_frames();

	TLNSequence();
	~TLNSequence();
};

class TLNSequencePack : public Reference {
	GDCLASS(TLNSequencePack, Reference);

	TLN_SequencePack pack;
	bool owned;

protected:
	static void _bind_methods();

public:
	void set_from(TLN_SequencePack p, bool p_owned);
	TLN_SequencePack get_handle() const { return pack; }

	void create();
	void load(const String &file);

	int get_count();
	Ref<TLNSequence> get_sequence(int idx);
	Ref<TLNSequence> find_sequence(const String &name);
	void add_sequence(Ref<TLNSequence> seq);

	TLNSequencePack();
	~TLNSequencePack();
};

class TLNObjectList : public Reference {
	GDCLASS(TLNObjectList, Reference);

	TLN_ObjectList list;
	bool owned;

protected:
	static void _bind_methods();

public:
	void set_from(TLN_ObjectList l, bool p_owned);
	TLN_ObjectList get_handle() const { return list; }

	void create();
	void load(const String &file, const String &layername);
	Ref<TLNObjectList> duplicate();

	int get_num_objects();
	Dictionary get_object();
	void add_tile_object(int id, int gid, int flags, int x, int y);

	TLNObjectList();
	~TLNObjectList();
};

// ---------------------------------------------------------------------------
// Engine node (Node2D)
// ---------------------------------------------------------------------------

class TLNEngine : public Node2D {
	GDCLASS(TLNEngine, Node2D);

public:
	enum BlendMode {
		BLEND_NONE = ::BLEND_NONE,
		BLEND_MIX25 = ::BLEND_MIX25,
		BLEND_MIX50 = ::BLEND_MIX50,
		BLEND_MIX75 = ::BLEND_MIX75,
		BLEND_ADD = ::BLEND_ADD,
		BLEND_SUB = ::BLEND_SUB,
		BLEND_MOD = ::BLEND_MOD,
	};

	enum TileFlag {
		TILE_FLIPX = FLAG_FLIPX,
		TILE_FLIPY = FLAG_FLIPY,
		TILE_ROTATE = FLAG_ROTATE,
		TILE_PRIORITY = FLAG_PRIORITY,
		TILE_MASKED = FLAG_MASKED,
	};

private:
	TLN_Engine engine;
	int fb_width;
	int fb_height;
	int num_layers;
	int num_sprites;
	int num_animations;
	int frame_counter;

	Vector<uint8_t> framebuffer;
	Ref<Image> image;
	Ref<ImageTexture> texture;

	static TLNEngine *current_raster_target;
	static void _raster_callback(int scanline);

	void _ensure_context();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Setup
	void init_engine(int w, int h, int layers, int sprites, int animations);
	void deinit_engine();
	bool is_initialized() const { return engine != nullptr; }
	void set_load_path(const String &path);

	int get_fb_width() const { return fb_width; }
	int get_fb_height() const { return fb_height; }

	// Background
	void set_bg_color(Color c);
	void set_bg_bitmap(Ref<TLNBitmap> bmp);
	void set_bg_palette(Ref<TLNPalette> pal);
	void disable_bg_color();

	// Layers
	void set_layer(int nlayer, Ref<TLNTileset> ts, Ref<TLNTilemap> tm);
	void set_layer_tilemap(int nlayer, Ref<TLNTilemap> tm);
	void set_layer_bitmap(int nlayer, Ref<TLNBitmap> bmp);
	void set_layer_palette(int nlayer, Ref<TLNPalette> pal);
	void set_layer_position(int nlayer, int hstart, int vstart);
	void set_layer_scaling(int nlayer, float xfactor, float yfactor);
	void set_layer_transform(int nlayer, float angle, Vector2 offset, Vector2 scale);
	void set_layer_blend_mode(int nlayer, int mode, int factor);
	void set_layer_clip(int nlayer, int x1, int y1, int x2, int y2);
	void disable_layer_clip(int nlayer);
	void set_layer_mosaic(int nlayer, int width, int height);
	void disable_layer_mosaic(int nlayer);
	void set_layer_objects(int nlayer, Ref<TLNObjectList> objects, Ref<TLNTileset> ts);
	void set_layer_priority(int nlayer, bool enable);
	void set_layer_parent(int nlayer, int parent);
	void disable_layer_parent(int nlayer);
	void set_layer_parallax_factor(int nlayer, float x, float y);
	void enable_layer(int nlayer);
	void disable_layer(int nlayer);
	Dictionary get_layer_tile(int nlayer, int x, int y);

	// Sprites
	void config_sprite(int nsprite, Ref<TLNSpriteset> ss, int flags);
	void set_sprite_position(int nsprite, int x, int y);
	void set_sprite_picture(int nsprite, int entry);
	void set_sprite_palette(int nsprite, Ref<TLNPalette> pal);
	void set_sprite_scaling(int nsprite, float sx, float sy);
	void set_sprite_blend_mode(int nsprite, int mode, int factor);
	void set_sprite_flags(int nsprite, int flags);
	void set_sprite_pivot(int nsprite, float px, float py);
	void enable_sprite_collision(int nsprite, bool enable);
	bool get_sprite_collision(int nsprite);
	Dictionary get_sprite_state(int nsprite);
	void set_sprite_animation(int nsprite, Ref<TLNSequence> seq, int loop);
	void disable_sprite_animation(int nsprite);
	void set_first_sprite(int nsprite);
	void set_next_sprite(int nsprite, int next);
	void enable_sprite_masking(int nsprite, bool enable);
	void set_sprites_mask_region(int top_line, int bottom_line);
	void disable_sprite(int nsprite);

	// Animation
	void set_palette_animation(int index, Ref<TLNPalette> pal, Ref<TLNSequence> seq, bool blend);
	void disable_palette_animation(int index);
	bool get_animation_state(int index);

	// World / TMX
	void load_world(const String &tmxfile, int first_layer);
	void set_world_position(int x, int y);
	void release_world();

	// Raster callback
	void enable_raster_callback();
	void disable_raster_callback();

	// Framebuffer access
	Ref<Image> get_framebuffer_image();
	Ref<ImageTexture> get_texture();

	TLNEngine();
	~TLNEngine();
};

VARIANT_ENUM_CAST(TLNEngine::BlendMode);
VARIANT_ENUM_CAST(TLNEngine::TileFlag);

#endif // GD_TILENGINE_H
