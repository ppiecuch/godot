/*************************************************************************/
/*  cable_2d.h                                                           */
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

/* cable2d.h */
#ifndef CABLE2D_H
#define CABLE2D_H

#include "scene/2d/node_2d.h"

class Cable2D : public Node2D {
	GDCLASS(Cable2D, Node2D);

public:
	Cable2D();

	void set_points(const PoolVector<Vector2> &p_points);
	PoolVector<Vector2> get_points() const;

	void set_points_forces(const PoolVector<Vector2> &p_forces);
	PoolVector<Vector2> get_points_forces() const;

	void set_point_force(int index, Vector2 force);
	Vector2 get_point_force(int index) const;

	void set_color(Color color);
	Color get_color() const;

	void set_width(float width);
	float get_width() const;

	void set_segments(int segments);
	int get_segments() const;

	void set_restlength_scale(float scale);
	float get_restlength_scale() const;

	void set_iterations(int iterations);
	int get_iterations() const;

private:
	void rebuild_points();
	void update_rest_length();

	void update_cable(float delta);
	void update_constraints();

protected:
	void _notification(int p_what);
	void _draw();

	static void _bind_methods();

public:
	PoolVector<Vector2> _points; // Pinned points
	PoolVector<Vector2> _rendered_points;
	PoolVector<Vector2> _old_points;
	PoolVector<float> _rest_lengths;
	PoolVector<Vector2> _point_forces; // Allows for scripts to sway the cables per segment.
	int _segments; // Number of points inbetween the pinned points.
	float _width;
	float _restlength_scale;
	float _force_damping;
	int _iterations;
	Color _color;
};

#endif
