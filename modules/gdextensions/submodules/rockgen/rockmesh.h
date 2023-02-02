/*************************************************************************/
/*  rockmesh.h                                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef ROCKMESH_H
#define ROCKMESH_H

#include "scene/main/timer.h"
#include "scene/resources/mesh.h"

class RockMesh : public ArrayMesh {
	GDCLASS(RockMesh, ArrayMesh)

	int depth;
	int randseed;
	real_t smoothness;
	bool smoothed;

	Timer *refresh;

	bool _dirty;
	void _rebuild();

protected:
	static void _bind_methods();
	bool _is_generated() const { return true; }

public:
	void set_depth(int p_depth);
	int get_depth() const;
	void set_randseed(int p_randseed);
	int get_randseed() const;
	void set_smoothness(real_t p_smoothness);
	real_t get_smoothness() const;
	void set_smoothed(bool p_smoothed);
	bool get_smoothed() const;

	RockMesh();
};

#endif // ROCKMESH_H
