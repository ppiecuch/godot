/*************************************************************************/
/*  TTFTriangulator2D.cpp                                                */
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

#include <cstdint>
#include <exception>
#include <vector>

#include "TTFExceptions.h"
#include "TTFMath.h"
#include "TTFTriangulator2D.h"
#include "TTFTypes.h"

using namespace TTFCore;

// ---------------------------------------------------------------------------------------------------------------------------
//	TriSmall
// ---------------------------------------------------------------------------------------------------------------------------

TriSmall::TriSmall(size_t i0_, size_t i1_, size_t i2_, int32_t coef_) {
	i0 = static_cast<uint32_t>(i0_);
	i1 = static_cast<uint32_t>(i1_);
	i2 = static_cast<uint32_t>(i2_);
	coef = coef_;
}

TriLarge::TriLarge(size_t i0_, size_t i1_, size_t i2_, int16_t coef_) {
	i0 = static_cast<uint16_t>(i0_);
	i1 = static_cast<uint16_t>(i1_);
	i2 = static_cast<uint16_t>(i2_);
	coef = coef_;
}
