/*************************************************************************/
/*  intersect.hpp                                                        */
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

#ifndef UUID_72960EDDD32341DBBCADBFA777E98A81
#define UUID_72960EDDD32341DBBCADBFA777E98A81

#include <tuple>

#include "mat.hpp"
#include "vec.hpp"

namespace gml {

/// Compute a pickray for mouse coordinates.
/// @param mpos The mouse coordinates relative to the BOTTOM left corner of the screen.
/// @param view The view matrix
/// @param proj The projection matrix
/// @param viewportOrigin Bottom left corner of the viewport
/// @param viewportSize Size of the viewport
/// @return Ray origin and unit length ray direction vector.
template <typename T, typename TI, typename TS>
std::tuple<vec<T, 3>, vec<T, 3> > pickRay(
		const vec<TI, 2> &mpos,
		const mat<T, 4, 4> &view, const mat<T, 4, 4> &proj,
		const vec<TI, 2> &viewportOrigin, const vec<TS, 2> &viewportSize) {
	return pickRay(mpos, inverse(proj * view), viewportOrigin, viewportSize);
}

/// Compute a pickray for mouse coordinates.
/// @param mpos The mouse coordinates relative to the BOTTOM left corner of the screen.
/// @param invProjView Inverse view projection matrix: (proj*view)^-1
/// @param viewportOrigin Bottom left corner of the viewport
/// @param viewportSize Size of the viewport
/// @return Ray origin and unit length ray direction vector.
template <typename T, typename TI, typename TS>
std::tuple<vec<T, 3>, vec<T, 3> > pickRay(
		const vec<TI, 2> &mpos,
		const mat<T, 4, 4> &invProjView,
		const vec<TI, 2> &viewportOrigin, const vec<TS, 2> &viewportSize) {
	const T zero = static_cast<T>(0);
	const T one = static_cast<T>(1);
	const T half = static_cast<T>(0.5);

	const vec<T, 2> fmpos{
		static_cast<T>(mpos[0]) + half, static_cast<T>(mpos[1]) + half
	};

	const vec<T, 3> frontPos = unProject(
			vec<T, 3>{ fmpos, zero }, invProjView, viewportOrigin, viewportSize);

	const vec<T, 3> backPos = unProject(
			vec<T, 3>{ fmpos, one }, invProjView, viewportOrigin, viewportSize);

	return std::make_tuple(frontPos, normalize(backPos - frontPos));
}

/// Computes the intersection of a ray and a N-plane.
/// A plane if N == 3, a line if N == 2.
/// @param origin Origin of the ray
/// @param direction Unit length direction of the ray.
/// @param center Any point on the plane
/// @param normal Unit length normal vector of the plane
/// @return Returns a tuple consisting of a bool denoting whether there was an
/// intersection (true) or not (false, when the ray is parallel to the plane)
/// and a scalar denoting the distance to the intersection point (positive or
/// negative depending if the point is in front of or behind the origin).
template <typename T, int N>
std::tuple<bool, T> intersectRayPlane(
		const vec<T, N> &origin, const vec<T, N> &direction,
		const vec<T, N> &center, const vec<T, N> &normal) {
	using std::abs;

	const T divisor = dot(direction, normal);

	if (abs(divisor) < std::numeric_limits<T>::epsilon()) {
		return std::make_tuple(false, static_cast<T>(0));
	}

	const T d = dot(center, normal);

	return std::make_tuple(true, (d - dot(origin, normal)) / divisor);
}

/// Computes the intersection of a ray and N-sphere.
/// A sphere if N == 3, circle if N == 2.
/// @param origin Origin of the ray
/// @param direction Unit length direction of the ray.
/// @param center Position of the center of the sphere.
/// @param radius Radius of the sphere.
/// @return Returns a tuple consisting of a bool denoting whether there was an
/// intersection (true) or not (false) and two scalars denoting the distances to
/// the front and back intersection points (positive or negative depending if
/// the point is in front of or behind the origin).
template <typename T, int N>
std::tuple<bool, T, T> intersectRaySphere(
		const vec<T, N> &origin, const vec<T, N> &direction,
		const vec<T, N> &center, T radius) {
	using std::sqrt;

	const vec<T, N> L = center - origin;
	const T tca = dot(L, direction);
	const T d2 = dot(L, L) - tca * tca;

	const T radius2 = radius * radius;
	if (d2 > radius2) {
		return std::make_tuple(false, static_cast<T>(0), static_cast<T>(0));
	}

	const T thc = sqrt(radius2 - d2);

	return std::make_tuple(true, tca - thc, tca + thc);
}

} // namespace gml

#endif
