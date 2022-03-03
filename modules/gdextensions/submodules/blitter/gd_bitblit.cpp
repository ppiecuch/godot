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

BitBlit *BitBlit::singleton = nullptr;

static int _get_pixel_size(Image::Format format) {
	switch (format) {
		case Image::FORMAT_RGB8:
			return 3;
		case Image::FORMAT_RGBA8:
			return 4;
		case Image::FORMAT_LA8:
			return 2;
		case Image::FORMAT_L8:
		case Image::FORMAT_R8:
		case Image::FORMAT_RG8:
		case Image::FORMAT_RGBA4444:
		case Image::FORMAT_RF:
		case Image::FORMAT_RGF:
		case Image::FORMAT_RGBF:
		case Image::FORMAT_RGBAF:
		case Image::FORMAT_RH:
		case Image::FORMAT_RGH:
		case Image::FORMAT_RGBH:
		case Image::FORMAT_RGBAH:
		case Image::FORMAT_RGBE9995:
		case Image::FORMAT_DXT1:
		case Image::FORMAT_DXT3:
		case Image::FORMAT_DXT5:
		case Image::FORMAT_RGTC_R:
		case Image::FORMAT_RGTC_RG:
		case Image::FORMAT_BPTC_RGBA:
		case Image::FORMAT_BPTC_RGBF:
		case Image::FORMAT_BPTC_RGBFU:
		case Image::FORMAT_PVRTC2:
		case Image::FORMAT_PVRTC2A:
		case Image::FORMAT_PVRTC4:
		case Image::FORMAT_PVRTC4A:
		case Image::FORMAT_ETC:
		case Image::FORMAT_ETC2_R11:
		case Image::FORMAT_ETC2_R11S:
		case Image::FORMAT_ETC2_RG11:
		case Image::FORMAT_ETC2_RG11S:
		case Image::FORMAT_ETC2_RGB8:
		case Image::FORMAT_ETC2_RGBA8:
		case Image::FORMAT_ETC2_RGB8A1:
#if VERSION_MAJOR >= 4
		case Image::FORMAT_RGB565:
		case Image::FORMAT_ETC2_RA_AS_RG:
		case Image::FORMAT_DXT5_RA_AS_RG:
#else
		case Image::FORMAT_RGBA5551:
#endif
		case Image::FORMAT_MAX:
			return 0;
	}

	return 0;
}


PoolByteArray BlitSurface::get_data() const { return data; }

void BlitSurface::_bind_methods() {
}

BlitSurface::BlitSurface(int p_width, int p_height, Image::Format p_format) {
	ERR_FAIL_COND(p_width > 0);
	ERR_FAIL_COND(p_height > 0);
	const int size = _get_pixel_size(p_format);
	if (size > 0) {
		data.resize(p_width * p_height * size);
	} else {
		WARN_PRINT("Unsuported pixel format");
	}
}

BlitSurface::~BlitSurface() { }


void BitBlit::_bind_methods() {
}

BitBlit::BitBlit() {
	singleton = this;
}

BitBlit::~BitBlit() {
	singleton = nullptr;
}
