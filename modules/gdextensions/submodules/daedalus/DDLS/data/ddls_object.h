/**************************************************************************/
/*  ddls_object.h                                                         */
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

#pragma once

#include "core/math/transform_2d.h"
#include "core/reference.h"
#include "core/vector.h"

#include "ddls_fwd.h"

class DDLS_Object : public Reference {
	unsigned id;

	Transform2D matrix;
	Vector<Point2> coordinates;
	DDLSConstraintShape constraint_shape;

	Point2 pivot;

	Size2 scale;
	real_t rotation;
	Point2 translate;

	bool has_changed;

public:
	unsigned get_id() const { return id; }

	void update_values_from_matrix() {}

	void update_matrix_from_values() {
		matrix = Transform2D();
		matrix.translate(-pivot);
		matrix.scale(scale);
		matrix.rotate(rotation);
		matrix.translate(translate);
	}

	Point2 get_pivot() const { return pivot; }
	void set_pivot_x(const Point2 p_value) {
		if (pivot != p_value) {
			pivot = p_value;
			has_changed = true;
		}
	}

	Size2 get_scale() const { return scale; }
	void set_scale(const Size2 &p_value) {
		if (scale != p_value) {
			scale = p_value;
			has_changed = true;
		}
	}

	real_t get_rotation() const { return rotation; }
	void set_rotation(real_t p_value) {
		if (rotation != p_value) {
			rotation = p_value;
			has_changed = true;
		}
	}

	Vector2 get_translate() const { return translate; }
	void set_translate(const Vector2 p_translate) {
		if (translate != p_translate) {
			translate = p_translate;
			has_changed = true;
		}
	}

	Transform2D get_matrix() const { return matrix; }
	void set_matrix(const Transform2D &value) {
		matrix = value;
		has_changed = true;
	}

	Vector<Point2> get_coordinates() const { return coordinates; }
	void set_coordinates(Vector<Point2> p_coordinates) {
		coordinates = p_coordinates;
		has_changed = true;
	}
	void add_coordinates(const Point2 &p_coord) { coordinates.push_back(p_coord); }
	void add_coordinates(const Point2 &p_coord1, const Point2 &p_coord2) { coordinates.push_back(p_coord1, p_coord2); }

	DDLSConstraintShape get_constraint_shape() const;
	void set_constraint_shape(DDLSConstraintShape p_shape);

	bool is_changed() const { return has_changed; }
	void set_changed(bool p_state) { has_changed = p_state; }

	Vector<DDLSEdge> get_edges() const;

	DDLS_Object();
};
