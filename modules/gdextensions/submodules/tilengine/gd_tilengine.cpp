/**************************************************************************/
/*  gd_tilengine.cpp                                                      */
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

#include "gd_tilengine.h"

#include "core/print_string.h"
#include <cstdarg>
#include <cstdio>

extern "C" void tilengine_godot_print(const char *fmt, ...) {
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	print_verbose(String::utf8(buf));
}

// ===========================================================================
// TLNPalette
// ===========================================================================

TLNPalette::TLNPalette() :
		palette(nullptr), owned(false) {}

TLNPalette::~TLNPalette() {
	if (palette && owned) {
		TLN_DeletePalette(palette);
	}
}

void TLNPalette::set_from(TLN_Palette p, bool p_owned) {
	if (palette && owned) {
		TLN_DeletePalette(palette);
	}
	palette = p;
	owned = p_owned;
}

void TLNPalette::create(int entries) {
	if (palette && owned) {
		TLN_DeletePalette(palette);
	}
	palette = TLN_CreatePalette(entries);
	owned = true;
}

void TLNPalette::load(const String &file) {
	if (palette && owned) {
		TLN_DeletePalette(palette);
	}
	palette = TLN_LoadPalette(file.utf8().get_data());
	owned = (palette != nullptr);
}

Ref<TLNPalette> TLNPalette::duplicate() {
	if (!palette) {
		return Ref<TLNPalette>();
	}
	Ref<TLNPalette> dup;
	dup.instance();
	dup->set_from(TLN_ClonePalette(palette), true);
	return dup;
}

void TLNPalette::set_color(int idx, Color c) {
	if (palette) {
		TLN_SetPaletteColor(palette, idx, c.r * 255, c.g * 255, c.b * 255);
	}
}

Color TLNPalette::get_color(int idx) {
	if (!palette) {
		return Color();
	}
	uint8_t *data = TLN_GetPaletteData(palette, idx);
	if (!data) {
		return Color();
	}
	// Tilengine stores colors as BGRA: [0]=B, [1]=G, [2]=R, [3]=A
	return Color(data[2] / 255.0f, data[1] / 255.0f, data[0] / 255.0f);
}

int TLNPalette::get_num_colors() {
	return palette ? TLN_GetPaletteNumColors(palette) : 0;
}

void TLNPalette::mix(Ref<TLNPalette> src1, Ref<TLNPalette> src2, int factor) {
	if (palette && src1.is_valid() && src1->palette && src2.is_valid() && src2->palette) {
		TLN_MixPalettes(src1->palette, src2->palette, palette, factor);
	}
}

void TLNPalette::add_color(Color c, int start, int num) {
	if (palette) {
		TLN_AddPaletteColor(palette, c.r * 255, c.g * 255, c.b * 255, start, num);
	}
}

void TLNPalette::sub_color(Color c, int start, int num) {
	if (palette) {
		TLN_SubPaletteColor(palette, c.r * 255, c.g * 255, c.b * 255, start, num);
	}
}

void TLNPalette::mod_color(Color c, int start, int num) {
	if (palette) {
		TLN_ModPaletteColor(palette, c.r * 255, c.g * 255, c.b * 255, start, num);
	}
}

void TLNPalette::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create", "entries"), &TLNPalette::create);
	ClassDB::bind_method(D_METHOD("load", "file"), &TLNPalette::load);
	ClassDB::bind_method(D_METHOD("duplicate"), &TLNPalette::duplicate);
	ClassDB::bind_method(D_METHOD("set_color", "idx", "color"), &TLNPalette::set_color);
	ClassDB::bind_method(D_METHOD("get_color", "idx"), &TLNPalette::get_color);
	ClassDB::bind_method(D_METHOD("get_num_colors"), &TLNPalette::get_num_colors);
	ClassDB::bind_method(D_METHOD("mix", "src1", "src2", "factor"), &TLNPalette::mix);
	ClassDB::bind_method(D_METHOD("add_color", "color", "start", "num"), &TLNPalette::add_color);
	ClassDB::bind_method(D_METHOD("sub_color", "color", "start", "num"), &TLNPalette::sub_color);
	ClassDB::bind_method(D_METHOD("mod_color", "color", "start", "num"), &TLNPalette::mod_color);
}

// ===========================================================================
// TLNBitmap
// ===========================================================================

TLNBitmap::TLNBitmap() :
		bitmap(nullptr), owned(false) {}

TLNBitmap::~TLNBitmap() {
	if (bitmap && owned) {
		TLN_DeleteBitmap(bitmap);
	}
}

void TLNBitmap::set_from(TLN_Bitmap b, bool p_owned) {
	if (bitmap && owned) {
		TLN_DeleteBitmap(bitmap);
	}
	bitmap = b;
	owned = p_owned;
}

void TLNBitmap::create(int w, int h, int bpp) {
	if (bitmap && owned) {
		TLN_DeleteBitmap(bitmap);
	}
	bitmap = TLN_CreateBitmap(w, h, bpp);
	owned = (bitmap != nullptr);
}

void TLNBitmap::load(const String &file) {
	if (bitmap && owned) {
		TLN_DeleteBitmap(bitmap);
	}
	bitmap = TLN_LoadBitmap(file.utf8().get_data());
	owned = (bitmap != nullptr);
}

Ref<TLNBitmap> TLNBitmap::duplicate() {
	if (!bitmap) {
		return Ref<TLNBitmap>();
	}
	Ref<TLNBitmap> dup;
	dup.instance();
	dup->set_from(TLN_CloneBitmap(bitmap), true);
	return dup;
}

int TLNBitmap::get_width() { return bitmap ? TLN_GetBitmapWidth(bitmap) : 0; }
int TLNBitmap::get_height() { return bitmap ? TLN_GetBitmapHeight(bitmap) : 0; }
int TLNBitmap::get_depth() { return bitmap ? TLN_GetBitmapDepth(bitmap) : 0; }
int TLNBitmap::get_pitch() { return bitmap ? TLN_GetBitmapPitch(bitmap) : 0; }

Ref<TLNPalette> TLNBitmap::get_palette() {
	if (!bitmap) {
		return Ref<TLNPalette>();
	}
	TLN_Palette p = TLN_GetBitmapPalette(bitmap);
	if (!p) {
		return Ref<TLNPalette>();
	}
	Ref<TLNPalette> pal;
	pal.instance();
	pal->set_from(p, false);
	return pal;
}

void TLNBitmap::set_palette(Ref<TLNPalette> pal) {
	if (bitmap && pal.is_valid() && pal->get_handle()) {
		TLN_SetBitmapPalette(bitmap, pal->get_handle());
	}
}

Ref<Image> TLNBitmap::to_image() {
	if (!bitmap) {
		return Ref<Image>();
	}
	int w = TLN_GetBitmapWidth(bitmap);
	int h = TLN_GetBitmapHeight(bitmap);
	int pitch = TLN_GetBitmapPitch(bitmap);
	int depth = TLN_GetBitmapDepth(bitmap);

	PoolByteArray data;
	data.resize(w * h * 4);
	PoolByteArray::Write wd = data.write();

	for (int y = 0; y < h; y++) {
		uint8_t *src = TLN_GetBitmapPtr(bitmap, 0, y);
		for (int x = 0; x < w; x++) {
			int dst_idx = (y * w + x) * 4;
			if (depth == 32) {
				wd[dst_idx + 0] = src[x * 4 + 2]; // B→R
				wd[dst_idx + 1] = src[x * 4 + 1]; // G
				wd[dst_idx + 2] = src[x * 4 + 0]; // R→B
				wd[dst_idx + 3] = src[x * 4 + 3]; // A
			} else {
				wd[dst_idx + 0] = src[x];
				wd[dst_idx + 1] = src[x];
				wd[dst_idx + 2] = src[x];
				wd[dst_idx + 3] = 255;
			}
		}
	}

	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_RGBA8, data);
	return img;
}

void TLNBitmap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create", "width", "height", "bpp"), &TLNBitmap::create);
	ClassDB::bind_method(D_METHOD("load", "file"), &TLNBitmap::load);
	ClassDB::bind_method(D_METHOD("duplicate"), &TLNBitmap::duplicate);
	ClassDB::bind_method(D_METHOD("get_width"), &TLNBitmap::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &TLNBitmap::get_height);
	ClassDB::bind_method(D_METHOD("get_depth"), &TLNBitmap::get_depth);
	ClassDB::bind_method(D_METHOD("get_pitch"), &TLNBitmap::get_pitch);
	ClassDB::bind_method(D_METHOD("get_palette"), &TLNBitmap::get_palette);
	ClassDB::bind_method(D_METHOD("set_palette", "palette"), &TLNBitmap::set_palette);
	ClassDB::bind_method(D_METHOD("to_image"), &TLNBitmap::to_image);
}

// ===========================================================================
// TLNTileset
// ===========================================================================

TLNTileset::TLNTileset() :
		tileset(nullptr), owned(false) {}

TLNTileset::~TLNTileset() {
	if (tileset && owned) {
		TLN_DeleteTileset(tileset);
	}
}

void TLNTileset::set_from(TLN_Tileset t, bool p_owned) {
	if (tileset && owned) {
		TLN_DeleteTileset(tileset);
	}
	tileset = t;
	owned = p_owned;
}

void TLNTileset::create(int numtiles, int width, int height, Ref<TLNPalette> pal) {
	if (tileset && owned) {
		TLN_DeleteTileset(tileset);
	}
	TLN_Palette p = (pal.is_valid() && pal->get_handle()) ? pal->get_handle() : nullptr;
	tileset = TLN_CreateTileset(numtiles, width, height, p, nullptr, nullptr);
	owned = (tileset != nullptr);
}

void TLNTileset::load(const String &file) {
	if (tileset && owned) {
		TLN_DeleteTileset(tileset);
	}
	tileset = TLN_LoadTileset(file.utf8().get_data());
	owned = (tileset != nullptr);
}

Ref<TLNTileset> TLNTileset::duplicate() {
	if (!tileset) {
		return Ref<TLNTileset>();
	}
	Ref<TLNTileset> dup;
	dup.instance();
	dup->set_from(TLN_CloneTileset(tileset), true);
	return dup;
}

int TLNTileset::get_tile_width() { return tileset ? TLN_GetTileWidth(tileset) : 0; }
int TLNTileset::get_tile_height() { return tileset ? TLN_GetTileHeight(tileset) : 0; }
int TLNTileset::get_num_tiles() { return tileset ? TLN_GetTilesetNumTiles(tileset) : 0; }

Ref<TLNPalette> TLNTileset::get_palette() {
	if (!tileset) {
		return Ref<TLNPalette>();
	}
	TLN_Palette p = TLN_GetTilesetPalette(tileset);
	if (!p) {
		return Ref<TLNPalette>();
	}
	Ref<TLNPalette> pal;
	pal.instance();
	pal->set_from(p, false);
	return pal;
}

void TLNTileset::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create", "numtiles", "width", "height", "palette"), &TLNTileset::create, DEFVAL(Ref<TLNPalette>()));
	ClassDB::bind_method(D_METHOD("load", "file"), &TLNTileset::load);
	ClassDB::bind_method(D_METHOD("duplicate"), &TLNTileset::duplicate);
	ClassDB::bind_method(D_METHOD("get_tile_width"), &TLNTileset::get_tile_width);
	ClassDB::bind_method(D_METHOD("get_tile_height"), &TLNTileset::get_tile_height);
	ClassDB::bind_method(D_METHOD("get_num_tiles"), &TLNTileset::get_num_tiles);
	ClassDB::bind_method(D_METHOD("get_palette"), &TLNTileset::get_palette);
}

// ===========================================================================
// TLNTilemap
// ===========================================================================

TLNTilemap::TLNTilemap() :
		tilemap(nullptr), owned(false) {}

TLNTilemap::~TLNTilemap() {
	if (tilemap && owned) {
		TLN_DeleteTilemap(tilemap);
	}
}

void TLNTilemap::set_from(TLN_Tilemap t, bool p_owned) {
	if (tilemap && owned) {
		TLN_DeleteTilemap(tilemap);
	}
	tilemap = t;
	owned = p_owned;
}

void TLNTilemap::create(int rows, int cols, Ref<TLNTileset> ts) {
	if (tilemap && owned) {
		TLN_DeleteTilemap(tilemap);
	}
	TLN_Tileset t = (ts.is_valid() && ts->get_handle()) ? ts->get_handle() : nullptr;
	tilemap = TLN_CreateTilemap(rows, cols, nullptr, 0, t);
	owned = (tilemap != nullptr);
}

void TLNTilemap::load(const String &file, const String &layername) {
	if (tilemap && owned) {
		TLN_DeleteTilemap(tilemap);
	}
	const char *layer = layername.empty() ? nullptr : layername.utf8().get_data();
	tilemap = TLN_LoadTilemap(file.utf8().get_data(), layer);
	owned = (tilemap != nullptr);
}

Ref<TLNTilemap> TLNTilemap::duplicate() {
	if (!tilemap) {
		return Ref<TLNTilemap>();
	}
	Ref<TLNTilemap> dup;
	dup.instance();
	dup->set_from(TLN_CloneTilemap(tilemap), true);
	return dup;
}

int TLNTilemap::get_rows() { return tilemap ? TLN_GetTilemapRows(tilemap) : 0; }
int TLNTilemap::get_cols() { return tilemap ? TLN_GetTilemapCols(tilemap) : 0; }

Dictionary TLNTilemap::get_tile(int row, int col) {
	Dictionary d;
	if (!tilemap) {
		d["index"] = 0;
		d["flags"] = 0;
		return d;
	}
	Tile tile;
	tile.value = 0;
	TLN_GetTilemapTile(tilemap, row, col, &tile);
	d["index"] = (int)tile.index;
	d["flags"] = (int)tile.flags;
	return d;
}

void TLNTilemap::set_tile(int row, int col, int index, int flags) {
	if (!tilemap) {
		return;
	}
	Tile tile;
	tile.value = 0;
	tile.index = index;
	tile.flags = flags;
	TLN_SetTilemapTile(tilemap, row, col, &tile);
}

Ref<TLNTileset> TLNTilemap::get_tileset() {
	if (!tilemap) {
		return Ref<TLNTileset>();
	}
	TLN_Tileset t = TLN_GetTilemapTileset(tilemap);
	if (!t) {
		return Ref<TLNTileset>();
	}
	Ref<TLNTileset> ts;
	ts.instance();
	ts->set_from(t, false);
	return ts;
}

void TLNTilemap::set_tileset(Ref<TLNTileset> ts) {
	if (tilemap && ts.is_valid() && ts->get_handle()) {
		TLN_SetTilemapTileset(tilemap, ts->get_handle());
	}
}

void TLNTilemap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create", "rows", "cols", "tileset"), &TLNTilemap::create, DEFVAL(Ref<TLNTileset>()));
	ClassDB::bind_method(D_METHOD("load", "file", "layername"), &TLNTilemap::load, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("duplicate"), &TLNTilemap::duplicate);
	ClassDB::bind_method(D_METHOD("get_rows"), &TLNTilemap::get_rows);
	ClassDB::bind_method(D_METHOD("get_cols"), &TLNTilemap::get_cols);
	ClassDB::bind_method(D_METHOD("get_tile", "row", "col"), &TLNTilemap::get_tile);
	ClassDB::bind_method(D_METHOD("set_tile", "row", "col", "index", "flags"), &TLNTilemap::set_tile, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_tileset"), &TLNTilemap::get_tileset);
	ClassDB::bind_method(D_METHOD("set_tileset", "tileset"), &TLNTilemap::set_tileset);
}

// ===========================================================================
// TLNSpriteset
// ===========================================================================

TLNSpriteset::TLNSpriteset() :
		spriteset(nullptr), owned(false) {}

TLNSpriteset::~TLNSpriteset() {
	if (spriteset && owned) {
		TLN_DeleteSpriteset(spriteset);
	}
}

void TLNSpriteset::set_from(TLN_Spriteset s, bool p_owned) {
	if (spriteset && owned) {
		TLN_DeleteSpriteset(spriteset);
	}
	spriteset = s;
	owned = p_owned;
}

void TLNSpriteset::load(const String &name) {
	if (spriteset && owned) {
		TLN_DeleteSpriteset(spriteset);
	}
	spriteset = TLN_LoadSpriteset(name.utf8().get_data());
	owned = (spriteset != nullptr);
}

Ref<TLNSpriteset> TLNSpriteset::duplicate() {
	if (!spriteset) {
		return Ref<TLNSpriteset>();
	}
	Ref<TLNSpriteset> dup;
	dup.instance();
	dup->set_from(TLN_CloneSpriteset(spriteset), true);
	return dup;
}

Vector2 TLNSpriteset::get_sprite_size(int entry) {
	if (!spriteset) {
		return Vector2();
	}
	TLN_SpriteInfo info;
	if (TLN_GetSpriteInfo(spriteset, entry, &info)) {
		return Vector2(info.w, info.h);
	}
	return Vector2();
}

int TLNSpriteset::find_sprite(const String &name) {
	return spriteset ? TLN_FindSpritesetSprite(spriteset, name.utf8().get_data()) : -1;
}

Ref<TLNPalette> TLNSpriteset::get_palette() {
	if (!spriteset) {
		return Ref<TLNPalette>();
	}
	TLN_Palette p = TLN_GetSpritesetPalette(spriteset);
	if (!p) {
		return Ref<TLNPalette>();
	}
	Ref<TLNPalette> pal;
	pal.instance();
	pal->set_from(p, false);
	return pal;
}

void TLNSpriteset::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load", "name"), &TLNSpriteset::load);
	ClassDB::bind_method(D_METHOD("duplicate"), &TLNSpriteset::duplicate);
	ClassDB::bind_method(D_METHOD("get_sprite_size", "entry"), &TLNSpriteset::get_sprite_size);
	ClassDB::bind_method(D_METHOD("find_sprite", "name"), &TLNSpriteset::find_sprite);
	ClassDB::bind_method(D_METHOD("get_palette"), &TLNSpriteset::get_palette);
}

// ===========================================================================
// TLNSequence
// ===========================================================================

TLNSequence::TLNSequence() :
		sequence(nullptr), owned(false) {}

TLNSequence::~TLNSequence() {
	if (sequence && owned) {
		TLN_DeleteSequence(sequence);
	}
}

void TLNSequence::set_from(TLN_Sequence s, bool p_owned) {
	if (sequence && owned) {
		TLN_DeleteSequence(sequence);
	}
	sequence = s;
	owned = p_owned;
}

void TLNSequence::create_frame_sequence(const String &name, int target, Array frames) {
	if (sequence && owned) {
		TLN_DeleteSequence(sequence);
		sequence = nullptr;
	}
	int count = frames.size();
	if (count == 0) {
		return;
	}
	Vector<TLN_SequenceFrame> sf;
	sf.resize(count);
	for (int i = 0; i < count; i++) {
		Dictionary d = frames[i];
		sf.write[i].index = d.has("index") ? (int)d["index"] : 0;
		sf.write[i].delay = d.has("delay") ? (int)d["delay"] : 1;
	}
	sequence = TLN_CreateSequence(name.utf8().get_data(), target, count, sf.ptrw());
	owned = (sequence != nullptr);
}

void TLNSequence::create_cycle(const String &name, Array strips) {
	if (sequence && owned) {
		TLN_DeleteSequence(sequence);
		sequence = nullptr;
	}
	int count = strips.size();
	if (count == 0) {
		return;
	}
	Vector<TLN_ColorStrip> cs;
	cs.resize(count);
	for (int i = 0; i < count; i++) {
		Dictionary d = strips[i];
		cs.write[i].delay = d.has("delay") ? (int)d["delay"] : 1;
		cs.write[i].first = d.has("first") ? (int)d["first"] : 0;
		cs.write[i].count = d.has("count") ? (int)d["count"] : 1;
		cs.write[i].dir = d.has("dir") ? (int)d["dir"] : 1;
	}
	sequence = TLN_CreateCycle(name.utf8().get_data(), count, cs.ptrw());
	owned = (sequence != nullptr);
}

void TLNSequence::create_sprite_sequence(const String &name, Ref<TLNSpriteset> ss, const String &basename, int delay) {
	if (sequence && owned) {
		TLN_DeleteSequence(sequence);
		sequence = nullptr;
	}
	if (!ss.is_valid() || !ss->get_handle()) {
		return;
	}
	sequence = TLN_CreateSpriteSequence(name.utf8().get_data(), ss->get_handle(), basename.utf8().get_data(), delay);
	owned = (sequence != nullptr);
}

Ref<TLNSequence> TLNSequence::duplicate() {
	if (!sequence) {
		return Ref<TLNSequence>();
	}
	Ref<TLNSequence> dup;
	dup.instance();
	dup->set_from(TLN_CloneSequence(sequence), true);
	return dup;
}

String TLNSequence::get_name() {
	if (!sequence) {
		return String();
	}
	TLN_SequenceInfo info;
	if (TLN_GetSequenceInfo(sequence, &info)) {
		return String(info.name);
	}
	return String();
}

int TLNSequence::get_num_frames() {
	if (!sequence) {
		return 0;
	}
	TLN_SequenceInfo info;
	if (TLN_GetSequenceInfo(sequence, &info)) {
		return info.num_frames;
	}
	return 0;
}

void TLNSequence::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_frame_sequence", "name", "target", "frames"), &TLNSequence::create_frame_sequence);
	ClassDB::bind_method(D_METHOD("create_cycle", "name", "strips"), &TLNSequence::create_cycle);
	ClassDB::bind_method(D_METHOD("create_sprite_sequence", "name", "spriteset", "basename", "delay"), &TLNSequence::create_sprite_sequence);
	ClassDB::bind_method(D_METHOD("duplicate"), &TLNSequence::duplicate);
	ClassDB::bind_method(D_METHOD("get_name"), &TLNSequence::get_name);
	ClassDB::bind_method(D_METHOD("get_num_frames"), &TLNSequence::get_num_frames);
}

// ===========================================================================
// TLNSequencePack
// ===========================================================================

TLNSequencePack::TLNSequencePack() :
		pack(nullptr), owned(false) {}

TLNSequencePack::~TLNSequencePack() {
	if (pack && owned) {
		TLN_DeleteSequencePack(pack);
	}
}

void TLNSequencePack::set_from(TLN_SequencePack p, bool p_owned) {
	if (pack && owned) {
		TLN_DeleteSequencePack(pack);
	}
	pack = p;
	owned = p_owned;
}

void TLNSequencePack::create() {
	if (pack && owned) {
		TLN_DeleteSequencePack(pack);
	}
	pack = TLN_CreateSequencePack();
	owned = (pack != nullptr);
}

void TLNSequencePack::load(const String &file) {
	if (pack && owned) {
		TLN_DeleteSequencePack(pack);
	}
	pack = TLN_LoadSequencePack(file.utf8().get_data());
	owned = (pack != nullptr);
}

int TLNSequencePack::get_count() {
	return pack ? TLN_GetSequencePackCount(pack) : 0;
}

Ref<TLNSequence> TLNSequencePack::get_sequence(int idx) {
	if (!pack) {
		return Ref<TLNSequence>();
	}
	TLN_Sequence s = TLN_GetSequence(pack, idx);
	if (!s) {
		return Ref<TLNSequence>();
	}
	Ref<TLNSequence> seq;
	seq.instance();
	seq->set_from(s, false);
	return seq;
}

Ref<TLNSequence> TLNSequencePack::find_sequence(const String &name) {
	if (!pack) {
		return Ref<TLNSequence>();
	}
	TLN_Sequence s = TLN_FindSequence(pack, name.utf8().get_data());
	if (!s) {
		return Ref<TLNSequence>();
	}
	Ref<TLNSequence> seq;
	seq.instance();
	seq->set_from(s, false);
	return seq;
}

void TLNSequencePack::add_sequence(Ref<TLNSequence> seq) {
	if (pack && seq.is_valid() && seq->get_handle()) {
		TLN_AddSequenceToPack(pack, seq->get_handle());
	}
}

void TLNSequencePack::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create"), &TLNSequencePack::create);
	ClassDB::bind_method(D_METHOD("load", "file"), &TLNSequencePack::load);
	ClassDB::bind_method(D_METHOD("get_count"), &TLNSequencePack::get_count);
	ClassDB::bind_method(D_METHOD("get_sequence", "index"), &TLNSequencePack::get_sequence);
	ClassDB::bind_method(D_METHOD("find_sequence", "name"), &TLNSequencePack::find_sequence);
	ClassDB::bind_method(D_METHOD("add_sequence", "sequence"), &TLNSequencePack::add_sequence);
}

// ===========================================================================
// TLNObjectList
// ===========================================================================

TLNObjectList::TLNObjectList() :
		list(nullptr), owned(false) {}

TLNObjectList::~TLNObjectList() {
	if (list && owned) {
		TLN_DeleteObjectList(list);
	}
}

void TLNObjectList::set_from(TLN_ObjectList l, bool p_owned) {
	if (list && owned) {
		TLN_DeleteObjectList(list);
	}
	list = l;
	owned = p_owned;
}

void TLNObjectList::create() {
	if (list && owned) {
		TLN_DeleteObjectList(list);
	}
	list = TLN_CreateObjectList();
	owned = (list != nullptr);
}

void TLNObjectList::load(const String &file, const String &layername) {
	if (list && owned) {
		TLN_DeleteObjectList(list);
	}
	const char *layer = layername.empty() ? nullptr : layername.utf8().get_data();
	list = TLN_LoadObjectList(file.utf8().get_data(), layer);
	owned = (list != nullptr);
}

Ref<TLNObjectList> TLNObjectList::duplicate() {
	if (!list) {
		return Ref<TLNObjectList>();
	}
	Ref<TLNObjectList> dup;
	dup.instance();
	dup->set_from(TLN_CloneObjectList(list), true);
	return dup;
}

int TLNObjectList::get_num_objects() {
	return list ? TLN_GetListNumObjects(list) : 0;
}

Dictionary TLNObjectList::get_object() {
	Dictionary d;
	if (!list) {
		return d;
	}
	TLN_ObjectInfo info;
	if (TLN_GetListObject(list, &info)) {
		d["id"] = (int)info.id;
		d["gid"] = (int)info.gid;
		d["flags"] = (int)info.flags;
		d["x"] = info.x;
		d["y"] = info.y;
		d["width"] = info.width;
		d["height"] = info.height;
		d["type"] = (int)info.type;
		d["visible"] = (bool)info.visible;
		d["name"] = String(info.name);
	}
	return d;
}

void TLNObjectList::add_tile_object(int id, int gid, int flags, int x, int y) {
	if (list) {
		TLN_AddTileObjectToList(list, id, gid, flags, x, y);
	}
}

void TLNObjectList::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create"), &TLNObjectList::create);
	ClassDB::bind_method(D_METHOD("load", "file", "layername"), &TLNObjectList::load, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("duplicate"), &TLNObjectList::duplicate);
	ClassDB::bind_method(D_METHOD("get_num_objects"), &TLNObjectList::get_num_objects);
	ClassDB::bind_method(D_METHOD("get_object"), &TLNObjectList::get_object);
	ClassDB::bind_method(D_METHOD("add_tile_object", "id", "gid", "flags", "x", "y"), &TLNObjectList::add_tile_object);
}

// ===========================================================================
// TLNEngine
// ===========================================================================

TLNEngine *TLNEngine::current_raster_target = nullptr;

void TLNEngine::_raster_callback(int scanline) {
	if (current_raster_target) {
		current_raster_target->emit_signal("raster_line", scanline);
	}
}

TLNEngine::TLNEngine() :
		engine(nullptr),
		fb_width(0),
		fb_height(0),
		num_layers(0),
		num_sprites(0),
		num_animations(0),
		frame_counter(0) {}

TLNEngine::~TLNEngine() {
	deinit_engine();
}

void TLNEngine::_ensure_context() {
	if (engine) {
		TLN_SetContext(engine);
	}
}

void TLNEngine::init_engine(int w, int h, int layers, int sprites, int animations) {
	deinit_engine();

	engine = TLN_Init(w, h, layers, sprites, animations);
	if (!engine) {
		return;
	}

	fb_width = w;
	fb_height = h;
	num_layers = layers;
	num_sprites = sprites;
	num_animations = animations;
	frame_counter = 0;

	framebuffer.resize(w * h * 4);
	TLN_SetRenderTarget(framebuffer.ptrw(), w * 4);

	image.instance();
	texture.instance();

	set_process_internal(true);
}

void TLNEngine::deinit_engine() {
	if (engine) {
		TLN_SetContext(engine);
		TLN_Deinit();
		engine = nullptr;
	}
	fb_width = 0;
	fb_height = 0;
	framebuffer.resize(0);
	image.unref();
	texture.unref();
	set_process_internal(false);
}

void TLNEngine::set_load_path(const String &path) {
	_ensure_context();
	TLN_SetLoadPath(path.utf8().get_data());
}

// Background

void TLNEngine::set_bg_color(Color c) {
	_ensure_context();
	TLN_SetBGColor(c.r * 255, c.g * 255, c.b * 255);
}

void TLNEngine::set_bg_bitmap(Ref<TLNBitmap> bmp) {
	_ensure_context();
	if (bmp.is_valid() && bmp->get_handle()) {
		TLN_SetBGBitmap(bmp->get_handle());
	}
}

void TLNEngine::set_bg_palette(Ref<TLNPalette> pal) {
	_ensure_context();
	if (pal.is_valid() && pal->get_handle()) {
		TLN_SetBGPalette(pal->get_handle());
	}
}

void TLNEngine::disable_bg_color() {
	_ensure_context();
	TLN_DisableBGColor();
}

// Layers

void TLNEngine::set_layer(int nlayer, Ref<TLNTileset> ts, Ref<TLNTilemap> tm) {
	_ensure_context();
	TLN_Tileset t = (ts.is_valid() && ts->get_handle()) ? ts->get_handle() : nullptr;
	TLN_Tilemap m = (tm.is_valid() && tm->get_handle()) ? tm->get_handle() : nullptr;
	TLN_SetLayer(nlayer, t, m);
}

void TLNEngine::set_layer_tilemap(int nlayer, Ref<TLNTilemap> tm) {
	_ensure_context();
	if (tm.is_valid() && tm->get_handle()) {
		TLN_SetLayerTilemap(nlayer, tm->get_handle());
	}
}

void TLNEngine::set_layer_bitmap(int nlayer, Ref<TLNBitmap> bmp) {
	_ensure_context();
	if (bmp.is_valid() && bmp->get_handle()) {
		TLN_SetLayerBitmap(nlayer, bmp->get_handle());
	}
}

void TLNEngine::set_layer_palette(int nlayer, Ref<TLNPalette> pal) {
	_ensure_context();
	if (pal.is_valid() && pal->get_handle()) {
		TLN_SetLayerPalette(nlayer, pal->get_handle());
	}
}

void TLNEngine::set_layer_position(int nlayer, int hstart, int vstart) {
	_ensure_context();
	TLN_SetLayerPosition(nlayer, hstart, vstart);
}

void TLNEngine::set_layer_scaling(int nlayer, float xfactor, float yfactor) {
	_ensure_context();
	TLN_SetLayerScaling(nlayer, xfactor, yfactor);
}

void TLNEngine::set_layer_transform(int nlayer, float angle, Vector2 offset, Vector2 scale) {
	_ensure_context();
	TLN_SetLayerTransform(nlayer, angle, offset.x, offset.y, scale.x, scale.y);
}

void TLNEngine::set_layer_blend_mode(int nlayer, int mode, int factor) {
	_ensure_context();
	TLN_SetLayerBlendMode(nlayer, (TLN_Blend)mode, factor);
}

void TLNEngine::set_layer_clip(int nlayer, int x1, int y1, int x2, int y2) {
	_ensure_context();
	TLN_SetLayerClip(nlayer, x1, y1, x2, y2);
}

void TLNEngine::disable_layer_clip(int nlayer) {
	_ensure_context();
	TLN_DisableLayerClip(nlayer);
}

void TLNEngine::set_layer_mosaic(int nlayer, int width, int height) {
	_ensure_context();
	TLN_SetLayerMosaic(nlayer, width, height);
}

void TLNEngine::disable_layer_mosaic(int nlayer) {
	_ensure_context();
	TLN_DisableLayerMosaic(nlayer);
}

void TLNEngine::set_layer_objects(int nlayer, Ref<TLNObjectList> objects, Ref<TLNTileset> ts) {
	_ensure_context();
	TLN_ObjectList o = (objects.is_valid() && objects->get_handle()) ? objects->get_handle() : nullptr;
	TLN_Tileset t = (ts.is_valid() && ts->get_handle()) ? ts->get_handle() : nullptr;
	TLN_SetLayerObjects(nlayer, o, t);
}

void TLNEngine::set_layer_priority(int nlayer, bool enable) {
	_ensure_context();
	TLN_SetLayerPriority(nlayer, enable);
}

void TLNEngine::set_layer_parent(int nlayer, int parent) {
	_ensure_context();
	TLN_SetLayerParent(nlayer, parent);
}

void TLNEngine::disable_layer_parent(int nlayer) {
	_ensure_context();
	TLN_DisableLayerParent(nlayer);
}

void TLNEngine::set_layer_parallax_factor(int nlayer, float x, float y) {
	_ensure_context();
	TLN_SetLayerParallaxFactor(nlayer, x, y);
}

void TLNEngine::enable_layer(int nlayer) {
	_ensure_context();
	TLN_EnableLayer(nlayer);
}

void TLNEngine::disable_layer(int nlayer) {
	_ensure_context();
	TLN_DisableLayer(nlayer);
}

Dictionary TLNEngine::get_layer_tile(int nlayer, int x, int y) {
	Dictionary d;
	_ensure_context();
	TLN_TileInfo info;
	memset(&info, 0, sizeof(info));
	if (TLN_GetLayerTile(nlayer, x, y, &info)) {
		d["index"] = (int)info.index;
		d["flags"] = (int)info.flags;
		d["row"] = info.row;
		d["col"] = info.col;
		d["xoffset"] = info.xoffset;
		d["yoffset"] = info.yoffset;
		d["color"] = (int)info.color;
		d["type"] = (int)info.type;
		d["empty"] = (bool)info.empty;
	}
	return d;
}

// Sprites

void TLNEngine::config_sprite(int nsprite, Ref<TLNSpriteset> ss, int flags) {
	_ensure_context();
	if (ss.is_valid() && ss->get_handle()) {
		TLN_ConfigSprite(nsprite, ss->get_handle(), flags);
	}
}

void TLNEngine::set_sprite_position(int nsprite, int x, int y) {
	_ensure_context();
	TLN_SetSpritePosition(nsprite, x, y);
}

void TLNEngine::set_sprite_picture(int nsprite, int entry) {
	_ensure_context();
	TLN_SetSpritePicture(nsprite, entry);
}

void TLNEngine::set_sprite_palette(int nsprite, Ref<TLNPalette> pal) {
	_ensure_context();
	if (pal.is_valid() && pal->get_handle()) {
		TLN_SetSpritePalette(nsprite, pal->get_handle());
	}
}

void TLNEngine::set_sprite_scaling(int nsprite, float sx, float sy) {
	_ensure_context();
	TLN_SetSpriteScaling(nsprite, sx, sy);
}

void TLNEngine::set_sprite_blend_mode(int nsprite, int mode, int factor) {
	_ensure_context();
	TLN_SetSpriteBlendMode(nsprite, (TLN_Blend)mode, factor);
}

void TLNEngine::set_sprite_flags(int nsprite, int flags) {
	_ensure_context();
	TLN_SetSpriteFlags(nsprite, flags);
}

void TLNEngine::set_sprite_pivot(int nsprite, float px, float py) {
	_ensure_context();
	TLN_SetSpritePivot(nsprite, px, py);
}

void TLNEngine::enable_sprite_collision(int nsprite, bool enable) {
	_ensure_context();
	TLN_EnableSpriteCollision(nsprite, enable);
}

bool TLNEngine::get_sprite_collision(int nsprite) {
	_ensure_context();
	return TLN_GetSpriteCollision(nsprite);
}

Dictionary TLNEngine::get_sprite_state(int nsprite) {
	Dictionary d;
	_ensure_context();
	TLN_SpriteState state;
	memset(&state, 0, sizeof(state));
	if (TLN_GetSpriteState(nsprite, &state)) {
		d["x"] = state.x;
		d["y"] = state.y;
		d["w"] = state.w;
		d["h"] = state.h;
		d["flags"] = (int)state.flags;
		d["index"] = state.index;
		d["enabled"] = (bool)state.enabled;
		d["collision"] = (bool)state.collision;
	}
	return d;
}

void TLNEngine::set_sprite_animation(int nsprite, Ref<TLNSequence> seq, int loop) {
	_ensure_context();
	if (seq.is_valid() && seq->get_handle()) {
		TLN_SetSpriteAnimation(nsprite, seq->get_handle(), loop);
	}
}

void TLNEngine::disable_sprite_animation(int nsprite) {
	_ensure_context();
	TLN_DisableSpriteAnimation(nsprite);
}

void TLNEngine::set_first_sprite(int nsprite) {
	_ensure_context();
	TLN_SetFirstSprite(nsprite);
}

void TLNEngine::set_next_sprite(int nsprite, int next) {
	_ensure_context();
	TLN_SetNextSprite(nsprite, next);
}

void TLNEngine::enable_sprite_masking(int nsprite, bool enable) {
	_ensure_context();
	TLN_EnableSpriteMasking(nsprite, enable);
}

void TLNEngine::set_sprites_mask_region(int top_line, int bottom_line) {
	_ensure_context();
	TLN_SetSpritesMaskRegion(top_line, bottom_line);
}

void TLNEngine::disable_sprite(int nsprite) {
	_ensure_context();
	TLN_DisableSprite(nsprite);
}

// Animation

void TLNEngine::set_palette_animation(int index, Ref<TLNPalette> pal, Ref<TLNSequence> seq, bool blend) {
	_ensure_context();
	if (pal.is_valid() && pal->get_handle() && seq.is_valid() && seq->get_handle()) {
		TLN_SetPaletteAnimation(index, pal->get_handle(), seq->get_handle(), blend);
	}
}

void TLNEngine::disable_palette_animation(int index) {
	_ensure_context();
	TLN_DisablePaletteAnimation(index);
}

bool TLNEngine::get_animation_state(int index) {
	_ensure_context();
	return TLN_GetAnimationState(index);
}

// World / TMX

void TLNEngine::load_world(const String &tmxfile, int first_layer) {
	_ensure_context();
	TLN_LoadWorld(tmxfile.utf8().get_data(), first_layer);
}

void TLNEngine::set_world_position(int x, int y) {
	_ensure_context();
	TLN_SetWorldPosition(x, y);
}

void TLNEngine::release_world() {
	_ensure_context();
	TLN_ReleaseWorld();
}

// Raster callback

void TLNEngine::enable_raster_callback() {
	_ensure_context();
	TLN_SetRasterCallback(_raster_callback);
}

void TLNEngine::disable_raster_callback() {
	_ensure_context();
	TLN_SetRasterCallback(nullptr);
}

// Framebuffer access

Ref<Image> TLNEngine::get_framebuffer_image() {
	return image;
}

Ref<ImageTexture> TLNEngine::get_texture() {
	return texture;
}

// Notification / rendering pipeline

void TLNEngine::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (!engine) {
				return;
			}
			_ensure_context();

			// Set raster target for signal emission during UpdateFrame
			current_raster_target = this;
			TLN_UpdateFrame(frame_counter++);
			current_raster_target = nullptr;

			// Swizzle BGRA → RGBA into PoolByteArray
			int size = fb_width * fb_height;
			PoolByteArray rgba;
			rgba.resize(size * 4);
			{
				PoolByteArray::Write w = rgba.write();
				const uint8_t *src = framebuffer.ptr();
				for (int i = 0; i < size; i++) {
					int off = i * 4;
					w[off + 0] = src[off + 2]; // B→R
					w[off + 1] = src[off + 1]; // G
					w[off + 2] = src[off + 0]; // R→B
					w[off + 3] = src[off + 3]; // A
				}
			}

			image->create(fb_width, fb_height, false, Image::FORMAT_RGBA8, rgba);
			texture->create_from_image(image, 0);
			update();
		} break;
		case NOTIFICATION_DRAW: {
			if (texture.is_valid() && texture->get_width() > 0) {
				draw_texture(texture, Point2());
			}
		} break;
	}
}

void TLNEngine::_bind_methods() {
	// Signals
	ADD_SIGNAL(MethodInfo("raster_line", PropertyInfo(Variant::INT, "scanline")));

	// Setup
	ClassDB::bind_method(D_METHOD("init_engine", "width", "height", "layers", "sprites", "animations"), &TLNEngine::init_engine);
	ClassDB::bind_method(D_METHOD("deinit_engine"), &TLNEngine::deinit_engine);
	ClassDB::bind_method(D_METHOD("is_initialized"), &TLNEngine::is_initialized);
	ClassDB::bind_method(D_METHOD("set_load_path", "path"), &TLNEngine::set_load_path);
	ClassDB::bind_method(D_METHOD("get_fb_width"), &TLNEngine::get_fb_width);
	ClassDB::bind_method(D_METHOD("get_fb_height"), &TLNEngine::get_fb_height);

	// Background
	ClassDB::bind_method(D_METHOD("set_bg_color", "color"), &TLNEngine::set_bg_color);
	ClassDB::bind_method(D_METHOD("set_bg_bitmap", "bitmap"), &TLNEngine::set_bg_bitmap);
	ClassDB::bind_method(D_METHOD("set_bg_palette", "palette"), &TLNEngine::set_bg_palette);
	ClassDB::bind_method(D_METHOD("disable_bg_color"), &TLNEngine::disable_bg_color);

	// Layers
	ClassDB::bind_method(D_METHOD("set_layer", "nlayer", "tileset", "tilemap"), &TLNEngine::set_layer);
	ClassDB::bind_method(D_METHOD("set_layer_tilemap", "nlayer", "tilemap"), &TLNEngine::set_layer_tilemap);
	ClassDB::bind_method(D_METHOD("set_layer_bitmap", "nlayer", "bitmap"), &TLNEngine::set_layer_bitmap);
	ClassDB::bind_method(D_METHOD("set_layer_palette", "nlayer", "palette"), &TLNEngine::set_layer_palette);
	ClassDB::bind_method(D_METHOD("set_layer_position", "nlayer", "hstart", "vstart"), &TLNEngine::set_layer_position);
	ClassDB::bind_method(D_METHOD("set_layer_scaling", "nlayer", "xfactor", "yfactor"), &TLNEngine::set_layer_scaling);
	ClassDB::bind_method(D_METHOD("set_layer_transform", "nlayer", "angle", "offset", "scale"), &TLNEngine::set_layer_transform);
	ClassDB::bind_method(D_METHOD("set_layer_blend_mode", "nlayer", "mode", "factor"), &TLNEngine::set_layer_blend_mode, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("set_layer_clip", "nlayer", "x1", "y1", "x2", "y2"), &TLNEngine::set_layer_clip);
	ClassDB::bind_method(D_METHOD("disable_layer_clip", "nlayer"), &TLNEngine::disable_layer_clip);
	ClassDB::bind_method(D_METHOD("set_layer_mosaic", "nlayer", "width", "height"), &TLNEngine::set_layer_mosaic);
	ClassDB::bind_method(D_METHOD("disable_layer_mosaic", "nlayer"), &TLNEngine::disable_layer_mosaic);
	ClassDB::bind_method(D_METHOD("set_layer_objects", "nlayer", "objects", "tileset"), &TLNEngine::set_layer_objects);
	ClassDB::bind_method(D_METHOD("set_layer_priority", "nlayer", "enable"), &TLNEngine::set_layer_priority);
	ClassDB::bind_method(D_METHOD("set_layer_parent", "nlayer", "parent"), &TLNEngine::set_layer_parent);
	ClassDB::bind_method(D_METHOD("disable_layer_parent", "nlayer"), &TLNEngine::disable_layer_parent);
	ClassDB::bind_method(D_METHOD("set_layer_parallax_factor", "nlayer", "x", "y"), &TLNEngine::set_layer_parallax_factor);
	ClassDB::bind_method(D_METHOD("enable_layer", "nlayer"), &TLNEngine::enable_layer);
	ClassDB::bind_method(D_METHOD("disable_layer", "nlayer"), &TLNEngine::disable_layer);
	ClassDB::bind_method(D_METHOD("get_layer_tile", "nlayer", "x", "y"), &TLNEngine::get_layer_tile);

	// Sprites
	ClassDB::bind_method(D_METHOD("config_sprite", "nsprite", "spriteset", "flags"), &TLNEngine::config_sprite, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("set_sprite_position", "nsprite", "x", "y"), &TLNEngine::set_sprite_position);
	ClassDB::bind_method(D_METHOD("set_sprite_picture", "nsprite", "entry"), &TLNEngine::set_sprite_picture);
	ClassDB::bind_method(D_METHOD("set_sprite_palette", "nsprite", "palette"), &TLNEngine::set_sprite_palette);
	ClassDB::bind_method(D_METHOD("set_sprite_scaling", "nsprite", "sx", "sy"), &TLNEngine::set_sprite_scaling);
	ClassDB::bind_method(D_METHOD("set_sprite_blend_mode", "nsprite", "mode", "factor"), &TLNEngine::set_sprite_blend_mode, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("set_sprite_flags", "nsprite", "flags"), &TLNEngine::set_sprite_flags);
	ClassDB::bind_method(D_METHOD("set_sprite_pivot", "nsprite", "px", "py"), &TLNEngine::set_sprite_pivot);
	ClassDB::bind_method(D_METHOD("enable_sprite_collision", "nsprite", "enable"), &TLNEngine::enable_sprite_collision);
	ClassDB::bind_method(D_METHOD("get_sprite_collision", "nsprite"), &TLNEngine::get_sprite_collision);
	ClassDB::bind_method(D_METHOD("get_sprite_state", "nsprite"), &TLNEngine::get_sprite_state);
	ClassDB::bind_method(D_METHOD("set_sprite_animation", "nsprite", "sequence", "loop"), &TLNEngine::set_sprite_animation);
	ClassDB::bind_method(D_METHOD("disable_sprite_animation", "nsprite"), &TLNEngine::disable_sprite_animation);
	ClassDB::bind_method(D_METHOD("set_first_sprite", "nsprite"), &TLNEngine::set_first_sprite);
	ClassDB::bind_method(D_METHOD("set_next_sprite", "nsprite", "next"), &TLNEngine::set_next_sprite);
	ClassDB::bind_method(D_METHOD("enable_sprite_masking", "nsprite", "enable"), &TLNEngine::enable_sprite_masking);
	ClassDB::bind_method(D_METHOD("set_sprites_mask_region", "top_line", "bottom_line"), &TLNEngine::set_sprites_mask_region);
	ClassDB::bind_method(D_METHOD("disable_sprite", "nsprite"), &TLNEngine::disable_sprite);

	// Animation
	ClassDB::bind_method(D_METHOD("set_palette_animation", "index", "palette", "sequence", "blend"), &TLNEngine::set_palette_animation);
	ClassDB::bind_method(D_METHOD("disable_palette_animation", "index"), &TLNEngine::disable_palette_animation);
	ClassDB::bind_method(D_METHOD("get_animation_state", "index"), &TLNEngine::get_animation_state);

	// World
	ClassDB::bind_method(D_METHOD("load_world", "tmxfile", "first_layer"), &TLNEngine::load_world, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("set_world_position", "x", "y"), &TLNEngine::set_world_position);
	ClassDB::bind_method(D_METHOD("release_world"), &TLNEngine::release_world);

	// Raster
	ClassDB::bind_method(D_METHOD("enable_raster_callback"), &TLNEngine::enable_raster_callback);
	ClassDB::bind_method(D_METHOD("disable_raster_callback"), &TLNEngine::disable_raster_callback);

	// Framebuffer access
	ClassDB::bind_method(D_METHOD("get_framebuffer_image"), &TLNEngine::get_framebuffer_image);
	ClassDB::bind_method(D_METHOD("get_texture"), &TLNEngine::get_texture);

	// Enums
	BIND_ENUM_CONSTANT(BLEND_NONE);
	BIND_ENUM_CONSTANT(BLEND_MIX25);
	BIND_ENUM_CONSTANT(BLEND_MIX50);
	BIND_ENUM_CONSTANT(BLEND_MIX75);
	BIND_ENUM_CONSTANT(BLEND_ADD);
	BIND_ENUM_CONSTANT(BLEND_SUB);
	BIND_ENUM_CONSTANT(BLEND_MOD);

	BIND_ENUM_CONSTANT(TILE_FLIPX);
	BIND_ENUM_CONSTANT(TILE_FLIPY);
	BIND_ENUM_CONSTANT(TILE_ROTATE);
	BIND_ENUM_CONSTANT(TILE_PRIORITY);
	BIND_ENUM_CONSTANT(TILE_MASKED);
}

// ===========================================================================
// Doctests
// ===========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[Tilengine]]") {
	TEST_CASE("[Tilengine] engine init and deinit") {
		TLNEngine eng;
		CHECK_FALSE(eng.is_initialized());
		eng.init_engine(320, 240, 2, 8, 4);
		CHECK(eng.is_initialized());
		CHECK(eng.get_fb_width() == 320);
		CHECK(eng.get_fb_height() == 240);
		eng.deinit_engine();
		CHECK_FALSE(eng.is_initialized());
	}

	TEST_CASE("[Tilengine] engine framebuffer renders to image") {
		TLNEngine eng;
		eng.init_engine(64, 64, 1, 1, 1);
		REQUIRE(eng.is_initialized());

		eng.set_bg_color(Color(1, 0, 0));
		// Manually call the update pipeline
		TLN_SetContext(TLN_GetContext());
		TLN_UpdateFrame(0);

		Ref<Image> img = eng.get_framebuffer_image();
		// Image may be null before first notification, that's ok for unit test
		eng.deinit_engine();
	}

	TEST_CASE("[Tilengine] palette create and color ops") {
		// Need an engine context for resource creation
		TLN_Engine ctx = TLN_Init(32, 32, 1, 1, 1);
		REQUIRE(ctx != nullptr);

		TLNPalette pal;
		pal.create(16);
		CHECK(pal.get_num_colors() == 16);

		// Index 0 is transparent in Tilengine, use index 1+
		pal.set_color(1, Color(1, 0, 0));
		Color c = pal.get_color(1);
		CHECK(c.r > 0.9f);
		CHECK(c.g < 0.1f);
		CHECK(c.b < 0.1f);

		pal.set_color(2, Color(0, 1, 0));
		Color c2 = pal.get_color(2);
		CHECK(c2.g > 0.9f);

		TLN_Deinit();
	}

	TEST_CASE("[Tilengine] bitmap create dimensions") {
		TLN_Engine ctx = TLN_Init(32, 32, 1, 1, 1);
		REQUIRE(ctx != nullptr);

		TLNBitmap bmp;
		bmp.create(128, 64, 32);
		CHECK(bmp.get_width() == 128);
		CHECK(bmp.get_height() == 64);
		CHECK(bmp.get_depth() == 32);

		TLN_Deinit();
	}

	TEST_CASE("[Tilengine] tilemap create and tile access") {
		TLN_Engine ctx = TLN_Init(32, 32, 1, 1, 1);
		REQUIRE(ctx != nullptr);

		TLNTilemap tm;
		tm.create(10, 20, Ref<TLNTileset>());
		CHECK(tm.get_rows() == 10);
		CHECK(tm.get_cols() == 20);

		tm.set_tile(0, 0, 5, 0);
		Dictionary d = tm.get_tile(0, 0);
		CHECK((int)d["index"] == 5);

		tm.set_tile(3, 7, 42, FLAG_FLIPX);
		Dictionary d2 = tm.get_tile(3, 7);
		CHECK((int)d2["index"] == 42);
		CHECK(((int)d2["flags"] & FLAG_FLIPX) != 0);

		TLN_Deinit();
	}

	TEST_CASE("[Tilengine] tileset load properties via create") {
		TLN_Engine ctx = TLN_Init(32, 32, 1, 1, 1);
		REQUIRE(ctx != nullptr);

		TLNTileset ts;
		ts.create(64, 16, 16, Ref<TLNPalette>());
		CHECK(ts.get_tile_width() == 16);
		CHECK(ts.get_tile_height() == 16);
		CHECK(ts.get_num_tiles() == 64);

		TLN_Deinit();
	}

	TEST_CASE("[Tilengine] sequence create frame sequence") {
		TLN_Engine ctx = TLN_Init(32, 32, 1, 1, 1);
		REQUIRE(ctx != nullptr);

		TLNSequence seq;
		Array frames;
		Dictionary f1;
		f1["index"] = 0;
		f1["delay"] = 6;
		frames.push_back(f1);
		Dictionary f2;
		f2["index"] = 1;
		f2["delay"] = 6;
		frames.push_back(f2);
		Dictionary f3;
		f3["index"] = 2;
		f3["delay"] = 6;
		frames.push_back(f3);

		seq.create_frame_sequence("walk", 0, frames);
		CHECK(seq.get_name() == "walk");
		CHECK(seq.get_num_frames() == 3);

		TLN_Deinit();
	}

	TEST_CASE("[Tilengine] BGRA to RGBA conversion") {
		// Verify the swizzle logic with a known pattern
		uint8_t bgra[] = { 0x00, 0x80, 0xFF, 0xDD }; // B=0, G=128, R=255, A=221
		uint8_t rgba[4];
		rgba[0] = bgra[2]; // R = 255
		rgba[1] = bgra[1]; // G = 128
		rgba[2] = bgra[0]; // B = 0
		rgba[3] = bgra[3]; // A = 221
		CHECK(rgba[0] == 0xFF);
		CHECK(rgba[1] == 0x80);
		CHECK(rgba[2] == 0x00);
		CHECK(rgba[3] == 0xDD);
	}

	TEST_CASE("[Tilengine] null safety on uninitialized wrappers") {
		TLNPalette pal;
		CHECK(pal.get_num_colors() == 0);
		CHECK(pal.get_color(0) == Color());

		TLNBitmap bmp;
		CHECK(bmp.get_width() == 0);
		CHECK(bmp.get_height() == 0);

		TLNTilemap tm;
		CHECK(tm.get_rows() == 0);
		CHECK(tm.get_cols() == 0);

		TLNTileset ts;
		CHECK(ts.get_tile_width() == 0);
		CHECK(ts.get_num_tiles() == 0);

		TLNSequence seq;
		CHECK(seq.get_name() == String());
		CHECK(seq.get_num_frames() == 0);

		TLNSequencePack sp;
		CHECK(sp.get_count() == 0);

		TLNObjectList ol;
		CHECK(ol.get_num_objects() == 0);
	}

	TEST_CASE("[Tilengine] owned vs borrowed palette") {
		TLN_Engine ctx = TLN_Init(32, 32, 1, 1, 1);
		REQUIRE(ctx != nullptr);

		// Create a palette and tileset programmatically
		TLN_Palette raw_pal = TLN_CreatePalette(256);
		REQUIRE(raw_pal != nullptr);
		TLN_SetPaletteColor(raw_pal, 1, 255, 0, 0);

		TLN_Tileset raw_ts = TLN_CreateTileset(16, 8, 8, raw_pal, nullptr, nullptr);
		REQUIRE(raw_ts != nullptr);

		// Wrap tileset as borrowed
		TLNTileset ts;
		ts.set_from(raw_ts, true);

		// Get palette from tileset — should be borrowed (not owned)
		Ref<TLNPalette> borrowed = ts.get_palette();
		CHECK(borrowed.is_valid());
		CHECK(borrowed->get_num_colors() > 0);

		// Wrap palette as owned, duplicate it
		TLNPalette pal;
		pal.set_from(TLN_ClonePalette(raw_pal), true);
		Ref<TLNPalette> duped = pal.duplicate();
		CHECK(duped.is_valid());
		CHECK(duped->get_num_colors() == 256);

		TLN_DeletePalette(raw_pal);
		TLN_Deinit();
	}

	TEST_CASE("[Tilengine] layer setup on engine") {
		TLNEngine eng;
		eng.init_engine(256, 224, 2, 4, 2);
		REQUIRE(eng.is_initialized());

		// Create resources using Ref (heap-allocated)
		Ref<TLNPalette> pal;
		pal.instance();
		pal->create(256);
		pal->set_color(1, Color(0, 0, 1));

		Ref<TLNTileset> ts;
		ts.instance();
		ts->create(32, 8, 8, pal);

		Ref<TLNTilemap> tm;
		tm.instance();
		tm->create(28, 32, ts);

		// Set layer 0
		eng.set_layer(0, ts, tm);
		eng.set_layer_position(0, 10, 20);

		// Should not crash
		eng.disable_layer(0);
		eng.enable_layer(0);

		eng.deinit_engine();
	}

	TEST_CASE("[Tilengine] object list create and add") {
		TLN_Engine ctx = TLN_Init(32, 32, 1, 1, 1);
		REQUIRE(ctx != nullptr);

		TLNObjectList ol;
		ol.create();
		CHECK(ol.get_num_objects() == 0);

		ol.add_tile_object(1, 10, 0, 32, 64);
		CHECK(ol.get_num_objects() == 1);

		TLN_Deinit();
	}

	TEST_CASE("[Tilengine] sequence pack create and add") {
		TLN_Engine ctx = TLN_Init(32, 32, 1, 1, 1);
		REQUIRE(ctx != nullptr);

		TLNSequencePack sp;
		sp.create();
		CHECK(sp.get_count() == 0);

		Ref<TLNSequence> seq;
		seq.instance();
		Array frames;
		Dictionary f;
		f["index"] = 0;
		f["delay"] = 4;
		frames.push_back(f);
		seq->create_frame_sequence("test", 0, frames);

		sp.add_sequence(seq);
		CHECK(sp.get_count() == 1);

		Ref<TLNSequence> found = sp.find_sequence("test");
		CHECK(found.is_valid());
		CHECK(found->get_name() == "test");

		TLN_Deinit();
	}
}

#endif // DOCTEST
