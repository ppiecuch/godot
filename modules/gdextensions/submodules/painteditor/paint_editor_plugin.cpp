/**************************************************************************/
/*  paint_editor_plugin.cpp                                               */
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

// Reference:
// ----------
// 1. https://github.com/ethiccinema/qtmypaint

#include "mypaint-brush.h"
#include "mypaint-glib-compat.h"
#include "mypaint-tiled-surface.h"

#include "common/gd_core.h"
#include "core/image.h"
#include "core/variant.h"

//-------------------------------------------------------------------------
// This basic class store a tile info & display it. the uint16_t table is the
// real info modified by libMyPaint. Before any screen refresh, we transfer
// it to a Ref<Image> acting as a cache. this Ref<Image> is only necessary to paint.
// NOTE that the uint16_t data (premul RGB 15 bits) is transfered in premul
// format. This is only useful if you plan to have several layers.
// if it is not the case, you could simply convert to RGBA (not premul)

#define CONV_16_8(x) ((x * 255) / (1 << 15))
#define CONV_8_16(x) ((x * (1 << 15)) / 255)

class MPTile {
	uint16_t t_pixels[k_tile_dim][k_tile_dim][4];
	Ref<Image> cache_img;
	bool cache_valid;

public:
	enum { k_tile_dim = 64 };
	enum { k_red = 0,
		k_green = 1,
		k_blue = 2,
		k_alpha = 3 }; // Index to access RGBA values in myPaint

	Ref<Image> image() const { return cache_img; }

	void paint(CanvasItem *canvas) {
		ERR_NULL_FAIL(canvas);
		if (!cache_valid) {
			update_cache(); // we need to transfer the uint16_t table to the Ref<Image> cache
		}
		canvas->draw_image(QPoint(), cache_img, cache_img.rect());
	}

	_FORCE_INLINE_ void draw_point(uint32_t x, uint32_t y, uint16_t r, uint16_t g, uint16_t b, uint16_t a) {
		cache_valid = false;
		t_pixels[y][x][k_red] = r, t_pixels[y][x][k_green] = g, t_pixels[y][x][k_blue] = b, t_pixels[y][x][k_alpha] = a;
	}
	void update_cache() {
		for (int y = 0; y < k_tile_dim; y++) {
			for (int x = 0; x < k_tile_dim; x++) {
				const uint16_t alpha = t_pixels[y][x][k_alpha];
				// alpha is 0 => all is zero (little optimization)
				cache_img->_set_pixel(x, y, alpha ? g_rgba(CONV_16_8(t_pixels[y][x][k_red]), CONV_16_8(t_pixels[y][x][k_green]), CONV_16_8(t_pixels[y][x][k_blue]), CONV_16_8(alpha)) : 0);
			}
		}
		cache_valid = true;
	}

	void clear() {
		memset(t_pixels, 0, sizeof(t_pixels)); // tile is transparent
		cache_img.fill(Color()); // image cache is transparent too, and aligned to the pixel table
		cache_valid = true;
	}

	void set_image(const Ref<Image> &image) {
		ERR_NULL_FAIL(image);

		const Size2 tile_size = cache_img->get_size();
		if (tile_size != image->get_size()) {
			cache_img = image->resized(tile_size.width, tile_size.height); // make sure the image has the same dimentions as the tile
		} else {
			cache_img = image;
		}
		for (int y = 0; y < tile_size.height; y++) {
			for (int x = 0; x < tile_size.width; x++) {
				const uint32_t pixel_color = cache_img._get_pixel32(x, y);
				t_pixels[y][x][k_alpha] = CONV_8_16(g_alpha(pixel_color));
				t_pixels[y][x][k_red] = CONV_8_16(g_red(pixel_color));
				t_pixels[y][x][k_green] = CONV_8_16(g_green(pixel_color));
				t_pixels[y][x][k_blue] = CONV_8_16(g_blue(pixel_color));
			}
		}
		cache_valid = true;
	}

	MPTile() {
		cache_img = memnew(Image(k_tile_dim, k_tile_dim, Image::FORMAT_RGBA8 /* Premultiplied */));
		clear();
	}
};

class MPSurface : public MyPaintTiledSurface {
	void reset_null_tile();
	void reset_surface(const Size2 &size);
	bool is_fully_transparent(const Ref<Image> &image);
	String key;

	int tiles_width; // width in tiles
	int tiles_height; // height in tiles
	int width; // width in pixels
	int height; // height in pixels

	MPBrush *brush;
	Color color;

protected:
	Map<Point2, MPTile *> tiles;

public:
	uint16_t *tile_buffer; // Stores tiles in a linear chunk of memory (16bpc RGBA)
	uint16_t *null_tile; // Single tile that we hand out and ignore writes to

	int get_tiles_width();
	int get_tiles_height();
	int get_width();
	int get_height();

	enum { k_center = 50,
		k_max = 2 * k_center };

	MPTile *get_tile_from_pos(const Point2 &pos);
	MPTile *get_tile_from_idx(const Point2 &idx);
	inline bool check_index(uint32_t n);
	inline Point2 get_tile_pos(const Point2 &idx);
	inline Point2 get_tile_index(const Point2 &pos);
	inline Point2F get_tile_findex(const Point2 &pos);

	typedef void (*MPOnUpdateTileFunction)(MPSurface *surface, MPTile *tile);
	typedef void (*MPOnUpdateSurfaceFunction)(MPSurface *surface);

	void set_on_update_tile(MPOnUpdateTileFunction handler);
	void set_on_new_tile(MPOnUpdateTileFunction handler);
	void set_on_cleared_surface(MPOnUpdateSurfaceFunction handler);
	/
			MPOnUpdateTileFunction on_update_tile_function;
	MPOnUpdateTileFunction on_new_tile_function;
	MPOnUpdateSurfaceFunction on_cleared_surface_function;

	void set_size(const Size2 &size);
	Size2 get_size() const;

	void clear();
	Ref<Image> render_image();

	void load_image(const Ref<Image> &image);

	MPSurface(Size2 size);
	~MPSurface();
};

class MPBrush {
	Color m_color;

public:
	MyPaintBrush *brush;

	void init_brush() {
		brush = mypaint_brush_new();
		mypaint_brush_from_defaults(brush);

		set_value(MYPAINT_BRUSH_SETTING_COLOR_H, 0);
		set_value(MYPAINT_BRUSH_SETTING_COLOR_S, 0);
		set_value(MYPAINT_BRUSH_SETTING_COLOR_V, 0);
		set_value(MYPAINT_BRUSH_SETTING_SNAP_TO_PIXEL, 0.0);
		set_value(MYPAINT_BRUSH_SETTING_ANTI_ALIASING, 1.0);
		set_value(MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, 0.3);
		// set_value(MYPAINT_BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC, 4.0);
		// set_value(MYPAINT_BRUSH_SETTING_SPEED2_SLOWNESS, 0.8);
		// set_value(MYPAINT_BRUSH_SETTING_SPEED2_GAMMA, 10);
		// set_value(MYPAINT_BRUSH_SETTING_SPEED1_SLOWNESS, 0.04);
		// set_value(MYPAINT_BRUSH_SETTING_SPEED1_GAMMA, 10);
		// set_value(MYPAINT_BRUSH_SETTING_SMUDGE_LENGTH, 0.5);
		// set_value(MYPAINT_BRUSH_SETTING_SLOW_TRACKING_PER_DAB, 1.5);
		// set_value(MYPAINT_BRUSH_SETTING_SLOW_TRACKING, 1.03);
		// set_value(MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM, 0.5);
		set_value(MYPAINT_BRUSH_SETTING_DIRECTION_FILTER, 10.0);
		set_value(MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS, 4.0);
	}

	void load(const PoolByteArray &content) {
		mypaint_brush_from_defaults(brush);

		if (!mypaint_brush_from_string(brush, content.ptr())) {
			// Not able to load the selected brush. Let's execute some backup code...
			WARN_PRINT("Trouble when reading the selected brush !");
		}
		set_color(color);
	}

	Color get_color() const { return color; }
	void set_color(const Color &new_color) {
		color = new_color;
		const float h = color.get_h();
		const float s = color.get_s();
		const float v = color.get_v();

		// Opacity is not handled here as it is defined by the brush settings.
		// If you wish to force opacity, use MPObject::set_brush_value()
		//
		// const float opacity = color.get_a();
		// mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_OPAQUE, opacity);

		set_value(MYPAINT_BRUSH_SETTING_COLOR_H, h);
		set_value(MYPAINT_BRUSH_SETTING_COLOR_S, s);
		set_value(MYPAINT_BRUSH_SETTING_COLOR_V, v);
	}

	real_t get_value(MyPaintBrushSetting setting) const { return mypaint_brush_get_base_value(brush, setting); }
	void set_value(MyPaintBrushSetting setting, real_t value) { mypaint_brush_set_base_value(brush, setting, value); }

	MPBrush() {
		init_brush();
		set_color(Color()); // Set default color to black
	}

	~MPBrush() {
		mypaint_brush_unref(brush);
	}
};

class MPBrushLib {
	Map<String, StringList> brush_lib;
	const String brushes_path;

public:
	void select_brush(const String &brush_name) {}

	MPBrushLib(const String &brush_lib_path) {}
};

class MPObject : public Object {
	GDCLASS()

	static bool instance_flag;
	static MPObject *current_fandler;

	MPBrush *brush;
	MPSurface *surface;

	MPObject();

public:
	static MPObject *handler();

	typedef void (*MPOnUpdateFunction)(MPObject *handler, MPSurface *surface, MPTile *tile);

	void start_stroke();
	void stroke_to(real_t x, real_t y, real_t pressure, real_t xtilt, real_t ytilt);
	void stroke_to(real_t x, real_t y);
	void end_stroke();

	real_t get_brush_value(MyPaintBrushSetting setting);

	void set_brush_color(Color newColor);
	void set_brush_value(MyPaintBrushSetting setting, real_t value);

	void request_update_tile(MPSurface *surface, MPTile *tile);
	void has_new_tile(MPSurface *surface, MPTile *tile);
	void has_cleared_surface(MPSurface *surface);

	void set_surface_size(const Size2 &size);
	Size2 get_surface_size() const;

	void clear_surface();
	Ref<Image> render_image();

	void load_image(const Ref<Image> &image);

	~MPObject();

public slots:
	void load_brush(const PoolByteArray &content);

signals:
	void update_tile(MPSurface *surface, MPTile *tile);
	void new_tile(MPSurface *surface, MPTile *tile);
	void cleared_surface(MPSurface *surface);
};
