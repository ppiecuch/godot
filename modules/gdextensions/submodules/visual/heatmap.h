/**************************************************************************/
/*  heatmap.h                                                             */
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

// Heatmap
// Francois "@Razoric480" Belair
//
// Native Godot class that uses a floodfill algorithm. Radiating out from the player position, it covers the whole tilemap.
// Each step removed adds one to the layer count. This can then be used by agents for quick
// lookup based pathfinding, instead of calculating A-star paths or other, potentially more expensive pathfinding routines.

#ifndef GD_HEATMAP_H
#define GD_HEATMAP_H

#include "scene/2d/node_2d.h"

#include <future>
#include <vector>

// HeatCell. Simple structure used by the `refresh_cells_heat` member function to contain both
// position and current layer in the flood fill search.

class TileMap;

struct HeatCell {
	HeatCell(Vector2 t_position, int t_layer) {
		position = t_position;
		layer = t_layer;
	}
	Vector2 position;
	int layer;

	bool operator==(HeatCell const &other) {
		return other.layer == layer && other.position == position;
	}

	bool operator!=(HeatCell const &other) {
		return other.layer != layer || other.position != position;
	}
};

class Heatmap : public Node2D {
	GDCLASS(Heatmap, Node2D)

public:
	Heatmap();
	~Heatmap();

	Vector2 best_direction_for(Vector2 t_location, bool t_is_world_location);
	unsigned int calculate_point_index(Vector2 t_point);
	unsigned int calculate_point_index_for_world_position(Vector2 t_world_position);

protected:
	static void _bind_methods();

	void _notification(int p_what);

	void _ready();
	void _draw();
	void _process(float delta);

private:
	void find_all_obstacles();
	Vector2 refresh_cells_heat(Vector2 t_cell_position);
	bool is_out_of_bounds(Vector2 t_position);
	void on_Events_player_moved(Node *t_player);
	void thread_done(Vector2 t_cell_position);

private:
	NodePath m_pathfinding_tilemap;
	bool m_draw_debug;

	TileMap *m_grid;
	Rect2 m_map_limits;
	float m_y_min;
	float m_x_min;
	float m_y_max;
	float m_x_max;
	int m_max_heat;
	int m_max_heat_cache;
	bool m_updating;

	std::vector<int> m_cells_heat;
	std::vector<int> m_cells_heat_cache;
	std::vector<Vector2> m_obstacles;
	Vector2 m_last_player_cell_position;
	std::future<Vector2> m_future;
};

#endif /* GD_HEATMAP_H */
