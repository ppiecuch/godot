/**************************************************************************/
/*  rockgen.cpp                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "imath.h"
#include "utils.h"

#include "core/print_string.h"
#include "core/variant.h"

struct Point {
	real_t alpha, beta, rayon;

	int operator==(Point &b);
	int operator>(Point &b);
	int operator<(Point &b);

	Point();
	Point(real_t a, real_t b, real_t r);
	Point(const Point &a);
};

Point Canonic(const Point &a) {
	Point b;
	b.alpha = Math::fmod(360 + a.alpha, 360);
	b.beta = Math::fmod(360 + a.beta, 360);
	b.rayon = a.rayon;
	return b;
}

Point::Point() {
	alpha = -1;
	beta = 0;
	rayon = 0;
}

Point::Point(real_t a, real_t b, real_t r) {
	alpha = a;
	beta = b;
	rayon = r;
}

Point::Point(const Point &a) {
	alpha = a.alpha;
	beta = a.beta;
	rayon = a.rayon;
}

int Point::operator==(Point &b) {
	return (alpha == b.alpha) && (beta == b.beta);
}

int Point::operator>(Point &b) {
	return (alpha > b.alpha) || ((alpha == b.alpha) && (beta > b.beta));
}

int Point::operator<(Point &b) {
	return (alpha < b.alpha) || ((alpha == b.alpha) && (beta < b.beta));
}

// Global variables.
static int MaxProf;
static int mode = 0; // sharp or smoothed triangles ?

// Look-up table functions and variables.
static uint_t MaxPoint = 0; // sizeof table
static uint_t PointPtr = 0; // pointer to next entry past last
static uint_t dp; // prime displacement
static Point *PointTable = nullptr; // point table

static _FORCE_INLINE_ uint_t PrimaryHash(Point &a) {
	const uint_t p1 = *((uint_t *)&a.alpha);
	const uint_t p2 = *((uint_t *)&a.beta);
	return (p1 ^ (p2 >> 10)) % MaxPoint;
}

static Point Empty(-1, 0, 0);

uint_t PosTable(Point &a) {
	uint_t i = PrimaryHash(a);
	while ((!(PointTable[i] == Empty)) && (!(PointTable[i] == a))) {
		i = (i + dp) % MaxPoint;
	}
	return i;
}

// Checks if a point is in the look-up table
uint_t PointExiste(Point &a) {
	if (PointPtr) {
		Point b = Canonic(a);
		uint_t t = PosTable(b);
		return (PointTable[t] == b);
	} else {
		return 0;
	}
}

// Puts a point in the look-up table
void PutPoint(Point &a) {
	Point b = Canonic(a);
	uint_t t = PosTable(b);
	PointTable[t] = b;
	PointPtr++;
}

// Gets a point from the look-up table
void GetPoint(Point &a) {
	Point b = Canonic(a);
	uint_t t = PosTable(b);
	a.rayon = PointTable[t].rayon;
}

static const real_t degree = 57.29577951308232; // From spherical to rectangular coordinates

_FORCE_INLINE_ real_t Polar2Cart_X(const Point &a) {
	return (real_t)(a.rayon * Math::sin(a.beta / degree) * Math::cos(a.alpha / degree));
}

_FORCE_INLINE_ real_t Polar2Cart_Y(const Point &a) {
	return (real_t)(a.rayon * Math::cos(a.beta / degree));
}

_FORCE_INLINE_ real_t Polar2Cart_Z(const Point &a) {
	return (real_t)(a.rayon * Math::sin(a.alpha / degree) * Math::sin(a.beta / degree));
}

Point Normalise(real_t x, real_t y, real_t z) {
	Point a;
	const real_t norm = Math::sqrt(x * x + y * y + z * z);

	a.alpha = x / norm;
	a.beta = y / norm;
	a.rayon = z / norm;

	return a;
}

void InstancieTriangle(PoolVector3Array &result, const Point &a, const Point &b, const Point &c) {
	if (mode == 0) {
		result.push_back({ Polar2Cart_X(a), Polar2Cart_Y(a), Polar2Cart_Z(a) });
		result.push_back({ Polar2Cart_X(b), Polar2Cart_Y(b), Polar2Cart_Z(b) });
		result.push_back({ Polar2Cart_X(c), Polar2Cart_Y(c), Polar2Cart_Z(c) });
	} else {
		const Point N1 = Normalise(Polar2Cart_X(a), Polar2Cart_Y(a), Polar2Cart_Z(a));
		const Point N2 = Normalise(Polar2Cart_X(b), Polar2Cart_Y(b), Polar2Cart_Z(b));
		const Point N3 = Normalise(Polar2Cart_X(c), Polar2Cart_Y(c), Polar2Cart_Z(c));

		result.push_back({ Polar2Cart_X(a), Polar2Cart_Y(a), Polar2Cart_Z(a) });
		result.push_back({ N1.alpha, N1.beta, N1.rayon });

		result.push_back({ Polar2Cart_X(b), Polar2Cart_Y(b), Polar2Cart_Z(b) });
		result.push_back({ N2.alpha, N2.beta, N2.rayon });

		result.push_back({ Polar2Cart_X(c), Polar2Cart_Y(c), Polar2Cart_Z(c) });
		result.push_back({ N3.alpha, N3.beta, N3.rayon });
	}
}

static real_t Expos = 1;

real_t Perturbation(int profondeur) {
	return (Math::randf() - Math::randf()) / Math::exp(Expos * (1 + MaxProf - profondeur));
}

real_t Perturbe(const Point &a, const Point &b, int profondeur) {
	return ((a.rayon + b.rayon) / 2) + Perturbation(profondeur);
}

void Recurse(PoolVector3Array &result, const Point &a, const Point &b, const Point &c, int profondeur, bool Polar = false) {
	if (profondeur) {
		Point d, e, f; // recursons joyeusement

		if (Polar) { // north polar region
			d.alpha = b.alpha;
			d.beta = (a.beta + b.beta) / 2;
			if (PointExiste(d)) {
				GetPoint(d);
			} else {
				d.rayon = Perturbe(a, b, profondeur);
				PutPoint(d);
			}

			e.alpha = c.alpha;
			e.beta = (a.beta + c.beta) / 2;
			if (PointExiste(e)) {
				GetPoint(e);
			} else {
				e.rayon = Perturbe(a, c, profondeur);
				PutPoint(e);
			}

			f = Point((b.alpha + c.alpha) / 2, (b.beta + c.beta) / 2, 0);
			if (PointExiste(f)) {
				GetPoint(f);
			} else {
				f.rayon = Perturbe(b, c, profondeur);
				PutPoint(f);
			}

			Recurse(result, a, d, e, profondeur - 1, true);
			Recurse(result, d, e, f, profondeur - 1);
			Recurse(result, d, b, f, profondeur - 1);
			Recurse(result, e, f, c, profondeur - 1);
		} else {
			d = Point((a.alpha + b.alpha) / 2, (a.beta + b.beta) / 2, 0);
			if (PointExiste(d)) {
				GetPoint(d);
			} else {
				d.rayon = Perturbe(a, b, profondeur);
				PutPoint(d);
			}

			e = Point((a.alpha + c.alpha) / 2, (a.beta + c.beta) / 2, 0);
			if (PointExiste(e)) {
				GetPoint(e);
			} else {
				e.rayon = Perturbe(a, c, profondeur);
				PutPoint(e);
			}

			f = Point((b.alpha + c.alpha) / 2, (b.beta + c.beta) / 2, 0);
			if (PointExiste(f)) {
				GetPoint(f);
			} else {
				f.rayon = Perturbe(b, c, profondeur);
				PutPoint(f);
			}
			Recurse(result, a, d, e, profondeur - 1);
			Recurse(result, d, e, f, profondeur - 1);
			Recurse(result, d, b, f, profondeur - 1);
			Recurse(result, e, f, c, profondeur - 1);
		}
	} else {
		InstancieTriangle(result, a, b, c);
	}
}

Array rock_gen(int depth = 3, int randseed = 0, real_t smoothness = 1, bool smoothed = false) {
	MaxProf = MAX(0, MIN(8, depth));

	if (randseed == 0) {
		Math::randomize();
	} else {
		Math::seed(randseed);
	}

	Expos = MAX(0, smoothness);

	print_verbose(vformat("mesh has %d triangles", 20 * ExpoDiscrete(4, MaxProf)));
	MaxPoint = 13 * ExpoDiscrete(2, MaxProf) * (ExpoDiscrete(2, MaxProf) + 1); // MaxPoint = 10 * ExpoDiscrete(2, MaxProf) * (ExpoDiscrete(2, MaxProf) + 1);
	print_verbose(vformat("mesh has ~%d summits\n", MaxPoint));

	dp = RelativementPremierPhi(MaxPoint);

	PointPtr = 0;
	PoolVector3Array result;

	Recurse(result, Point(0, 0, 1), Point(-36, 60, 1), Point(+36, 60, 1), MaxProf, true);
	Recurse(result, Point(72, 0, 1), Point(+36, 60, 1), Point(108, 60, 1), MaxProf, true);
	Recurse(result, Point(144, 0, 1), Point(108, 60, 1), Point(180, 60, 1), MaxProf, true);
	Recurse(result, Point(-72, 0, 1), Point(-108, 60, 1), Point(-36, 60, 1), MaxProf, true);
	Recurse(result, Point(-144, 0, 1), Point(-180, 60, 1), Point(-108, 60, 1), MaxProf, true);

	Recurse(result, Point(0, 120, 1), Point(-36, 60, 1), Point(+36, 60, 1), MaxProf);
	Recurse(result, Point(72, 120, 1), Point(+36, 60, 1), Point(108, 60, 1), MaxProf);
	Recurse(result, Point(144, 120, 1), Point(108, 60, 1), Point(180, 60, 1), MaxProf);
	Recurse(result, Point(-72, 120, 1), Point(-108, 60, 1), Point(-36, 60, 1), MaxProf);
	Recurse(result, Point(-144, 120, 1), Point(-180, 60, 1), Point(-108, 60, 1), MaxProf);

	Recurse(result, Point(36, 60, 1), Point(0, 120, 1), Point(72, 120, 1), MaxProf);
	Recurse(result, Point(108, 60, 1), Point(72, 120, 1), Point(144, 120, 1), MaxProf);
	Recurse(result, Point(-180, 60, 1), Point(-144, 120, 1), Point(-216, 120, 1), MaxProf);
	Recurse(result, Point(-36, 60, 1), Point(0, 120, 1), Point(-72, 120, 1), MaxProf);
	Recurse(result, Point(-108, 60, 1), Point(-72, 120, 1), Point(-144, 120, 1), MaxProf);

	Recurse(result, Point(36, 180, 1), Point(0, 120, 1), Point(72, 120, 1), MaxProf, true);
	Recurse(result, Point(108, 180, 1), Point(72, 120, 1), Point(144, 120, 1), MaxProf, true);
	Recurse(result, Point(-180, 180, 1), Point(-144, 120, 1), Point(-216, 120, 1), MaxProf, true);
	Recurse(result, Point(-36, 180, 1), Point(0, 120, 1), Point(-72, 120, 1), MaxProf, true);
	Recurse(result, Point(-108, 180, 1), Point(-72, 120, 1), Point(-144, 120, 1), MaxProf, true);

	return Array();
}
