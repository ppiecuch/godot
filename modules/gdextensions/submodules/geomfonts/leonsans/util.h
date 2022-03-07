/*************************************************************************/
/*  util.h                                                               */
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

#include "core/dictionary.h"
#include "core/math/math_funcs.h"

#include <map>
#include <vector>

constexpr real_t ROTATE_HORIZONTAL = 180 * (Math_PI / 180);
constexpr real_t ROTATE_VERTICAL = 90 * (Math_PI / 180);
constexpr real_t ROTATE_NONE = -100;
constexpr real_t VERTEX_GAP = 3;
constexpr real_t VERTEX_GAP2 = VERTEX_GAP / 2;
constexpr real_t FONT_HEIGHT = 824;
const char *TOFU = "tofu";

union InfoValue {
	real_t f;
	int i;
	operator float() const { return f; }
	operator int() const { return i; }
	InfoValue(double v) :
			f(v) {}
	InfoValue(float v) :
			f(v) {}
	InfoValue(int v) :
			i(v) {}
};

struct FontPathSeg {
	char op;
	union {
		struct {
			real_t _1, _2, _3, _4, _5, _6;
		};
		real_t d[6];
	};
	std::map<char, InfoValue> info;
	real_t &operator[](int index) { return d[index]; }

	FontPathSeg(char op, real_t p1, real_t p2, const std::map<char, InfoValue> &info) :
			op(op), _1(p1), _2(p2), info(info) {}
	FontPathSeg(char op, real_t p1, real_t p2, real_t p3, real_t p4, real_t p5, real_t p6, const std::map<char, InfoValue> &info) :
			op(op), _1(p1), _2(p2), _3(p3), _4(p4), _5(p5), _6(p6), info(info) {}
	FontPathSeg(char op, real_t p1, real_t p2, real_t p3, real_t p4, real_t p5, real_t p6) :
			op(op), _1(p1), _2(p2), _3(p3), _4(p4), _5(p5), _6(p6) {}
};

struct FontPath {
	int d;
	std::vector<FontPathSeg> v;
};

struct FontData {
	struct {
		real_t w, h, fw, fh;
	} rect;
	struct {
		real_t x1, x2, y1, y2;
	} ratio;
	std::vector<FontPath> p;
};

static std::vector<FontPathSeg> setCenter(const std::vector<FontPathSeg> &arr, int fw, int fh) {
	const int cx = fw / 2;
	const int cy = fh / 2;
	std::vector<FontPathSeg> ct;

	for (int i = 0; i < arr.size(); i++) {
		auto mp = arr[i];
		mp[1] -= cx;
		mp[2] -= cy;
		if (mp[0] == 'b') {
			mp[3] -= cx;
			mp[4] -= cy;
			mp[5] -= cx;
			mp[6] -= cy;
		}
		ct.push_back(mp);
	}
	return ct;
}

static real_t getR(real_t x1, real_t y1, real_t x2, real_t y2) {
	const real_t x = x1 - x2;
	const real_t y = y1 - y2;
	return -Math::atan2(x, y);
}

// http://qaru.site/questions/10657973/quadratic-curve-with-rope-pattern
// https://stackoverflow.com/questions/32322966/quadratic-curve-with-rope-pattern
static real_t bezierTangent(real_t a, real_t b, real_t c, real_t d, real_t t) {
	return (3 * t * t * (-a + 3 * b - 3 * c + d) + 6 * t * (a - 2 * b + c) + 3 * (-a + b));
}

static real_t getCurveR(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3, real_t x4, real_t y4, real_t t) {
	const real_t x = bezierTangent(x1, x2, x3, x4, t);
	const real_t y = bezierTangent(y1, y2, y3, y4, t);
	return -Math::atan2(x, y);
}

FontData generateFontData(real_t w, real_t fw, real_t fh, real_t x1, real_t x2, real_t y1, real_t y2, std::vector<FontPath> path) {
	std::vector<FontPath> arr;
	for (int i = 0; i < path.size(); i++) {
		arr.push_back({ path[i].d, setCenter(path[i].v, fw, fh) });
	}

	return {
		{ w, FONT_HEIGHT, fw, fh }, // rect
		{ x1, x2, y1, y2 }, // ratio
		arr // p
	};
}
