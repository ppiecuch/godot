/*************************************************************************/
/*  utils.hpp                                                            */
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

#ifndef GENERATOR_UTILS_HPP
#define GENERATOR_UTILS_HPP

namespace generator {

/// Will have a type named "Type" that has same type as value returned by method
/// generate() of type Generator.
template <typename Generator>
class GeneratedType {
public:
	using Type = decltype(static_cast<const Generator *>(nullptr)->generate());
};

/// Will have a type named "Type" that has same type as value returned by method
/// edges() for type Primitive.
template <typename Primitive>
class EdgeGeneratorType {
public:
	using Type = decltype(static_cast<const Primitive *>(nullptr)->edges());
};

/// Will have a type named "Type" that has same type as value returned by method
/// triangles() for type Primitive.
template <typename Primitive>
class TriangleGeneratorType {
public:
	using Type = decltype(static_cast<const Primitive *>(nullptr)->triangles());
};

/// Will have a type named "Type" that has same type as value returned by method
/// vertices() for type Primitive.
template <typename Primitive>
class VertexGeneratorType {
public:
	using Type = decltype(static_cast<const Primitive *>(nullptr)->vertices());
};

/// Counts the number of steps left in the generator.
template <typename Generator>
int count(const Generator &generator) noexcept {
	Generator temp{ generator };
	int c = 0;
	while (!temp.done()) {
		++c;
		temp.next();
	}
	return c;
}

} // namespace generator

#endif
