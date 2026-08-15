/**************************************************************************/
/*  test_label.cpp                                                        */
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

#include "test_label.h"

#include "core/os/os.h"
#include "scene/gui/label.h"

#define CHECK(X)                                                             \
	if (!(X)) {                                                              \
		OS::get_singleton()->print("\tFAIL at line %d: %s\n", __LINE__, #X); \
		return false;                                                        \
	} else {                                                                 \
		OS::get_singleton()->print("\tPASS\n");                              \
	}

#define CHECK_APPROX(a, b)                                                                                                   \
	if (Math::abs((a) - (b)) > 0.001) {                                                                                      \
		OS::get_singleton()->print("\tFAIL at line %d: %s ≈ %s (got %f vs %f)\n", __LINE__, #a, #b, (float)(a), (float)(b)); \
		return false;                                                                                                        \
	} else {                                                                                                                 \
		OS::get_singleton()->print("\tPASS\n");                                                                              \
	}

namespace TestLabel {

// --- compensate_spacing tests ---

bool test_spacing_scale_1() {
	OS::get_singleton()->print("  compensate_spacing: scale=1.0 should return spacing unchanged\n");
	// At scale 1.0, spacing stays the same.
	CHECK_APPROX(Label::compensate_spacing(2.0, 1.0), 2.0);
	CHECK_APPROX(Label::compensate_spacing(5.0, 1.0), 5.0);
	CHECK_APPROX(Label::compensate_spacing(0.5, 1.0), 1.0); // min 1px: 0.5 / 1.0 = 0.5, but MAX(1.0, 0.5) = 1.0
	return true;
}

bool test_spacing_scale_up() {
	OS::get_singleton()->print("  compensate_spacing: scale > 1 should divide spacing by scale\n");
	// Scale 2.0: spacing 4px → 4/2 = 2 label-local px → 2*2 = 4 screen px ✓
	CHECK_APPROX(Label::compensate_spacing(4.0, 2.0), 2.0);
	// Scale 3.0: spacing 6px → 6/3 = 2 label-local px → 2*3 = 6 screen px ✓
	CHECK_APPROX(Label::compensate_spacing(6.0, 3.0), 2.0);
	return true;
}

bool test_spacing_scale_down() {
	OS::get_singleton()->print("  compensate_spacing: scale < 1 should divide spacing by scale (increase)\n");
	// Scale 0.5: spacing 2px → 2/0.5 = 4 label-local px → 4*0.5 = 2 screen px ✓
	CHECK_APPROX(Label::compensate_spacing(2.0, 0.5), 4.0);
	// Scale 0.25: spacing 2px → 2/0.25 = 8 label-local px → 8*0.25 = 2 screen px ✓
	CHECK_APPROX(Label::compensate_spacing(2.0, 0.25), 8.0);
	return true;
}

bool test_spacing_min_1px() {
	OS::get_singleton()->print("  compensate_spacing: result * scale should never be < 1 screen px\n");
	// Scale 2.0: spacing 1px → MAX(1/2, 1/2) = 0.5 → 0.5*2 = 1 screen px ✓
	real_t r1 = Label::compensate_spacing(1.0, 2.0);
	CHECK_APPROX(r1 * 2.0, 1.0);
	// Scale 4.0: spacing 2px → MAX(1/4, 2/4) = 0.5 → 0.5*4 = 2 screen px ✓
	real_t r2 = Label::compensate_spacing(2.0, 4.0);
	CHECK_APPROX(r2 * 4.0, 2.0);
	// Scale 10.0: spacing 1px → MAX(1/10, 1/10) = 0.1 → 0.1*10 = 1 screen px ✓
	real_t r3 = Label::compensate_spacing(1.0, 10.0);
	CHECK_APPROX(r3 * 10.0, 1.0);
	// Scale 0.1: spacing 0.05px → MAX(1/0.1, 0.05/0.1) = MAX(10, 0.5) = 10 → 10*0.1 = 1 screen px ✓
	real_t r4 = Label::compensate_spacing(0.05, 0.1);
	CHECK_APPROX(r4 * 0.1, 1.0);
	return true;
}

bool test_spacing_zero() {
	OS::get_singleton()->print("  compensate_spacing: spacing=0 should pass through unchanged\n");
	CHECK_APPROX(Label::compensate_spacing(0.0, 1.0), 0.0);
	CHECK_APPROX(Label::compensate_spacing(0.0, 2.0), 0.0);
	CHECK_APPROX(Label::compensate_spacing(0.0, 0.5), 0.0);
	return true;
}

bool test_spacing_negative() {
	OS::get_singleton()->print("  compensate_spacing: negative spacing should be divided by scale\n");
	// -2 / 1.0 = -2
	CHECK_APPROX(Label::compensate_spacing(-2.0, 1.0), -2.0);
	// -3 / 0.5 = -6 (compensate for small scale)
	CHECK_APPROX(Label::compensate_spacing(-3.0, 0.5), -6.0);
	// -1 / 2.0 = -0.5 (compensate for large scale)
	CHECK_APPROX(Label::compensate_spacing(-1.0, 2.0), -0.5);
	return true;
}

bool test_spacing_zero_scale() {
	OS::get_singleton()->print("  compensate_spacing: scale=0 should pass through unchanged (avoid div by zero)\n");
	CHECK_APPROX(Label::compensate_spacing(2.0, 0.0), 2.0);
	CHECK_APPROX(Label::compensate_spacing(0.0, 0.0), 0.0);
	CHECK_APPROX(Label::compensate_spacing(-1.0, 0.0), -1.0);
	return true;
}

bool test_spacing_negative_scale() {
	OS::get_singleton()->print("  compensate_spacing: negative scale should pass through unchanged\n");
	CHECK_APPROX(Label::compensate_spacing(2.0, -1.0), 2.0);
	CHECK_APPROX(Label::compensate_spacing(5.0, -0.5), 5.0);
	return true;
}

bool test_spacing_screen_px_identity() {
	OS::get_singleton()->print("  compensate_spacing: result * scale should always equal original spacing (when >= 1px)\n");
	// For various scales, verify: compensate_spacing(s, sc) * sc == s
	real_t spacings[] = { 1.0, 2.0, 3.5, 10.0 };
	real_t scales[] = { 0.25, 0.5, 1.0, 1.5, 2.0, 3.0, 4.0 };
	for (real_t s : spacings) {
		for (real_t sc : scales) {
			real_t compensated = Label::compensate_spacing(s, sc);
			real_t screen_px = compensated * sc;
			// Screen pixels should be MAX(1.0, s)
			real_t expected = MAX(1.0, s);
			CHECK_APPROX(screen_px, expected);
		}
	}
	return true;
}

bool test_spacing_cap_positive_small_scale() {
	OS::get_singleton()->print("  compensate_spacing: spacing=1, scale=0.5 should cap to 1 screen px\n");
	// spacing=1, scale=0.5: 1/0.5 = 2 label-local → 2*0.5 = 1 screen px
	real_t r = Label::compensate_spacing(1.0, 0.5);
	CHECK_APPROX(r * 0.5, 1.0);
	// spacing=1, scale=0.25: 1/0.25 = 4 label-local → 4*0.25 = 1 screen px
	real_t r2 = Label::compensate_spacing(1.0, 0.25);
	CHECK_APPROX(r2 * 0.25, 1.0);
	return true;
}

bool test_spacing_cap_negative_small_scale() {
	OS::get_singleton()->print("  compensate_spacing: spacing=-1, scale=0.5 should cap to -1 screen px\n");
	// spacing=-1, scale=0.5: -1/0.5 = -2 label-local → -2*0.5 = -1 screen px
	real_t r = Label::compensate_spacing(-1.0, 0.5);
	CHECK_APPROX(r * 0.5, -1.0);
	// spacing=-1, scale=0.25: -1/0.25 = -4 label-local → -4*0.25 = -1 screen px
	real_t r2 = Label::compensate_spacing(-1.0, 0.25);
	CHECK_APPROX(r2 * 0.25, -1.0);
	// spacing=-1, scale=2.0: -1/2 = -0.5 label-local → -0.5*2 = -1 screen px
	real_t r3 = Label::compensate_spacing(-1.0, 2.0);
	CHECK_APPROX(r3 * 2.0, -1.0);
	return true;
}

typedef bool (*TestFunc)();
TestFunc test_funcs[] = {
	test_spacing_scale_1,
	test_spacing_scale_up,
	test_spacing_scale_down,
	test_spacing_min_1px,
	test_spacing_zero,
	test_spacing_negative,
	test_spacing_zero_scale,
	test_spacing_negative_scale,
	test_spacing_screen_px_identity,
	test_spacing_cap_positive_small_scale,
	test_spacing_cap_negative_small_scale,
	nullptr
};

MainLoop *test() {
	OS::get_singleton()->print("\n== TestLabel ==\n");
	int count = 0;
	int passed = 0;

	while (true) {
		if (!test_funcs[count]) {
			break;
		}
		bool pass = test_funcs[count]();
		if (pass) {
			passed++;
		}
		OS::get_singleton()->print("\t%s\n", pass ? "PASS" : "FAILED");
		count++;
	}

	OS::get_singleton()->print("\n*************\n");
	OS::get_singleton()->print("Passed %i of %i tests\n", passed, count);

	return nullptr;
}
} // namespace TestLabel

#undef CHECK
#undef CHECK_APPROX
