/**************************************************************************/
/*  poly_shape_2d.h                                                       */
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

#include "core/math/geometry/2d/poly/poly_node_2d.h"

class PolyShape2D : public Node2D {
	GDCLASS(PolyShape2D, Node2D);

public:
	enum BuildMode {
		BUILD_TRIANGLES,
		BUILD_CONVEX,
		BUILD_SEGMENTS,
	};

private:
	Vector<Vector<Point2>> _collect_outlines();

protected:
	Vector<Vector<Point2>> shapes;
	bool update_queued = false;

	BuildMode build_mode = BUILD_TRIANGLES;
	Rect2 rect = Rect2(-10, -10, 20, 20);
	PolyNode2D *child = nullptr;

	virtual Vector<Vector<Point2>> _build_shapes();
	virtual void _apply_shapes(){};
	void _update_shapes();
	void _queue_update();

	virtual void add_child_notify(Node *p_child);
	virtual void remove_child_notify(Node *p_child);

	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_build_mode(BuildMode p_mode);
	BuildMode get_build_mode() const { return build_mode; }

	void update_shapes();
	Array get_shapes_array();

	virtual String get_configuration_warning() const;
};

VARIANT_ENUM_CAST(PolyShape2D::BuildMode);
