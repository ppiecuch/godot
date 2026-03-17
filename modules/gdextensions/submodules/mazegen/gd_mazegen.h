/**************************************************************************/
/*  gd_mazegen.h                                                          */
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

#ifndef GD_MAZEGEN_H
#define GD_MAZEGEN_H

#include "core/image.h"
#include "core/reference.h"

#include "misc/mazegen.h"

class GdMazeGenerator : public Reference {
	GDCLASS(GdMazeGenerator, Reference);

	mazegen::Generator generator;

	// Config parameters (exposed to GDScript)
	float deadend_chance = 0.5f;
	float reconnect_deadends_chance = 0.5f;
	float wiggle_chance = 0.5f;
	float extra_connection_chance = 0.0f;
	int room_base_number = 30;
	int room_size_min = 7;
	int room_size_max = 9;
	bool constrain_hall_only = false;

	// Generation state
	int maze_seed = -1; // -1 means random

protected:
	static void _bind_methods();

public:
	// Configuration setters/getters
	void set_deadend_chance(float p_chance);
	float get_deadend_chance() const;

	void set_reconnect_deadends_chance(float p_chance);
	float get_reconnect_deadends_chance() const;

	void set_wiggle_chance(float p_chance);
	float get_wiggle_chance() const;

	void set_extra_connection_chance(float p_chance);
	float get_extra_connection_chance() const;

	void set_room_base_number(int p_count);
	int get_room_base_number() const;

	void set_room_size_min(int p_size);
	int get_room_size_min() const;

	void set_room_size_max(int p_size);
	int get_room_size_max() const;

	void set_constrain_hall_only(bool p_constrain);
	bool get_constrain_hall_only() const;

	void set_maze_seed(int p_seed);
	int get_maze_seed() const;

	// Generation
	void generate(int p_width, int p_height);
	void generate_with_constraints(int p_width, int p_height, PoolVector2Array p_constraints);

	// Query results
	int get_width() const;
	int get_height() const;
	int get_region_at(int p_x, int p_y) const;
	bool is_wall(int p_x, int p_y) const;
	bool is_room_id(int p_id) const;
	bool is_hall_id(int p_id) const;
	bool is_door_id(int p_id) const;
	String get_warnings() const;
	int get_used_seed() const;

	// Room data
	int get_room_count() const;
	Dictionary get_room(int p_index) const;
	Array get_rooms() const;

	// Hall data
	int get_hall_count() const;
	Dictionary get_hall(int p_index) const;
	Array get_halls() const;

	// Door data
	int get_door_count() const;
	Dictionary get_door(int p_index) const;
	Array get_doors() const;

	// Grid data export
	PoolIntArray get_grid_row(int p_y) const;
	PoolIntArray get_flat_grid() const;

	// Image export
	Ref<Image> to_image(int p_cell_size = 1) const;
	Ref<Image> to_image_colored(int p_cell_size = 1) const;

	// TileMap helper: returns Array of Vector2 positions
	Array get_wall_cells() const;
	Array get_floor_cells() const;

	GdMazeGenerator();
};

#endif // GD_MAZEGEN_H
