/**************************************************************************/
/*  iso_sorting_solver.h                                                  */
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

#ifndef ISO_SORTING_SOLVER_H
#define ISO_SORTING_SOLVER_H

#include "iso_object.h"
#include "iso_screen_solver.h"

#include "servers/visual_server.h"

class IsoWorld;

class IsoSortingSolver {
public:
	void on_add_iso_object(IsoObject *p_object) {
		// Nothing needed for 2D mode
	}

	void on_remove_iso_object(IsoObject *p_object) {
		// Nothing needed for 2D mode
	}

	bool on_mark_dirty_iso_object(IsoObject *p_object) {
		return false;
	}

	bool step_sorting_action(IsoWorld *p_world, IsoScreenSolver &p_screen_solver);

	void clear() {}

private:
	bool _resolve_visibles(IsoScreenSolver &p_screen_solver);
	void _place_visibles(IsoWorld *p_world, IsoScreenSolver &p_screen_solver);
	int _recursive_place_iso_object(IsoObject *p_object, real_t p_step_depth, int p_z_index);
	void _place_iso_object(IsoObject *p_object, int p_z_index);
};

#endif // ISO_SORTING_SOLVER_H
