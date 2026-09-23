/**************************************************************************/
/*  procrockgen.h                                                         */
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

#ifndef PROCROCKGEN_H
#define PROCROCKGEN_H

#include "core/color.h"
#include "core/dictionary.h"
#include "core/variant.h"
#include "scene/resources/material.h"
#include "scene/resources/texture.h"

#include "../shared/texture_gen.h"

// Method 3 — ProcRock: icosphere + noise displacement + optional cut-plane, entirely
// built from Godot core primitives (no thirdparty dependency).
Array rock_pipeline_gen(int p_subdivisions, real_t p_width, real_t p_height, real_t p_depth,
		real_t p_noise_frequency, real_t p_noise_amplitude, int p_noise_octaves, real_t p_noise_persistence,
		int p_randseed, bool p_cutplane_enabled, real_t p_cutplane_offset, bool p_smoothed);

// ProcRockPipelineTextures / rock_pipeline_gen_textures() / rock_pipeline_make_material()
// now live in generators/shared/texture_gen.h — they were always pure 2D image/material
// code with no ProcRock-specific coupling, so any generator can use them (see memo.md's
// "Texture generation" section). Only the JSON-driven variant and texture adders below
// are inherently ProcRock-specific (they consume procrocklib's JSON pipeline format,
// which only Method 3 ever produces).

// JSON pipeline preset support — p_pipeline_json is an already-parsed Variant::DICTIONARY
// matching procrocklib's {"generator","modifiers","parameterizer","textureAdders",
// "textureGenerator"} pipeline shape (see editor/proc_rocks_demo/presets/*.json and
// memo.md's "Noise graph interpreter" / "JSON pipeline reader" sections for scope and
// limitations — the modifier chain and non-Icosahedron generators aren't implemented yet).
bool rock_pipeline_json_is_valid(const Dictionary &p_pipeline_json);

Array rock_pipeline_gen_from_json(int p_subdivisions, real_t p_width, real_t p_height, real_t p_depth,
		const Dictionary &p_pipeline_json, int p_randseed,
		bool p_cutplane_enabled, real_t p_cutplane_offset, bool p_smoothed);

ProcRockPipelineTextures rock_pipeline_gen_textures_from_json(int p_size, const Dictionary &p_pipeline_json);

// Editor-only baked PBR texture packs (gravel/mossy/rock) live in
// modules/gdextensions/editor/proc_rocks_editor_plugin.{h,cpp} — the ProcRock dock's
// "Demo Texture" picker is their only consumer, and that file is TOOLS_ENABLED-only.

#endif // PROCROCKGEN_H
