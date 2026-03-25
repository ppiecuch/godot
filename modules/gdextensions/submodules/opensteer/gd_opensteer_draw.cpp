/**************************************************************************/
/*  gd_opensteer_draw.cpp                                                 */
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

// Implements the OpenSteer Draw:: namespace functions and App stubs.
//
// OpenSteer's plugin demos call these functions for debug visualization.
// The draw calls are routed through a simple callback interface so Godot
// code can hook in with an ImmediateGeometry node or similar.
//
// When no callback is registered the functions are no-ops — safe for
// headless / test / export-template builds.

#include "OpenSteer/App.h"
#include "OpenSteer/LocalSpace.h"
#include "OpenSteer/Vec3.h"

#include <sstream>

// =========================================================================
// Draw callback interface
// =========================================================================

namespace OpenSteer {
namespace Draw {

// --- Callback function pointers (set by Godot wrapper) ---

typedef void (*DrawLineFunc)(const Vec3 &a, const Vec3 &b, const Vec3 &color, float alpha, float width);
typedef void (*DrawCircleFunc)(float radius, const Vec3 &axis, const Vec3 &center, const Vec3 &color, int segments, bool filled, bool in3d);
typedef void (*DrawQuadFunc)(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, const Vec3 &p4, const Vec3 &color);
typedef void (*DrawTextFunc)(const char *text, const Vec3 &position, const Vec3 &color, bool is3d);

static DrawLineFunc _draw_line_cb = nullptr;
static DrawCircleFunc _draw_circle_cb = nullptr;
static DrawQuadFunc _draw_quad_cb = nullptr;
static DrawTextFunc _draw_text_cb = nullptr;

void setDrawLineCallback(DrawLineFunc cb) { _draw_line_cb = cb; }
void setDrawCircleCallback(DrawCircleFunc cb) { _draw_circle_cb = cb; }
void setDrawQuadCallback(DrawQuadFunc cb) { _draw_quad_cb = cb; }
void setDrawTextCallback(DrawTextFunc cb) { _draw_text_cb = cb; }

// =========================================================================
// Line drawing
// =========================================================================

void drawLine(const Vec3 &startPoint, const Vec3 &endPoint, const Vec3 &color) {
	if (_draw_line_cb)
		_draw_line_cb(startPoint, endPoint, color, 1.0f, 1.0f);
}

void drawLine(const Vec3 &startPoint, const Vec3 &endPoint, const Vec3 &color, const float alpha) {
	if (_draw_line_cb)
		_draw_line_cb(startPoint, endPoint, color, alpha, 1.0f);
}

void drawWideLine(const Vec3 &startPoint, const Vec3 &endPoint, const Vec3 &color, const float width) {
	if (_draw_line_cb)
		_draw_line_cb(startPoint, endPoint, color, 1.0f, width);
}

// =========================================================================
// Grid drawing
// =========================================================================

void drawLineGrid(int hseg, int vseg, const Vec3 &center, const Vec3 &color) {
	if (!_draw_line_cb)
		return;
	float half_h = hseg * 0.5f;
	float half_v = vseg * 0.5f;
	for (int i = 0; i <= hseg; i++) {
		float x = -half_h + i;
		_draw_line_cb(center + Vec3(x, 0, -half_v), center + Vec3(x, 0, half_v), color, 1.0f, 1.0f);
	}
	for (int i = 0; i <= vseg; i++) {
		float z = -half_v + i;
		_draw_line_cb(center + Vec3(-half_h, 0, z), center + Vec3(half_h, 0, z), color, 1.0f, 1.0f);
	}
}

void drawCheckerboardGrid(const float size, const int subsquares, const Vec3 &center, const Vec3 &color1, const Vec3 &color2) {
	if (!_draw_quad_cb)
		return;
	float step = size / subsquares;
	float half = size * 0.5f;
	for (int i = 0; i < subsquares; i++) {
		for (int j = 0; j < subsquares; j++) {
			const Vec3 &c = ((i + j) & 1) ? color2 : color1;
			float x = center.x - half + i * step;
			float z = center.z - half + j * step;
			_draw_quad_cb(
					Vec3(x, center.y, z),
					Vec3(x + step, center.y, z),
					Vec3(x + step, center.y, z + step),
					Vec3(x, center.y, z + step), c);
		}
	}
}

// =========================================================================
// Circle / disk drawing
// =========================================================================

void drawCircle(const float radius, const Vec3 &axis, const Vec3 &center, const Vec3 &color, const int segments, const bool filled, const bool in3d) {
	if (_draw_circle_cb)
		_draw_circle_cb(radius, axis, center, color, segments, filled, in3d);
}

void drawCircle(const float radius, const Vec3 &center, const Vec3 &color, const int segments, const bool filled) {
	if (_draw_circle_cb)
		_draw_circle_cb(radius, Vec3::up, center, color, segments, filled, false);
}

void drawCircle(const AbstractLocalSpace &localSpace, const Vec3 &color, float radius, bool filled, float up_offset) {
	Vec3 center = localSpace.position() + localSpace.up() * up_offset;
	if (_draw_circle_cb)
		_draw_circle_cb(radius, localSpace.up(), center, color, 24, filled, true);
}

// =========================================================================
// Quad / polygon drawing
// =========================================================================

void drawQuadrangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, const Vec3 &p4, const Vec3 &color) {
	if (_draw_quad_cb)
		_draw_quad_cb(p1, p2, p3, p4, color);
}

// =========================================================================
// Box drawing
// =========================================================================

void drawBox(const AbstractLocalSpace &localSpace, const Vec3 &size, const Vec3 &color) {
	drawBox(localSpace, size, color, false);
}

void drawBox(const AbstractLocalSpace &localSpace, const Vec3 &size, const Vec3 &color, bool filled) {
	if (!_draw_line_cb && !_draw_quad_cb)
		return;

	Vec3 s = size * 0.5f;
	Vec3 p = localSpace.position();
	Vec3 f = localSpace.forward();
	Vec3 u = localSpace.up();
	Vec3 r = localSpace.side();

	// 8 corners
	Vec3 corners[8] = {
		p + f * s.z + u * s.y + r * s.x,
		p + f * s.z + u * s.y - r * s.x,
		p + f * s.z - u * s.y - r * s.x,
		p + f * s.z - u * s.y + r * s.x,
		p - f * s.z + u * s.y + r * s.x,
		p - f * s.z + u * s.y - r * s.x,
		p - f * s.z - u * s.y - r * s.x,
		p - f * s.z - u * s.y + r * s.x,
	};

	if (filled && _draw_quad_cb) {
		_draw_quad_cb(corners[0], corners[1], corners[2], corners[3], color); // front
		_draw_quad_cb(corners[4], corners[5], corners[6], corners[7], color); // back
		_draw_quad_cb(corners[0], corners[4], corners[7], corners[3], color); // right
		_draw_quad_cb(corners[1], corners[5], corners[6], corners[2], color); // left
		_draw_quad_cb(corners[0], corners[1], corners[5], corners[4], color); // top
		_draw_quad_cb(corners[3], corners[2], corners[6], corners[7], color); // bottom
	} else if (_draw_line_cb) {
		// 12 edges
		for (int i = 0; i < 4; i++) {
			_draw_line_cb(corners[i], corners[(i + 1) % 4], color, 1.0f, 1.0f);
			_draw_line_cb(corners[i + 4], corners[(i + 1) % 4 + 4], color, 1.0f, 1.0f);
			_draw_line_cb(corners[i], corners[i + 4], color, 1.0f, 1.0f);
		}
	}
}

// =========================================================================
// Text drawing
// =========================================================================

void drawTextAt2dLocation(const std::ostringstream &text, const Vec3 &position, const Vec3 &color) {
	if (_draw_text_cb)
		_draw_text_cb(text.str().c_str(), position, color, false);
}

void drawTextAt2dLocation(const char *text, const Vec3 &position, const Vec3 &color) {
	if (_draw_text_cb)
		_draw_text_cb(text, position, color, false);
}

void drawTextAt3dLocation(const std::ostringstream &text, const Vec3 &position, const Vec3 &color) {
	if (_draw_text_cb)
		_draw_text_cb(text.str().c_str(), position, color, true);
}

void drawTextAt3dLocation(const char *text, const Vec3 &position, const Vec3 &color) {
	if (_draw_text_cb)
		_draw_text_cb(text, position, color, true);
}

// =========================================================================
// Camera
// =========================================================================

void drawCameraLookAt(const Vec3 &cameraPosition, const Vec3 &pointToLookAt, const Vec3 &up) {
	// No-op: Godot manages its own camera
}

} // namespace Draw

// =========================================================================
// App stubs — not used in Godot integration but required by plugins
// =========================================================================

Vec3 App::cameraToScreenPosition(int x, int y) {
	return Vec3((float)x, (float)y, 0);
}

} // namespace OpenSteer
