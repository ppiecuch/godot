/**************************************************************************/
/*  visual_shape_2d.h                                                     */
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

#include "scene/2d/node_2d.h"
#include "scene/resources/concave_polygon_shape_2d.h"
#include "scene/resources/convex_polygon_shape_2d.h"
#include "scene/resources/shape_2d.h"

class VisualShape2D : public Node2D {
	GDCLASS(VisualShape2D, Node2D);

	Ref<Shape2D> shape;

	Ref<Shape2D> parent_shape;
	bool use_parent_shape = false;

	// Cache polygon-based parent geometry with this instance.
	Ref<ConvexPolygonShape2D> polygon_shape;

	Color color = Color(1, 1, 1, 1);

	bool debug_use_default_color = false;
	bool debug_sync_visible_collision_shapes = false;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_shape(const Ref<Shape2D> &p_shape);
	Ref<Shape2D> get_shape() const;

	void set_use_parent_shape(bool p_use_parent_shape);
	bool is_using_parent_shape() const;
	// Returns `true` if parent shape is changed.
	bool update_parent_shape();

	void set_color(const Color &p_color);
	Color get_color() const;

	void set_debug_use_default_color(bool p_debug_use_default_color);
	bool is_using_debug_default_color() const;

	void set_debug_sync_visible_collision_shapes(bool p_debug_sync_visible_collision_shapes);
	bool is_debug_sync_visible_collision_shapes() const;

	String get_configuration_warning() const;
};
