/**************************************************************************/
/*  tween_easing.cpp                                                      */
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
// Based on libtween (C) 2013, tween.js (c) 2010-2012.

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "tween_easing.h"

#include "core/math/math_funcs.h"

#include <cstring>

// -- Easing implementations --
// All functions take t in [0, 1] and return the eased value.

static real_t ease_linear(real_t k) {
	return k;
}

// Quadratic

static real_t ease_quad_in(real_t k) {
	return k * k;
}

static real_t ease_quad_out(real_t k) {
	return k * ((real_t)2.0 - k);
}

static real_t ease_quad_in_out(real_t k) {
	k *= (real_t)2.0;
	if (k < (real_t)1.0) {
		return (real_t)0.5 * k * k;
	}
	k -= (real_t)1.0;
	return (real_t)-0.5 * (k * (k - (real_t)2.0) - (real_t)1.0);
}

// Cubic

static real_t ease_cubic_in(real_t k) {
	return k * k * k;
}

static real_t ease_cubic_out(real_t k) {
	k -= (real_t)1.0;
	return k * k * k + (real_t)1.0;
}

static real_t ease_cubic_in_out(real_t k) {
	k *= (real_t)2.0;
	if (k < (real_t)1.0) {
		return (real_t)0.5 * k * k * k;
	}
	k -= (real_t)2.0;
	return (real_t)0.5 * (k * k * k + (real_t)2.0);
}

// Quartic

static real_t ease_quart_in(real_t k) {
	return k * k * k * k;
}

static real_t ease_quart_out(real_t k) {
	k -= (real_t)1.0;
	return (real_t)1.0 - k * k * k * k;
}

static real_t ease_quart_in_out(real_t k) {
	k *= (real_t)2.0;
	if (k < (real_t)1.0) {
		return (real_t)0.5 * k * k * k * k;
	}
	k -= (real_t)2.0;
	return (real_t)-0.5 * (k * k * k * k - (real_t)2.0);
}

// Quintic

static real_t ease_quint_in(real_t k) {
	return k * k * k * k * k;
}

static real_t ease_quint_out(real_t k) {
	k -= (real_t)1.0;
	return k * k * k * k * k + (real_t)1.0;
}

static real_t ease_quint_in_out(real_t k) {
	k *= (real_t)2.0;
	if (k < (real_t)1.0) {
		return (real_t)0.5 * k * k * k * k * k;
	}
	k -= (real_t)2.0;
	return (real_t)0.5 * (k * k * k * k * k + (real_t)2.0);
}

// Sinusoidal

static real_t ease_sine_in(real_t k) {
	return (real_t)1.0 - Math::cos(k * Math_PI * (real_t)0.5);
}

static real_t ease_sine_out(real_t k) {
	return Math::sin(k * Math_PI * (real_t)0.5);
}

static real_t ease_sine_in_out(real_t k) {
	return (real_t)0.5 * ((real_t)1.0 - Math::cos(Math_PI * k));
}

// Exponential

static real_t ease_expo_in(real_t k) {
	return k == (real_t)0.0 ? (real_t)0.0 : Math::pow((real_t)1024.0, k - (real_t)1.0);
}

static real_t ease_expo_out(real_t k) {
	return k == (real_t)1.0 ? (real_t)1.0 : (real_t)1.0 - Math::pow((real_t)2.0, (real_t)-10.0 * k);
}

static real_t ease_expo_in_out(real_t k) {
	if (k == (real_t)0.0) return (real_t)0.0;
	if (k == (real_t)1.0) return (real_t)1.0;
	k *= (real_t)2.0;
	if (k < (real_t)1.0) {
		return (real_t)0.5 * Math::pow((real_t)1024.0, k - (real_t)1.0);
	}
	return (real_t)0.5 * (-Math::pow((real_t)2.0, (real_t)-10.0 * (k - (real_t)1.0)) + (real_t)2.0);
}

// Circular

static real_t ease_circ_in(real_t k) {
	return (real_t)1.0 - Math::sqrt((real_t)1.0 - k * k);
}

static real_t ease_circ_out(real_t k) {
	k -= (real_t)1.0;
	return Math::sqrt((real_t)1.0 - k * k);
}

static real_t ease_circ_in_out(real_t k) {
	k *= (real_t)2.0;
	if (k < (real_t)1.0) {
		return (real_t)-0.5 * (Math::sqrt((real_t)1.0 - k * k) - (real_t)1.0);
	}
	k -= (real_t)2.0;
	return (real_t)0.5 * (Math::sqrt((real_t)1.0 - k * k) + (real_t)1.0);
}

// Elastic

static real_t ease_elastic_in(real_t k) {
	if (k == (real_t)0.0) return (real_t)0.0;
	if (k == (real_t)1.0) return (real_t)1.0;
	real_t p = (real_t)0.4;
	real_t s = p / (real_t)4.0;
	k -= (real_t)1.0;
	return -(Math::pow((real_t)2.0, (real_t)10.0 * k) * Math::sin((k - s) * Math_TAU / p));
}

static real_t ease_elastic_out(real_t k) {
	if (k == (real_t)0.0) return (real_t)0.0;
	if (k == (real_t)1.0) return (real_t)1.0;
	real_t p = (real_t)0.4;
	real_t s = p / (real_t)4.0;
	return Math::pow((real_t)2.0, (real_t)-10.0 * k) * Math::sin((k - s) * Math_TAU / p) + (real_t)1.0;
}

static real_t ease_elastic_in_out(real_t k) {
	if (k == (real_t)0.0) return (real_t)0.0;
	if (k == (real_t)1.0) return (real_t)1.0;
	real_t p = (real_t)0.4;
	real_t s = p / (real_t)4.0;
	k *= (real_t)2.0;
	if (k < (real_t)1.0) {
		k -= (real_t)1.0;
		return (real_t)-0.5 * (Math::pow((real_t)2.0, (real_t)10.0 * k) * Math::sin((k - s) * Math_TAU / p));
	}
	k -= (real_t)1.0;
	return Math::pow((real_t)2.0, (real_t)-10.0 * k) * Math::sin((k - s) * Math_TAU / p) * (real_t)0.5 + (real_t)1.0;
}

// Back

static const real_t BACK_S = (real_t)1.70158;

static real_t ease_back_in(real_t k) {
	return k * k * ((BACK_S + (real_t)1.0) * k - BACK_S);
}

static real_t ease_back_out(real_t k) {
	k -= (real_t)1.0;
	return k * k * ((BACK_S + (real_t)1.0) * k + BACK_S) + (real_t)1.0;
}

static real_t ease_back_in_out(real_t k) {
	real_t s = BACK_S * (real_t)1.525;
	k *= (real_t)2.0;
	if (k < (real_t)1.0) {
		return (real_t)0.5 * (k * k * ((s + (real_t)1.0) * k - s));
	}
	k -= (real_t)2.0;
	return (real_t)0.5 * (k * k * ((s + (real_t)1.0) * k + s) + (real_t)2.0);
}

// Bounce

static real_t ease_bounce_out(real_t k);

static real_t ease_bounce_in(real_t k) {
	return (real_t)1.0 - ease_bounce_out((real_t)1.0 - k);
}

static real_t ease_bounce_out(real_t k) {
	if (k < (real_t)(1.0 / 2.75)) {
		return (real_t)7.5625 * k * k;
	} else if (k < (real_t)(2.0 / 2.75)) {
		k -= (real_t)(1.5 / 2.75);
		return (real_t)7.5625 * k * k + (real_t)0.75;
	} else if (k < (real_t)(2.5 / 2.75)) {
		k -= (real_t)(2.25 / 2.75);
		return (real_t)7.5625 * k * k + (real_t)0.9375;
	}
	k -= (real_t)(2.625 / 2.75);
	return (real_t)7.5625 * k * k + (real_t)0.984375;
}

static real_t ease_bounce_in_out(real_t k) {
	if (k < (real_t)0.5) {
		return ease_bounce_in(k * (real_t)2.0) * (real_t)0.5;
	}
	return ease_bounce_out(k * (real_t)2.0 - (real_t)1.0) * (real_t)0.5 + (real_t)0.5;
}

// -- Function pointer table --

Tween_Easing_Func tweenEasingFuncs[] = {
	ease_linear,
	ease_quad_in, ease_quad_out, ease_quad_in_out,
	ease_cubic_in, ease_cubic_out, ease_cubic_in_out,
	ease_quart_in, ease_quart_out, ease_quart_in_out,
	ease_quint_in, ease_quint_out, ease_quint_in_out,
	ease_sine_in, ease_sine_out, ease_sine_in_out,
	ease_expo_in, ease_expo_out, ease_expo_in_out,
	ease_circ_in, ease_circ_out, ease_circ_in_out,
	ease_elastic_in, ease_elastic_out, ease_elastic_in_out,
	ease_back_in, ease_back_out, ease_back_in_out,
	ease_bounce_in, ease_bounce_out, ease_bounce_in_out,
};

// -- Convenience functions --

real_t tween_ease(Tween_Easing p_easing, real_t p_t) {
	if (p_easing >= 0 && p_easing < TWEEN_EASING_COUNT) {
		return tweenEasingFuncs[p_easing](p_t);
	}
	return p_t;
}

static const char *_easing_names[] = {
	"Linear",
	"QuadraticIn", "QuadraticOut", "QuadraticInOut",
	"CubicIn", "CubicOut", "CubicInOut",
	"QuarticIn", "QuarticOut", "QuarticInOut",
	"QuinticIn", "QuinticOut", "QuinticInOut",
	"SinusoidalIn", "SinusoidalOut", "SinusoidalInOut",
	"ExponentialIn", "ExponentialOut", "ExponentialInOut",
	"CircularIn", "CircularOut", "CircularInOut",
	"ElasticIn", "ElasticOut", "ElasticInOut",
	"BackIn", "BackOut", "BackInOut",
	"BounceIn", "BounceOut", "BounceInOut",
};

const char *tween_easing_name(Tween_Easing p_easing) {
	if (p_easing >= 0 && p_easing < TWEEN_EASING_COUNT) {
		return _easing_names[p_easing];
	}
	return "Unknown";
}

// -- Tests --

#ifdef DOCTEST

TEST_CASE("[TweenEasing] all functions return 0 at t=0") {
	for (int i = 0; i < TWEEN_EASING_COUNT; i++) {
		real_t val = tweenEasingFuncs[i]((real_t)0.0);
		CHECK_MESSAGE(val == doctest::Approx(0.0).epsilon(0.01),
			"Easing ", tween_easing_name((Tween_Easing)i), " at t=0 returned ", val);
	}
}

TEST_CASE("[TweenEasing] all functions return 1 at t=1") {
	for (int i = 0; i < TWEEN_EASING_COUNT; i++) {
		real_t val = tweenEasingFuncs[i]((real_t)1.0);
		CHECK_MESSAGE(val == doctest::Approx(1.0).epsilon(0.01),
			"Easing ", tween_easing_name((Tween_Easing)i), " at t=1 returned ", val);
	}
}

TEST_CASE("[TweenEasing] linear is identity") {
	for (int i = 0; i <= 10; i++) {
		real_t t = i / (real_t)10.0;
		CHECK(tween_ease(TWEEN_EASING_LINEAR, t) == doctest::Approx(t));
	}
}

TEST_CASE("[TweenEasing] quadratic_in is t^2") {
	CHECK(tween_ease(TWEEN_EASING_QUADRATIC_IN, (real_t)0.5) == doctest::Approx(0.25));
	CHECK(tween_ease(TWEEN_EASING_QUADRATIC_IN, (real_t)0.25) == doctest::Approx(0.0625));
}

TEST_CASE("[TweenEasing] cubic_in is t^3") {
	CHECK(tween_ease(TWEEN_EASING_CUBIC_IN, (real_t)0.5) == doctest::Approx(0.125));
}

TEST_CASE("[TweenEasing] in_out symmetry") {
	// InOut functions should have f(0.5) ≈ 0.5 (symmetric midpoint)
	int in_out_indices[] = {
		TWEEN_EASING_QUADRATIC_IN_OUT,
		TWEEN_EASING_CUBIC_IN_OUT,
		TWEEN_EASING_QUARTIC_IN_OUT,
		TWEEN_EASING_QUINTIC_IN_OUT,
		TWEEN_EASING_SINUSOIDAL_IN_OUT,
		TWEEN_EASING_EXPONENTIAL_IN_OUT,
		TWEEN_EASING_CIRCULAR_IN_OUT,
		TWEEN_EASING_BOUNCE_IN_OUT,
	};
	for (int idx : in_out_indices) {
		real_t val = tweenEasingFuncs[idx]((real_t)0.5);
		CHECK_MESSAGE(val == doctest::Approx(0.5).epsilon(0.01),
			"InOut easing ", tween_easing_name((Tween_Easing)idx), " at t=0.5 returned ", val);
	}
}

TEST_CASE("[TweenEasing] bounce_out mid-range") {
	// Bounce out at 0.5 should be around 0.765625
	real_t val = tween_ease(TWEEN_EASING_BOUNCE_OUT, (real_t)0.5);
	CHECK(val == doctest::Approx(0.765625).epsilon(0.01));
}

TEST_CASE("[TweenEasing] back overshoot") {
	// Back easing should overshoot: back_in at t≈0.5 should be < 0 at some point
	real_t val = tween_ease(TWEEN_EASING_BACK_IN, (real_t)0.2);
	CHECK(val < (real_t)0.0); // overshoots below zero
}

TEST_CASE("[TweenEasing] tween_ease invalid enum") {
	// Out-of-range enum should return t unchanged
	CHECK(tween_ease((Tween_Easing)-1, (real_t)0.5) == doctest::Approx(0.5));
	CHECK(tween_ease((Tween_Easing)999, (real_t)0.7) == doctest::Approx(0.7));
}

TEST_CASE("[TweenEasing] tween_easing_name") {
	CHECK(strcmp(tween_easing_name(TWEEN_EASING_LINEAR), "Linear") == 0);
	CHECK(strcmp(tween_easing_name(TWEEN_EASING_BOUNCE_OUT), "BounceOut") == 0);
	CHECK(strcmp(tween_easing_name((Tween_Easing)999), "Unknown") == 0);
}

TEST_CASE("[TweenEasing] function count matches enum") {
	// Verify the array has exactly TWEEN_EASING_COUNT entries
	CHECK(sizeof(tweenEasingFuncs) / sizeof(tweenEasingFuncs[0]) == TWEEN_EASING_COUNT);
	CHECK(sizeof(_easing_names) / sizeof(_easing_names[0]) == TWEEN_EASING_COUNT);
}

#endif
