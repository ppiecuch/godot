/**************************************************************************/
/*  image_frames_loader_gif.h                                             */
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

#include "image_frames_loader.h"

class ImageFramesLoaderGIF : public ImageFramesFormatLoader {
public:
	virtual Error load_image_frames(Ref<ImageFrames> &r_image_frames, FileAccess *f, int max_frames = 0) const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;

	ImageFramesLoaderGIF();
};

struct GifFileType;

class GifLoader {
	GifFileType *gif = nullptr;

	enum SourceType {
		FILE,
		BUFFER
	};
	Error parse_error(Error parse_error, const String &message);
	Error gif_error(int gif_error);

	Error _open(void *source, SourceType source_type);
	Error _load_frames(Ref<ImageFrames> &r_image_frames, int max_frames = 0);
	Error _close();

public:
	Error load_from_file_access(Ref<ImageFrames> &r_image_frames, FileAccess *f, int max_frames = 0);
	Error load_from_buffer(Ref<ImageFrames> &r_image_frames, const PoolByteArray &p_data, int max_frames = 0);
};
