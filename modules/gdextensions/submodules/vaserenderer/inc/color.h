/*************************************************************************/
/*  color.h                                                              */
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

float &Color_get(Color &C, int index) {
	switch (index) {
		case 0:
			return C.r;
		case 1:
			return C.g;
		case 2:
			return C.b;
		default:
			return C.r;
	}
}
bool Color_valid_range(float t) {
	return t >= 0 && t <= 1;
}
Color color_with_alpha(const Color &c, float alpha) {
	return Color{ c.r, c.g, c.b, alpha };
}
Color Color_between(const Color &A, const Color &B, float t = 0.5) {
	if (t < 0.0) {
		t = 0.0;
	}
	if (t > 1.0) {
		t = 1.0;
	}
	float kt = 1.0 - t;
	Color C = {
		A.r * kt + B.r * t,
		A.g * kt + B.g * t,
		A.b * kt + B.b * t,
		A.a * kt + B.a * t
	};
	return C;
}
void sRGBtolinear(Color &C, bool exact = false) { //de-Gamma 2.2
	// from: http://www.xsi-blog.com/archives/133
	if (exact) {
		for (int i = 0; i < 3; i++) {
			float &cc = Color_get(C, i);

			if (cc > 0.04045)
				cc = Math::pow((cc + 0.055) / 1.055, 2.4);
			else
				cc /= 12.92;
		}
	} else { // approximate
		constexpr real_t degamma = 2.2;
		for (int i = 0; i < 3; i++) {
			float &cc = Color_get(C, i);
			cc = Math::pow(cc, degamma);
		}
	}
}
void lineartosRGB(Color &C, bool exact = false) { // Gamma 2.2
	if (exact) {
		constexpr real_t gamma = 1 / 2.4;
		for (int i = 0; i < 3; i++) {
			float &cc = Color_get(C, i);
			if (cc > 0.0031308)
				cc = 1.055 * Math::pow(cc, gamma) - 0.055;
			else
				cc *= 12.92;
		}
	} else { // approximate
		constexpr real_t gamma = 1 / 2.2;
		for (int i = 0; i < 3; i++) {
			float &cc = Color_get(C, i);
			cc = Math::pow(cc, gamma);
		}
	}
}
float color_max(float r, float g, float b) {
	return r > g ? (g > b ? r : (r > b ? r : b)) : (g > b ? g : b);
}
float color_min(float r, float g, float b) {
	return -color_max(-r, -g, -b);
}
void RGBtoHSV(float r, float g, float b, float *h, float *s, float *v) { // from: http://www.cs.rit.edu/~ncs/color/t_convert.html
	// r,g,b values are from 0 to 1
	// h = [0,360], s = [0,1], v = [0,1]
	// if s == 0, then h = -1 (undefined)
	float min, max, delta;
	min = color_min(r, g, b);
	max = color_max(r, g, b);
	*v = max; // v
	delta = max - min;
	if (max != 0)
		*s = delta / max; // s
	else {
		// r = g = b = 0 : s = 0, v is undefined
		*s = 0;
		*h = -1;
		return;
	}
	if (r == max)
		*h = (g - b) / delta; // between yellow & magenta
	else if (g == max)
		*h = 2 + (b - r) / delta; // between cyan & yellow
	else
		*h = 4 + (r - g) / delta; // between magenta & cyan
	*h *= 60; // degrees
	if (*h < 0) {
		*h += 360;
	}
}
void HSVtoRGB(float *r, float *g, float *b, float h, float s, float v) {
	int i;
	float f, p, q, t;
	if (s == 0) {
		// achromatic (grey)
		*r = *g = *b = v;
		return;
	}
	h /= 60; // sector 0 to 5
	i = Math::floor(h);
	f = h - i; // factorial part of h
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));
	switch (i) {
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2:
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default: // case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}
