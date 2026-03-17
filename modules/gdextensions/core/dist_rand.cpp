/**************************************************************************/
/*  dist_rand.cpp                                                         */
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

//
// distrand
//
// Copyright © 2017 M T Harry Ayres
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GNU Terry Pratchett
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "dist_rand.h"

template <typename T, class C>
real_t BaseNormal<T, C>::dr_boxmuller(const T mu, const T sigma) { // mu = mean, sigma = standard deviation
	real_t epsilon = 0; // Hoping this will sort out any rounding error
	static real_t Zu, Zv; // The two deviates
	real_t U, V, s, R, Q; // Two random numbers (U & V), the sum of their cubes (s), the root of that (R) and Q which is the square root of the natural log of s multiplied by negative two.
	static bool generate, negative; // These two bools control function behaviour.

	int control = (rand());
	negative = ((control & 1 << 0) == 1); // See if our new random int is odd
	// Seems biased.

	generate = !generate; // Generate two at a time, so only do it every other time.

	if (!generate) { // So if we're not generating a number this time
		if (negative) { // and we want a negative (half the time)
			return real_t(mu) - Zv * real_t(sigma);
		} else { // or a positive (half the time)
			return real_t(mu) + Zv * real_t(sigma);
		}
	}

	do { // Generate the random values
		U = Math::randf(); // Godot idiom.
		V = Math::randf();
		s = (U * U + V * V); // This one is calculated here because it controls the loop
	} while (s <= epsilon || s >= 1);

	// These two are calculated outside the loop to save a miniscule bit of runtime
	R = sqrt(s);
	Q = sqrt(-2 * log(s));

	// Calculate both variates. Common terms became variables to, again, save a miniscule bit of runtime
	// Cast to typename T
	Zu = (U / R) * Q;
	Zv = (V / R) * Q;

	if (negative) { // If we want a negative value, return here
		return real_t(mu) - Zu * real_t(sigma);
	}

	return real_t(mu) + Zu * real_t(sigma); // Otherwise return here
}

template <typename T, class C>
void BaseNormal<T, C>::setparameters(T mean, T deviation) {
	if (deviation > 0) {
		mu = mean;
		sigma = deviation;
		contents.resize(0);
		bookmark = 0;
	}
}

template <typename T, class C>
void BaseNormal<T, C>::generate(int quantity) {
	if (sigma > 0) {
		contents.resize(0);
		for (int i = 0; i < quantity; ++i) {
			contents.push_back(dr_boxmuller(mu, sigma));
		}
		bookmark = 0;
	}
}

template <typename T, class C>
T BaseNormal<T, C>::getvalue(int i) {
	if (i < contents.size()) {
		bookmark = i;
		return contents[bookmark];
	}

	ERR_FAIL_COND_V(i >= contents.size(), 0.0);

	return 0.0;
}

template <typename T, class C>
T BaseNormal<T, C>::getnext() {
	int i = bookmark + 1;

	if (i < contents.size()) {
		bookmark = i;
		return contents[bookmark];
	}

	ERR_FAIL_COND_V("Out of bounds", 0.0);

	return 0.0;
}

template <typename T, class C>
T BaseNormal<T, C>::getsingle() {
	return dr_boxmuller(mu, sigma);
}

template <typename T, class C>
BaseNormal<T, C>::BaseNormal() {
	Math::randomize();
	bookmark = 0;
	mu = 0;
	sigma = 1;
}

// Explicitly create variates to avoid "undefined reference" errors.
template class BaseNormal<real_t, PoolRealArray>;
template class BaseNormal<int, PoolIntArray>;

int IntNormal::round(real_t n) {
	// The purpose of the algorithm is to accurately round floating-point numbers.
	// C++11 has a builtin for this but I'm using C++98 here.
	// Would use C++11 but it doesn't play well with Java via Scons, for me.

	if (n < 0) // If n is negative then flip the sign, recurse, then flip again and return.
		return -round(-n);
	// Implicit else

	real_t t = n - floor(n);
	t *= 10;
	if (t > 4 && t < 5) { // Then we need to look at the next digit!
		t = round(t); // Round the current value of t
	}
	// Implicit else
	if (t >= 5)
		return (int)(floor(n) + 1);
	// Implicit else
	return (int)floor(n);
}

void IntNormal::generate(int count) {
	if (sigma > 0) {
		contents.resize(0);
		for (int i = 0; i < count; ++i) {
			contents.push_back(round(dr_boxmuller(mu, sigma))); // Rounding the boxmuller result.
		}
		bookmark = 0;
	}
}

void IntNormal::_bind_methods() {
	ClassDB::bind_method("normal", &IntNormal::setparameters);
	ClassDB::bind_method("generate", &IntNormal::generate);
	ClassDB::bind_method("getvalue", &IntNormal::getvalue);
	ClassDB::bind_method("getnext", &IntNormal::getnext);
	ClassDB::bind_method("getsingle", &IntNormal::getsingle);
}

void RealNormal::_bind_methods() {
	ClassDB::bind_method("normal", &RealNormal::setparameters);
	ClassDB::bind_method("generate", &RealNormal::generate);
	ClassDB::bind_method("getvalue", &RealNormal::getvalue);
	ClassDB::bind_method("getnext", &RealNormal::getnext);
	ClassDB::bind_method("getsingle", &RealNormal::getsingle);
}
