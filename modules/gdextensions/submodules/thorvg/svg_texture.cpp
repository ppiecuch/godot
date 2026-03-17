/**************************************************************************/
/*  svg_texture.cpp                                                       */
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

#include "svg_texture.h"

#include "core/os/file_access.h"

#ifdef TOOLS_ENABLED
#include "image_loader_thor_svg.h"
#endif

void SVGTexture::_rasterize() {
	if (svg_source.empty()) {
		return;
	}

#ifdef TOOLS_ENABLED
	Ref<Image> img;
	img.instance();

	ImageLoaderThorSVG loader;
	Error err = loader.create_image_from_string(img, svg_source, scale_factor, false, HashMap<Color, Color>());

	if (err == OK && !img->empty()) {
		create_from_image(img);
		dirty = false;
	} else {
		ERR_PRINT("SVGTexture: Failed to rasterize SVG.");
	}
#else
	WARN_PRINT_ONCE("SVGTexture: SVG rasterization requires editor build (tools=yes). Use pre-rasterized images in export builds.");
#endif
}

void SVGTexture::set_svg_string(const String &p_svg) {
	if (svg_source == p_svg) {
		return;
	}
	svg_source = p_svg;
	dirty = true;
	_rasterize();
	emit_changed();
}

String SVGTexture::get_svg_string() const {
	return svg_source;
}

Error SVGTexture::load_svg(const String &p_path) {
	FileAccess *fa = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_NULL_V_MSG(fa, ERR_FILE_CANT_OPEN, "SVGTexture: Cannot open file: " + p_path);
	String svg = fa->get_as_utf8_string();
	memdelete(fa);

	if (svg.empty()) {
		return ERR_FILE_CORRUPT;
	}

	set_svg_string(svg);
	return OK;
}

void SVGTexture::set_scale_factor(float p_scale) {
	ERR_FAIL_COND_MSG(p_scale <= 0, "SVGTexture: Scale factor must be positive.");
	if (Math::is_equal_approx(scale_factor, p_scale)) {
		return;
	}
	scale_factor = p_scale;
	dirty = true;
	_rasterize();
	emit_changed();
}

float SVGTexture::get_scale_factor() const {
	return scale_factor;
}

void SVGTexture::update() {
	if (dirty) {
		_rasterize();
	}
}

int SVGTexture::get_width() const {
	return ImageTexture::get_width();
}

int SVGTexture::get_height() const {
	return ImageTexture::get_height();
}

void SVGTexture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_svg_string", "svg"), &SVGTexture::set_svg_string);
	ClassDB::bind_method(D_METHOD("get_svg_string"), &SVGTexture::get_svg_string);
	ClassDB::bind_method(D_METHOD("load_svg", "path"), &SVGTexture::load_svg);
	ClassDB::bind_method(D_METHOD("set_scale_factor", "scale"), &SVGTexture::set_scale_factor);
	ClassDB::bind_method(D_METHOD("get_scale_factor"), &SVGTexture::get_scale_factor);
	ClassDB::bind_method(D_METHOD("update"), &SVGTexture::update);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "svg_string", PROPERTY_HINT_MULTILINE_TEXT), "set_svg_string", "get_svg_string");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_factor", PROPERTY_HINT_RANGE, "0.1,10.0,0.1"), "set_scale_factor", "get_scale_factor");
}

SVGTexture::SVGTexture() {
	scale_factor = 1.0;
	dirty = false;
}
