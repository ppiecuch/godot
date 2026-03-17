/**************************************************************************/
/*  svg_texture.h                                                         */
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

#ifndef SVG_TEXTURE_H
#define SVG_TEXTURE_H

#include "scene/resources/texture.h"

// SVGTexture: A texture resource that rasterizes an SVG at a configurable
// scale factor. Inspired by Godot 4.x SVGTexture/DPITexture concept.
// The SVG is re-rasterized when the scale factor changes, producing crisp
// results at any resolution from a single vector source.

class SVGTexture : public ImageTexture {
	GDCLASS(SVGTexture, ImageTexture);

	String svg_source;
	float scale_factor;
	bool dirty;

	void _rasterize();

protected:
	static void _bind_methods();

public:
	void set_svg_string(const String &p_svg);
	String get_svg_string() const;

	Error load_svg(const String &p_path);

	void set_scale_factor(float p_scale);
	float get_scale_factor() const;

	// Re-rasterize at current scale. Call after changing SVG or scale.
	void update();

	int get_width() const override;
	int get_height() const override;

	SVGTexture();
};

#endif // SVG_TEXTURE_H
