/**************************************************************************/
/*  pixel_spaceships.h                                                    */
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

/* pixel_spaceships.h */
#pragma once

#ifndef DS_PIXEL_SPACESHIPS_H
#define DS_PIXEL_SPACESHIPS_H

#include "core/math/math_funcs.h"
#include "core/reference.h"
#include "scene/resources/texture.h"

typedef Vector<int> MaskData;
typedef Vector<Color> TextureColors;

//////////////////////////////////////////////////////////////////////////
// MASK
//////////////////////////////////////////////////////////////////////////
class PixelSpaceshipsMask : public Reference {
	GDCLASS(PixelSpaceshipsMask, Reference);

private:
	MaskData data;
	Vector2 size;
	bool mirrorX;
	bool mirrorY;

protected:
	static void _bind_methods();

public:
	MaskData get_data();
	void set_data(MaskData, Vector2 size = Vector2());

	//WHITE = 0; RED = -1; GREEN = 1; BLUE = 2;!!!!!!!!!!
	void set_data_from_texture(Ref<Texture>);

	Vector2 get_size();
	void set_size(Vector2);

	bool get_mirror_x();
	void set_mirror_x(bool);

	bool get_mirror_y();
	void set_mirror_y(bool);

	PixelSpaceshipsMask();
	~PixelSpaceshipsMask();
};

//////////////////////////////////////////////////////////////////////////
// OPTIONS
//////////////////////////////////////////////////////////////////////////
class PixelSpaceshipsOptions : public Reference {
	GDCLASS(PixelSpaceshipsOptions, Reference);

private:
	bool colored;
	float edge_brightnes;
	float color_variation;
	float brightness_noise;
	float saturation;
	float hue;

protected:
	static void _bind_methods();

public:
	bool get_colored();
	void set_colored(bool);

	float get_edge_brightness();
	void set_edge_brightness(float);

	float get_color_variation();
	void set_color_variation(float);

	float get_brightness_noise();
	void set_brightness_noise(float);

	float get_saturation();
	void set_saturation(float);

	float get_hue();
	void set_hue(float);

	void setup_options(bool _colored = true, float _edge_brightness = 0.15f, float _col_variations = 0.2f, float _brightness_noise = 0.8f, float _saturation = 0.7f);

	PixelSpaceshipsOptions();
	~PixelSpaceshipsOptions();
};

//////////////////////////////////////////////////////////////////////////
// PIXEL SPACESHIPS
//////////////////////////////////////////////////////////////////////////
class PixelSpaceships : public Reference {
	GDCLASS(PixelSpaceships, Reference);

private:
	MaskData ResultData;
	TextureColors ResultColors;
	int Width;
	int Height;
	int HalfWidth;
	int HalfHeight;
	int Seed;
	Ref<PixelSpaceshipsMask> PSMask;
	Ref<PixelSpaceshipsOptions> PSOptions;

	int pos_to_idx(int, int, int);
	int get_data(MaskData, int, int, int);
	Color hsl2rgb(float, float, float, Color);
	float random();

	void generate_random_sample();
	void mirror_data();
	void generate_edges();
	void generate_colors();
	Ref<ImageTexture> make_texture();

protected:
	static void _bind_methods();

public:
	Ref<ImageTexture> generate_texture(Ref<PixelSpaceshipsMask>, Ref<PixelSpaceshipsOptions>, int = 0);
	void generate(Ref<PixelSpaceshipsMask>, Ref<PixelSpaceshipsOptions>, int = 0);

	Ref<PixelSpaceshipsMask> get_mask_object();
	Ref<PixelSpaceshipsOptions> get_options_object();

	Array get_mask_matrix();
	Array get_colors_matrix();

	PixelSpaceships();
	~PixelSpaceships();
};

#endif // DS_PIXEL_SPACESHIPS_H
