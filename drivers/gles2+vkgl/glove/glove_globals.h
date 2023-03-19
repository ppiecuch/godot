/**************************************************************************/
/*  glove_globals.h                                                       */
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

/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 */

/// A header file that includes several global variables.

#ifndef GLOVE_GLOBALS_H
#define GLOVE_GLOBALS_H

#include <assert.h>
#include <string.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "GLES3/gl3.h"
#include "GLES3/gl3ext.h"

#include "glLogger.h"
#include "rendering_api_interface.h"
#include "vulkan/vulkan.h"

// Compiler
#ifdef __GNUC__
#define ASSERT_ONLY __attribute__((unused))
#define COMPILER_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define FORCE_INLINE __attribute__((always_inline))
#else
#define ASSERT_ONLY
#define COMPILER_WARN_UNUSED_RESULT
#endif // __GNUC__

// GL ES Limits
#define GLOVE_MAX_VERTEX_ATTRIBS 32 // MIN VALUE:   8
#define GLOVE_MAX_VARYING_VECTORS 8 // MIN VALUE:   8
#define GLOVE_MAX_VERTEX_UNIFORM_VECTORS 128 // MIN VALUE: 128
#define GLOVE_MAX_FRAGMENT_UNIFORM_VECTORS 128 // MIN VALUE:  16
#define GLOVE_MAX_COMBINED_UNIFORM_VECTORS GLOVE_MAX_VERTEX_UNIFORM_VECTORS + GLOVE_MAX_FRAGMENT_UNIFORM_VECTORS

#define GLOVE_MAX_VERTEX_TEXTURE_IMAGE_UNITS 8 // MIN VALUE:  0
#define GLOVE_MAX_TEXTURE_IMAGE_UNITS 32 // MIN VALUE:  8
#define GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS GLOVE_MAX_VERTEX_TEXTURE_IMAGE_UNITS + GLOVE_MAX_TEXTURE_IMAGE_UNITS

#define GLOVE_MAX_DRAW_BUFFERS 1 // MIN VALUE:  1

#define GLOVE_SUBPIXEL_BITS 4 // MIN VALUE:  4

#define GLOVE_MAX_TEXTURE_SIZE 4096
#define GLOVE_MAX_CUBE_MAP_TEXTURE_SIZE 4096
#define GLOVE_MAX_RENDERBUFFER_SIZE 4096

#define GLOVE_NUM_SHADER_BINARY_FORMATS 0
#define GLOVE_NUM_PROGRAM_BINARY_FORMATS 1

/// Global switches
#define GLOVE_SAVE_SHADER_SOURCES_TO_FILES false
#define GLOVE_SAVE_PROCESSED_SHADER_SOURCES_TO_FILES false
#define GLOVE_SAVE_SPIRV_BINARY_TO_FILES false
#define GLOVE_SAVE_SPIRV_TEXT_TO_FILE false

#define GLOVE_SAVE_READPIXELS_TO_FILE false
#define GLOVE_SAVE_TEXTURES_TO_FILE false

#define GLOVE_DUMP_INPUT_SHADER_REFLECTION false
#define GLOVE_DUMP_VULKAN_SHADER_REFLECTION false
#define GLOVE_DUMP_ORIGINAL_SHADER_SOURCE false
#define GLOVE_DUMP_PROCESSED_SHADER_SOURCE false
#define GLOVE_DUMP_SPIRV_SHADER_SOURCE false

#define GLOVE_INVALID_OFFSET UINT32_MAX

#define GLOVE_VULKAN_DEPTH_RANGE vulkan_DepthRange

#endif // GLOVE_GLOBALS_H
