/*************************************************************************/
/*  RotateShape.hpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef GENERATOR_ROTATESHAPE_HPP
#define GENERATOR_ROTATESHAPE_HPP

#include "TransformShape.hpp"

namespace generator {

/// Rotates a shape around the origin on the xy-plane.
template <typename Shape>
class RotateShape {
private:
	using Impl = TransformShape<Shape>;
	Impl transformShape_;

public:
	/// @param shape Source data shape.
	/// @param angle Counterclockwise angle.
	RotateShape(Shape shape, double angle) :
			transformShape_{
				std::move(shape),
				[angle](ShapeVertex &value) {
					auto rotation = gml::rotate(angle);
					value.position = gml::transform(rotation, value.position);
					value.tangent = gml::transform(rotation, value.tangent);
				}
			} {}

	using Edges = typename Impl::Edges;

	Edges edges() const noexcept { return transformShape_.edges(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformShape_.vertices(); }
};

template <typename Shape>
RotateShape<Shape> rotateShape(Shape shape, double angle) {
	return RotateShape<Shape>{ std::move(shape), angle };
}

} // namespace generator

#endif
