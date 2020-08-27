/*************************************************************************/
/*  AnyGenerator.hpp                                                     */
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

#ifndef GENERATOR_ANYGENERATOR_HPP
#define GENERATOR_ANYGENERATOR_HPP

#include <memory>

namespace generator {

/// A type erasing container that can store any generator that generates type T.
/// @tparam T Type returned by the generate() -function.
template <typename T>
class AnyGenerator {
public:
	template <typename Generator>
	AnyGenerator(Generator generator) :
			base_{ new Derived<Generator>{ std::move(generator) } } {}

	AnyGenerator(const AnyGenerator &that) :
			base_{ that.base_->clone() } {}

	AnyGenerator(AnyGenerator &&) = default;

	AnyGenerator &operator=(const AnyGenerator &that) {
		base_ = that.base_->clone();
		return *this;
	}

	AnyGenerator &operator=(AnyGenerator &&) = default;

	T generate() const { return base_->generate(); }

	bool done() const noexcept { return base_->done(); }

	void next() { base_->next(); }

private:
	class Base {
	public:
		virtual ~Base() {}
		virtual std::unique_ptr<Base> clone() const = 0;
		virtual T generate() const = 0;
		virtual bool done() const noexcept = 0;
		virtual void next() = 0;
	};

	template <typename Generator>
	class Derived : public Base {
	public:
		Derived(Generator generator) :
				generator_{ std::move(generator) } {}

		virtual std::unique_ptr<Base> clone() const override {
			return std::unique_ptr<Base>(new Derived{ generator_ });
		}

		virtual T generate() const override {
			return generator_.generate();
		}

		virtual bool done() const noexcept override {
			return generator_.done();
		}

		virtual void next() override {
			generator_.next();
		}

	private:
		Generator generator_;
	};

	std::unique_ptr<Base> base_;
};

} // namespace generator

#endif
