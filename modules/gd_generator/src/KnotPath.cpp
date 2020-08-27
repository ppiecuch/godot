/*************************************************************************/
/*  KnotPath.cpp                                                         */
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

// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include "generator/KnotPath.hpp"

using namespace generator;

namespace {

gml::dvec3 knot(int p, int q, double t) {
	t *= gml::radians(360.0);

	const double pt = p * t;
	const double qt = q * t;

	const double sinpt = std::sin(pt);
	const double cospt = std::cos(pt);
	const double sinqt = std::sin(qt);
	const double cosqt = std::cos(qt);

	const double r = 0.5 * (2.0 + sinqt);

	return gml::dvec3{ r * cospt, r * sinpt, r * cosqt };
}

} // namespace

KnotPath::KnotPath(
		int p,
		int q,
		int segments) :
		parametricPath_{
			[p, q](double t) {
				PathVertex vertex;

				vertex.position = knot(p, q, t);

				const gml::dvec3 prev = knot(p, q, t - 0.01);
				const gml::dvec3 next = knot(p, q, t + 0.01);

				vertex.tangent = normalize(next - prev);

				vertex.normal = normalize(cross(next - prev, next + prev));

				vertex.texCoord = t;

				return vertex;
			},
			segments
		} {}
