/**************************************************************************/
/*  baked_textures.h                                                      */
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

// Editor-only baked PBR texture packs for the ProcRock dock's "Demo Texture"
// picker (see ProcRockDialog). Never compiled into export templates — only
// included from proc_rocks_editor_plugin.cpp within an #ifdef TOOLS_ENABLED block.

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

#define ROOT "editor/proc_rocks_demo/"

INCBIN(gravel_albedo_jpg, ROOT "gravel/albedo.jpg");
INCBIN(gravel_ambientOcc_jpg, ROOT "gravel/ambientOcc.jpg");
INCBIN(gravel_displacement_jpg, ROOT "gravel/displacement.jpg");
INCBIN(gravel_normals_jpg, ROOT "gravel/normals.jpg");
INCBIN(gravel_roughness_jpg, ROOT "gravel/roughness.jpg");
INCBIN(moss_albedo_jpg, ROOT "mossy/albedo.jpg");
INCBIN(moss_ambientOcc_jpg, ROOT "mossy/ambientOcc.jpg");
INCBIN(moss_displacement_jpg, ROOT "mossy/displacement.jpg");
INCBIN(moss_normals_jpg, ROOT "mossy/normals.jpg");
INCBIN(moss_roughness_jpg, ROOT "mossy/roughness.jpg");
INCBIN(rock_jpg, ROOT "rock/rock.jpg");
