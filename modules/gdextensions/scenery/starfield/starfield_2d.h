/*************************************************************************/
/*  starfield_2d.h                                                       */
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

// -*- C++ -*-
//

#ifndef _STARFIELD_2D_H_
#define _STARFIELD_2D_H_

#include "core/reference.h"
#include "scene/2d/node_2d.h"

class Starfield;

class Starfield2D : public Node2D {
	GDCLASS(Starfield2D, Node2D);

private:
	Size2 virtual_size;
	Vector2 movement_vector;

	Ref<Starfield> _starfield;
	Dictionary _image_atlas;

	void _update();

public:
	void set_virtual_size(const Size2 &p_size);
	Vector2 get_virtual_size() const;
	void set_movement_vector(const Vector2 &p_movement);
	Vector2 get_movement_vector() const;

	Starfield2D();
	~Starfield2D();

protected:
	static void _bind_methods();

	void _notification(int p_what);
};

#endif /* _STARFIELD_2D_H_ */
