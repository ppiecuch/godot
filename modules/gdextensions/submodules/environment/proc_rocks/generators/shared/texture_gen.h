/**************************************************************************/
/*  texture_gen.h                                                         */
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

#ifndef PROC_ROCKS_SHARED_TEXTURE_GEN_H
#define PROC_ROCKS_SHARED_TEXTURE_GEN_H

#include "core/array.h"
#include "core/color.h"
#include "scene/resources/gradient.h"
#include "scene/resources/material.h"
#include "scene/resources/texture.h"

// Generic PBR texture synthesis — shared across every ProcRock generator (RockGen,
// IcoRock, RockStudio, ProcRock). Originally lived only in generators/procrockgen/
// (Method 3), but none of this is mesh/UV/JSON-specific: it's pure 2D image math
// over a height field, so any generator's resulting mesh can wear it. procrockgen.cpp
// still owns the JSON-pipeline-driven variant (rock_pipeline_gen_textures_from_json())
// and the texture-adder blending, since those consume procrocklib's JSON format that
// only Method 3 ever produces — see that file's "Texture pipeline" section.

struct ProcRockPipelineTextures {
	Ref<Texture> albedo;
	Ref<Texture> normal;
	Ref<Texture> roughness;
	Ref<Texture> metalness;
	Ref<Texture> ambient_occlusion;
};

// A flat height field synthesized from a self-contained fractal noise sum (same
// formula as procrocklib's Perlin node: plain octave sum, lacunarity 2.0, using
// Godot's own OpenSimplexNoise as the per-octave coherent-noise kernel) — no
// dependency on procrockgen.cpp's full node-graph interpreter, since this is the
// only shape of noise the scalar (non-JSON) texture API ever needs.
Ref<Image> make_height_image(int p_size, real_t p_noise_frequency, int p_noise_octaves, real_t p_noise_persistence, int p_randseed);

Ref<Image> make_albedo_image_from_gradient(Ref<Image> p_height, Ref<Gradient> p_gradient);
Ref<Image> make_albedo_image(Ref<Image> p_height, const Color &p_low, const Color &p_high);
Ref<Image> make_normal_image(Ref<Image> p_height, real_t p_strength);
Ref<Image> make_scaled_grayscale_image(Ref<Image> p_height, real_t p_scale, real_t p_bias);
Ref<Image> make_constant_image(int p_size, const Color &p_color);
Ref<ImageTexture> to_texture(const Ref<Image> &p_image);

// PBR texture set derived from a single noise height field, correlated with a
// generator's own mesh displacement when called with the same seed/frequency/
// octaves/persistence.
ProcRockPipelineTextures rock_pipeline_gen_textures(
		int p_size, real_t p_noise_frequency, int p_noise_octaves, real_t p_noise_persistence, int p_randseed,
		const Color &p_albedo_low, const Color &p_albedo_high, real_t p_normal_strength,
		real_t p_roughness_scale, real_t p_roughness_bias,
		real_t p_metalness_scale, real_t p_metalness_bias,
		real_t p_ao_scale, real_t p_ao_bias);

Ref<SpatialMaterial> rock_pipeline_make_material(const ProcRockPipelineTextures &p_textures);

// Adds real (MikkTSpace) tangents to p_arrays, needed for rock_pipeline_make_material()'s
// FEATURE_NORMAL_MAPPING to render correctly: without ARRAY_TANGENT, Godot leaves that
// vertex attribute disabled, which the GLES3 shader reads as the GL-spec default
// (0,0,0,1) and immediately normalize()s to NaN, corrupting lighting on scattered
// triangles (see memo.md's "Bugs Fixed" — this was the "missing/dark triangles" bug).
// Requires p_arrays to already have ARRAY_VERTEX/ARRAY_NORMAL/ARRAY_TEX_UV — returns
// p_arrays unchanged (with a WARN_PRINT_ONCE) if UV data is missing, since
// SurfaceTool::generate_tangents() itself hard-requires it. Correctly round-trips both
// indexed and non-indexed (ARRAY_INDEX absent) triangle arrays.
Array ensure_tangents(const Array &p_arrays);

#endif // PROC_ROCKS_SHARED_TEXTURE_GEN_H
