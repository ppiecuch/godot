/**************************************************************************/
/*  rockmesh.cpp                                                          */
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

#include "rockmesh.h"

#include "core/variant.h"

// main proc
extern Array rock_gen(int depth = 3, int randseed = 0, real_t smoothness = 1, bool smoothed = false);

void RockMesh::set_depth(int p_depth) {
	if (depth != p_depth) {
		depth = p_depth;
		_dirty = true;
		refresh->start();
	}
}

int RockMesh::get_depth() const {
	return depth;
}

void RockMesh::set_randseed(int p_randseed) {
	if (randseed != p_randseed) {
		randseed = p_randseed;
		_dirty = true;
		refresh->start();
	}
}

int RockMesh::get_randseed() const {
	return randseed;
}

void RockMesh::_rebuild() {
	if (_dirty) {
		clear_surfaces();
		Array mesh = rock_gen(depth, randseed, smoothness, smoothed);
		_dirty = false;
	}
	refresh->stop();
}

void RockMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_depth", "depth"), &RockMesh::set_depth);
	ClassDB::bind_method(D_METHOD("get_depth"), &RockMesh::get_depth);
	ClassDB::bind_method(D_METHOD("set_randseed", "seed"), &RockMesh::set_randseed);
	ClassDB::bind_method(D_METHOD("get_randseed"), &RockMesh::get_randseed);

	ClassDB::bind_method(D_METHOD("_rebuild"), &RockMesh::_rebuild);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "depth"), "set_depth", "get_depth");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "randseed"), "set_randseed", "get_randseed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "smoothness"), "set_smoothness", "get_smoothness");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "smoothed"), "set_smoothed", "get_smoothed");
}

RockMesh::RockMesh() {
	_dirty = true;
	depth = 3;
	randseed = 0;
	smoothness = 1;
	smoothed = false;
	refresh = memnew(Timer);
	refresh->connect("timeout", this, "_rebuild");
	refresh->set_one_shot(true);
	refresh->set_wait_time(0.25);
}
