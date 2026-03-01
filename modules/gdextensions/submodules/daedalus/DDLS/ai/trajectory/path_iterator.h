/**************************************************************************/
/*  path_iterator.h                                                       */
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

#include "core/math/vector2.h"
#include "core/reference.h"
#include "core/vector.h"

#include "ddls_fwd.h"

class DDLS_PathIterator : public Reference {
	GDCLASS(DDLS_PathIterator, Reference);

	DDLSEntityAI entity;
	Point2 current_pos;
	bool has_prev;
	bool has_next;
	Vector<Point2> path;
	int count;
	int count_max;

	void update_entity();

public:
	DDLSEntityAI get_entity() const { return entity; }
	void set_entity(DDLSEntityAI p_entity) { entity = p_entity; }

	real_t get_x() const { return current_pos.x; }
	real_t get_y() const { return current_pos.y; }

	bool get_has_prev() const { return has_prev; }
	bool get_has_next() const { return has_next; }

	int get_count() const { return count; }
	int get_count_max() const { return count_max; }

	void set_path(const Vector<Point2> &p_path);
	void reset();
	bool prev();
	bool next();

	DDLS_PathIterator();
};
