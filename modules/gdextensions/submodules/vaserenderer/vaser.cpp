/*************************************************************************/
/*  vaser.cpp                                                            */
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

#include "vaser.h"

#ifdef VASER_DEBUG
#define DEBUG printf
#else
#define DEBUG ; //
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

namespace VASEr {
namespace VASErin { // VASEr internal namespace

const real_t vaser_min_alw = 0.0000001; //smallest value not regarded as zero
const Color default_color = { 0, 0, 0, 1 };
const real_t default_weight = 1;
#include "inc/color.h"
#include "inc/point.h"
class vertex_array_holder;
#include "inc/backend.h"
#include "inc/vertex_array_holder.h"
#include "inl/agg_curve4.cpp.inl"
} // namespace VASErin

#include "inl/curve.cpp.inl"
#include "inl/gradient.cpp.inl"
#include "inl/opengl.cpp.inl"
#include "inl/polyline.cpp.inl"
} //namespace VASEr

#undef DEBUG
