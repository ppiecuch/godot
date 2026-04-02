/**************************************************************************/
/*  gdal_bitmap_gfx.h                                                     */
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

#ifndef GDAL_BITMAP_GFX_H
#define GDAL_BITMAP_GFX_H

#include "core/image.h"
#include "core/object.h"
#include "core/reference.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

struct BITMAP;
struct FONT;
struct RLE_SPRITE;
typedef struct BITMAP ZBUFFER;

class GdAlFont;

// GdAlFont — Godot wrapper around an al_gfx FONT.
// Fonts can be loaded from bitmap images containing character cells
// separated by a marker color (yellow for truecolor, color 255 for 8-bit).
class GdAlFont : public Reference {
	GDCLASS(GdAlFont, Reference)

	FONT *fnt;

protected:
	static void _bind_methods();

public:
	int get_height() const;
	int get_length(const String &str) const;
	bool is_valid() const;
	bool is_mono() const;
	bool is_color() const;

	void _init_from_font(FONT *p_font);
	FONT *_get_font() const { return fnt; }

	GdAlFont();
	~GdAlFont();
};

// GdAlRleSprite — Godot wrapper around an al_gfx RLE_SPRITE.
// RLE sprites are compressed representations of bitmaps that are faster
// to draw than regular bitmaps when the sprite contains transparency.
class GdAlRleSprite : public Reference {
	GDCLASS(GdAlRleSprite, Reference)

	RLE_SPRITE *rle;

protected:
	static void _bind_methods();

public:
	int get_width() const;
	int get_height() const;
	int get_color_depth() const;
	int get_size() const;
	bool is_valid() const;

	void _init_from_rle(RLE_SPRITE *p_rle);
	RLE_SPRITE *_get_rle() const { return rle; }

	GdAlRleSprite();
	~GdAlRleSprite();
};

// GdAlBitmapGfx — Godot wrapper around an al_gfx BITMAP.
// Exposes Allegro 4 software rendering primitives as a Reference-counted
// object suitable for use from GDScript.
//
// Usage:
//   var bmp = AlBitmapGfx.create_bitmap(320, 240)
//   bmp.clear(Color.BLACK)
//   bmp.putpixel(10, 10, Color.RED)
//   bmp.circle(160, 120, 50, Color.YELLOW)
//   var img = bmp.get_image()
//   $TextureRect.texture = img  # ImageTexture from get_image()

class GdAlBitmapGfx : public Reference {
	GDCLASS(GdAlBitmapGfx, Reference)

	BITMAP *bmp;
	bool owns_bitmap; // false for sub-bitmaps (parent owns memory)
	Ref<GdAlFont> current_font; // custom font (null = use built-in 8x8)
	ZBUFFER *zbuf; // per-bitmap Z-buffer (null = no Z-buffering)

	FONT *_active_font() const; // returns current_font or built-in
	int _color_from_godot(const Color &c) const;
	Color _color_to_godot(int c) const;

protected:
	static void _bind_methods();

public:
	// Bitmap info
	int get_width() const;
	int get_height() const;
	int get_color_depth() const;
	bool is_valid() const;

	// Clear
	void clear(const Color &p_color);

	// Pixel operations
	void putpixel(int x, int y, const Color &color);
	Color getpixel(int x, int y) const;

	// Lines
	void hline(int x1, int y, int x2, const Color &color);
	void vline(int x, int y1, int y2, const Color &color);
	void line(int x1, int y1, int x2, int y2, const Color &color);

	// Rectangles
	void rect(int x1, int y1, int x2, int y2, const Color &color);
	void rectfill(int x1, int y1, int x2, int y2, const Color &color);

	// Circles and ellipses
	void circle(int x, int y, int radius, const Color &color);
	void circlefill(int x, int y, int radius, const Color &color);
	void ellipse(int x, int y, int rx, int ry, const Color &color);
	void ellipsefill(int x, int y, int rx, int ry, const Color &color);

	// Arc (center, angle range in degrees, radius)
	void arc(const Vector2 &center, float ang1_deg, float ang2_deg, int radius, const Color &color);

	// Triangle (3 vertices)
	void triangle(const Vector2 &v1, const Vector2 &v2, const Vector2 &v3, const Color &color);

	// Polygon (array of Vector2 vertices)
	void polygon(const PoolVector2Array &vertices, const Color &color);

	// Flood fill
	void floodfill(int x, int y, const Color &color);

	// Spline (4 control points as Vector2 array)
	void spline(const PoolVector2Array &points, const Color &color);

	// Text rendering (uses current font — built-in 8x8 or custom)
	void set_font(Ref<GdAlFont> p_font);
	Ref<GdAlFont> get_font() const;
	void text(const String &str, int x, int y, const Color &fg, const Color &bg = Color(-1, -1, -1, -1));
	void text_centered(const String &str, int x, int y, const Color &fg, const Color &bg = Color(-1, -1, -1, -1));
	void text_right(const String &str, int x, int y, const Color &fg, const Color &bg = Color(-1, -1, -1, -1));
	int get_text_length(const String &str) const;
	int get_text_height() const;

	// Blitting (source_rect = Rect2(sx,sy,w,h), dest_pos = Vector2(dx,dy))
	void blit_from(Ref<GdAlBitmapGfx> source, const Rect2 &src_rect, const Vector2 &dest_pos);
	void stretch_blit_from(Ref<GdAlBitmapGfx> source, const Rect2 &src_rect, const Rect2 &dest_rect);
	void masked_blit_from(Ref<GdAlBitmapGfx> source, const Rect2 &src_rect, const Vector2 &dest_pos);

	// Sprite drawing
	void draw_sprite(Ref<GdAlBitmapGfx> sprite, int x, int y);
	void draw_sprite_h_flip(Ref<GdAlBitmapGfx> sprite, int x, int y);
	void draw_sprite_v_flip(Ref<GdAlBitmapGfx> sprite, int x, int y);
	void draw_sprite_vh_flip(Ref<GdAlBitmapGfx> sprite, int x, int y);
	void draw_trans_sprite(Ref<GdAlBitmapGfx> sprite, int x, int y);
	void draw_lit_sprite(Ref<GdAlBitmapGfx> sprite, int x, int y, int color);

	// RLE sprites
	Ref<GdAlRleSprite> get_rle_sprite() const;
	void draw_rle_sprite(Ref<GdAlRleSprite> sprite, int x, int y);

	// Rotation/scaling (angle in degrees)
	void rotate_sprite(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, float angle_deg);
	void rotate_scaled_sprite(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, float angle_deg, float scale);

	// Pivot rotation (angle in degrees, pivot = point within sprite)
	void pivot_sprite(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, const Vector2 &pivot, float angle_deg);
	void pivot_sprite_v_flip(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, const Vector2 &pivot, float angle_deg);
	void pivot_scaled_sprite(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, const Vector2 &pivot, float angle_deg, float scale);
	void pivot_scaled_sprite_v_flip(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, const Vector2 &pivot, float angle_deg, float scale);

	// Drawing modes (prefixed MODE_ to avoid collision with al_gfx macros)
	enum DrawMode {
		MODE_SOLID = 0,
		MODE_XOR = 1,
		MODE_COPY_PATTERN = 2,
		MODE_SOLID_PATTERN = 3,
		MODE_MASKED_PATTERN = 4,
		MODE_TRANS = 5,
	};

	void set_drawing_mode(int mode, Ref<GdAlBitmapGfx> pattern = Ref<GdAlBitmapGfx>(), int x_anchor = 0, int y_anchor = 0);
	void solid_mode();
	void set_xor_mode(bool enabled);

	// Blending
	void set_trans_blender(int r, int g, int b, int a);
	void set_add_blender(int r, int g, int b, int a);
	void set_alpha_blender();

	// Clipping
	void set_clip_rect(int x1, int y1, int x2, int y2);
	Rect2 get_clip_rect() const;

	// Sub-bitmap
	Ref<GdAlBitmapGfx> create_sub_bitmap(int x, int y, int w, int h);

	// Conversion to Godot Image
	Ref<Image> get_image() const;

	// 3D polygon types (prefixed POLY_ to avoid collision with al_gfx macros)
	enum PolyType {
		POLY_FLAT = 0,
		POLY_GCOL = 1,
		POLY_GRGB = 2,
		POLY_ATEX = 3,
		POLY_PTEX = 4,
		POLY_ATEX_MASK = 5,
		POLY_PTEX_MASK = 6,
		POLY_ATEX_LIT = 7,
		POLY_PTEX_LIT = 8,
		POLY_ATEX_MASK_LIT = 9,
		POLY_PTEX_MASK_LIT = 10,
		POLY_ATEX_TRANS = 11,
		POLY_PTEX_TRANS = 12,
		POLY_ATEX_MASK_TRANS = 13,
		POLY_PTEX_MASK_TRANS = 14,
	};

	// 3D flat-shaded triangle (Vector3: x, y, z)
	void triangle3d_flat(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, const Color &color);
	// 3D gouraud-shaded triangle (3 vertices, 3 colors)
	void triangle3d_gouraud(const PoolVector3Array &vertices, const PoolColorArray &colors);
	// 3D generic triangle (polytype, 3 vertices, 3 colors, optional texture)
	void triangle3d(int polytype, const PoolVector3Array &vertices, const PoolColorArray &colors, Ref<GdAlBitmapGfx> texture = Ref<GdAlBitmapGfx>());

	// 3D flat-shaded quad (4 vertices, 1 color)
	void quad3d_flat(const PoolVector3Array &vertices, const Color &color);
	// 3D gouraud-shaded quad (4 vertices, 4 colors)
	void quad3d_gouraud(const PoolVector3Array &vertices, const PoolColorArray &colors);

	// Z-buffer management
	void create_zbuffer();
	void clear_zbuffer(float z = 0.0f);
	void enable_zbuffer();
	void disable_zbuffer();
	bool has_zbuffer() const;
	void destroy_zbuffer();

	// Demo rendering (reimplements demo.c)
	void render_demo();

	// Internal: wrap an existing BITMAP (takes ownership or not)
	void _init_from_bitmap(BITMAP *p_bmp, bool p_owns);
	BITMAP *_get_bitmap() const { return bmp; }

	GdAlBitmapGfx();
	~GdAlBitmapGfx();
};

VARIANT_ENUM_CAST(GdAlBitmapGfx::DrawMode);
VARIANT_ENUM_CAST(GdAlBitmapGfx::PolyType);

// AlBitmapGfx — Singleton factory for creating GdAlBitmapGfx instances
// and managing global al_gfx state.
class AlBitmapGfx : public Object {
	GDCLASS(AlBitmapGfx, Object)

	static AlBitmapGfx *singleton;
	bool initialized;

protected:
	static void _bind_methods();

public:
	static AlBitmapGfx *get_singleton() { return singleton; }

	// Factory
	Ref<GdAlBitmapGfx> create_bitmap(int width, int height);
	Ref<GdAlBitmapGfx> create_bitmap_ex(int color_depth, int width, int height);

	// Global color depth
	void set_color_depth(int depth);
	int get_color_depth() const;

	// Load/save TGA
	Ref<GdAlBitmapGfx> load_tga(const String &path);
	Error save_tga(const String &path, Ref<GdAlBitmapGfx> bmp);

	// Create bitmap from Godot Image
	Ref<GdAlBitmapGfx> from_image(Ref<Image> image);

	// Font loading
	Ref<GdAlFont> load_bitmap_font(const String &path);
	Ref<GdAlFont> font_from_bitmap(Ref<GdAlBitmapGfx> bitmap);

	AlBitmapGfx();
	~AlBitmapGfx();
};

// AlBitmapGfxNode — Node2D that renders a GdAlBitmapGfx bitmap directly
// via VisualServer for real-time display without per-frame allocation.
//
// Usage (GDScript):
//   var node = AlBitmapGfxNode.new()
//   node.bitmap = AlBitmapGfx.create_bitmap(320, 240)
//   add_child(node)
//   # draw into node.bitmap each frame; node auto-updates the display
class AlBitmapGfxNode : public Node2D {
	GDCLASS(AlBitmapGfxNode, Node2D)

	Ref<GdAlBitmapGfx> bitmap;
	Ref<ImageTexture> texture;
	Ref<Image> image;
	bool centered;
	bool dirty;
	bool auto_update;
	int tex_flags;

	void _update_texture();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_bitmap(Ref<GdAlBitmapGfx> p_bitmap);
	Ref<GdAlBitmapGfx> get_bitmap() const;

	void set_centered(bool p_centered);
	bool is_centered() const;

	void set_auto_update(bool p_auto);
	bool is_auto_update() const;

	void set_texture_flags(int p_flags);
	int get_texture_flags() const;

	void mark_dirty();
	Ref<ImageTexture> get_texture() const;

	AlBitmapGfxNode();
};

#endif // GDAL_BITMAP_GFX_H
