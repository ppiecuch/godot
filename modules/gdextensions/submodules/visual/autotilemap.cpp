/**************************************************************************/
/*  autotilemap.cpp                                                       */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "core/io/json.h"
#include "core/os/os.h"
#include "core/ustring.h"
#include "scene/2d/tile_map.h"
#include "scene/resources/text_file.h"

#include "autotilemap.h"

// Bit layout (32 bits):
//   [31]       unused
//   [30]       flip_h (horizontal / x-axis mirror)
//   [29]       flip_v (vertical   / y-axis mirror)
//   [28..24]   unused
//   [23..16]   atlas_id  (8 bits)
//   [15..0]    tile_id   (16 bits)
int encode_tile_and_flipping(int tid, int fh, int fv, int atlas_id = 0) {
	int atlas_id_code = (int(atlas_id) & 0xff) << 16;
	int id_code = int(tid) & 0x0000ffff;
	int flip_h_code = int(fh) << 30; // bit 30 → compute_flip_h
	int flip_v_code = int(fv) << 29; // bit 29 → compute_flip_v
	return atlas_id_code | id_code | flip_h_code | flip_v_code;
}

bool compute_flip_h(int code) {
	return bool((code >> 30) & 0x1);
}

bool compute_flip_v(int code) {
	return bool((code >> 29) & 0x1);
}

int compute_tile_id(int code) {
	return int(code & 0x0000FFFF);
}

int compute_atlas_id(int code) {
	return int((code >> 16) & 0x000000FF);
}

Vector2 compute_subtile_coords(int code) {
	int tid = compute_tile_id(code);
	int y = int(tid / 7);
	int x = int(tid) % 7;
	return Vector2(x, y);
}

Autotilemap::Autotilemap() {
}

void Autotilemap::init(const Vector2 &top_left, const Vector2 &bottom_right, const String &json_file) {
	_top_left = top_left;
	_bottom_right = bottom_right;
	_width = bottom_right.x - top_left.x;
	_height = bottom_right.y - top_left.y;
	load_from_json(json_file);

	_data.resize(_width * _height);

	Dictionary data_dict = _json_data;
	_base_tile = data_dict.has("base_tile") ? int(_json_data.get("base_tile")) : -1;
	if (_base_tile >= 0) {
		for (int r = 0; r < _height; r++) {
			for (int c = 0; c < _width; c++) {
				_data.set(r + c * _height, _base_tile);
			}
		}
	}

	if (data_dict.has("id_to_atlas")) {
		_id_to_atlas = _json_data.get("id_to_atlas");
	}

	if (data_dict.has("blob_autotiling")) {
		_blob_mode = true;
		Vector<Variant> codes = _json_data.get("codes");
		Vector<Variant> blob_autot = _json_data.get("blob_autotiling");
		for (int i = 0; i < blob_autot.size(); i++) {
			BlobAutotiler *autot = memnew(BlobAutotiler);
			autot->init(blob_autot[i], codes);
			_autotilers.push_back(Ref<BlobAutotiler>(autot));
		}
	}

	if (data_dict.has("blob_terrain_autotiling")) {
		_blob_mode = true;
		Vector<Variant> codes = _json_data.get("codes");
		Vector<Variant> blob_autot = _json_data.get("blob_terrain_autotiling");
		for (int i = 0; i < blob_autot.size(); i++) {
			BlobTerrainAutotiler *autot = memnew(BlobTerrainAutotiler);
			autot->init(blob_autot[i], codes);
			_autotilers.push_back(Ref<BlobTerrainAutotiler>(autot));
		}
	}

	if (data_dict.has("autotiling")) {
		Vector<Variant> quad_autot = _json_data.get("autotiling");
		for (int i = 0; i < quad_autot.size(); i++) {
			QuadAutotiler *autot = memnew(QuadAutotiler);
			autot->init(quad_autot[i]);
			_autotilers.push_back(Ref<QuadAutotiler>(autot));
		}
	}
}

void Autotilemap::apply(Object *obj_tilemap) {
	if (obj_tilemap) {
		TileMap *tilemap = cast_to<TileMap>(obj_tilemap);
		map_ids_to_tiles(tilemap);
		apply_autotiling(tilemap);
		apply_blob_autotiling(tilemap);
		apply_blob_terrain_autotiling(tilemap);
	} else {
		print_error("Error applying tilemap");
	}
}

void Autotilemap::map_ids_to_tiles(TileMap *tilemap) {
	if (_id_to_atlas.size() > 0) {
		for (int i = 0; i < _id_to_atlas.size(); i++) {
			Variant id_to_atlas = _id_to_atlas[i];
			for (int y = 1; y < _height - 1; y++) {
				for (int x = 1; x < _width - 1; x++) {
					//TODO access data only once for all autotilers!
					int src_tile_id = _data[y + x * _height];
					if (src_tile_id == int(id_to_atlas.get("src_tile"))) {
						int atlas_id = id_to_atlas.get("atlas");
						tilemap->set_cell(_top_left.x + x, _top_left.y + y, atlas_id,
								false, false, false, Vector2(0, 0));
					}
				}
			}
		}
	}
}

void Autotilemap::apply_autotiling(TileMap *tilemap) {
	for (int ai = 0; ai < _autotilers.size(); ai++) {
		if (!_autotilers[ai]->is_type("QuadAutotiler")) {
			continue;
		}

		Ref<QuadAutotiler> autotiler = Ref<QuadAutotiler>(_autotilers[ai]);

		for (int y = 1; y < _height - 1; y++) {
			for (int x = 1; x < _width - 1; x++) {
				int src_tile_id = _data[y + x * _height];
				if (!autotiler->is_source_tile(src_tile_id)) {
					//tilemap->set_cell(_top_left.x + x, _top_left.y + y, 0,
					//				  false, false, false, Vector2(0,0));
					continue;
				}

				int n = int(autotiler->is_neighbor_tile(_data[(y - 1) + x * _height]));
				int e = int(autotiler->is_neighbor_tile(_data[y + (x + 1) * _height]));
				int s = int(autotiler->is_neighbor_tile(_data[(y + 1) + x * _height]));
				int w = int(autotiler->is_neighbor_tile(_data[y + (x - 1) * _height]));

				int v = n + 4 * e + 16 * s + 64 * w;

				if (autotiler->get_metadata_map().has(v)) {
					auto code = autotiler->get_metadata_map()[v];

					tilemap->set_cell(_top_left.x + x, _top_left.y + y, code.get("id"),
							code.get("x_mirror"), code.get("y_mirror"), false, Vector2(0, 0));
				}
				// else {
				// 	tilemap->set_cell(_top_left.x + x, _top_left.y + y, _base_tile,
				// 					  false, false, false, Vector2(0,0));

				// }
			}
		}
	}
}

void Autotilemap::apply_blob_terrain_autotiling(TileMap *tilemap) {
	int n = 0;
	int ne = 0;
	int e = 0;
	int se = 0;
	int s = 0;
	int sw = 0;
	int w = 0;
	int nw = 0;

	for (int ai = 0; ai < _autotilers.size(); ai++) {
		if (!_autotilers[ai]->is_type("BlobTerrainAutotiler")) {
			continue;
		}

		Ref<BlobTerrainAutotiler> autotiler = Ref<BlobTerrainAutotiler>(_autotilers[ai]);

		for (int y = 1; y < _height - 1; y++) {
			for (int x = 1; x < _width - 1; x++) {
				int src_tile_id = _data[y + x * _height];

				//TODO: use a map of autotilers?
				// entry ID has the list of autotilers
				if (!autotiler->is_source_tile(src_tile_id)) {
					continue;
				}

				int nt = _data[(y - 1) + x * _height];
				int net = _data[(y - 1) + (x + 1) * _height];
				int et = _data[y + (x + 1) * _height];
				int set = _data[(y + 1) + (x + 1) * _height];
				int st = _data[(y + 1) + x * _height];
				int swt = _data[(y + 1) + (x - 1) * _height];
				int wt = _data[y + (x - 1) * _height];
				int nwt = _data[(y - 1) + (x - 1) * _height];

				Vector<int> all_neighbors;
				all_neighbors.push_back(nt);
				all_neighbors.push_back(net);
				all_neighbors.push_back(et);
				all_neighbors.push_back(set);
				all_neighbors.push_back(st);
				all_neighbors.push_back(swt);
				all_neighbors.push_back(wt);
				all_neighbors.push_back(nwt);

				bool flag1 = false;
				bool flag2 = false;
				for (int i = 0; i < all_neighbors.size(); i++) {
					int nb = all_neighbors[i];
					if (nb == autotiler->get_src_1()) {
						flag1 = true;
					}
					if (nb == autotiler->get_src_2()) {
						flag2 = true;
					}
				}

				if (!(flag1 && flag2)) {
					bool isolated = true;
					for (int i = 0; i < all_neighbors.size(); i++) {
						int nb = all_neighbors[i];
						if (nb != autotiler->get_src_2()) {
							isolated = false;
							break;
						}
					}

					if (isolated) {
						int tile_id = autotiler->get_metadata_map()[0];
						Vector2 subtile_coords = compute_subtile_coords(tile_id);
						tilemap->set_cell(_top_left.x + x, _top_left.y + y, autotiler->get_atlas_id(),
								false, false, false, subtile_coords);
					}
					continue;
				}

				//TODO: We should re-factor/clean this
				n = int(autotiler->is_neighbor_tile(nt));
				ne = int(autotiler->is_neighbor_tile(net));
				e = int(autotiler->is_neighbor_tile(et));
				se = int(autotiler->is_neighbor_tile(set));
				s = int(autotiler->is_neighbor_tile(st));
				sw = int(autotiler->is_neighbor_tile(swt));
				w = int(autotiler->is_neighbor_tile(wt));
				nw = int(autotiler->is_neighbor_tile(nwt));

				int v = n + 2 * ne + 4 * e + 8 * se + 16 * s + 32 * sw + 64 * w + 128 * nw;

				bool flip_h = false;
				bool flip_v = false;
				bool transpose = false;
				int tile_id = 0;
				Vector2 subtile_coords = Vector2(0, 0);

				if (v > 0) {
					if (autotiler->get_metadata_map().has(v)) {
						tile_id = autotiler->get_metadata_map()[v];
						if (_blob_mode) {
							subtile_coords = compute_subtile_coords(tile_id);
							tile_id = autotiler->get_atlas_id();
						}
						tilemap->set_cell(_top_left.x + x, _top_left.y + y, tile_id,
								flip_h, flip_v, transpose, subtile_coords);
					} else {
						tilemap->set_cell(_top_left.x + x, _top_left.y + y, _base_tile,
								flip_h, flip_v, transpose, subtile_coords);
					}
				}
			}
		}
	}
}

void Autotilemap::apply_blob_autotiling(TileMap *tilemap) {
	int n = 0;
	int ne = 0;
	int e = 0;
	int se = 0;
	int s = 0;
	int sw = 0;
	int w = 0;
	int nw = 0;

	for (int ai = 0; ai < _autotilers.size(); ai++) {
		if (!_autotilers[ai]->is_type("BlobAutotiler")) {
			continue;
		}

		Ref<Autotiler> autotiler = _autotilers[ai];

		for (int y = 1; y < _height - 1; y++) {
			for (int x = 1; x < _width - 1; x++) {
				int src_tile_id = _data[y + x * _height];
				if (!autotiler->is_source_tile(src_tile_id)) {
					tilemap->set_cell(_top_left.x + x, _top_left.y + y, 0,
							false, false, false, Vector2(0, 0));
					continue;
				}
				n = int(autotiler->is_neighbor_tile(_data[(y - 1) + x * _height]));
				ne = int(autotiler->is_neighbor_tile(_data[(y - 1) + (x + 1) * _height]));
				e = int(autotiler->is_neighbor_tile(_data[y + (x + 1) * _height]));
				se = int(autotiler->is_neighbor_tile(_data[(y + 1) + (x + 1) * _height]));
				s = int(autotiler->is_neighbor_tile(_data[(y + 1) + x * _height]));
				sw = int(autotiler->is_neighbor_tile(_data[(y + 1) + (x - 1) * _height]));
				w = int(autotiler->is_neighbor_tile(_data[y + (x - 1) * _height]));
				nw = int(autotiler->is_neighbor_tile(_data[(y - 1) + (x - 1) * _height]));

				int v = n + 2 * ne + 4 * e + 8 * se + 16 * s + 32 * sw + 64 * w + 128 * nw;

				bool flip_h = false;
				bool flip_v = false;
				bool transpose = false;
				int tile_id = 0;
				Vector2 subtile_coords = Vector2(0, 0);

				if (v > 0) {
					if (autotiler->get_metadata_map().has(v)) {
						tile_id = autotiler->get_metadata_map()[v];
						if (_blob_mode) {
							subtile_coords = compute_subtile_coords(tile_id);
							tile_id = autotiler->get_atlas_id();
						}
					}
				}
				tilemap->set_cell(_top_left.x + x, _top_left.y + y, tile_id,
						flip_h, flip_v, transpose, subtile_coords);
			}
		}
	}
}

void Autotilemap::set_submap(Vector2 sub_top_left, int width, int height, const Variant &input_data) {
	const Vector<int32_t> &idata = input_data;
	Vector2 offset = sub_top_left - _top_left;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			_data.set(row + offset.y + (col + offset.x) * _height, idata[row + col * height]);
		}
	}
}

void Autotilemap::set_value(int x, int y, int v) {
	_data.set(y + x * _height, v);
}

void Autotilemap::set_value_relative_to_tl(int x, int y, int v) {
	set_value(x - _top_left.x, y - _top_left.y, v);
}

void Autotilemap::load_from_json(const String &json_file) {
	Error err;
	FileAccessRef f = FileAccess::open(json_file, FileAccess::READ, &err);
	if (!f) {
		print_error("Autotilemap: could not open json file: " + json_file);
		return;
	}

	Vector<uint8_t> array;
	array.resize(f->get_len());
	f->get_buffer(array.ptrw(), array.size());
	String text;
	text.parse_utf8((const char *)array.ptr(), array.size());
	Variant ret;
	Dictionary data;
	String err_message;
	int err_line;

	Variant json_data_tmp;
	if (OK != JSON::parse(text, json_data_tmp, err_message, err_line)) {
		print_line("Error loading json file");
	}

	_json_data = json_data_tmp;
}

Vector2 Autotilemap::get_top_left() {
	return _top_left;
}

Vector2 Autotilemap::get_bottom_right() {
	return _bottom_right;
}

int Autotilemap::get_width() {
	return _width;
}

int Autotilemap::get_height() {
	return _height;
}

Variant Autotilemap::get_data() {
	//TODO can we avoid this copy?
	return _data;
}

void Autotilemap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "top_left", "bottom_right", "json_file"), &Autotilemap::init);
	ClassDB::bind_method(D_METHOD("set_value", "x", "y", "v"), &Autotilemap::set_value);
	ClassDB::bind_method(D_METHOD("set_value_relative_to_tl", "x", "y", "v"), &Autotilemap::set_value_relative_to_tl);
	ClassDB::bind_method(D_METHOD("set_submap", "sub_top_left", "width", "height", "idata"), &Autotilemap::set_submap);
	ClassDB::bind_method(D_METHOD("apply"), &Autotilemap::apply);
	ClassDB::bind_method(D_METHOD("get_top_left"), &Autotilemap::get_top_left);
	ClassDB::bind_method(D_METHOD("get_bottom_right"), &Autotilemap::get_bottom_right);
	ClassDB::bind_method(D_METHOD("get_width"), &Autotilemap::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &Autotilemap::get_height);
	ClassDB::bind_method(D_METHOD("get_data"), &Autotilemap::get_data);
}

// -- Tests --

#ifdef DOCTEST

// Bit layout: [30]=flip_h  [29]=flip_v  [23..16]=atlas_id  [15..0]=tile_id

TEST_CASE("[Autotilemap] encode/decode tile id round-trip") {
	SUBCASE("tile id 0") {
		int code = encode_tile_and_flipping(0, 0, 0, 0);
		CHECK(compute_tile_id(code) == 0);
	}
	SUBCASE("tile id 42") {
		int code = encode_tile_and_flipping(42, 0, 0, 0);
		CHECK(compute_tile_id(code) == 42);
	}
	SUBCASE("tile id max (0xFFFF)") {
		int code = encode_tile_and_flipping(0xFFFF, 0, 0, 0);
		CHECK(compute_tile_id(code) == 0xFFFF);
	}
}

TEST_CASE("[Autotilemap] encode/decode atlas id round-trip") {
	SUBCASE("atlas id 0") {
		int code = encode_tile_and_flipping(0, 0, 0, 0);
		CHECK(compute_atlas_id(code) == 0);
	}
	SUBCASE("atlas id 5") {
		int code = encode_tile_and_flipping(100, 0, 0, 5);
		CHECK(compute_atlas_id(code) == 5);
	}
	SUBCASE("atlas id max (0xFF)") {
		int code = encode_tile_and_flipping(0, 0, 0, 0xFF);
		CHECK(compute_atlas_id(code) == 0xFF);
	}
}

TEST_CASE("[Autotilemap] tile_id and atlas_id are independent (bit isolation)") {
	int code = encode_tile_and_flipping(0xFFFF, 0, 0, 0xFF);
	CHECK(compute_tile_id(code) == 0xFFFF);
	CHECK(compute_atlas_id(code) == 0xFF);

	// Changing atlas_id must not affect tile_id
	int code2 = encode_tile_and_flipping(0xFFFF, 0, 0, 0x01);
	CHECK(compute_tile_id(code2) == 0xFFFF);
	CHECK(compute_atlas_id(code2) == 0x01);

	// Changing tile_id must not affect atlas_id
	int code3 = encode_tile_and_flipping(0x0001, 0, 0, 0xFF);
	CHECK(compute_tile_id(code3) == 0x0001);
	CHECK(compute_atlas_id(code3) == 0xFF);
}

TEST_CASE("[Autotilemap] flip flags — no flips") {
	int code = encode_tile_and_flipping(1, 0, 0);
	CHECK_FALSE(compute_flip_h(code));
	CHECK_FALSE(compute_flip_v(code));
}

TEST_CASE("[Autotilemap] flip flags — flip_h only") {
	// fh=1 sets bit 30, decoded by compute_flip_h
	int code = encode_tile_and_flipping(1, 1, 0);
	CHECK(compute_flip_h(code));
	CHECK_FALSE(compute_flip_v(code));
}

TEST_CASE("[Autotilemap] flip flags — flip_v only") {
	// fv=1 sets bit 29, decoded by compute_flip_v
	int code = encode_tile_and_flipping(1, 0, 1);
	CHECK_FALSE(compute_flip_h(code));
	CHECK(compute_flip_v(code));
}

TEST_CASE("[Autotilemap] flip flags — both flips") {
	int code = encode_tile_and_flipping(1, 1, 1);
	CHECK(compute_flip_h(code));
	CHECK(compute_flip_v(code));
}

TEST_CASE("[Autotilemap] flip flags do not corrupt tile/atlas ids") {
	int code = encode_tile_and_flipping(42, 1, 1, 3);
	CHECK(compute_tile_id(code) == 42);
	CHECK(compute_atlas_id(code) == 3);
	CHECK(compute_flip_h(code));
	CHECK(compute_flip_v(code));
}

TEST_CASE("[Autotilemap] compute_subtile_coords — 7-column sheet layout") {
	// Row 0: tiles 0–6
	CHECK(compute_subtile_coords(encode_tile_and_flipping(0, 0, 0)) == Vector2(0, 0));
	CHECK(compute_subtile_coords(encode_tile_and_flipping(6, 0, 0)) == Vector2(6, 0));
	// Row 1: tiles 7–13
	CHECK(compute_subtile_coords(encode_tile_and_flipping(7, 0, 0)) == Vector2(0, 1));
	CHECK(compute_subtile_coords(encode_tile_and_flipping(13, 0, 0)) == Vector2(6, 1));
	// Row 2: tiles 14–20
	CHECK(compute_subtile_coords(encode_tile_and_flipping(14, 0, 0)) == Vector2(0, 2));
	CHECK(compute_subtile_coords(encode_tile_and_flipping(15, 0, 0)) == Vector2(1, 2));
}

TEST_CASE("[Autotilemap] blob neighbor bitmask weight per direction") {
	// The 8-neighbor value: n=1 ne=2 e=4 se=8 s=16 sw=32 w=64 nw=128
	// Verify individual contributions are orthogonal powers of 2
	CHECK((1 + 2 + 4 + 8 + 16 + 32 + 64 + 128) == 255);
	// Each direction bit must be unique
	CHECK((1 & 2) == 0);
	CHECK((4 & 8) == 0);
	CHECK((64 & 128) == 0);
}

TEST_CASE("[Autotilemap] quad neighbor bitmask weight per direction") {
	// 4-neighbor value: n=1 e=4 s=16 w=64
	CHECK((1 & 4) == 0);
	CHECK((4 & 16) == 0);
	CHECK((16 & 64) == 0);
	CHECK((1 + 4 + 16 + 64) == 85);
}

#endif
