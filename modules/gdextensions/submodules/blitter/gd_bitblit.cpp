/*************************************************************************/
/*  gd_bitblit.cpp                                                       */
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

#include "gd_bitblit.h"
#include "_surface.h"

BitBlit *BitBlit::singleton = nullptr;

PoolByteArray BlitSurface::get_data() const { return data; }

void BlitSurface::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_data"), &BlitSurface::get_data);
}

BlitSurface::BlitSurface(int p_width, int p_height, int p_depth) : surface(nullptr) {
	ERR_FAIL_COND(p_width > 0);
	ERR_FAIL_COND(p_height > 0);
	ERR_FAIL_COND(p_depth > 0);

	surface = SDL_CreateRGBEmptySurface(p_width, p_height, p_depth,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
	);
	data.resize(p_width * p_height * p_depth / 8);
	surface->pixels = data.write().ptr();
}

BlitSurface::~BlitSurface() { }


Ref<BlitSurface> BitBlit::create_surface(int p_width, int p_height, int p_depth) {
	Ref<BlitSurface> surf = memnew(BlitSurface(p_width, p_height, p_depth));
	return surf;
}

void BitBlit::_bind_methods() {
}

BitBlit::BitBlit() {
	singleton = this;
}

BitBlit::~BitBlit() {
	singleton = nullptr;
}
