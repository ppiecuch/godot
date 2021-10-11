/*************************************************************************/
/*  area_prober.h                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef AREAPROBER_H
#define AREAPROBER_H

#include "core/object.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "core/resource.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/shape_2d.h"

#include <vector>

class AreaProber : public Node2D {
	GDCLASS(AreaProber, Node2D);

protected:
	static void _bind_methods();

public:
	Array probe(Vector2 position);
	void set_collision_shape(const Ref<Shape2D> &p_shape);
	Ref<Shape2D> get_collision_shape() const;

	void set_collision_mask(int p_mask);
	int get_collision_mask() const;

	void set_collision_detect_bodies(bool p_enabled);
	bool get_collision_detect_bodies() const;

	void set_collision_detect_areas(bool p_enabled);
	bool get_collision_detect_areas() const;
	AreaProber();

private:
	Ref<Shape2D> collision_shape;
	int collision_mask;
	bool collision_detect_bodies;
	bool collision_detect_areas;
};

#endif // AREAPROBER_H
