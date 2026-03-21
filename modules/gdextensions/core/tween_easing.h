/**************************************************************************/
/*  tween_easing.h                                                        */
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

// Robert Penner easing equations.
// Based on libtween (C) 2013 libtween authors, tween.js (c) 2010-2013.
// http://robertpenner.com/easing/

#ifndef TWEEN_EASING_H
#define TWEEN_EASING_H

#include "core/math/math_defs.h"

enum Tween_Easing {
	TWEEN_EASING_LINEAR,
	TWEEN_EASING_QUADRATIC_IN,
	TWEEN_EASING_QUADRATIC_OUT,
	TWEEN_EASING_QUADRATIC_IN_OUT,
	TWEEN_EASING_CUBIC_IN,
	TWEEN_EASING_CUBIC_OUT,
	TWEEN_EASING_CUBIC_IN_OUT,
	TWEEN_EASING_QUARTIC_IN,
	TWEEN_EASING_QUARTIC_OUT,
	TWEEN_EASING_QUARTIC_IN_OUT,
	TWEEN_EASING_QUINTIC_IN,
	TWEEN_EASING_QUINTIC_OUT,
	TWEEN_EASING_QUINTIC_IN_OUT,
	TWEEN_EASING_SINUSOIDAL_IN,
	TWEEN_EASING_SINUSOIDAL_OUT,
	TWEEN_EASING_SINUSOIDAL_IN_OUT,
	TWEEN_EASING_EXPONENTIAL_IN,
	TWEEN_EASING_EXPONENTIAL_OUT,
	TWEEN_EASING_EXPONENTIAL_IN_OUT,
	TWEEN_EASING_CIRCULAR_IN,
	TWEEN_EASING_CIRCULAR_OUT,
	TWEEN_EASING_CIRCULAR_IN_OUT,
	TWEEN_EASING_ELASTIC_IN,
	TWEEN_EASING_ELASTIC_OUT,
	TWEEN_EASING_ELASTIC_IN_OUT,
	TWEEN_EASING_BACK_IN,
	TWEEN_EASING_BACK_OUT,
	TWEEN_EASING_BACK_IN_OUT,
	TWEEN_EASING_BOUNCE_IN,
	TWEEN_EASING_BOUNCE_OUT,
	TWEEN_EASING_BOUNCE_IN_OUT,
	TWEEN_EASING_COUNT
};

typedef real_t (*Tween_Easing_Func)(real_t);
extern Tween_Easing_Func tweenEasingFuncs[];

// Evaluate an easing function by enum value.
real_t tween_ease(Tween_Easing p_easing, real_t p_t);

// Get the name of an easing function.
const char *tween_easing_name(Tween_Easing p_easing);

#endif // TWEEN_EASING_H
