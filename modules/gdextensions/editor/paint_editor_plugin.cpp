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

#include "mypaint/mypaint-brush.h"
#include "mypaint/mypaint-tiled-surface.h"

#include "common/gd_core.h"
#include "core/color.h"
#include "core/error_macros.h"
#include "core/image.h"
#include "core/variant.h"
#include "core/vector.h"
#include "scene/2d/canvas_item.h"

#include "paint_editor_plugin.h"

//-------------------------------------------------------------------------
// This basic class store a tile info & display it. the uint16_t table is the
// real info modified by libMyPaint. Before any screen refresh, we transfer
// it to a Ref<Image> acting as a cache. this Ref<Image> is only necessary to paint.
// NOTE that the uint16_t data (premul RGB 15 bits) is transfered in premul
// format. This is only useful if you plan to have several layers.
// if it is not the case, you could simply convert to RGBA (not premul)

#define CONV_16_8(x) ((x * 255) / (1 << 15))
#define CONV_8_16(x) ((x * (1 << 15)) / 255)

enum {
	k_tile_dim = 64
};

enum {
	k_red = 0,
	k_green = 1,
	k_blue = 2,
	k_alpha = 3,
}; // index to access RGBA values in myPaint

typedef Vector<String> StringList;

class MPBrush;

class MPTile {
	uint16_t t_pixels[k_tile_dim][k_tile_dim][4];
	Ref<Image> cache_img;
	bool cache_valid;

public:
	Ref<Image> image() const { return cache_img; }

	uint16_t *get_pixel_buffer() { return &t_pixels[0][0][0]; }

	void paint(CanvasItem *canvas) {
		ERR_FAIL_NULL(canvas);
		if (!cache_valid) {
			update_cache(); // we need to transfer the uint16_t table to the Ref<Image> cache
		}
		canvas->draw_texture(cache_img, Point2());
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
				cache_img->_set_pixel32(x, y, alpha ? g_rgba(CONV_16_8(t_pixels[y][x][k_red]), CONV_16_8(t_pixels[y][x][k_green]), CONV_16_8(t_pixels[y][x][k_blue]), CONV_16_8(alpha)) : 0);
			}
		}
		cache_valid = true;
	}

	void clear() {
		memset(t_pixels, 0, sizeof(t_pixels)); // tile is transparent
		cache_img->fill(Color()); // image cache is transparent too, and aligned to the pixel table
		cache_valid = true;
	}

	void set_image(const Ref<Image> &image) {
		ERR_FAIL_NULL(image);

		const Size2 tile_size = cache_img->get_size();
		if (tile_size != image->get_size()) {
			cache_img = image->resized(tile_size.width, tile_size.height); // make sure the image has the same dimentions as the tile
		} else {
			cache_img = image;
		}
		for (int y = 0; y < tile_size.height; y++) {
			for (int x = 0; x < tile_size.width; x++) {
				const uint32_t pixel_color = cache_img->_get_pixel32(x, y);
				t_pixels[y][x][k_alpha] = CONV_8_16(g_alpha(pixel_color));
				t_pixels[y][x][k_red] = CONV_8_16(g_red(pixel_color));
				t_pixels[y][x][k_green] = CONV_8_16(g_green(pixel_color));
				t_pixels[y][x][k_blue] = CONV_8_16(g_blue(pixel_color));
			}
		}
		cache_valid = true;
	}

	MPTile() {
		cache_img = newref(Image, k_tile_dim, k_tile_dim, false, Image::FORMAT_RGBA8 /* Premultiplied */);
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

public:
	Map<Point2, MPTile *> tiles;

public:
	uint16_t *tile_buffer; // Stores tiles in a linear chunk of memory (16bpc RGBA)
	uint16_t *null_tile; // Single tile that we hand out and ignore writes to

	int get_tiles_width();
	int get_tiles_height();
	int get_width();
	int get_height();

	enum {
		k_center = 50,
		k_max = 2 * k_center,
	};

	MPTile *get_tile_from_pos(const Point2 &pos);
	MPTile *get_tile_from_idx(const Point2 &idx);
	_FORCE_INLINE_ bool check_index(uint32_t n);
	_FORCE_INLINE_ Point2 get_tile_pos(const Point2 &idx);
	_FORCE_INLINE_ Point2 get_tile_index(const Point2 &pos);
	_FORCE_INLINE_ Point2 get_tile_findex(const Point2 &pos);

	typedef void (*MPOnUpdateTileFunction)(MPSurface *surface, MPTile *tile);
	typedef void (*MPOnUpdateSurfaceFunction)(MPSurface *surface);

	void set_on_update_tile(MPOnUpdateTileFunction handler);
	void set_on_new_tile(MPOnUpdateTileFunction handler);
	void set_on_cleared_surface(MPOnUpdateSurfaceFunction handler);
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
	Color color;

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

	void load(const String &content) {
		mypaint_brush_from_defaults(brush);

		if (!mypaint_brush_from_string(brush, content.utf8().c_str())) {
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

/// MPSurface implementation

static void on_tile_request_start(MyPaintTiledSurface *self, MyPaintTileRequest *request) {
	MPSurface *surface = static_cast<MPSurface *>(self);
	const int tx = request->tx;
	const int ty = request->ty;
	Point2 idx(tx, ty);

	MPTile *tile = surface->get_tile_from_idx(idx);
	if (!tile) {
		// Create new tile
		tile = memnew(MPTile);
		surface->tiles[idx] = tile;
		if (surface->on_new_tile_function) {
			surface->on_new_tile_function(surface, tile);
		}
	}

	request->buffer = tile->get_pixel_buffer();
}

static void on_tile_request_end(MyPaintTiledSurface *self, MyPaintTileRequest *request) {
	MPSurface *surface = static_cast<MPSurface *>(self);
	const int tx = request->tx;
	const int ty = request->ty;
	Point2 idx(tx, ty);

	MPTile *tile = surface->get_tile_from_idx(idx);
	if (tile && surface->on_update_tile_function) {
		surface->on_update_tile_function(surface, tile);
	}
}

MPSurface::MPSurface(Size2 size) {
	on_update_tile_function = nullptr;
	on_new_tile_function = nullptr;
	on_cleared_surface_function = nullptr;

	// Init the mypaint tiled surface with our callbacks
	mypaint_tiled_surface_init(this, on_tile_request_start, on_tile_request_end);

	// Allocate null tile
	null_tile = (uint16_t *)memalloc(k_tile_dim * k_tile_dim * 4 * sizeof(uint16_t));
	memset(null_tile, 0, k_tile_dim * k_tile_dim * 4 * sizeof(uint16_t));
	tile_buffer = nullptr;

	brush = nullptr;

	set_size(size);
}

MPSurface::~MPSurface() {
	mypaint_tiled_surface_destroy(this);

	// Free all tiles
	for (Map<Point2, MPTile *>::Element *E = tiles.front(); E; E = E->next()) {
		memdelete(E->value());
	}
	tiles.clear();

	if (null_tile) {
		memfree(null_tile);
	}
}

MPTile *MPSurface::get_tile_from_pos(const Point2 &pos) {
	return get_tile_from_idx(get_tile_index(pos));
}

MPTile *MPSurface::get_tile_from_idx(const Point2 &idx) {
	if (tiles.has(idx)) {
		return tiles[idx];
	}
	return nullptr;
}

bool MPSurface::check_index(uint32_t n) {
	return n < uint32_t(k_max);
}

Point2 MPSurface::get_tile_pos(const Point2 &idx) {
	return Point2(idx.x * k_tile_dim, idx.y * k_tile_dim);
}

Point2 MPSurface::get_tile_index(const Point2 &pos) {
	return Point2(pos.x / k_tile_dim, pos.y / k_tile_dim);
}

Point2 MPSurface::get_tile_findex(const Point2 &pos) {
	return Point2(Math::floor(pos.x / k_tile_dim), Math::floor(pos.y / k_tile_dim));
}

int MPSurface::get_tiles_width() { return tiles_width; }
int MPSurface::get_tiles_height() { return tiles_height; }
int MPSurface::get_width() { return width; }
int MPSurface::get_height() { return height; }

void MPSurface::set_size(const Size2 &size) {
	width = size.width;
	height = size.height;
	tiles_width = (width + k_tile_dim - 1) / k_tile_dim;
	tiles_height = (height + k_tile_dim - 1) / k_tile_dim;
	clear();
}

Size2 MPSurface::get_size() const {
	return Size2(width, height);
}

void MPSurface::clear() {
	for (Map<Point2, MPTile *>::Element *E = tiles.front(); E; E = E->next()) {
		E->value()->clear();
	}
	if (on_cleared_surface_function) {
		on_cleared_surface_function(this);
	}
}

Ref<Image> MPSurface::render_image() {
	Ref<Image> image;
	image.instance();
	image->create(width, height, false, Image::FORMAT_RGBA8);
	image->fill(Color(0, 0, 0, 0));

	for (Map<Point2, MPTile *>::Element *E = tiles.front(); E; E = E->next()) {
		Point2 pos = get_tile_pos(E->key());
		MPTile *tile = E->value();
		tile->update_cache();
		Ref<Image> tile_img = tile->image();
		if (tile_img.is_valid()) {
			image->blit_rect(tile_img, Rect2(0, 0, k_tile_dim, k_tile_dim), pos);
		}
	}

	return image;
}

void MPSurface::load_image(const Ref<Image> &image) {
	ERR_FAIL_COND(image.is_null());

	set_size(image->get_size());

	for (int ty = 0; ty < tiles_height; ty++) {
		for (int tx = 0; tx < tiles_width; tx++) {
			Point2 idx(tx, ty);
			Point2 pos = get_tile_pos(idx);

			Ref<Image> tile_img;
			tile_img.instance();
			tile_img->create(k_tile_dim, k_tile_dim, false, Image::FORMAT_RGBA8);

			// Extract tile region from source image
			Rect2 src_rect(pos.x, pos.y, MIN(k_tile_dim, width - pos.x), MIN(k_tile_dim, height - pos.y));
			tile_img->blit_rect(image, src_rect, Point2(0, 0));

			if (!tiles.has(idx)) {
				tiles[idx] = memnew(MPTile);
			}
			tiles[idx]->set_image(tile_img);
		}
	}
}

void MPSurface::set_on_update_tile(MPOnUpdateTileFunction handler) {
	on_update_tile_function = handler;
}

void MPSurface::set_on_new_tile(MPOnUpdateTileFunction handler) {
	on_new_tile_function = handler;
}

void MPSurface::set_on_cleared_surface(MPOnUpdateSurfaceFunction handler) {
	on_cleared_surface_function = handler;
}

void MPSurface::reset_null_tile() {
	memset(null_tile, 0, k_tile_dim * k_tile_dim * 4 * sizeof(uint16_t));
}

void MPSurface::reset_surface(const Size2 &size) {
	set_size(size);
}

bool MPSurface::is_fully_transparent(const Ref<Image> &image) {
	ERR_FAIL_COND_V(image.is_null(), true);
	for (int y = 0; y < image->get_height(); y++) {
		for (int x = 0; x < image->get_width(); x++) {
			if (image->get_pixel(x, y).a > 0) {
				return false;
			}
		}
	}
	return true;
}

/// PaintEditorPlugin

// Static callbacks for MPSurface -> PaintEditorPlugin bridging
static PaintEditorPlugin *s_paint_instance = nullptr;

static void _on_update_tile_cb(MPSurface *surface, MPTile *tile) {
	if (s_paint_instance) {
		s_paint_instance->request_update_tile(surface, tile);
	}
}

static void _on_new_tile_cb(MPSurface *surface, MPTile *tile) {
	if (s_paint_instance) {
		s_paint_instance->has_new_tile(surface, tile);
	}
}

static void _on_cleared_surface_cb(MPSurface *surface) {
	if (s_paint_instance) {
		s_paint_instance->has_cleared_surface(surface);
	}
}

bool PaintEditorPlugin::instance_flag = false;

void PaintEditorPlugin::_notification(int p_what) {
	// No editor UI in this phase
}

void PaintEditorPlugin::start_stroke() {
	ERR_FAIL_NULL(brush);
	ERR_FAIL_NULL(surface);
	mypaint_brush_reset(brush->brush);
	mypaint_brush_new_stroke(brush->brush);
	mypaint_surface_begin_atomic((MyPaintSurface *)surface);
}

void PaintEditorPlugin::stroke_to(real_t x, real_t y, real_t pressure, real_t xtilt, real_t ytilt) {
	ERR_FAIL_NULL(brush);
	ERR_FAIL_NULL(surface);
	mypaint_brush_stroke_to(brush->brush, (MyPaintSurface *)surface, x, y, pressure, xtilt, ytilt, 1.0);
}

void PaintEditorPlugin::stroke_to(real_t x, real_t y) {
	stroke_to(x, y, 1.0, 0.0, 0.0);
}

void PaintEditorPlugin::end_stroke() {
	ERR_FAIL_NULL(surface);
	MyPaintRectangle roi;
	mypaint_surface_end_atomic((MyPaintSurface *)surface, &roi);
}

real_t PaintEditorPlugin::get_brush_value(MyPaintBrushSetting setting) {
	ERR_FAIL_NULL_V(brush, 0);
	return brush->get_value(setting);
}

void PaintEditorPlugin::set_brush_color(Color newColor) {
	ERR_FAIL_NULL(brush);
	brush->set_color(newColor);
}

void PaintEditorPlugin::set_brush_value(MyPaintBrushSetting setting, real_t value) {
	ERR_FAIL_NULL(brush);
	brush->set_value(setting, value);
}

void PaintEditorPlugin::request_update_tile(MPSurface *surface, MPTile *tile) {
	emit_signal("update_tile", Variant(), Variant());
}

void PaintEditorPlugin::has_new_tile(MPSurface *surface, MPTile *tile) {
	emit_signal("new_tile", Variant(), Variant());
}

void PaintEditorPlugin::has_cleared_surface(MPSurface *surface) {
	emit_signal("cleared_surface", Variant());
}

void PaintEditorPlugin::set_surface_size(const Size2 &size) {
	ERR_FAIL_NULL(surface);
	surface->set_size(size);
}

Size2 PaintEditorPlugin::get_surface_size() const {
	ERR_FAIL_NULL_V(surface, Size2());
	return surface->get_size();
}

void PaintEditorPlugin::clear_surface() {
	ERR_FAIL_NULL(surface);
	surface->clear();
}

Ref<Image> PaintEditorPlugin::render_image() {
	ERR_FAIL_NULL_V(surface, Ref<Image>());
	return surface->render_image();
}

void PaintEditorPlugin::load_image(const Ref<Image> &image) {
	ERR_FAIL_NULL(surface);
	surface->load_image(image);
}

void PaintEditorPlugin::load_brush(const String &content) {
	ERR_FAIL_NULL(brush);
	brush->load(content);
}

PaintEditorPlugin::PaintEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	brush = memnew(MPBrush);
	surface = memnew(MPSurface(Size2(512, 512)));
	surface->set_on_update_tile(_on_update_tile_cb);
	surface->set_on_new_tile(_on_new_tile_cb);
	surface->set_on_cleared_surface(_on_cleared_surface_cb);
	s_paint_instance = this;
}

PaintEditorPlugin::~PaintEditorPlugin() {
	s_paint_instance = nullptr;
	if (brush) {
		memdelete(brush);
	}
	if (surface) {
		memdelete(surface);
	}
}

void PaintEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_stroke"), &PaintEditorPlugin::start_stroke);
	ClassDB::bind_method(D_METHOD("end_stroke"), &PaintEditorPlugin::end_stroke);
	ClassDB::bind_method(D_METHOD("set_brush_color", "color"), &PaintEditorPlugin::set_brush_color);
	ClassDB::bind_method(D_METHOD("set_surface_size", "size"), &PaintEditorPlugin::set_surface_size);
	ClassDB::bind_method(D_METHOD("get_surface_size"), &PaintEditorPlugin::get_surface_size);
	ClassDB::bind_method(D_METHOD("clear_surface"), &PaintEditorPlugin::clear_surface);
	ClassDB::bind_method(D_METHOD("render_image"), &PaintEditorPlugin::render_image);
	ClassDB::bind_method(D_METHOD("load_image", "image"), &PaintEditorPlugin::load_image);
	ClassDB::bind_method(D_METHOD("load_brush", "content"), &PaintEditorPlugin::load_brush);

	ADD_SIGNAL(MethodInfo("update_tile", PropertyInfo(Variant::OBJECT, "surface"), PropertyInfo(Variant::OBJECT, "tile")));
	ADD_SIGNAL(MethodInfo("new_tile", PropertyInfo(Variant::OBJECT, "surface"), PropertyInfo(Variant::OBJECT, "tile")));
	ADD_SIGNAL(MethodInfo("cleared_surface", PropertyInfo(Variant::OBJECT, "surface")));
}
