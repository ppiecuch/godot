/**************************************************************************/
/*  image_loader_thor_svg.cpp                                             */
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

#include "image_loader_thor_svg.h"

#include "core/os/memory.h"
#include "core/variant.h"

#include <thorvg.h>

HashMap<Color, Color> ImageLoaderThorSVG::forced_color_map = HashMap<Color, Color>();

void ImageLoaderThorSVG::set_forced_color_map(const HashMap<Color, Color> &p_color_map) {
	forced_color_map = p_color_map;
}

void ImageLoaderThorSVG::_replace_color_property(const HashMap<Color, Color> &p_color_map, const String &p_prefix, String &r_string) {
	// Replace colors in the SVG based on what is passed in `p_color_map`.
	// Used to change the colors of editor icons based on the used theme.
	// The strings being replaced are typically of the form:
	//   fill="#5abbef"
	// But can also be 3-letter codes, include alpha, be "none" or a named color
	// string ("blue"). So we convert to Godot Color to compare with `p_color_map`.

	const int prefix_len = p_prefix.length();
	int pos = r_string.find(p_prefix);
	while (pos != -1) {
		pos += prefix_len; // Skip prefix.
		int end_pos = r_string.find("\"", pos);
		ERR_FAIL_COND_MSG(end_pos == -1, vformat("Malformed SVG string after property \"%s\".", p_prefix));
		const String color_code = r_string.substr(pos, end_pos - pos);
		if (color_code != "none" && !color_code.begins_with("url(")) {
			const Color color = Color::html(color_code); // Handles both HTML codes and named colors.
			if (p_color_map.has(color)) {
				r_string = r_string.left(pos) + "#" + p_color_map[color].to_html(false) + r_string.substr(end_pos);
			}
		}
		// Search for other occurrences.
		pos = r_string.find(p_prefix, pos);
	}
}

Error ImageLoaderThorSVG::_rasterize(Ref<Image> p_image, const uint8_t *p_data, int p_size, uint32_t p_width_px, uint32_t p_height_px) {
	std::unique_ptr<tvg::Picture> picture = tvg::Picture::gen();

	tvg::Result result = picture->load((const char *)p_data, p_size, "svg", true);
	if (result != tvg::Result::Success) {
		return ERR_INVALID_DATA;
	}

	const uint32_t max_dimension = 16384;
	uint32_t width = MAX(1u, p_width_px);
	uint32_t height = MAX(1u, p_height_px);
	if (width > max_dimension || height > max_dimension) {
		float aspect = (float)width / (float)height;
		if (width > height) {
			width = max_dimension;
			height = MAX(1u, (uint32_t)round(max_dimension / aspect));
		} else {
			height = max_dimension;
			width = MAX(1u, (uint32_t)round(max_dimension * aspect));
		}
		WARN_PRINT(vformat("ImageLoaderThorSVG: SVG dimensions clamped to %dx%d.", width, height));
	}

	picture->size(width, height);

	std::unique_ptr<tvg::SwCanvas> sw_canvas = tvg::SwCanvas::gen();
	// Note: memalloc here, be sure to memfree before any return.
	uint32_t *buffer = (uint32_t *)memalloc(sizeof(uint32_t) * width * height);

	tvg::Result res = sw_canvas->target(buffer, width, width, height, tvg::SwCanvas::ARGB8888S);
	if (res != tvg::Result::Success) {
		memfree(buffer);
		ERR_FAIL_V_MSG(FAILED, "ImageLoaderThorSVG: Couldn't set target on ThorVG canvas.");
	}

	res = sw_canvas->push(std::move(picture));
	if (res != tvg::Result::Success) {
		memfree(buffer);
		ERR_FAIL_V_MSG(FAILED, "ImageLoaderThorSVG: Couldn't insert ThorVG picture on canvas.");
	}

	res = sw_canvas->draw();
	if (res != tvg::Result::Success) {
		memfree(buffer);
		ERR_FAIL_V_MSG(FAILED, "ImageLoaderThorSVG: Couldn't draw ThorVG pictures on canvas.");
	}

	res = sw_canvas->sync();
	if (res != tvg::Result::Success) {
		memfree(buffer);
		ERR_FAIL_V_MSG(FAILED, "ImageLoaderThorSVG: Couldn't sync ThorVG canvas.");
	}

	PoolVector<uint8_t> image;
	image.resize(width * height * sizeof(uint32_t));
	{
		PoolVector<uint8_t>::Write w = image.write();
		uint8_t *dst = w.ptr();
		for (uint32_t y = 0; y < height; y++) {
			for (uint32_t x = 0; x < width; x++) {
				uint32_t n = buffer[y * width + x];
				const size_t offset = sizeof(uint32_t) * width * y + sizeof(uint32_t) * x;
				dst[offset + 0] = (n >> 16) & 0xff; // R
				dst[offset + 1] = (n >> 8) & 0xff; // G
				dst[offset + 2] = n & 0xff; // B
				dst[offset + 3] = (n >> 24) & 0xff; // A
			}
		}
	}

	res = sw_canvas->clear(true);
	memfree(buffer);

	p_image->create(width, height, false, Image::FORMAT_RGBA8, image);
	return OK;
}

Error ImageLoaderThorSVG::create_image_from_utf8_buffer(Ref<Image> p_image, const uint8_t *p_data, int p_size, float p_scale, bool p_upsample) {
	ERR_FAIL_COND_V_MSG(Math::is_zero_approx(p_scale), ERR_INVALID_PARAMETER, "ImageLoaderThorSVG: Can't load SVG with a scale of 0.");
	ERR_FAIL_COND_V_MSG(p_scale < 0, ERR_INVALID_PARAMETER, "ImageLoaderThorSVG: Can't load SVG with a negative scale.");

	// Probe intrinsic dimensions, then forward to _rasterize with explicit pixel size.
	std::unique_ptr<tvg::Picture> probe = tvg::Picture::gen();
	tvg::Result probe_res = probe->load((const char *)p_data, p_size, "svg", true);
	if (probe_res != tvg::Result::Success) {
		return ERR_INVALID_DATA;
	}
	float fw, fh;
	probe->size(&fw, &fh);
	probe.reset();

	uint32_t width = MAX(1u, (uint32_t)round(fw * p_scale));
	uint32_t height = MAX(1u, (uint32_t)round(fh * p_scale));
	return _rasterize(p_image, p_data, p_size, width, height);
}

Error ImageLoaderThorSVG::create_image_sized_from_utf8_buffer(Ref<Image> p_image, const uint8_t *p_data, int p_size, int p_width_px, int p_height_px) {
	ERR_FAIL_COND_V_MSG(p_width_px < 0 || p_height_px < 0, ERR_INVALID_PARAMETER, "ImageLoaderThorSVG: width/height must be >= 0.");

	uint32_t width = (uint32_t)p_width_px;
	uint32_t height = (uint32_t)p_height_px;

	// Probe intrinsic dimensions when we need to derive the missing axis.
	if (width == 0 || height == 0) {
		std::unique_ptr<tvg::Picture> probe = tvg::Picture::gen();
		tvg::Result probe_res = probe->load((const char *)p_data, p_size, "svg", true);
		if (probe_res != tvg::Result::Success) {
			return ERR_INVALID_DATA;
		}
		float fw, fh;
		probe->size(&fw, &fh);
		probe.reset();

		if (width == 0 && height == 0) {
			width = MAX(1u, (uint32_t)round(fw));
			height = MAX(1u, (uint32_t)round(fh));
		} else if (width == 0) {
			float aspect = (fh > 0.0f) ? (fw / fh) : 1.0f;
			width = MAX(1u, (uint32_t)round((float)height * aspect));
		} else { // height == 0
			float aspect = (fh > 0.0f) ? (fw / fh) : 1.0f;
			height = MAX(1u, (uint32_t)round((float)width / aspect));
		}
	}

	return _rasterize(p_image, p_data, p_size, width, height);
}

Error ImageLoaderThorSVG::create_image_from_string(Ref<Image> p_image, String p_string, float p_scale, bool p_upsample, const HashMap<Color, Color> &p_color_map) {
	if (p_color_map.size()) {
		_replace_color_property(p_color_map, "stop-color=\"", p_string);
		_replace_color_property(p_color_map, "fill=\"", p_string);
		_replace_color_property(p_color_map, "stroke=\"", p_string);
	}

	CharString cs = p_string.utf8();

	return create_image_from_utf8_buffer(p_image, (const uint8_t *)cs.get_data(), cs.length(), p_scale, p_upsample);
}

Error ImageLoaderThorSVG::create_image_sized_from_string(Ref<Image> p_image, String p_string, int p_width_px, int p_height_px, const HashMap<Color, Color> &p_color_map) {
	if (p_color_map.size()) {
		_replace_color_property(p_color_map, "stop-color=\"", p_string);
		_replace_color_property(p_color_map, "fill=\"", p_string);
		_replace_color_property(p_color_map, "stroke=\"", p_string);
	}

	CharString cs = p_string.utf8();

	return create_image_sized_from_utf8_buffer(p_image, (const uint8_t *)cs.get_data(), cs.length(), p_width_px, p_height_px);
}

void ImageLoaderThorSVG::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("th-svg");
	p_extensions->push_back("th.svg");
}

Error ImageLoaderThorSVG::load_image(Ref<Image> p_image, FileAccess *p_fileaccess, bool p_force_linear, float p_scale) {
	String svg = p_fileaccess->get_as_utf8_string();

	Error err;
	if (!forced_color_map.empty()) {
		err = create_image_from_string(p_image, svg, p_scale, false, forced_color_map);
	} else {
		err = create_image_from_string(p_image, svg, p_scale, false, HashMap<Color, Color>());
	}

	if (err != OK) {
		return err;
	} else if (p_image->empty()) {
		return ERR_INVALID_DATA;
	}

	if (p_force_linear) {
		p_image->srgb_to_linear();
	}
	return OK;
}
