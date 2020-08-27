/*************************************************************************/
/*  AnyShape.hpp                                                         */
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

#ifndef GENERATOR_ANYSHAPE_HPP
#define GENERATOR_ANYSHAPE_HPP

#include <memory>

#include "AnyGenerator.hpp"
#include "Edge.hpp"
#include "ShapeVertex.hpp"

namespace generator {

/// A type erasing container that can store any shape.
class AnyShape {
public:
	template <typename Shape>
	AnyShape(Shape shape) :
			base_{ new Derived<Shape>{ std::move(shape) } } {}

	AnyShape(const AnyShape &that);

	AnyShape(AnyShape &&) = default;

	AnyShape &operator=(const AnyShape &that);

	AnyShape &operator=(AnyShape &&) = default;

	AnyGenerator<Edge> edges() const noexcept;

	AnyGenerator<ShapeVertex> vertices() const noexcept;

private:
	class Base {
	public:
		virtual ~Base();
		virtual std::unique_ptr<Base> clone() const = 0;
		virtual AnyGenerator<Edge> edges() const = 0;
		virtual AnyGenerator<ShapeVertex> vertices() const = 0;
	};

	template <typename Shape>
	class Derived : public Base {
	public:
		Derived(Shape shape) :
				shape_(std::move(shape)) {}

		virtual std::unique_ptr<Base> clone() const override {
			return std::unique_ptr<Base>{ new Derived{ shape_ } };
		}

		virtual AnyGenerator<Edge> edges() const override {
			return shape_.edges();
		}

		virtual AnyGenerator<ShapeVertex> vertices() const override {
			return shape_.vertices();
		}

		Shape shape_;
	};

	std::unique_ptr<Base> base_;
};

} // namespace generator

#endif
