/**************************************************************************/
/*  image_frames.h                                                        */
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

#pragma once

#include "core/image.h"

class ImageFrames;

typedef Error (*LoadImageFramesFunction)(Ref<ImageFrames> &r_image_frames, const Variant &source, int max_frames);

class ImageFrames : public Resource {
	GDCLASS(ImageFrames, Resource);

	struct Frame {
		Ref<Image> image;
		float delay;
	};
	Vector<Frame> frames;

protected:
	static void _bind_methods();

public:
	static LoadImageFramesFunction load_gif_func;

	Error load(const String &p_path, int max_frames = 0);
	Error load_gif_from_buffer(const PoolByteArray &p_data, int max_frames = 0);

	Error save_gif(const String &p_path, int p_color_count = 256);

	void add_frame(const Ref<Image> &p_image, float p_delay);
	void remove_frame(int p_idx);

	void set_frame_image(int p_idx, const Ref<Image> &p_image);
	Ref<Image> get_frame_image(int p_idx) const;

	void set_frame_delay(int p_idx, float p_delay);
	float get_frame_delay(int p_idx) const;

	Rect2 get_bounding_rect() const;
	int get_frame_count() const;

	void clear();
};
