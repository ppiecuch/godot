/*************************************************************************/
/*  numberlist.h                                                         */
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

// Copyright (C) 2013  Nicholas Gill

#ifndef PARSERS_NUMBERLIST_H_
#define PARSERS_NUMBERLIST_H_

#include "basic.h"
#include <array>
#include <stdexcept>

namespace svg {
namespace types {
namespace parsers {

namespace detail {

inline bool throw_if(bool cond, const std::string &what) {
	if (cond)
		throw std::runtime_error(what);
	return cond;
}

} //namespace detail

template <size_t N>
std::array<float, N> parse_numberlist(const char *c, const char *const end) {
	using namespace detail;
	std::array<float, N> list;
	for (size_t i = 0; i < N; ++i) {
		throw_if(!parse_number(c, end, list[i]), "expected number");
		if (i != N - 1) {
			throw_if(c == end, "unexpected eof");
			throw_if(!parse_comma_wsp(c, end), "expected comma-wsp");
			throw_if(c == end, "unexpected eof");
		}
	}
	return list;
}

} //namespace parsers
} //namespace types
} //namespace svg

#endif /* PARSERS_NUMBERLIST_H_ */
