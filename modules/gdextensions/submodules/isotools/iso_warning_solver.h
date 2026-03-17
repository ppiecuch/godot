/**************************************************************************/
/*  iso_warning_solver.h                                                  */
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

#ifndef ISO_WARNING_SOLVER_H
#define ISO_WARNING_SOLVER_H

#include "iso_object.h"
#include "iso_utils.h"

class IsoWorld;

class IsoWarningSolver {
#ifdef TOOLS_ENABLED
	static const int ISO_OBJECTS_PER_FRAME = 100;
	unsigned int _obj_counter;
	Vector<IsoObject *> _iso_objects;

	void _check_changed_transform(IsoObject *p_object, IsoWorld *p_world);

public:
	IsoWarningSolver() :
			_obj_counter(0) {}

	void on_add_iso_object(IsoObject *p_object) {
		_iso_objects.push_back(p_object);
	}

	void on_remove_iso_object(IsoObject *p_object) {
		int idx = _iso_objects.find(p_object);
		if (idx >= 0) {
			int last = _iso_objects.size() - 1;
			if (idx != last) {
				_iso_objects.write[idx] = _iso_objects[last];
			}
			_iso_objects.resize(last);
		}
	}

	bool on_mark_dirty_iso_object(IsoObject *p_object) {
		return false;
	}

	void step_sorting_action(IsoWorld *p_world);

	void clear() {
		_iso_objects.clear();
		_obj_counter = 0;
	}
#else
public:
	IsoWarningSolver() {}
	void on_add_iso_object(IsoObject *p_object) {}
	void on_remove_iso_object(IsoObject *p_object) {}
	bool on_mark_dirty_iso_object(IsoObject *p_object) { return false; }
	void step_sorting_action(IsoWorld *p_world) {}
	void clear() {}
#endif
};

#endif // ISO_WARNING_SOLVER_H
