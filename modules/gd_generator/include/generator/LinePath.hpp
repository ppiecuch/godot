/*************************************************************************/
/*  LinePath.hpp                                                         */
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

#ifndef GENERATOR_LINEPATH_HPP
#define GENERATOR_LINEPATH_HPP

#include "ParametricPath.hpp"

namespace generator {

/// A path from point to point.
/// @image html LinePath.svg
class LinePath {
private:
	using Impl = ParametricPath;
	Impl parametricPath_;

public:
	/// @param start Start point of the line.
	/// @param end End point of the line.
	/// @param normal Line normal. Should be parallel to the line.
	/// @param segments Number of subdivisions along the line.
	LinePath(
			const gml::dvec3 &start = { 0.0, 0.0, -1.0 },
			const gml::dvec3 &end = { 0.0, 0.0, 1.0 },
			const gml::dvec3 &normal = { 1.0, 0.0, 0.0 },
			int segments = 8);

	using Edges = typename Impl::Edges;

	Edges edges() const noexcept { return parametricPath_.edges(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return parametricPath_.vertices(); }
};

} // namespace generator

#endif