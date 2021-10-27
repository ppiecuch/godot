/*************************************************************************/
/*  test_random_number_generator.h                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "test_random_number_generator.h"

#include "core/math/random_number_generator.h"
#include "core/os/os.h"

namespace TestRandomNumberGenerator {

bool test_rng_on_unit_circle() {
	bool ok = true;
	Ref<RandomNumberGenerator> rng = memnew(RandomNumberGenerator);
	rng->set_seed(0);
	for (int i = 0; i < 100; ++i) {
		const Vector2 &point = rng->randv_circle();
		ok = ok && Math::is_equal_approx(point.length(), real_t(1.0));
		ok = ok && point.is_normalized();
	}
	return ok;
}

bool stress_rng_on_unit_circle() {
	Ref<RandomNumberGenerator> rng = memnew(RandomNumberGenerator);
	rng->set_seed(0);
	real_t start = OS::get_singleton()->get_system_time_secs();
	printf("randv_circle stress test (100000)\n");
	for (int i = 0; i < 100000; ++i) {
		rng->randv_circle();
	}
	real_t stop = OS::get_singleton()->get_system_time_secs();
	printf("  .. %0.f/call\n", stop - start);
	return true;
}

typedef bool (*TestFunc)();

TestFunc test_funcs[] = {
	test_rng_on_unit_circle,
	stress_rng_on_unit_circle,
	nullptr
};

MainLoop *test() {
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
	OS::get_singleton()->print("\n");
	OS::get_singleton()->print("Passed %i of %i tests\n", passed, count);
	return nullptr;
}


} // namespace TestRandomNumberGenerator
