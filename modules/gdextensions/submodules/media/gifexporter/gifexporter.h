/*************************************************************************/
/*  gifexporter.h                                                        */
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

#ifndef GIFEXPORTER_H
#define GIFEXPORTER_H

#include "core/image.h"
#include "core/reference.h"

#define GIF_FREE
#include <gifanimcplusplus/gifanim.h>

class GifExporter : public Reference {
	GDCLASS(GifExporter, Reference);

	GifAnim ganim;
	GifWriter gwriter;

protected:
	static void _bind_methods();

public:
	String filename;

	void set_filename(const String file);
	void begin_export(const Size2 &size, float frame_delay, int loop_count = 0, int32_t bit_depth = 8, bool dither = false);
	void end_export();
	void write_frame(const Ref<Image> frame, const Color &background_color, float frame_delay, int32_t bit_depth = 8, bool dither = false);

	GifExporter();
	~GifExporter();
};

#endif //GIFEXPORTER_H
