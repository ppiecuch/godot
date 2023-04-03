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

#include "OpenSteer/LocalSpace.h"
#include "OpenSteer/Vec3.h"

#include <sstream>

namespace OpenSteer {
// OpenSteer system/drawing interface
namespace Draw {
void drawCameraLookAt(const Vec3 &cameraPosition, const Vec3 &pointToLookAt, const Vec3 &up) {
}
void drawLine(const Vec3 &startPoint, const Vec3 &endPoint, const Vec3 &color) {
}
void drawLine(const Vec3 &startPoint, const Vec3 &endPoint, const Vec3 &color, const float alpha) {
}
void drawWideLine(const Vec3 &startPoint, const Vec3 &endPoint, const Vec3 &color, const float width) {
}
void drawLineGrid(int hseg, int vseg, const Vec3 &center, const Vec3 &color) {
}
void drawCircle(const float radius, const Vec3 &axis, const Vec3 &center, const Vec3 &color, const int segments, const bool filled, const bool in3d) {
}
void drawCircle(const float radius, const Vec3 &center, const Vec3 &color, const int segments, const bool filled) {
}
void drawQuadrangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, const Vec3 &p4, const Vec3 &color) {
}
void drawCheckerboardGrid(const float size, const int subsquares, const Vec3 &center, const Vec3 &color1, const Vec3 &color2) {
}
void drawBox(const AbstractLocalSpace &localSpace, const Vec3 &size, const Vec3 &color) {
}
void drawCircle(const AbstractLocalSpace &localSpace, const Vec3 &color, float radius, bool filled, float up_offset) {
}
void drawTextAt2dLocation(const std::ostringstream &text, const Vec3 &position, const Vec3 &color) {
}
void drawTextAt2dLocation(const char *text, const Vec3 &position, const Vec3 &color) {
}
void drawTextAt3dLocation(const std::ostringstream &text, const Vec3 &position, const Vec3 &color) {
}
void drawTextAt3dLocation(const char *text, const Vec3 &position, const Vec3 &color) {
}
} // namespace Draw
} // namespace OpenSteer
