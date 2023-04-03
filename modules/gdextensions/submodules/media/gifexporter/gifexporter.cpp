/**************************************************************************/
/*  gifexporter.cpp                                                       */
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

#include "gifexporter.h"

#include "core/array.h"

#include <vector>

GifExporter::GifExporter() {
}

GifExporter::~GifExporter() {
}

void GifExporter::_bind_methods() {
	ClassDB::bind_method("set_filename", &GifExporter::set_filename);
	ClassDB::bind_method("begin_export", &GifExporter::begin_export);
	ClassDB::bind_method("end_export", &GifExporter::end_export);
	ClassDB::bind_method("write_frame", &GifExporter::write_frame);
}
void GifExporter::set_filename(const String file) {
	filename = file;
}

void GifExporter::begin_export(const Size2 &size, float frame_delay, int loop_count, int32_t bit_depth, bool dither) {
	// opens a new gif file
	ganim.GifBegin(&gwriter, filename.utf8().get_data(), size.width, size.height, frame_delay, loop_count, bit_depth, dither);
}

void GifExporter::end_export() {
	// closes the gif file
	ganim.GifEnd(&gwriter);
}

uint8_t get_r8(const Color &c) {
	return (uint8_t)(c.r * 255.0);
}
uint8_t get_g8(const Color &c) {
	return (uint8_t)(c.g * 255.0);
}
uint8_t get_b8(const Color &c) {
	return (uint8_t)(c.b * 255.0);
}
uint8_t get_a8(const Color &c) {
	return (uint8_t)(c.a * 255.0);
}

void GifExporter::write_frame(const Ref<Image> frame, const Color &background_color, float frame_delay, int32_t bit_depth, bool dither) {
	// get raw bytes from frame
	PoolByteArray pool = frame->get_data();

	std::vector<uint8_t> data(pool.size());
	for (int i = 0; i < pool.size(); i += 4) {
		// blend color with the background color because gif doesn't support alpha channel
		uint8_t red = pool[i];
		uint8_t green = pool[i + 1];
		uint8_t blue = pool[i + 2];
		uint8_t alpha = pool[i + 3];

		// background always has to have a solid alpha
		data[i + 3] = alpha + 255 * (255 - alpha);
		data[i] = (red * alpha + get_r8(background_color) * 255 * (255 - alpha)) / data[i + 3];
		data[i + 1] = (green * alpha + get_g8(background_color) * 255 * (255 - alpha)) / data[i + 3];
		data[i + 2] = (blue * alpha + get_b8(background_color) * 255 * (255 - alpha)) / data[i + 3];
	}
	ganim.GifWriteFrame(&gwriter, &data[0], frame->get_width(), frame->get_height(), frame_delay, bit_depth, dither);
}
