/*************************************************************************/
/*  spline.hpp                                                           */
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

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_C740D32DBADF41BEA4FF5B55C29796CE
#define UUID_C740D32DBADF41BEA4FF5B55C29796CE

#include "mat.hpp"
#include "vec.hpp"

namespace gml {

namespace detail {

template <typename T, int N>
vec<T, N> bezierImpl(
		const vec<T, N> *p, int n, T t1, T t2, int stride = 1) {
	if (n == 1) return *p;
	if (n == 2) return t1 * p[0] + t2 * p[stride];
	return t1 * bezierImpl(p, n - 1, t1, t2, stride) +
		   t2 * bezierImpl(p + stride, n - 1, t1, t2, stride);
}

} // namespace detail

/// Evaluates a bezier curve with D control points at t.
/// @tparam D The number of control points.
/// @param p Control points.
/// @param t Interpolation value [0-1].
template <int D, typename T, int N>
vec<T, N> bezier(const vec<T, N> (&p)[D], T t) {
	static_assert(D > 0, "At least one control point needed.");

	return detail::bezierImpl(&p[0], D, static_cast<T>(1) - t, t);
}

/// Evaluates a bezier surface with D0 X D1 control points at t.
/// @tparam D0 The number of control points along t[0] direction.
/// @tparam D1 The number of control points along t[1] direction.
/// @param p Control points.
/// @param t Interpolation value [0-1].
template <int D0, int D1, typename T, int N>
vec<T, N> bezier2(
		const vec<T, N> (&p)[D1][D0], const vec<T, 2> &t) {
	static_assert(D0 > 0, "At least one control point needed.");
	static_assert(D1 > 0, "At least one control point needed.");

	vec<T, N> temp[D1];
	for (int i = 0; i < D1; ++i) {
		temp[i] = bezier(p[i], t[0]);
	}
	return bezier(temp, t[1]);
}

namespace detail {

template <int O, int D, typename T, int N>
struct bezierDerivativeImpl {
	static vec<T, N> calc(const vec<T, N> (&p)[D], T t) {
		vec<T, N> temp[D - 1];
		for (int i = 0; i < D - 1; ++i) {
			temp[i] = static_cast<T>(D - 1) * (p[i + 1] - p[i]);
		}

		return bezierDerivativeImpl<O - 1, D - 1, T, N>::calc(temp, t);
	}
};

template <int D, typename T, int N>
struct bezierDerivativeImpl<0, D, T, N> {
	static vec<T, N> calc(const vec<T, N> (&p)[D], T t) {
		return bezier(p, t);
	}
};

template <typename T, int N>
struct bezierDerivativeImpl<0, 1, T, N> {
	static vec<T, N> calc(const vec<T, N> (&p)[1], T t) {
		return bezier(p, t);
	}
};

template <int O, typename T, int N>
struct bezierDerivativeImpl<O, 1, T, N> {
	static vec<T, N> calc(const vec<T, N> (&)[1], T) {
		return vec<T, N>{ static_cast<T>(0) };
	}
};

} // namespace detail

/// Evaluates Oth order derivative of a bezier curve with D control points at t.
/// In the case of O=1 the derivative is tangential to the curve at t.
/// @tparam O The order of the derivative. 1 for the first order.
/// @tparam D The number of control points.
/// @param p Control points.
/// @param t Interpolation value [0-1].
template <int O, int D, typename T, int N>
vec<T, N> bezierDerivative(const vec<T, N> (&p)[D], T t) {
	static_assert(O > 0, "The derivative order must be at least one.");
	static_assert(D > 0, "At least one control point needed.");

	return detail::bezierDerivativeImpl<O, D, T, N>::calc(p, t);
}

/// Evaluates the Oth order Jacobian matrix of a bezier surface at point t.
/// In the case of O=1 the two column vectors are tangential to the surface at t
/// and in the case of N=3 the cross product of the columns is perpendicular to
/// the surface.
/// @tparam O The order of the Jacobian matrix. 1 = first order.
/// @tparam D0 The number of control points along the t[0] direction.
/// @tparam D1 The number of control points along the t[1] direction.
/// @param p Control points.
/// @param t Interpolation value [0-1].
template <int O, int D0, int D1, typename T, int N>
mat<T, 2, N> bezier2Jacobian(
		const vec<T, N> (&p)[D1][D0], const vec<T, 2u> &t) {
	static_assert(O > 0, "Order of the jacobian must be at least one.");
	static_assert(D0 > 0, "At least one control point needed.");
	static_assert(D1 > 0, "At least one control point needed.");

	vec<T, N> temp0[D0];
	for (int i = 0; i < D0; ++i) {
		temp0[i] = detail::bezierImpl(&p[0][i], D1, static_cast<T>(1) - t[1], t[1], D0);
	}

	vec<T, N> temp1[D1];
	for (int i = 0; i < D1; ++i) {
		temp1[i] = bezier(p[i], t[0]);
	}

	return mat<T, 2, N>{
		bezierDerivative<O>(temp0, t[0]),
		bezierDerivative<O>(temp1, t[1])
	};
}

} // namespace gml

#endif
