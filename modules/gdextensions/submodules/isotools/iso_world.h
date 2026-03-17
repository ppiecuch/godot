/**************************************************************************/
/*  iso_world.h                                                           */
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

#ifndef ISO_WORLD_H
#define ISO_WORLD_H

#include "iso_screen_solver.h"
#include "iso_sorting_solver.h"
#include "iso_warning_solver.h"

#include "scene/2d/node_2d.h"

class IsoObject;

class IsoWorld : public Node2D {
	GDCLASS(IsoWorld, Node2D);

	// Sorting properties
	real_t _tile_size;
	real_t _tile_ratio;
	real_t _tile_angle;
	real_t _tile_height;
	real_t _step_depth;
	real_t _start_depth;

	// Editor flags
	bool _show_iso_bounds;
	bool _show_screen_bounds;
	bool _show_depends;
	bool _show_quad_tree;

	// Iso matrix (2D projection)
	Transform2D _iso_matrix;
	Transform2D _iso_rmatrix;

	// Solvers
	IsoScreenSolver _screen_solver;
	IsoSortingSolver _sorting_solver;
	IsoWarningSolver _warning_solver;

	// Tracked objects
	Vector<IsoObject *> _iso_objects;

	void _update_iso_matrix();
	void _fix_iso_object_transforms();
	void _change_sorting_property();
	void _step_sorting_process();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	// Properties
	void set_tile_size(real_t p_size);
	real_t get_tile_size() const;

	void set_tile_ratio(real_t p_ratio);
	real_t get_tile_ratio() const;

	void set_tile_angle(real_t p_angle);
	real_t get_tile_angle() const;

	void set_tile_height(real_t p_height);
	real_t get_tile_height() const;

	void set_step_depth(real_t p_depth);
	real_t get_step_depth() const;

	void set_start_depth(real_t p_depth);
	real_t get_start_depth() const;

	void set_show_iso_bounds(bool p_show);
	bool get_show_iso_bounds() const;

	void set_show_screen_bounds(bool p_show);
	bool get_show_screen_bounds() const;

	void set_show_depends(bool p_show);
	bool get_show_depends() const;

	void set_show_quad_tree(bool p_show);
	bool get_show_quad_tree() const;

	// Coordinate conversion
	Vector2 iso_to_screen(const Vector3 &p_iso) const;
	Vector3 screen_to_iso(const Vector2 &p_screen) const;
	Vector3 screen_to_iso(const Vector2 &p_screen, real_t p_iso_z) const;

	// Vector/force conversion (iso Vector3 <-> screen Vector2)
	Vector2 iso_vector_to_screen(const Vector3 &p_iso_vec) const;
	Vector3 screen_vector_to_iso(const Vector2 &p_screen_vec) const;

	// Mouse helpers
	Vector3 mouse_iso_position(real_t p_iso_z = 0) const;
	Vector3 mouse_iso_tile_position(real_t p_iso_z = 0) const;

	// Internal object management (called by IsoObject)
	void internal_add_iso_object(IsoObject *p_object);
	void internal_remove_iso_object(IsoObject *p_object);
	void internal_mark_dirty(IsoObject *p_object);
	bool internal_is_visible(IsoObject *p_object) const;

	IsoWorld();
};

#endif // ISO_WORLD_H
