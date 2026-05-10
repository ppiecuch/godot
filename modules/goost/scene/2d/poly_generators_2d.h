/**************************************************************************/
/*  poly_generators_2d.h                                                  */
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

#include "core/math/geometry/2d/poly/offset/poly_offset.h"
#include "core/math/geometry/2d/poly/poly_node_2d.h"

#include "scene/2d/path_2d.h"

class PolyCircle2D : public PolyNode2D {
	GDCLASS(PolyCircle2D, PolyNode2D);

	real_t radius = 32.0;
	real_t max_error = 0.25;

protected:
	static void _bind_methods();
	virtual Vector<Vector<Point2>> _build_outlines();

public:
	void set_radius(real_t p_radius);
	real_t get_radius() const { return radius; }

	void set_max_error(real_t p_max_error);
	real_t get_max_error() const { return max_error; }

	PolyCircle2D() {
		build_outlines();
	}
};

class PolyEllipse2D : public PolyNode2D {
	GDCLASS(PolyEllipse2D, PolyNode2D);

	real_t width = 64.0;
	real_t height = 32.0;
	real_t max_error = 0.25;

protected:
	static void _bind_methods();
	virtual Vector<Vector<Point2>> _build_outlines();

public:
	void set_width(real_t p_width);
	real_t get_width() const { return width; }

	void set_height(real_t p_height);
	real_t get_height() const { return height; }

	void set_max_error(real_t p_max_error);
	real_t get_max_error() const { return max_error; }

	PolyEllipse2D() {
		build_outlines();
	}
};

class PolyCapsule2D : public PolyNode2D {
	GDCLASS(PolyCapsule2D, PolyNode2D);

	real_t radius = 32.0;
	real_t height = 64.0;
	real_t max_error = 0.25;

protected:
	static void _bind_methods();
	virtual Vector<Vector<Point2>> _build_outlines();

public:
	void set_radius(real_t p_radius);
	real_t get_radius() const { return radius; }

	void set_height(real_t p_height);
	real_t get_height() const { return height; }

	void set_max_error(real_t p_max_error);
	real_t get_max_error() const { return max_error; }

	PolyCapsule2D() {
		build_outlines();
	}
};

class PolyRectangle2D : public PolyNode2D {
	GDCLASS(PolyRectangle2D, PolyNode2D);

	Vector2 extents = Vector2(32, 32);

protected:
	static void _bind_methods();
	virtual Vector<Vector<Point2>> _build_outlines();

public:
	void set_extents(const Vector2 &p_extents);
	Vector2 get_extents() const { return extents; }

	PolyRectangle2D() {
		build_outlines();
	}
};

class PolyPath2D : public PolyNode2D {
	GDCLASS(PolyPath2D, PolyNode2D);

	Map<ObjectID, Ref<Curve2D>> paths; // Path2D : Cached Curve2D

	real_t buffer_offset = 32.0;
	Ref<PolyOffsetParameters2D> buffer_parameters;

	int tessellation_stages = 4;
	float tessellation_tolerance_degrees = 4.0f;

protected:
	void _notification(int p_what);
	static void _bind_methods();
	virtual Vector<Vector<Point2>> _build_outlines();

	virtual void add_child_notify(Node *p_child);
	virtual void remove_child_notify(Node *p_child);

public:
	void set_buffer_offset(real_t p_buffer_offset);
	real_t get_buffer_offset() const { return buffer_offset; }

	void set_buffer_parameters(const Ref<PolyOffsetParameters2D> &p_buffer_parameters);
	Ref<PolyOffsetParameters2D> get_buffer_parameters() const { return buffer_parameters; }

	void set_tessellation_stages(int p_tessellation_stages);
	int get_tessellation_stages() const { return tessellation_stages; }

	void set_tessellation_tolerance_degrees(float p_tessellation_tolerance_degrees);
	float get_tessellation_tolerance_degrees() const { return tessellation_tolerance_degrees; }

	virtual String get_configuration_warning() const;

	PolyPath2D() {
		set_process_internal(true);
		build_outlines();
	}
	~PolyPath2D() {
		paths.clear();
	}
};
