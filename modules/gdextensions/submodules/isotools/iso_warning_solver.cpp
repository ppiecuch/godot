/**************************************************************************/
/*  iso_warning_solver.cpp                                                */
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

#include "iso_warning_solver.h"
#include "iso_world.h"

#ifdef TOOLS_ENABLED

void IsoWarningSolver::step_sorting_action(IsoWorld *p_world) {
	if (_iso_objects.size() == 0) {
		return;
	}
	int check_count = MIN(ISO_OBJECTS_PER_FRAME, _iso_objects.size());
	for (int i = 0; i < check_count; i++) {
		int obj_index = (_obj_counter++) % _iso_objects.size();
		IsoObject *obj = _iso_objects[obj_index];
		_check_changed_transform(obj, p_world);
	}
}

void IsoWarningSolver::_check_changed_transform(IsoObject *p_object, IsoWorld *p_world) {
	if (!p_world) {
		return;
	}
	real_t precision = MIN(p_world->get_tile_size(), p_world->get_tile_height()) * 0.01f;
	Vector2 needed_position = p_world->iso_to_screen(p_object->get_iso_position());
	Vector2 current_position = p_object->get_position();
	if (!IsoUtils::vec2_approximately(needed_position, current_position, precision)) {
		WARN_PRINT("Don't change 'IsoObject.position' manually! Use 'IsoObject.iso_position' instead.");
		p_object->fix_transform();
	}
}

#endif // TOOLS_ENABLED
