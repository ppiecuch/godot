/**************************************************************************/
/*  demo.h                                                                */
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

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

#define ROOT "modules/gdextensions/submodules/environment/proc_rocks/generators/procrockgen/"

INCBIN(json_1, ROOT "demo/example/1.json");
INCBIN(json_2, ROOT "demo/example/2.json");
INCBIN(json_3, ROOT "demo/example/3.json");
INCBIN(json_4, ROOT "demo/example/4.json");
INCBIN(json_5, ROOT "demo/example/5.json");
INCBIN(json_6, ROOT "demo/example/6.json");
INCBIN(json_7, ROOT "demo/example/7.json");
INCBIN(json_8, ROOT "demo/example/8.json");
INCBIN(json_9, ROOT "demo/example/9.json");
INCBIN(json_10, ROOT "demo/example/10.json");
INCBIN(json_11, ROOT "demo/example/11.json");
INCBIN(json_12, ROOT "demo/example/12.json");
INCBIN(json_granite_custom, ROOT "demo/example/granite_custom.json");

INCBIN(gravel_albedo_jpg, ROOT "demo/textures/gravel/albedo.jpg");
INCBIN(gravel_ambientOcc_jpg, ROOT "demo/textures/gravel/ambientOcc.jpg");
INCBIN(gravel_displacement_jpg, ROOT "demo/textures/gravel/displacement.jpg");
INCBIN(gravel_normals_jpg, ROOT "demo/textures/gravel/normals.jpg");
INCBIN(gravel_roughness_jpg, ROOT "demo/textures/gravel/roughness.jpg");
INCBIN(moss_albedo_jpg, ROOT "demo/textures/mossy/albedo.jpg");
INCBIN(moss_ambientOcc_jpg, ROOT "demo/textures/mossy/ambientOcc.jpg");
INCBIN(moss_displacement_jpg, ROOT "demo/textures/mossy/displacement.jpg");
INCBIN(moss_normals_jpg, ROOT "demo/textures/mossy/normals.jpg");
INCBIN(moss_roughness_jpg, ROOT "demo/textures/mossy/roughness.jpg");
INCBIN(rock_jpg, ROOT "demo/textures/rock/rock.jpg");
