/**************************************************************************/
/*  hw_psp.h                                                              */
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

#ifndef HW_PSP_H
#define HW_PSP_H

#include "core/math/math_funcs.h"

#ifdef PSP
#include <pspmath.h>

// VFPU hardware math — faster than software libm on PSP
static float hw_psp_vfpu_sqrt(float p_value) { return vfpu_sqrtf(p_value); }
static float hw_psp_vfpu_sin(float p_value) { return vfpu_sinf(p_value); }
static float hw_psp_vfpu_cos(float p_value) { return vfpu_cosf(p_value); }

#else // !PSP — software fallbacks

static float hw_psp_vfpu_sqrt(float p_value) { return Math::sqrt(p_value); }
static float hw_psp_vfpu_sin(float p_value) { return Math::sin(p_value); }
static float hw_psp_vfpu_cos(float p_value) { return Math::cos(p_value); }

#endif // PSP

#endif // HW_PSP_H
