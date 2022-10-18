/*************************************************************************/
/*  pixel_terrain.cpp                                                    */
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

#include "core/image.h"
#include "core/math/math_funcs.h"

#include "pixel_terrain.h"

#define TWOPI 6.28
#define PI 3.14
#define ftrunc(a) ((a) >> 16)
#define height(h, d) ((((h)-128) * (32) / (d)) + 100)

template<class T> void _ignore_( const T& ) { }

// 60 degree vision, or pi/3-- (pi/3)/320 (320 v.lines on screen)
// this is set for 1280x200 rendering view.
// 320*4 = 1280 width
// Ray step is the incrimenting variable for the raycaster.
const real_t RAYSTEP = (TWOPI) / 1280;

_FORCE_INLINE_ static int tofixed(real_t a) {
	return (int)(a * 65536);
}

_FORCE_INLINE_ static void vline(Ref<Image> bmp, int x, int y1, int y2, Color color) {
	bmp->fill_rect(Rect2(x, y2, 1, y1 - y2), color);
}

// takes bitmap to draw onto, a map bitmap, a texture bitmap, the camera's x, y, angle and height
void renderframe(Ref<Image> dbl, Ref<Image> map, Ref<Image> cm, int px, int py, real_t pa, real_t pz) {
	// So how does it work?  It throws out rays until it hits something that would be
	// higher on the screen than the last hill the ray hit.  Then it looks up on the other chart
	// and finds the appropriate color for that point.  It then draws a vertical line from the previous
	// highest point to the point the ray just hit.

	real_t angle = pa; // the ray's current angle
	int rx, ry, rsin, rcos; // ray x, y, sin and cos (fixed)
	int rz, cz; // current ray's z and last ray hit's z
	int hr, hc; //on-screen heights of the current ray and last ray hit
	int rspeed; // ray's speed
	int rdist, ldist; // ldist is distance of last hit, rdist is current ray's distance

	for (int c = 0; c < 1280; c++) {
		angle += RAYSTEP;
		rx = px;
		ry = py;
		rdist = 0;
		rsin = tofixed(Math::sin(angle));
		rcos = tofixed(Math::cos(angle));
		rz = 0; // rz is ray height
		cz = 0;
		hc = 0;
		ldist = 1;
		rspeed = 1;
		for (int rlum = 120; rlum > 0; rlum--) { // luminance determines drawing distance of the ray

			// Move ray along sin and cos, recording current height
			// and drawing a line to the screen when height exceeds current height
			// This used to have ray acceleration to speed it up, but it flickered and I think
			// modern computers can handle this no problem (it will run on a 486 this way still).

			rx += rcos * rspeed;
			ry += rsin * rspeed;
			rdist++;

			const int frx = ftrunc(rx);
			const int fry = ftrunc(ry);

			rz = 0;
			if (rx < 41943040 && rx >= 0 && ry < 41943040 && ry >= 0) {
				rz = map->get_pixel(frx, fry).to_rgba32();
			}

			hr = height(rz, rdist);
			if (hr > hc) {
				// draw vertical line
				vline(dbl, c, 200 - hc, 200 - hr, cm->get_pixel(frx, fry));
				// copy over last visible ray's info
				cz = rz;
				ldist = rdist;
				hc = hr;
			}
		}
	}
	_ignore_(ldist);
	_ignore_(cz);
}

void PixelTerrain::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			dbl->fill(clear_color);
			renderframe(dbl, map, cm, px, py, pa, 128);
		}
	}
}

void PixelTerrain::action_left(real_t p_step) {
	pa -= 0.1;
	update();
}

void PixelTerrain::action_right(real_t p_step) {
	pa += 0.1;
	update();
}

void PixelTerrain::action_up() {
	px += tofixed(Math::cos(pa));
	py += tofixed(Math::sin(pa));
	update();
}

void PixelTerrain::action_down() {
	px -= tofixed(Math::cos(pa));
	py -= tofixed(Math::sin(pa));
	update();
}

PixelTerrain::PixelTerrain() {
	// player x, y and angle
	px = tofixed(300);
	py = tofixed(300);
	pa = 0;
}
