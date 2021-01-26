/*************************************************************************/
/*  starfield.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "core/color.h"
#include "core/reference.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"

#include <vector>

typedef unsigned int layerid_t;

class Starfield : public Reference {
public:
	Starfield();

	void regenerate();
	void regenerate(layerid_t p_layer);
	void regenerate(layerid_t p_layer, Vector2 p_size);
	void regenerate(layerid_t p_layer, unsigned int p_number_of_stars);
	void regenerate(layerid_t p_layer, Vector2 p_size, unsigned int p_number_of_stars);

	layerid_t add_stars(layerid_t p_number_of_stars = 100, Vector2 p_size = Vector2(), const Color &p_color = Color(160, 160, 160), Ref<Texture> p_tex = Ref<Texture>());

	void move(Vector2 p_movement);

	void set_color(layerid_t p_layer, const Color &p_color);

	void draw(Node2D &canvas);

private:
	Ref<ArrayMesh> _mesh;

	struct StarsLayer {
		PoolPoint2Array positions;
		PoolColorArray colors;
		Vector2 size;
		Color color;
	};

	std::vector<StarsLayer> _layers;

	void _update_mesh();
};

#endif /* STARFIELD_H */
