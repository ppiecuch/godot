/*************************************************************************/
/*  TTF.h                                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#ifdef __GNUC__
#define _unused __attribute__((unused))
#else
#define _unused
#endif

#include "common/gd_core.h"

#include "TTFExceptions.h"
#include "TTFMath.h"
#include "TTFTypes.h"

#include "TTFFont.h"
#include "TTFTriangulator2D.h"
#include "TTFTriangulator2DLinear.h"
#include "TTFTriangulator3D.h"
#include "TTFTriangulator3DBezel.h"

namespace TTF {

using TTFCore::ChecksumException;
using TTFCore::FileFailure;
using TTFCore::FileLengthError;
using TTFCore::FontException;
using TTFCore::InvalidFontException;
using TTFCore::TableDoesNotExist;
using TTFCore::UnsupportedCap;
using TTFCore::VersionException;

using TTFCore::vec2f;
using TTFCore::vec2t;
using TTFCore::vec3f;
using TTFCore::vec3t;
using TTFCore::vec4f;
using TTFCore::vec4t;

using TTFCore::TriangulatorFlags;

using TTFCore::CodePoint;
using TTFCore::Font;
using TTFCore::FontMetrics;
using TTFCore::GlyphMetrics;
using TTFCore::MapFromData;
using TTFCore::VFontMetrics;
using TTFCore::VGlyphMetrics;

using TTFCore::TriLarge;
using TTFCore::TriSmall;
typedef TTFCore::Triangulator2D<vec2t, TTFCore::TriSmall> Triangulator2DI;
typedef TTFCore::Triangulator2D<vec2t, TTFCore::TriLarge> Triangulator2DII;
typedef TTFCore::Triangulator2DLinear<vec2t, TTFCore::TriSmall> Triangulator2DLinearI;
typedef TTFCore::Triangulator2DLinear<vec2t, TTFCore::TriLarge> Triangulator2DLinearII;
typedef TTFCore::Triangulator3D<vec3t, TTFCore::TriSmall> Triangulator3DI;
typedef TTFCore::Triangulator3D<vec3t, TTFCore::TriLarge> Triangulator3DII;
typedef TTFCore::Triangulator3DBezel<vec3t, TTFCore::TriSmall> Triangulator3DBezelI;
typedef TTFCore::Triangulator3DBezel<vec3t, TTFCore::TriLarge> Triangulator3DBezelII;

const TTFCore::MapFromData map_from_data{};

} //namespace TTF
