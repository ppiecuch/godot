/**************************************************************************/
/*  line_builder_2d.cpp                                                   */
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

#include "line_builder_2d.h"

// Needed so we can bind functions
VARIANT_ENUM_CAST(Line2D::LineJointMode)
VARIANT_ENUM_CAST(Line2D::LineCapMode)
VARIANT_ENUM_CAST(Line2D::LineTextureMode)

#define CopyArray(fr, to)                   \
	{                                       \
		to.resize(fr.size());               \
		for (int i = 0; i < fr.size(); ++i) \
			to.set(i, fr[i]);               \
	}

void LineBuilder2D::set_points(const PoolVector2Array &p_points) {
	lb.points.clear();
	CopyArray(p_points, lb.points);
}
PoolVector2Array LineBuilder2D::get_points() const {
	PoolVector2Array ret;
	CopyArray(lb.points, ret);
	return ret;
}
void LineBuilder2D::set_default_color(const Color &p_default_color) {
	lb.default_color = p_default_color;
}
Color LineBuilder2D::get_default_color() const {
	return lb.default_color;
}
void LineBuilder2D::set_gradient(Ref<Gradient> p_gradient) {
	lb.gradient = *p_gradient;
}
Ref<Gradient> LineBuilder2D::get_gradient() const {
	return lb.gradient;
}
void LineBuilder2D::set_texture_mode(Line2D::LineTextureMode p_texture_mode) {
	lb.texture_mode = p_texture_mode;
}
Line2D::LineTextureMode LineBuilder2D::get_texture_mode() const {
	return lb.texture_mode;
}
void LineBuilder2D::set_tile_aspect(float p_tile_aspect) {
	lb.tile_aspect = p_tile_aspect;
}
float LineBuilder2D::get_tile_aspect() const {
	return lb.tile_aspect;
}
void LineBuilder2D::set_tile_region(Rect2 p_tile_region) {
	lb.tile_region = p_tile_region;
}
Rect2 LineBuilder2D::get_tile_region() const {
	return lb.tile_region;
}
void LineBuilder2D::set_joint_mode(Line2D::LineJointMode p_joint_mode) {
	lb.joint_mode = p_joint_mode;
}
Line2D::LineJointMode LineBuilder2D::get_joint_mode() const {
	return lb.joint_mode;
}
void LineBuilder2D::set_begin_cap_mode(Line2D::LineCapMode p_begin_cap_mode) {
	lb.begin_cap_mode = p_begin_cap_mode;
}
Line2D::LineCapMode LineBuilder2D::get_begin_cap_mode() const {
	return lb.begin_cap_mode;
}
void LineBuilder2D::set_end_cap_mode(Line2D::LineCapMode p_end_cap_mode) {
	lb.end_cap_mode = p_end_cap_mode;
}
Line2D::LineCapMode LineBuilder2D::get_end_cap_mode() const {
	return lb.end_cap_mode;
}
void LineBuilder2D::set_round_precision(int p_round_precision) {
	lb.round_precision = p_round_precision;
}
int LineBuilder2D::get_round_precision() const {
	return lb.round_precision;
}
void LineBuilder2D::set_sharp_limit(real_t p_sharp_limit) {
	lb.sharp_limit = p_sharp_limit;
}
real_t LineBuilder2D::get_sharp_limit() const {
	return lb.sharp_limit;
}
void LineBuilder2D::set_width(float p_width) {
	lb.width = p_width;
}
float LineBuilder2D::get_width() const {
	return lb.width;
}
void LineBuilder2D::set_curve(Ref<Curve> p_curve) {
	lb.curve = *p_curve;
}
Ref<Curve> LineBuilder2D::get_curve() const {
	return lb.curve;
}

PoolIntArray LineBuilder2D::get_indices() const {
	PoolIntArray ret;
	CopyArray(lb.indices, ret);
	return ret;
}
PoolVector2Array LineBuilder2D::get_vertices() const {
	PoolVector2Array ret;
	CopyArray(lb.vertices, ret);
	return ret;
}
PoolColorArray LineBuilder2D::get_colors() const {
	PoolColorArray ret;
	CopyArray(lb.colors, ret);
	return ret;
}
PoolVector2Array LineBuilder2D::get_uvs() const {
	PoolVector2Array ret;
	CopyArray(lb.uvs, ret);
	return ret;
}

LineBuilder2D::LineBuilder2D() {
	lb.joint_mode = Line2D::LINE_JOINT_ROUND;
	lb.begin_cap_mode = Line2D::LINE_CAP_NONE;
	lb.end_cap_mode = Line2D::LINE_CAP_NONE;
	lb.width = 10;
	lb.default_color = Color(0.4, 0.5, 1);
	lb.texture_mode = Line2D::LINE_TEXTURE_TILE;
	lb.sharp_limit = 2.f;
	lb.round_precision = 8;
}

void LineBuilder2D::_bind_methods() {
	ClassDB::bind_method("get_points", &LineBuilder2D::get_points);
	ClassDB::bind_method("set_points", &LineBuilder2D::set_points);
	ClassDB::bind_method("set_default_color", &LineBuilder2D::set_default_color);
	ClassDB::bind_method("get_default_color", &LineBuilder2D::get_default_color);
	ClassDB::bind_method("set_gradient", &LineBuilder2D::set_gradient);
	ClassDB::bind_method("get_gradient", &LineBuilder2D::get_gradient);
	ClassDB::bind_method("set_texture_mode", &LineBuilder2D::set_texture_mode);
	ClassDB::bind_method("get_texture_mode", &LineBuilder2D::get_texture_mode);
	ClassDB::bind_method("set_tile_aspect", &LineBuilder2D::set_tile_aspect);
	ClassDB::bind_method("get_tile_aspect", &LineBuilder2D::get_tile_aspect);
	ClassDB::bind_method("set_tile_region", &LineBuilder2D::set_tile_region);
	ClassDB::bind_method("get_tile_region", &LineBuilder2D::get_tile_region);
	ClassDB::bind_method("set_joint_mode", &LineBuilder2D::set_joint_mode);
	ClassDB::bind_method("get_joint_mode", &LineBuilder2D::get_joint_mode);
	ClassDB::bind_method("set_begin_cap_mode", &LineBuilder2D::set_begin_cap_mode);
	ClassDB::bind_method("get_begin_cap_mode", &LineBuilder2D::get_begin_cap_mode);
	ClassDB::bind_method("set_end_cap_mode", &LineBuilder2D::set_end_cap_mode);
	ClassDB::bind_method("get_end_cap_mode", &LineBuilder2D::get_end_cap_mode);
	ClassDB::bind_method("set_round_precision", &LineBuilder2D::set_round_precision);
	ClassDB::bind_method("get_round_precision", &LineBuilder2D::get_round_precision);
	ClassDB::bind_method("set_sharp_limit", &LineBuilder2D::set_sharp_limit);
	ClassDB::bind_method("get_sharp_limit", &LineBuilder2D::get_sharp_limit);
	ClassDB::bind_method("set_width", &LineBuilder2D::set_width);
	ClassDB::bind_method("get_width", &LineBuilder2D::get_width);
	ClassDB::bind_method(D_METHOD("set_curve", "curve"), &LineBuilder2D::set_curve);
	ClassDB::bind_method("get_curve", &LineBuilder2D::get_curve);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "points"), "set_points", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "width_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_curve", "get_curve");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_color"), "set_default_color", "get_default_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_mode", PROPERTY_HINT_ENUM, "None,Tile,Stretch"), "set_texture_mode", "get_texture_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tile_aspect"), "set_tile_aspect", "get_tile_aspect");
	ADD_PROPERTY(PropertyInfo(Variant::RECT2, "tile_region"), "set_tile_region", "get_tile_region");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "joint_mode", PROPERTY_HINT_ENUM, "Sharp,Bevel,Round"), "set_joint_mode", "get_joint_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "begin_cap_mode", PROPERTY_HINT_ENUM, "None,Box,Round"), "set_begin_cap_mode", "get_begin_cap_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "end_cap_mode", PROPERTY_HINT_ENUM, "None,Box,Round"), "set_end_cap_mode", "get_end_cap_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round_precision"), "set_round_precision", "get_round_precision");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharp_limit"), "set_sharp_limit", "get_sharp_limit");

	ClassDB::bind_method("get_vertices", &LineBuilder2D::get_vertices);
	ClassDB::bind_method("get_colors", &LineBuilder2D::get_colors);
	ClassDB::bind_method("get_uvs", &LineBuilder2D::get_uvs);
	ClassDB::bind_method("get_indices", &LineBuilder2D::get_indices);

	ClassDB::bind_method("build", &LineBuilder2D::build);
	ClassDB::bind_method("clear_output", &LineBuilder2D::clear_output);
}
