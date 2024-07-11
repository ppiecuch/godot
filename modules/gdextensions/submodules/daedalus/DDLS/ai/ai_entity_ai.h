/**************************************************************************/
/*  ai_entity_ai.h                                                        */
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

#include "core/math/vector2.h"
#include "core/reference.h"

#include "ddls_fwd.h"

class DDLS_EntityAI : public Reference {
	const int NUM_SEGMENTS = 6;

	real_t radius;
	real_t radius_squared;
	Point2 pos;
	Vector2 dir_norm;
	real_t angle_fov;
	real_t radius_fov;
	real_t radius_squared_fov;
	DDLSObject approximate_object;

public:
	void build_approximation();
	DDLSObject get_approximate_object();

	real_t get_radius_fov() const { return radius_fov; }
	void set_radius_fov(real_t pradius_fov) {
		radius_fov = pradius_fov;
		radius_squared_fov = radius_fov * radius_fov;
	}

	real_t get_angle_fov() { return angle_fov; }
	void set_angle_fov(real_t p_angle_fov) { angle_fov = p_angle_fov; }

	Point2 get_dir_norm() const { return dir_norm; }
	void set_dir_norm(const Vector2 p_dir) { dir_norm = p_dir; }

	Point2 get_pos() const { return pos; }
	void set_pos(const Point2 &p_pos) { pos = p_pos; }

	real_t get_radius() const { return radius; }
	void set_radius(real_t p_radius) {
		radius = p_radius;
		radius_squared = radius * radius;
	}
	real_t get_radius_squared() const { return radius_squared; }

	DDLS_EntityAI();
};
