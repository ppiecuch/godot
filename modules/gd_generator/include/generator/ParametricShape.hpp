/*************************************************************************/
/*  ParametricShape.hpp                                                  */
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

#ifndef GENERATOR_PARAMETRICSHAPE_HPP
#define GENERATOR_PARAMETRICSHAPE_HPP

#include <functional>

#include "Edge.hpp"
#include "ShapeVertex.hpp"

namespace generator {

/// A shape with values evaluated using a callback function.
class ParametricShape {
public:
	class Edges {
	public:
		Edge generate() const;
		bool done() const noexcept;
		void next();

	private:
		Edges(const ParametricShape &shape);

		const ParametricShape *shape_;

		int i_;

		friend class ParametricShape;
	};

	class Vertices {
	public:
		ShapeVertex generate() const;
		bool done() const noexcept;
		void next();

	private:
		Vertices(const ParametricShape &shape);

		const ParametricShape *shape_;

		int i_;

		friend class ParametricShape;
	};

	/// @param eval A callback that returns a ShapeVertex for a given value.
	/// @param segments The number of segments along the shape.
	/// Should be >= 1. Zero yields an empty shape.
	explicit ParametricShape(
			std::function<ShapeVertex(double)> eval,
			int segments = 16) noexcept;

	Edges edges() const noexcept;

	Vertices vertices() const noexcept;

private:
	std::function<ShapeVertex(double)> eval_;

	int segments_;

	double delta_;
};

} // namespace generator

#endif