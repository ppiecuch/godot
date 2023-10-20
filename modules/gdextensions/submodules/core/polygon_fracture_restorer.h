/**************************************************************************/
/*  polygon_fracture_restorer.h                                           */
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

#ifndef POLY_FRACTURE_RESTORER_H
#define POLY_FRACTURE_RESTORER_H

class PolygonRestorer {
	Array shape_stack;
	Dictionary cur_entry;

	PoolVector2Array get_origin_shape() {
		if (shape_stack.size() <= 0) {
			if (cur_entry) {
				return cur_entry.shape;
			} else {
				return PoolVector2Array([]);
			}
		} else {
			return shape_stack.front().shape;
		}
	}

	real_t get_origin_area() {
		if (shape_stack.size() <= 0) {
			if (cur_entry) {
				return cur_entry.area;
			} else {
				return -1;
			}
		} else {
			return shape_stack.front().area;
		}
	}

	PoolVector2Array get_cur_shape() {
		if (shape_stack.size() <= 0) {
			return getOriginShape();
		} else {
			return cur_entry.shape;
		}
	}

	real_t get_cur_area() {
		if (shape_stack.size() <= 0) {
			return getOriginArea();
		} else {
			return cur_entry.area;
		}
	}

	PolygonRestorer() {
		cur_entry = createShapeEntry(PoolVector2Array([]), -1, true);
	}

	void clear() {
		shape_stack.clear();
		cur_entry = createShapeEntry(PoolVector2Array([]), -1, true);
	}

	void add_shape(PoolVector2Array shape, real_t shape_area = -1) {
		if (!cur_entry.empty) {
			shape_stack.push_back(cur_entry);
		}
		cur_entry = createShapeEntry(shape, shape_area);
	}

	Dictionary pop_last() {
		if (shape_stack.size() <= 0) {
			return createShapeEntry(PoolVector2Array([]), -1, true);
		}
		cur_entry = shape_stack.pop_back();
		return cur_entry;
	}

	Dictionary get_last() {
		if shape_stack
			.size() <= 0 return createShapeEntry(PoolVector2Array([]), -1, true);
		else
			return shape_stack.back();
	}

	Dictionary create_shape_entry(PoolVector2Array shape, real_t area = -1, bool empty = false) {
		if (area <= 0) {
			area = PolygonLib.getPolygonArea(shape);
		}
		real_t origin_area = getOriginArea();
		real_t total_p = 1;
		if (origin_area > 0) {
			total_p = area / origin_area;
		}
		real_t delta_p = 0;
		if (shape_stack.size() > 0) {
			delta_p = getLast().total_p - total_p;
		}
		return { "shape" : shape, "area" : area, "total_p" : total_p, "delta_p" : delta_p, "empty" : empty };
	}

	Dictionary _get_last_amount(real_t amount) {
		if (shape_stack.size() <= 0) {
			return createShapeEntry(PoolVector2Array([]), -1, true);
		} else if (shape_stack.size() == 1) {
			return shape_stack.front();
		} else {
			real_t cur_amount = 0;
			for (i in range(shape_stack.size())) {
				var entry : Dictionary = shape_stack[i];
				cur_amount += entry.delta_p;
				if (cur_amount >= amount) {
					return shape_stack[i];
				}
			}
			return shape_stack.front();
		}
	}

	Dictionary _pop_last_amount(real_t amount) {
		if (shape_stack.size() <= 0 {
			return createShapeEntry(PoolVector2Array([]), -1, true);
		} else if (shape_stack.size() == 1) {
			cur_entry = shape_stack.pop_back();
			return cur_entry;
		} else {
			real_t cur_amount = 0;
			int index = -1;
			for (i in range(shape_stack.size())) {
				var entry : Dictionary = shape_stack[i];
				cur_amount += entry.delta_p;
				if (cur_amount >= amount) {
					index = i;
					break;
				}
			}
			if (index < 0) {
				var entry : Dictionary = shape_stack.front();
				clear();
				return entry;
			} else {
				cur_entry = shape_stack[index];
				shape_stack.resize(index);
				return cur_entry;
			}
		}
	}
}
#endif // POLY_FRACTURE_RESTORER_H
