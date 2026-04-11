/**************************************************************************/
/*  hex_tiling_repeat.h                                                   */
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

// References:
// Ben Cloward tutorial                 https://www.youtube.com/watch?v=hc6msdFcnA4
// Practical Real-Time Hex-Tiling Paper https://jcgt.org/published/0011/03/05/

#ifndef HEX_TILING_MATERIAL_H
#define HEX_TILING_MATERIAL_H

#include "scene/resources/material.h"
#include "scene/resources/texture.h"

// HexTilingMaterial — a ShaderMaterial that implements stochastic hex-tiling:
// each hexagonal cell samples the texture with a random rotation/scale/offset,
// eliminating visible repetition patterns across large surfaces.
//
// Usage: assign to the material slot of any Sprite/Polygon2D/etc., set
// hex_texture to the source texture, then tune hex_size and sharpness.
class HexTilingMaterial : public ShaderMaterial {
	GDCLASS(HexTilingMaterial, ShaderMaterial);

	Vector2 _tex_repeat;
	float _sharpness;
	float _hex_size;
	Ref<Texture> _hex_texture;

protected:
	static void _bind_methods();

public:
	// tex_repeat: UV tiling multiplier applied before hex decomposition.
	// (1,1) = no repeat; (2,2) = tile the source texture 2x in each axis.
	void set_tex_repeat(const Vector2 &p_repeat);
	Vector2 get_tex_repeat() const;

	// sharpness: blending exponent between adjacent hex cells.
	// Higher values produce harder, crisper transitions (range: 0.1–20).
	void set_sharpness(float p_sharpness);
	float get_sharpness() const;

	// hex_size: size of each hex cell in UV space.
	// Smaller = more, smaller hexagons; larger = fewer, bigger ones.
	void set_hex_size(float p_size);
	float get_hex_size() const;

	// hex_texture: the source texture to tile stochastically across hex cells.
	void set_hex_texture(const Ref<Texture> &p_tex);
	Ref<Texture> get_hex_texture() const;

	HexTilingMaterial();
};

#endif // HEX_TILING_MATERIAL_H
