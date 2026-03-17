/**************************************************************************/
/*  gd_mazegen.cpp                                                        */
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

#include "gd_mazegen.h"

// --- Config property accessors ---

void GdMazeGenerator::set_deadend_chance(float p_chance) { deadend_chance = CLAMP(p_chance, 0.0f, 1.0f); }
float GdMazeGenerator::get_deadend_chance() const { return deadend_chance; }

void GdMazeGenerator::set_reconnect_deadends_chance(float p_chance) { reconnect_deadends_chance = CLAMP(p_chance, 0.0f, 1.0f); }
float GdMazeGenerator::get_reconnect_deadends_chance() const { return reconnect_deadends_chance; }

void GdMazeGenerator::set_wiggle_chance(float p_chance) { wiggle_chance = CLAMP(p_chance, 0.0f, 1.0f); }
float GdMazeGenerator::get_wiggle_chance() const { return wiggle_chance; }

void GdMazeGenerator::set_extra_connection_chance(float p_chance) { extra_connection_chance = CLAMP(p_chance, 0.0f, 1.0f); }
float GdMazeGenerator::get_extra_connection_chance() const { return extra_connection_chance; }

void GdMazeGenerator::set_room_base_number(int p_count) { room_base_number = MAX(0, p_count); }
int GdMazeGenerator::get_room_base_number() const { return room_base_number; }

void GdMazeGenerator::set_room_size_min(int p_size) { room_size_min = MAX(3, p_size | 1); }
int GdMazeGenerator::get_room_size_min() const { return room_size_min; }

void GdMazeGenerator::set_room_size_max(int p_size) { room_size_max = MAX(3, p_size | 1); }
int GdMazeGenerator::get_room_size_max() const { return room_size_max; }

void GdMazeGenerator::set_constrain_hall_only(bool p_constrain) { constrain_hall_only = p_constrain; }
bool GdMazeGenerator::get_constrain_hall_only() const { return constrain_hall_only; }

void GdMazeGenerator::set_maze_seed(int p_seed) { maze_seed = p_seed; }
int GdMazeGenerator::get_maze_seed() const { return maze_seed; }

// --- Generation ---

void GdMazeGenerator::generate(int p_width, int p_height) {
	ERR_FAIL_COND_MSG(p_width < 3 || p_height < 3, "Maze dimensions must be at least 3x3.");

	mazegen::Config cfg;
	cfg.DEADEND_CHANCE = deadend_chance;
	cfg.RECONNECT_DEADENDS_CHANCE = reconnect_deadends_chance;
	cfg.WIGGLE_CHANCE = wiggle_chance;
	cfg.EXTRA_CONNECTION_CHANCE = extra_connection_chance;
	cfg.ROOM_BASE_NUMBER = room_base_number;
	cfg.ROOM_SIZE_MIN = room_size_min;
	cfg.ROOM_SIZE_MAX = room_size_max;
	cfg.CONSTRAIN_HALL_ONLY = constrain_hall_only;

	if (maze_seed >= 0) {
		generator.set_seed(static_cast<unsigned int>(maze_seed));
	}

	generator.generate(p_width, p_height, cfg);
}

void GdMazeGenerator::generate_with_constraints(int p_width, int p_height, PoolVector2Array p_constraints) {
	ERR_FAIL_COND_MSG(p_width < 3 || p_height < 3, "Maze dimensions must be at least 3x3.");

	mazegen::Config cfg;
	cfg.DEADEND_CHANCE = deadend_chance;
	cfg.RECONNECT_DEADENDS_CHANCE = reconnect_deadends_chance;
	cfg.WIGGLE_CHANCE = wiggle_chance;
	cfg.EXTRA_CONNECTION_CHANCE = extra_connection_chance;
	cfg.ROOM_BASE_NUMBER = room_base_number;
	cfg.ROOM_SIZE_MIN = room_size_min;
	cfg.ROOM_SIZE_MAX = room_size_max;
	cfg.CONSTRAIN_HALL_ONLY = constrain_hall_only;

	mazegen::PointSet constraints;
	PoolVector2Array::Read r = p_constraints.read();
	for (int i = 0; i < p_constraints.size(); i++) {
		constraints.insert(mazegen::Point(static_cast<int>(r[i].x), static_cast<int>(r[i].y)));
	}

	if (maze_seed >= 0) {
		generator.set_seed(static_cast<unsigned int>(maze_seed));
	}

	generator.generate(p_width, p_height, cfg, constraints);
}

// --- Query results ---

int GdMazeGenerator::get_width() const { return generator.maze_width(); }
int GdMazeGenerator::get_height() const { return generator.maze_height(); }

int GdMazeGenerator::get_region_at(int p_x, int p_y) const {
	return generator.region_at(p_x, p_y);
}

bool GdMazeGenerator::is_wall(int p_x, int p_y) const {
	return generator.region_at(p_x, p_y) == mazegen::NOTHING_ID;
}

bool GdMazeGenerator::is_room_id(int p_id) const { return mazegen::is_room(p_id); }
bool GdMazeGenerator::is_hall_id(int p_id) const { return mazegen::is_hall(p_id); }
bool GdMazeGenerator::is_door_id(int p_id) const { return mazegen::is_door(p_id); }

String GdMazeGenerator::get_warnings() const {
	return String(generator.get_warnings().c_str());
}

int GdMazeGenerator::get_used_seed() const {
	return static_cast<int>(generator.get_seed());
}

// --- Room data ---

int GdMazeGenerator::get_room_count() const {
	return static_cast<int>(generator.get_rooms().size());
}

Dictionary GdMazeGenerator::get_room(int p_index) const {
	Dictionary d;
	const auto &rooms = generator.get_rooms();
	ERR_FAIL_INDEX_V(p_index, static_cast<int>(rooms.size()), d);
	const mazegen::Room &room = rooms[p_index];
	d["id"] = room.id;
	d["position"] = Vector2(room.min_point.x, room.min_point.y);
	d["end"] = Vector2(room.max_point.x, room.max_point.y);
	d["size"] = Vector2(room.max_point.x - room.min_point.x + 1, room.max_point.y - room.min_point.y + 1);
	d["center"] = Vector2(
			(room.min_point.x + room.max_point.x) * 0.5f,
			(room.min_point.y + room.max_point.y) * 0.5f);
	return d;
}

Array GdMazeGenerator::get_rooms() const {
	Array arr;
	for (int i = 0; i < get_room_count(); i++) {
		arr.push_back(get_room(i));
	}
	return arr;
}

// --- Hall data ---

int GdMazeGenerator::get_hall_count() const {
	return static_cast<int>(generator.get_halls().size());
}

Dictionary GdMazeGenerator::get_hall(int p_index) const {
	Dictionary d;
	const auto &halls = generator.get_halls();
	ERR_FAIL_INDEX_V(p_index, static_cast<int>(halls.size()), d);
	const mazegen::Hall &hall = halls[p_index];
	d["id"] = hall.id;
	d["start"] = Vector2(hall.start.x, hall.start.y);
	return d;
}

Array GdMazeGenerator::get_halls() const {
	Array arr;
	for (int i = 0; i < get_hall_count(); i++) {
		arr.push_back(get_hall(i));
	}
	return arr;
}

// --- Door data ---

int GdMazeGenerator::get_door_count() const {
	return static_cast<int>(generator.get_doors().size());
}

Dictionary GdMazeGenerator::get_door(int p_index) const {
	Dictionary d;
	const auto &doors = generator.get_doors();
	ERR_FAIL_INDEX_V(p_index, static_cast<int>(doors.size()), d);
	const mazegen::Door &door = doors[p_index];
	d["id"] = door.id;
	d["position"] = Vector2(door.position.x, door.position.y);
	d["room_id"] = door.room_id;
	d["hall_id"] = door.hall_id;
	d["is_hidden"] = door.is_hidden;
	return d;
}

Array GdMazeGenerator::get_doors() const {
	Array arr;
	for (int i = 0; i < get_door_count(); i++) {
		arr.push_back(get_door(i));
	}
	return arr;
}

// --- Grid data ---

PoolIntArray GdMazeGenerator::get_grid_row(int p_y) const {
	PoolIntArray row;
	int w = get_width();
	ERR_FAIL_COND_V(p_y < 0 || p_y >= get_height(), row);
	row.resize(w);
	PoolIntArray::Write wr = row.write();
	for (int x = 0; x < w; x++) {
		wr[x] = generator.region_at(x, p_y);
	}
	return row;
}

PoolIntArray GdMazeGenerator::get_flat_grid() const {
	int w = get_width();
	int h = get_height();
	PoolIntArray grid;
	grid.resize(w * h);
	PoolIntArray::Write wr = grid.write();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			wr[y * w + x] = generator.region_at(x, y);
		}
	}
	return grid;
}

// --- Image export ---

Ref<Image> GdMazeGenerator::to_image(int p_cell_size) const {
	int w = get_width();
	int h = get_height();
	ERR_FAIL_COND_V(w == 0 || h == 0, Ref<Image>());
	p_cell_size = MAX(1, p_cell_size);

	Ref<Image> img;
	img.instance();
	img->create(w * p_cell_size, h * p_cell_size, false, Image::FORMAT_L8);
	img->lock();

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			float val = (generator.region_at(x, y) == mazegen::NOTHING_ID) ? 0.0f : 1.0f;
			Color c(val, val, val);
			for (int cy = 0; cy < p_cell_size; cy++) {
				for (int cx = 0; cx < p_cell_size; cx++) {
					img->set_pixel(x * p_cell_size + cx, y * p_cell_size + cy, c);
				}
			}
		}
	}

	img->unlock();
	return img;
}

Ref<Image> GdMazeGenerator::to_image_colored(int p_cell_size) const {
	int w = get_width();
	int h = get_height();
	ERR_FAIL_COND_V(w == 0 || h == 0, Ref<Image>());
	p_cell_size = MAX(1, p_cell_size);

	Ref<Image> img;
	img.instance();
	img->create(w * p_cell_size, h * p_cell_size, false, Image::FORMAT_RGBA8);
	img->lock();

	const Color wall_color(0.15f, 0.15f, 0.2f);
	const Color hall_color(0.85f, 0.85f, 0.8f);
	const Color room_color(0.6f, 0.8f, 0.6f);
	const Color door_color(0.9f, 0.6f, 0.3f);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int id = generator.region_at(x, y);
			Color c;
			if (id == mazegen::NOTHING_ID) {
				c = wall_color;
			} else if (mazegen::is_door(id)) {
				c = door_color;
			} else if (mazegen::is_room(id)) {
				c = room_color;
			} else {
				c = hall_color;
			}
			for (int cy = 0; cy < p_cell_size; cy++) {
				for (int cx = 0; cx < p_cell_size; cx++) {
					img->set_pixel(x * p_cell_size + cx, y * p_cell_size + cy, c);
				}
			}
		}
	}

	img->unlock();
	return img;
}

// --- TileMap helpers ---

Array GdMazeGenerator::get_wall_cells() const {
	Array arr;
	int w = get_width();
	int h = get_height();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (generator.region_at(x, y) == mazegen::NOTHING_ID) {
				arr.push_back(Vector2(x, y));
			}
		}
	}
	return arr;
}

Array GdMazeGenerator::get_floor_cells() const {
	Array arr;
	int w = get_width();
	int h = get_height();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (generator.region_at(x, y) != mazegen::NOTHING_ID) {
				arr.push_back(Vector2(x, y));
			}
		}
	}
	return arr;
}

// --- Constructor ---

GdMazeGenerator::GdMazeGenerator() {
}

// --- Bindings ---

void GdMazeGenerator::_bind_methods() {
	// Config properties
	ClassDB::bind_method(D_METHOD("set_deadend_chance", "chance"), &GdMazeGenerator::set_deadend_chance);
	ClassDB::bind_method(D_METHOD("get_deadend_chance"), &GdMazeGenerator::get_deadend_chance);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deadend_chance", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_deadend_chance", "get_deadend_chance");

	ClassDB::bind_method(D_METHOD("set_reconnect_deadends_chance", "chance"), &GdMazeGenerator::set_reconnect_deadends_chance);
	ClassDB::bind_method(D_METHOD("get_reconnect_deadends_chance"), &GdMazeGenerator::get_reconnect_deadends_chance);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "reconnect_deadends_chance", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_reconnect_deadends_chance", "get_reconnect_deadends_chance");

	ClassDB::bind_method(D_METHOD("set_wiggle_chance", "chance"), &GdMazeGenerator::set_wiggle_chance);
	ClassDB::bind_method(D_METHOD("get_wiggle_chance"), &GdMazeGenerator::get_wiggle_chance);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wiggle_chance", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_wiggle_chance", "get_wiggle_chance");

	ClassDB::bind_method(D_METHOD("set_extra_connection_chance", "chance"), &GdMazeGenerator::set_extra_connection_chance);
	ClassDB::bind_method(D_METHOD("get_extra_connection_chance"), &GdMazeGenerator::get_extra_connection_chance);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "extra_connection_chance", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_extra_connection_chance", "get_extra_connection_chance");

	ClassDB::bind_method(D_METHOD("set_room_base_number", "count"), &GdMazeGenerator::set_room_base_number);
	ClassDB::bind_method(D_METHOD("get_room_base_number"), &GdMazeGenerator::get_room_base_number);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "room_base_number", PROPERTY_HINT_RANGE, "0,1000"), "set_room_base_number", "get_room_base_number");

	ClassDB::bind_method(D_METHOD("set_room_size_min", "size"), &GdMazeGenerator::set_room_size_min);
	ClassDB::bind_method(D_METHOD("get_room_size_min"), &GdMazeGenerator::get_room_size_min);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "room_size_min", PROPERTY_HINT_RANGE, "3,51,2"), "set_room_size_min", "get_room_size_min");

	ClassDB::bind_method(D_METHOD("set_room_size_max", "size"), &GdMazeGenerator::set_room_size_max);
	ClassDB::bind_method(D_METHOD("get_room_size_max"), &GdMazeGenerator::get_room_size_max);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "room_size_max", PROPERTY_HINT_RANGE, "3,51,2"), "set_room_size_max", "get_room_size_max");

	ClassDB::bind_method(D_METHOD("set_constrain_hall_only", "constrain"), &GdMazeGenerator::set_constrain_hall_only);
	ClassDB::bind_method(D_METHOD("get_constrain_hall_only"), &GdMazeGenerator::get_constrain_hall_only);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "constrain_hall_only"), "set_constrain_hall_only", "get_constrain_hall_only");

	ClassDB::bind_method(D_METHOD("set_maze_seed", "seed"), &GdMazeGenerator::set_maze_seed);
	ClassDB::bind_method(D_METHOD("get_maze_seed"), &GdMazeGenerator::get_maze_seed);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "maze_seed"), "set_maze_seed", "get_maze_seed");

	// Generation
	ClassDB::bind_method(D_METHOD("generate", "width", "height"), &GdMazeGenerator::generate);
	ClassDB::bind_method(D_METHOD("generate_with_constraints", "width", "height", "constraints"), &GdMazeGenerator::generate_with_constraints);

	// Query
	ClassDB::bind_method(D_METHOD("get_width"), &GdMazeGenerator::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &GdMazeGenerator::get_height);
	ClassDB::bind_method(D_METHOD("get_region_at", "x", "y"), &GdMazeGenerator::get_region_at);
	ClassDB::bind_method(D_METHOD("is_wall", "x", "y"), &GdMazeGenerator::is_wall);
	ClassDB::bind_method(D_METHOD("is_room_id", "id"), &GdMazeGenerator::is_room_id);
	ClassDB::bind_method(D_METHOD("is_hall_id", "id"), &GdMazeGenerator::is_hall_id);
	ClassDB::bind_method(D_METHOD("is_door_id", "id"), &GdMazeGenerator::is_door_id);
	ClassDB::bind_method(D_METHOD("get_warnings"), &GdMazeGenerator::get_warnings);
	ClassDB::bind_method(D_METHOD("get_used_seed"), &GdMazeGenerator::get_used_seed);

	// Room/Hall/Door data
	ClassDB::bind_method(D_METHOD("get_room_count"), &GdMazeGenerator::get_room_count);
	ClassDB::bind_method(D_METHOD("get_room", "index"), &GdMazeGenerator::get_room);
	ClassDB::bind_method(D_METHOD("get_rooms"), &GdMazeGenerator::get_rooms);
	ClassDB::bind_method(D_METHOD("get_hall_count"), &GdMazeGenerator::get_hall_count);
	ClassDB::bind_method(D_METHOD("get_hall", "index"), &GdMazeGenerator::get_hall);
	ClassDB::bind_method(D_METHOD("get_halls"), &GdMazeGenerator::get_halls);
	ClassDB::bind_method(D_METHOD("get_door_count"), &GdMazeGenerator::get_door_count);
	ClassDB::bind_method(D_METHOD("get_door", "index"), &GdMazeGenerator::get_door);
	ClassDB::bind_method(D_METHOD("get_doors"), &GdMazeGenerator::get_doors);

	// Grid export
	ClassDB::bind_method(D_METHOD("get_grid_row", "y"), &GdMazeGenerator::get_grid_row);
	ClassDB::bind_method(D_METHOD("get_flat_grid"), &GdMazeGenerator::get_flat_grid);

	// Image export
	ClassDB::bind_method(D_METHOD("to_image", "cell_size"), &GdMazeGenerator::to_image, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("to_image_colored", "cell_size"), &GdMazeGenerator::to_image_colored, DEFVAL(1));

	// TileMap helpers
	ClassDB::bind_method(D_METHOD("get_wall_cells"), &GdMazeGenerator::get_wall_cells);
	ClassDB::bind_method(D_METHOD("get_floor_cells"), &GdMazeGenerator::get_floor_cells);
}
