/**************************************************************************/
/*  gd_bitblit.h                                                          */
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

#ifndef GD_BITBLIT_H
#define GD_BITBLIT_H

#include "core/image.h"
#include "core/object.h"
#include "core/pool_vector.h"
#include "core/reference.h"

struct SDL_Surface;

class BlitSurface : public Reference {
	GDCLASS(BlitSurface, Reference)

	PoolByteArray data;
	SDL_Surface *surface;

protected:
	static void _bind_methods();

public:
	PoolByteArray get_data() const;
	Ref<Image> get_image() const;

	BlitSurface(int p_width, int p_height, int p_depth);
	~BlitSurface();
};

class BitBlit : public Object {
	GDCLASS(BitBlit, Object)

	static BitBlit *singleton;

protected:
	static void _bind_methods();

public:
	static BitBlit *get_singleton() { return singleton; }

	Ref<BlitSurface> create_surface(int p_width, int p_height, int p_depth);

	BitBlit();
	~BitBlit();
};

#endif // GD_BITBLIT_H
