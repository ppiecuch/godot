/*************************************************************************/
/*  dist_rand.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef BASENORMAL_H
#define BASENORMAL_H

#ifdef __ANDROID__
#include <stdlib.h>
#else
#include <cstdlib>
#endif
#include "core/math/math_funcs.h" // Godot mathematical functions, including randf.
#include "core/reference.h"

template <typename T, class C>
class BaseNormal : public Reference {
	GDCLASS(BaseNormal, Reference);

protected:
	int bookmark;
	T mu, sigma;
	C contents;

	real_t dr_boxmuller(const T mu, const T sigma);

public:
	void generate(int);

	void setparameters(T mean, T deviation);

	T getvalue(int);
	T getnext();
	T getsingle();

	BaseNormal();
};

class IntNormal : public BaseNormal<int, PoolIntArray> { // Inherits Reference from base class
	GDCLASS(IntNormal, Reference);

	using BaseNormal<int, PoolIntArray>::dr_boxmuller;
	int round(float);

protected:
	static void _bind_methods();

public:
	// 	In order to work in gdscript it seems that these must be explicitly written.
	void generate(int count); // IntNormal needs to round the boxmuller result.
	void setparameters(int mean, int deviation) { BaseNormal<int, PoolIntArray>::setparameters(mean, deviation); };
	int getvalue(int i) { return BaseNormal<int, PoolIntArray>::getvalue(i); };
	int getnext() { return BaseNormal<int, PoolIntArray>::getnext(); };
	int getsingle() { return BaseNormal<int, PoolIntArray>::getsingle(); };

	IntNormal() :
			BaseNormal(){};
};

class RealNormal : public BaseNormal<real_t, PoolRealArray> { // Inherits Reference from base class
	GDCLASS(RealNormal, Reference);

	using BaseNormal<real_t, PoolRealArray>::dr_boxmuller;

protected:
	static void _bind_methods();

public:
	// 	In order to work in gdscript it seems that these must be explicitly written.
	void setparameters(real_t mean, real_t deviation) { BaseNormal<real_t, PoolRealArray>::setparameters(mean, deviation); };
	void generate(int count) { BaseNormal<real_t, PoolRealArray>::generate(count); };
	real_t getvalue(int i) { return BaseNormal<real_t, PoolRealArray>::getvalue(i); };
	real_t getnext() { return BaseNormal<real_t, PoolRealArray>::getnext(); };
	real_t getsingle() { return BaseNormal<real_t, PoolRealArray>::getsingle(); };

	RealNormal() :
			BaseNormal(){};
};

#endif

#if 0
**A Random Number Generation modules for users of [Godot Engine](http://godotengine.org/).**

## Purpose

The goal of the project is to provide a convenient and efficient tool for
generating random numbers with non-uniform distribution, which may then be used
in games either through GDScript or by other modules. As a C++ modules it offers
execution advantages over scripted solutions.

## Using distrand

### Types

Currently distrand exposes two object classes, RealNormal and IntNormal. Both
create normal (or Gaussian) distributions of random numbers.

**RealNormal** generates random value floats and holds them in a RealArray.

**IntNormal** generates random value integers and holds them in an IntArray.

#### Public Methods ####

* setparameters
* generate
* getvalue
* getnext
* getsingle

**setparameters( _mean_ , _deviation_ )**

> Takes two values, type-matched to the variant (ints for IntNormal, floats for
> RealNormal), and sets the parameters for the next set of numbers to be
> generated if those parameters are valid. Erases any set of values previously
> generated in the object instance.

**generate( _count_ )**

> Where _count_ is an integer. Generates _count_ random values according to the
> normal distribution defined by stored parameters (mean and deviation).

**getvalue( _i_ )**

> Where _i_ is an integer lesser than the _count_ last generated. Returns the
> value of the random number in position _i_ of the internal array. RealNormal
> returns a float while IntNormal returns an integer.

**getnext()**

> Starting at the first element, sequentially returns each value resultant from
> the previous generation. If getvalue has been called it continues from there.

**getsingle()**

> Generates and returns a new random number under the currently-defined curve.

#### GDScript Instantiation ####

RealNormal and IntNormal objects may be created like other objects in GDScript:

`var foo=RealNormal.new()`

To define and generate the distribution:

`foo.normal(5.0, 1.5)`

`foo.generate(50000)`

To access the data:

`n = foo.getvalue(42)`

`n = foo.getnext()`

## Future features

* Histogram-friendly data in Int* distributions, giving a count of each value.
* Heart-cut and dog-food subsets of normal distributions.
* Bi- and Multi-modal distributions.
* Weibull distributions.
#endif
