/**************************************************************************/
/*  iso_sorting_solver.cpp                                                */
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

#include "iso_sorting_solver.h"
#include "iso_world.h"

bool IsoSortingSolver::step_sorting_action(IsoWorld *p_world, IsoScreenSolver &p_screen_solver) {
	bool dirty = _resolve_visibles(p_screen_solver);
	if (dirty) {
		_place_visibles(p_world, p_screen_solver);
	}
	return dirty;
}

bool IsoSortingSolver::_resolve_visibles(IsoScreenSolver &p_screen_solver) {
	bool mark_dirty = false;
	IsoAssocList<IsoObject *> &old_visibles = p_screen_solver.get_old_visibles();
	IsoAssocList<IsoObject *> &cur_visibles = p_screen_solver.get_cur_visibles();

	for (int i = 0; i < cur_visibles.count(); i++) {
		IsoObject *obj = cur_visibles[i];
		if (obj->internal.dirty) {
			p_screen_solver.setup_iso_object_depends(obj);
			obj->internal.dirty = false;
			mark_dirty = true;
		}
	}

	for (int i = 0; i < old_visibles.count(); i++) {
		IsoObject *obj = old_visibles[i];
		if (!cur_visibles.contains(obj)) {
			p_screen_solver.clear_iso_object_depends(obj);
			obj->internal.dirty = true;
			mark_dirty = true;
		}
	}

	return mark_dirty;
}

void IsoSortingSolver::_place_visibles(IsoWorld *p_world, IsoScreenSolver &p_screen_solver) {
	real_t step_depth = p_world->get_step_depth();
	int z_index = (int)p_world->get_start_depth();

	IsoAssocList<IsoObject *> &cur_visibles = p_screen_solver.get_cur_visibles();
	for (int i = 0; i < cur_visibles.count(); i++) {
		z_index = _recursive_place_iso_object(cur_visibles[i], step_depth, z_index);
	}
}

int IsoSortingSolver::_recursive_place_iso_object(IsoObject *p_object, real_t p_step_depth, int p_z_index) {
	if (p_object->internal.placed) {
		return p_z_index;
	}
	p_object->internal.placed = true;

	IsoAssocList<IsoObject *> &self_depends = p_object->internal.self_depends;
	for (int i = 0; i < self_depends.count(); i++) {
		p_z_index = _recursive_place_iso_object(self_depends[i], p_step_depth, p_z_index);
	}

	_place_iso_object(p_object, p_z_index);
	return p_z_index + 1;
}

void IsoSortingSolver::_place_iso_object(IsoObject *p_object, int p_z_index) {
	// Use Godot's z_index for sorting instead of Unity's Transform.z depth
	p_object->set_z_index(CLAMP(p_z_index, VS::CANVAS_ITEM_Z_MIN, VS::CANVAS_ITEM_Z_MAX));
}
