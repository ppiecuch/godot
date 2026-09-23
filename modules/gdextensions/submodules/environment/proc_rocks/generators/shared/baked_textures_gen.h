/**************************************************************************/
/*  baked_textures_gen.h                                                  */
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

#ifndef PROC_ROCKS_SHARED_BAKED_TEXTURES_GEN_H
#define PROC_ROCKS_SHARED_BAKED_TEXTURES_GEN_H

#include "texture_gen.h"

// Editor-only baked PBR texture packs (gravel/mossy/rock), embedded via INCBIN from
// editor/proc_rocks_demo/baked_textures.h. Originally lived only in
// proc_rocks_editor_plugin.cpp (the ProcRock dock's "Demo Texture" picker was their
// only consumer) — moved here so ProcRockMesh's generic texture_source property
// (any generator, see proc_rocks.h) and the editor dock can share one loader instead
// of duplicating it. Only baked_textures_gen.cpp includes the actual INCBIN header,
// so this declaration-only header is safe to include unconditionally.

enum ProcRockBakedTexturePack {
	PROCROCK_BAKED_GRAVEL,
	PROCROCK_BAKED_MOSSY,
	PROCROCK_BAKED_ROCK,
};

ProcRockPipelineTextures load_baked_textures(ProcRockBakedTexturePack p_pack);

#endif // PROC_ROCKS_SHARED_BAKED_TEXTURES_GEN_H
