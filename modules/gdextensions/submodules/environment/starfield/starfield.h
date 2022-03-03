/*************************************************************************/
/*  starfield.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef STARFIELD_H
#define STARFIELD_H

// Example:
// --------
// func _ready():
//     # Add layers:
//     var points = add_point_stars_layer(800, view_size, Color(0.8, 0.8, 0.8))
//     var squares = add_stars_layer(50, view_size, 2, Color(0.7, 0.7, 0.7))
//     var textures1 = add_texture_stars_layer(10, view_size, 16, STAR1_TEXTURE, Color(0.6, 0.6, 0.6))
//     var textures2 = add_texture_stars_layer(4, view_size, 64, STAR12_TEXTURE, Color(0.9, 0.9, 0.9))
//     # Set options:
//     set_layer_movement_opt(points, Vector2(1, 1), true)
//     set_layer_movement_opt(squares, Vector2(0.5, 0.5), false)
//     set_layer_color_opt(textures2, Color(0.6, 0.6, 0.6), 0.4)

#include "core/color.h"
#include "core/reference.h"
#include "scene/2d/canvas_item.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"

#include <vector>

typedef unsigned int layerid_t;

class Starfield : public Reference {
public:
	struct CacheInfo : public Reference {
		Ref<Texture> texture;
		Dictionary rects;
	};

	enum StarTexture {
		STAR_POINT,
		STAR1_TEXTURE,
		STAR2_TEXTURE,
		STAR3_TEXTURE,
		STAR4_TEXTURE,
		STAR5_TEXTURE,
		STAR6_TEXTURE,
		STAR7_TEXTURE,
		STAR8_TEXTURE,
		STAR9_TEXTURE,
		STAR10_TEXTURE,
		STAR11_TEXTURE,
		STAR12_TEXTURE,
		STAR_TEXTURE_VALID = STAR12_TEXTURE,
		STAR12_TEXTURE_FRAME1 = STAR12_TEXTURE,
		STAR12_TEXTURE_FRAME2,
		STAR12_TEXTURE_FRAME3,
		STAR12_TEXTURE_FRAME4,
		STAR12_TEXTURE_FRAME5,
		STAR12_TEXTURE_FRAME6,
	};

	void regenerate(layerid_t p_layer);
	void regenerate(layerid_t p_layer, Size2 p_layer_size);
	void regenerate(layerid_t p_layer, int p_number_of_stars);
	void regenerate(layerid_t p_layer, Size2 p_layer_size, int p_number_of_stars);

	layerid_t add_stars(int p_number_of_stars, Size2 p_layer_size, real_t p_star_size, StarTexture p_texture_id, const Color &p_color = Color::solid(0.6));

	void move(real_t p_delta, Vector2 p_movement);

	void set_color(layerid_t p_layer, const Color &p_base_color, real_t p_star_pulsation);
	void set_movement(layerid_t p_layer, Vector2 p_movement_scale, bool p_with_alpha);

	void ready(Node2D *p_owner);
	void draw(Node2D *p_canvas);

	Starfield();

private:
	bool _needs_refresh;
	Ref<ArrayMesh> _mesh_solid, _mesh_textured;

	struct StarsLayer {
		int num_stars;
		PoolVector2Array vertexes, uv;
		PoolColorArray colors;
		Size2 layer_size;
		Vector2 movement_scale;
		bool movement_scale_with_alpha;
		StarTexture texture_id;
		real_t star_size;
		real_t star_pulsation; // could be alpha pulsationn or texture animation
		Color base_color;
		bool _dirty;
	};

	std::vector<StarsLayer> _layers;

	void _push_quad(PoolVector2Array &array, Point2 center, real_t size);
	void _insert_quad(PoolVector2Array &array, int position, Point2 origin, real_t size);
	void _update_mesh();
	void _regenerate();
};

VARIANT_ENUM_CAST(Starfield::StarTexture);

#endif /* STARFIELD_H */
