/**************************************************************************/
/*  canvas_raster.cpp                                                     */
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

#include "canvas_raster.h"

#define CANVAS_ITY_IMPLEMENTATION
#include "misc/canvas_ity.h"

#include "doctest/doctest.h"

using namespace canvas_ity;

// ============================================================
// CanvasIty (Reference)
// ============================================================

CanvasIty::CanvasIty() {
	ctx = nullptr;
	canvas_width = 0;
	canvas_height = 0;
	dirty = false;
}

CanvasIty::~CanvasIty() {
	if (ctx) {
		delete ctx;
		ctx = nullptr;
	}
}

void CanvasIty::_ensure_ctx() {
	if (!ctx) {
		initialize(256, 256);
	}
}

void CanvasIty::initialize(int p_width, int p_height) {
	ERR_FAIL_COND(p_width < 1 || p_width > 32768);
	ERR_FAIL_COND(p_height < 1 || p_height > 32768);
	if (ctx) {
		delete ctx;
	}
	ctx = new canvas(p_width, p_height);
	canvas_width = p_width;
	canvas_height = p_height;
	dirty = true;
	cached_image.unref();
	cached_texture.unref();
}

int CanvasIty::get_width() const {
	return canvas_width;
}

int CanvasIty::get_height() const {
	return canvas_height;
}

Ref<Image> CanvasIty::get_image() {
	ERR_FAIL_COND_V(!ctx, Ref<Image>());
	PoolByteArray data;
	int size = canvas_width * canvas_height * 4;
	data.resize(size);
	{
		PoolByteArray::Write wd = data.write();
		ctx->get_image_data(wd.ptr(), canvas_width, canvas_height, canvas_width * 4, 0, 0);
	}
	cached_image.instance();
	cached_image->create(canvas_width, canvas_height, false, Image::FORMAT_RGBA8, data);
	dirty = false;
	return cached_image;
}

Ref<ImageTexture> CanvasIty::get_texture() {
	ERR_FAIL_COND_V(!ctx, Ref<ImageTexture>());
	Ref<Image> img = get_image();
	ERR_FAIL_COND_V(img.is_null(), Ref<ImageTexture>());
	if (cached_texture.is_null()) {
		cached_texture.instance();
	}
	cached_texture->create_from_image(img, 0);
	return cached_texture;
}

// State

void CanvasIty::save() {
	_ensure_ctx();
	ctx->save();
}

void CanvasIty::restore() {
	_ensure_ctx();
	ctx->restore();
}

// Transforms

void CanvasIty::canvas_scale(const Vector2 &p_factor) {
	_ensure_ctx();
	ctx->scale(p_factor.x, p_factor.y);
}

void CanvasIty::canvas_rotate(float p_angle) {
	_ensure_ctx();
	ctx->rotate(p_angle);
}

void CanvasIty::canvas_translate(const Vector2 &p_offset) {
	_ensure_ctx();
	ctx->translate(p_offset.x, p_offset.y);
}

void CanvasIty::append_transform(const Transform2D &p_xform) {
	_ensure_ctx();
	ctx->transform(
			p_xform.elements[0].x, p_xform.elements[0].y,
			p_xform.elements[1].x, p_xform.elements[1].y,
			p_xform.elements[2].x, p_xform.elements[2].y);
}

void CanvasIty::set_canvas_transform(const Transform2D &p_xform) {
	_ensure_ctx();
	ctx->set_transform(
			p_xform.elements[0].x, p_xform.elements[0].y,
			p_xform.elements[1].x, p_xform.elements[1].y,
			p_xform.elements[2].x, p_xform.elements[2].y);
}

void CanvasIty::reset_transform() {
	_ensure_ctx();
	ctx->set_transform(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
}

// Compositing

void CanvasIty::set_global_alpha(float p_alpha) {
	_ensure_ctx();
	ctx->set_global_alpha(p_alpha);
}

void CanvasIty::set_composite_operation(int p_op) {
	_ensure_ctx();
	ctx->global_composite_operation = (composite_operation)p_op;
}

int CanvasIty::get_composite_operation() const {
	ERR_FAIL_COND_V(!ctx, COMPOSITE_SOURCE_OVER);
	return (int)ctx->global_composite_operation;
}

// Shadows

void CanvasIty::set_shadow_color(const Color &p_color) {
	_ensure_ctx();
	ctx->set_shadow_color(p_color.r, p_color.g, p_color.b, p_color.a);
}

void CanvasIty::set_shadow_offset(const Vector2 &p_offset) {
	_ensure_ctx();
	ctx->shadow_offset_x = p_offset.x;
	ctx->shadow_offset_y = p_offset.y;
}

Vector2 CanvasIty::get_shadow_offset() const {
	ERR_FAIL_COND_V(!ctx, Vector2());
	return Vector2(ctx->shadow_offset_x, ctx->shadow_offset_y);
}

void CanvasIty::set_shadow_blur(float p_level) {
	_ensure_ctx();
	ctx->set_shadow_blur(p_level);
}

// Line styles

void CanvasIty::set_line_width(float p_width) {
	_ensure_ctx();
	ctx->set_line_width(p_width);
}

void CanvasIty::set_line_cap(int p_cap) {
	_ensure_ctx();
	ctx->line_cap = (cap_style)p_cap;
}

int CanvasIty::get_line_cap() const {
	ERR_FAIL_COND_V(!ctx, CAP_BUTT);
	return (int)ctx->line_cap;
}

void CanvasIty::set_line_join(int p_join) {
	_ensure_ctx();
	ctx->line_join = (join_style)p_join;
}

int CanvasIty::get_line_join() const {
	ERR_FAIL_COND_V(!ctx, JOIN_MITER);
	return (int)ctx->line_join;
}

void CanvasIty::set_miter_limit(float p_limit) {
	_ensure_ctx();
	ctx->set_miter_limit(p_limit);
}

void CanvasIty::set_line_dash_offset(float p_offset) {
	_ensure_ctx();
	ctx->line_dash_offset = p_offset;
}

float CanvasIty::get_line_dash_offset() const {
	ERR_FAIL_COND_V(!ctx, 0.0f);
	return ctx->line_dash_offset;
}

void CanvasIty::set_line_dash(const PoolRealArray &p_segments) {
	_ensure_ctx();
	if (p_segments.size() == 0) {
		ctx->set_line_dash(nullptr, 0);
		return;
	}
	PoolRealArray::Read rd = p_segments.read();
	ctx->set_line_dash(rd.ptr(), p_segments.size());
}

// Fill/Stroke styles

void CanvasIty::set_fill_color(const Color &p_color) {
	_ensure_ctx();
	ctx->set_color(fill_style, p_color.r, p_color.g, p_color.b, p_color.a);
}

void CanvasIty::set_stroke_color(const Color &p_color) {
	_ensure_ctx();
	ctx->set_color(stroke_style, p_color.r, p_color.g, p_color.b, p_color.a);
}

void CanvasIty::set_fill_linear_gradient(const Vector2 &p_start, const Vector2 &p_end) {
	_ensure_ctx();
	ctx->set_linear_gradient(fill_style, p_start.x, p_start.y, p_end.x, p_end.y);
}

void CanvasIty::set_stroke_linear_gradient(const Vector2 &p_start, const Vector2 &p_end) {
	_ensure_ctx();
	ctx->set_linear_gradient(stroke_style, p_start.x, p_start.y, p_end.x, p_end.y);
}

void CanvasIty::set_fill_radial_gradient(const Vector2 &p_start, float p_start_r, const Vector2 &p_end, float p_end_r) {
	_ensure_ctx();
	ctx->set_radial_gradient(fill_style, p_start.x, p_start.y, p_start_r, p_end.x, p_end.y, p_end_r);
}

void CanvasIty::set_stroke_radial_gradient(const Vector2 &p_start, float p_start_r, const Vector2 &p_end, float p_end_r) {
	_ensure_ctx();
	ctx->set_radial_gradient(stroke_style, p_start.x, p_start.y, p_start_r, p_end.x, p_end.y, p_end_r);
}

void CanvasIty::add_fill_color_stop(float p_offset, const Color &p_color) {
	_ensure_ctx();
	ctx->add_color_stop(fill_style, p_offset, p_color.r, p_color.g, p_color.b, p_color.a);
}

void CanvasIty::add_stroke_color_stop(float p_offset, const Color &p_color) {
	_ensure_ctx();
	ctx->add_color_stop(stroke_style, p_offset, p_color.r, p_color.g, p_color.b, p_color.a);
}

void CanvasIty::set_fill_pattern(const Ref<Image> &p_image, int p_repetition) {
	_ensure_ctx();
	ERR_FAIL_COND(p_image.is_null());
	Ref<Image> img = p_image;
	if (img->get_format() != Image::FORMAT_RGBA8) {
		img = img->duplicate();
		img->convert(Image::FORMAT_RGBA8);
	}
	PoolByteArray data = img->get_data();
	PoolByteArray::Read rd = data.read();
	int w = img->get_width();
	int h = img->get_height();
	ctx->set_pattern(fill_style, rd.ptr(), w, h, w * 4, (repetition_style)p_repetition);
}

void CanvasIty::set_stroke_pattern(const Ref<Image> &p_image, int p_repetition) {
	_ensure_ctx();
	ERR_FAIL_COND(p_image.is_null());
	Ref<Image> img = p_image;
	if (img->get_format() != Image::FORMAT_RGBA8) {
		img = img->duplicate();
		img->convert(Image::FORMAT_RGBA8);
	}
	PoolByteArray data = img->get_data();
	PoolByteArray::Read rd = data.read();
	int w = img->get_width();
	int h = img->get_height();
	ctx->set_pattern(stroke_style, rd.ptr(), w, h, w * 4, (repetition_style)p_repetition);
}

// Path building

void CanvasIty::begin_path() {
	_ensure_ctx();
	ctx->begin_path();
	dirty = true;
}

void CanvasIty::move_to(const Vector2 &p_pos) {
	_ensure_ctx();
	ctx->move_to(p_pos.x, p_pos.y);
}

void CanvasIty::close_path() {
	_ensure_ctx();
	ctx->close_path();
}

void CanvasIty::line_to(const Vector2 &p_pos) {
	_ensure_ctx();
	ctx->line_to(p_pos.x, p_pos.y);
}

void CanvasIty::quadratic_curve_to(const Vector2 &p_control, const Vector2 &p_end) {
	_ensure_ctx();
	ctx->quadratic_curve_to(p_control.x, p_control.y, p_end.x, p_end.y);
}

void CanvasIty::bezier_curve_to(const Vector2 &p_cp1, const Vector2 &p_cp2, const Vector2 &p_end) {
	_ensure_ctx();
	ctx->bezier_curve_to(p_cp1.x, p_cp1.y, p_cp2.x, p_cp2.y, p_end.x, p_end.y);
}

void CanvasIty::arc_to(const Vector2 &p_vertex, const Vector2 &p_point, float p_radius) {
	_ensure_ctx();
	ctx->arc_to(p_vertex.x, p_vertex.y, p_point.x, p_point.y, p_radius);
}

void CanvasIty::arc(const Vector2 &p_center, float p_radius, float p_start_angle, float p_end_angle, bool p_ccw) {
	_ensure_ctx();
	ctx->arc(p_center.x, p_center.y, p_radius, p_start_angle, p_end_angle, p_ccw);
}

void CanvasIty::path_rectangle(const Rect2 &p_rect) {
	_ensure_ctx();
	ctx->rectangle(p_rect.position.x, p_rect.position.y, p_rect.size.x, p_rect.size.y);
}

// Drawing

void CanvasIty::fill() {
	_ensure_ctx();
	ctx->fill();
	dirty = true;
}

void CanvasIty::stroke() {
	_ensure_ctx();
	ctx->stroke();
	dirty = true;
}

void CanvasIty::clip() {
	_ensure_ctx();
	ctx->clip();
}

bool CanvasIty::is_point_in_path(const Vector2 &p_point) {
	ERR_FAIL_COND_V(!ctx, false);
	return ctx->is_point_in_path(p_point.x, p_point.y);
}

// Rectangle drawing

void CanvasIty::clear_rectangle(const Rect2 &p_rect) {
	_ensure_ctx();
	ctx->clear_rectangle(p_rect.position.x, p_rect.position.y, p_rect.size.x, p_rect.size.y);
	dirty = true;
}

void CanvasIty::fill_rectangle(const Rect2 &p_rect) {
	_ensure_ctx();
	ctx->fill_rectangle(p_rect.position.x, p_rect.position.y, p_rect.size.x, p_rect.size.y);
	dirty = true;
}

void CanvasIty::stroke_rectangle(const Rect2 &p_rect) {
	_ensure_ctx();
	ctx->stroke_rectangle(p_rect.position.x, p_rect.position.y, p_rect.size.x, p_rect.size.y);
	dirty = true;
}

// Text

void CanvasIty::set_text_align(int p_align) {
	_ensure_ctx();
	ctx->text_align = (align_style)p_align;
}

int CanvasIty::get_text_align() const {
	ERR_FAIL_COND_V(!ctx, ALIGN_LEFT);
	return (int)ctx->text_align;
}

void CanvasIty::set_text_baseline(int p_baseline) {
	_ensure_ctx();
	ctx->text_baseline = (baseline_style)p_baseline;
}

int CanvasIty::get_text_baseline() const {
	ERR_FAIL_COND_V(!ctx, BASELINE_ALPHABETIC);
	return (int)ctx->text_baseline;
}

bool CanvasIty::set_font_from_data(const PoolByteArray &p_data, float p_size) {
	_ensure_ctx();
	ERR_FAIL_COND_V(p_data.size() == 0, false);
	font_data = p_data;
	PoolByteArray::Read rd = font_data.read();
	return ctx->set_font(rd.ptr(), font_data.size(), p_size);
}

bool CanvasIty::set_font_size(float p_size) {
	_ensure_ctx();
	return ctx->set_font(nullptr, 0, p_size);
}

void CanvasIty::fill_text(const String &p_text, const Vector2 &p_pos, float p_max_width) {
	_ensure_ctx();
	CharString utf8 = p_text.utf8();
	if (p_max_width > 0.0f) {
		ctx->fill_text(utf8.get_data(), p_pos.x, p_pos.y, p_max_width);
	} else {
		ctx->fill_text(utf8.get_data(), p_pos.x, p_pos.y);
	}
	dirty = true;
}

void CanvasIty::stroke_text(const String &p_text, const Vector2 &p_pos, float p_max_width) {
	_ensure_ctx();
	CharString utf8 = p_text.utf8();
	if (p_max_width > 0.0f) {
		ctx->stroke_text(utf8.get_data(), p_pos.x, p_pos.y, p_max_width);
	} else {
		ctx->stroke_text(utf8.get_data(), p_pos.x, p_pos.y);
	}
	dirty = true;
}

float CanvasIty::measure_text(const String &p_text) {
	ERR_FAIL_COND_V(!ctx, 0.0f);
	CharString utf8 = p_text.utf8();
	return ctx->measure_text(utf8.get_data());
}

// Images

void CanvasIty::draw_canvas_image(const Ref<Image> &p_image, const Vector2 &p_pos, const Vector2 &p_size) {
	_ensure_ctx();
	ERR_FAIL_COND(p_image.is_null());
	Ref<Image> img = p_image;
	if (img->get_format() != Image::FORMAT_RGBA8) {
		img = img->duplicate();
		img->convert(Image::FORMAT_RGBA8);
	}
	PoolByteArray data = img->get_data();
	PoolByteArray::Read rd = data.read();
	int w = img->get_width();
	int h = img->get_height();
	ctx->draw_image(rd.ptr(), w, h, w * 4, p_pos.x, p_pos.y, p_size.x, p_size.y);
	dirty = true;
}

void CanvasIty::put_image_data(const Ref<Image> &p_image, const Vector2 &p_pos) {
	_ensure_ctx();
	ERR_FAIL_COND(p_image.is_null());
	Ref<Image> img = p_image;
	if (img->get_format() != Image::FORMAT_RGBA8) {
		img = img->duplicate();
		img->convert(Image::FORMAT_RGBA8);
	}
	PoolByteArray data = img->get_data();
	PoolByteArray::Read rd = data.read();
	int w = img->get_width();
	int h = img->get_height();
	ctx->put_image_data(rd.ptr(), w, h, w * 4, (int)p_pos.x, (int)p_pos.y);
	dirty = true;
}

// Bind methods

void CanvasIty::_bind_methods() {
	// Lifecycle
	ClassDB::bind_method(D_METHOD("initialize", "width", "height"), &CanvasIty::initialize);
	ClassDB::bind_method(D_METHOD("get_width"), &CanvasIty::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &CanvasIty::get_height);
	ClassDB::bind_method(D_METHOD("get_image"), &CanvasIty::get_image);
	ClassDB::bind_method(D_METHOD("get_texture"), &CanvasIty::get_texture);

	// State
	ClassDB::bind_method(D_METHOD("save"), &CanvasIty::save);
	ClassDB::bind_method(D_METHOD("restore"), &CanvasIty::restore);

	// Transforms
	ClassDB::bind_method(D_METHOD("canvas_scale", "factor"), &CanvasIty::canvas_scale);
	ClassDB::bind_method(D_METHOD("canvas_rotate", "angle"), &CanvasIty::canvas_rotate);
	ClassDB::bind_method(D_METHOD("canvas_translate", "offset"), &CanvasIty::canvas_translate);
	ClassDB::bind_method(D_METHOD("append_transform", "xform"), &CanvasIty::append_transform);
	ClassDB::bind_method(D_METHOD("set_canvas_transform", "xform"), &CanvasIty::set_canvas_transform);
	ClassDB::bind_method(D_METHOD("reset_transform"), &CanvasIty::reset_transform);

	// Compositing
	ClassDB::bind_method(D_METHOD("set_global_alpha", "alpha"), &CanvasIty::set_global_alpha);
	ClassDB::bind_method(D_METHOD("set_composite_operation", "op"), &CanvasIty::set_composite_operation);
	ClassDB::bind_method(D_METHOD("get_composite_operation"), &CanvasIty::get_composite_operation);

	// Shadows
	ClassDB::bind_method(D_METHOD("set_shadow_color", "color"), &CanvasIty::set_shadow_color);
	ClassDB::bind_method(D_METHOD("set_shadow_offset", "offset"), &CanvasIty::set_shadow_offset);
	ClassDB::bind_method(D_METHOD("get_shadow_offset"), &CanvasIty::get_shadow_offset);
	ClassDB::bind_method(D_METHOD("set_shadow_blur", "level"), &CanvasIty::set_shadow_blur);

	// Line styles
	ClassDB::bind_method(D_METHOD("set_line_width", "width"), &CanvasIty::set_line_width);
	ClassDB::bind_method(D_METHOD("set_line_cap", "cap"), &CanvasIty::set_line_cap);
	ClassDB::bind_method(D_METHOD("get_line_cap"), &CanvasIty::get_line_cap);
	ClassDB::bind_method(D_METHOD("set_line_join", "join"), &CanvasIty::set_line_join);
	ClassDB::bind_method(D_METHOD("get_line_join"), &CanvasIty::get_line_join);
	ClassDB::bind_method(D_METHOD("set_miter_limit", "limit"), &CanvasIty::set_miter_limit);
	ClassDB::bind_method(D_METHOD("set_line_dash_offset", "offset"), &CanvasIty::set_line_dash_offset);
	ClassDB::bind_method(D_METHOD("get_line_dash_offset"), &CanvasIty::get_line_dash_offset);
	ClassDB::bind_method(D_METHOD("set_line_dash", "segments"), &CanvasIty::set_line_dash);

	// Fill/Stroke styles
	ClassDB::bind_method(D_METHOD("set_fill_color", "color"), &CanvasIty::set_fill_color);
	ClassDB::bind_method(D_METHOD("set_stroke_color", "color"), &CanvasIty::set_stroke_color);
	ClassDB::bind_method(D_METHOD("set_fill_linear_gradient", "start", "end"), &CanvasIty::set_fill_linear_gradient);
	ClassDB::bind_method(D_METHOD("set_stroke_linear_gradient", "start", "end"), &CanvasIty::set_stroke_linear_gradient);
	ClassDB::bind_method(D_METHOD("set_fill_radial_gradient", "start", "start_radius", "end", "end_radius"), &CanvasIty::set_fill_radial_gradient);
	ClassDB::bind_method(D_METHOD("set_stroke_radial_gradient", "start", "start_radius", "end", "end_radius"), &CanvasIty::set_stroke_radial_gradient);
	ClassDB::bind_method(D_METHOD("add_fill_color_stop", "offset", "color"), &CanvasIty::add_fill_color_stop);
	ClassDB::bind_method(D_METHOD("add_stroke_color_stop", "offset", "color"), &CanvasIty::add_stroke_color_stop);
	ClassDB::bind_method(D_METHOD("set_fill_pattern", "image", "repetition"), &CanvasIty::set_fill_pattern);
	ClassDB::bind_method(D_METHOD("set_stroke_pattern", "image", "repetition"), &CanvasIty::set_stroke_pattern);

	// Path building
	ClassDB::bind_method(D_METHOD("begin_path"), &CanvasIty::begin_path);
	ClassDB::bind_method(D_METHOD("move_to", "pos"), &CanvasIty::move_to);
	ClassDB::bind_method(D_METHOD("close_path"), &CanvasIty::close_path);
	ClassDB::bind_method(D_METHOD("line_to", "pos"), &CanvasIty::line_to);
	ClassDB::bind_method(D_METHOD("quadratic_curve_to", "control", "end"), &CanvasIty::quadratic_curve_to);
	ClassDB::bind_method(D_METHOD("bezier_curve_to", "cp1", "cp2", "end"), &CanvasIty::bezier_curve_to);
	ClassDB::bind_method(D_METHOD("arc_to", "vertex", "point", "radius"), &CanvasIty::arc_to);
	ClassDB::bind_method(D_METHOD("arc", "center", "radius", "start_angle", "end_angle", "ccw"), &CanvasIty::arc, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("path_rectangle", "rect"), &CanvasIty::path_rectangle);

	// Drawing
	ClassDB::bind_method(D_METHOD("fill"), &CanvasIty::fill);
	ClassDB::bind_method(D_METHOD("stroke"), &CanvasIty::stroke);
	ClassDB::bind_method(D_METHOD("clip"), &CanvasIty::clip);
	ClassDB::bind_method(D_METHOD("is_point_in_path", "point"), &CanvasIty::is_point_in_path);

	// Rectangle drawing
	ClassDB::bind_method(D_METHOD("clear_rectangle", "rect"), &CanvasIty::clear_rectangle);
	ClassDB::bind_method(D_METHOD("fill_rectangle", "rect"), &CanvasIty::fill_rectangle);
	ClassDB::bind_method(D_METHOD("stroke_rectangle", "rect"), &CanvasIty::stroke_rectangle);

	// Text
	ClassDB::bind_method(D_METHOD("set_text_align", "align"), &CanvasIty::set_text_align);
	ClassDB::bind_method(D_METHOD("get_text_align"), &CanvasIty::get_text_align);
	ClassDB::bind_method(D_METHOD("set_text_baseline", "baseline"), &CanvasIty::set_text_baseline);
	ClassDB::bind_method(D_METHOD("get_text_baseline"), &CanvasIty::get_text_baseline);
	ClassDB::bind_method(D_METHOD("set_font_from_data", "data", "size"), &CanvasIty::set_font_from_data);
	ClassDB::bind_method(D_METHOD("set_font_size", "size"), &CanvasIty::set_font_size);
	ClassDB::bind_method(D_METHOD("fill_text", "text", "pos", "max_width"), &CanvasIty::fill_text, DEFVAL(-1.0f));
	ClassDB::bind_method(D_METHOD("stroke_text", "text", "pos", "max_width"), &CanvasIty::stroke_text, DEFVAL(-1.0f));
	ClassDB::bind_method(D_METHOD("measure_text", "text"), &CanvasIty::measure_text);

	// Images
	ClassDB::bind_method(D_METHOD("draw_canvas_image", "image", "pos", "size"), &CanvasIty::draw_canvas_image);
	ClassDB::bind_method(D_METHOD("put_image_data", "image", "pos"), &CanvasIty::put_image_data);

	// Enums
	BIND_ENUM_CONSTANT(COMPOSITE_SOURCE_IN);
	BIND_ENUM_CONSTANT(COMPOSITE_SOURCE_COPY);
	BIND_ENUM_CONSTANT(COMPOSITE_SOURCE_OUT);
	BIND_ENUM_CONSTANT(COMPOSITE_DESTINATION_IN);
	BIND_ENUM_CONSTANT(COMPOSITE_DESTINATION_ATOP);
	BIND_ENUM_CONSTANT(COMPOSITE_LIGHTER);
	BIND_ENUM_CONSTANT(COMPOSITE_DESTINATION_OVER);
	BIND_ENUM_CONSTANT(COMPOSITE_DESTINATION_OUT);
	BIND_ENUM_CONSTANT(COMPOSITE_SOURCE_ATOP);
	BIND_ENUM_CONSTANT(COMPOSITE_SOURCE_OVER);
	BIND_ENUM_CONSTANT(COMPOSITE_EXCLUSIVE_OR);

	BIND_ENUM_CONSTANT(CAP_BUTT);
	BIND_ENUM_CONSTANT(CAP_SQUARE);
	BIND_ENUM_CONSTANT(CAP_CIRCLE);

	BIND_ENUM_CONSTANT(JOIN_MITER);
	BIND_ENUM_CONSTANT(JOIN_BEVEL);
	BIND_ENUM_CONSTANT(JOIN_ROUND);

	BIND_ENUM_CONSTANT(REPEAT_BOTH);
	BIND_ENUM_CONSTANT(REPEAT_X);
	BIND_ENUM_CONSTANT(REPEAT_Y);
	BIND_ENUM_CONSTANT(REPEAT_NONE);

	BIND_ENUM_CONSTANT(ALIGN_LEFT);
	BIND_ENUM_CONSTANT(ALIGN_RIGHT);
	BIND_ENUM_CONSTANT(ALIGN_CENTER);

	BIND_ENUM_CONSTANT(BASELINE_ALPHABETIC);
	BIND_ENUM_CONSTANT(BASELINE_TOP);
	BIND_ENUM_CONSTANT(BASELINE_MIDDLE);
	BIND_ENUM_CONSTANT(BASELINE_BOTTOM);
	BIND_ENUM_CONSTANT(BASELINE_HANGING);
}

// ============================================================
// CanvasIty2D (Node2D)
// ============================================================

CanvasIty2D::CanvasIty2D() {
	canvas_size = Size2(256, 256);
	centered = true;
}

void CanvasIty2D::set_canvas_size(const Size2 &p_size) {
	if (canvas_size == p_size) {
		return;
	}
	canvas_size = p_size;
	if (canvas_ity.is_valid()) {
		canvas_ity->initialize((int)canvas_size.width, (int)canvas_size.height);
	}
	update();
}

Size2 CanvasIty2D::get_canvas_size() const {
	return canvas_size;
}

void CanvasIty2D::set_centered(bool p_centered) {
	centered = p_centered;
	update();
}

bool CanvasIty2D::is_centered() const {
	return centered;
}

Ref<CanvasIty> CanvasIty2D::get_canvas_ity() {
	if (canvas_ity.is_null()) {
		canvas_ity.instance();
		canvas_ity->initialize((int)canvas_size.width, (int)canvas_size.height);
	}
	return canvas_ity;
}

void CanvasIty2D::refresh() {
	update();
}

void CanvasIty2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (canvas_ity.is_null()) {
				return;
			}
			Ref<ImageTexture> tex = canvas_ity->get_texture();
			if (tex.is_null()) {
				return;
			}
			Point2 offset;
			if (centered) {
				offset = -canvas_size * 0.5;
			}
			draw_texture(tex, offset);
		} break;
	}
}

#ifdef TOOLS_ENABLED
Dictionary CanvasIty2D::_edit_get_state() const {
	Dictionary state;
	state["offset"] = get_position();
	return state;
}

void CanvasIty2D::_edit_set_state(const Dictionary &p_state) {
	set_position(p_state["offset"]);
}

bool CanvasIty2D::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	Rect2 rect = _edit_get_rect();
	return rect.has_point(p_point);
}

Rect2 CanvasIty2D::_edit_get_rect() const {
	if (centered) {
		return Rect2(-canvas_size * 0.5, canvas_size);
	}
	return Rect2(Point2(), canvas_size);
}

void CanvasIty2D::_edit_set_rect(const Rect2 &p_rect) {
	set_position(p_rect.position + (centered ? canvas_size * 0.5 : Size2()));
	canvas_size = p_rect.size;
}

bool CanvasIty2D::_edit_use_rect() const {
	return true;
}
#endif

void CanvasIty2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_canvas_size", "size"), &CanvasIty2D::set_canvas_size);
	ClassDB::bind_method(D_METHOD("get_canvas_size"), &CanvasIty2D::get_canvas_size);
	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &CanvasIty2D::set_centered);
	ClassDB::bind_method(D_METHOD("is_centered"), &CanvasIty2D::is_centered);
	ClassDB::bind_method(D_METHOD("get_canvas_ity"), &CanvasIty2D::get_canvas_ity);
	ClassDB::bind_method(D_METHOD("refresh"), &CanvasIty2D::refresh);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "canvas_size"), "set_canvas_size", "get_canvas_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "centered"), "set_centered", "is_centered");
}

// ============================================================
// Doctests
// ============================================================

TEST_SUITE("canvasraster") {
	TEST_CASE("[CanvasIty] Lifecycle - initialize and dimensions") {
		Ref<CanvasIty> c;
		c.instance();
		CHECK(c->get_width() == 0);
		CHECK(c->get_height() == 0);

		c->initialize(100, 80);
		CHECK(c->get_width() == 100);
		CHECK(c->get_height() == 80);

		// Reinitialize with new size
		c->initialize(200, 150);
		CHECK(c->get_width() == 200);
		CHECK(c->get_height() == 150);
	}

	TEST_CASE("[CanvasIty] Lifecycle - get_image returns valid RGBA8 image") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 32);

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		CHECK(img->get_width() == 64);
		CHECK(img->get_height() == 32);
		CHECK(img->get_format() == Image::FORMAT_RGBA8);

		// Freshly initialized canvas should be transparent black
		img->lock();
		Color pixel = img->get_pixel(0, 0);
		CHECK(pixel.r == doctest::Approx(0.0f));
		CHECK(pixel.g == doctest::Approx(0.0f));
		CHECK(pixel.b == doctest::Approx(0.0f));
		CHECK(pixel.a == doctest::Approx(0.0f));
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Lifecycle - get_texture returns valid texture") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(32, 32);

		Ref<ImageTexture> tex = c->get_texture();
		REQUIRE(tex.is_valid());
		CHECK(tex->get_width() == 32);
		CHECK(tex->get_height() == 32);
	}

	TEST_CASE("[CanvasIty] Fill rectangle draws colored pixels") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		// Fill entire canvas with red
		c->set_fill_color(Color(1, 0, 0, 1));
		c->fill_rectangle(Rect2(0, 0, 64, 64));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color center = img->get_pixel(32, 32);
		// canvas_ity uses gamma-correct blending and dithering, so values may
		// not be exactly 1.0/0.0 but should be very close
		CHECK(center.r > 0.9f);
		CHECK(center.g < 0.1f);
		CHECK(center.b < 0.1f);
		CHECK(center.a > 0.9f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Clear rectangle resets to transparent") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		// Fill with blue, then clear a region
		c->set_fill_color(Color(0, 0, 1, 1));
		c->fill_rectangle(Rect2(0, 0, 64, 64));
		c->clear_rectangle(Rect2(10, 10, 20, 20));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Cleared area should be transparent
		Color cleared = img->get_pixel(20, 20);
		CHECK(cleared.a < 0.1f);
		// Non-cleared area should still be blue
		Color blue = img->get_pixel(5, 5);
		CHECK(blue.b > 0.9f);
		CHECK(blue.a > 0.9f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Stroke rectangle draws outline") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_stroke_color(Color(0, 1, 0, 1));
		c->set_line_width(2.0f);
		c->stroke_rectangle(Rect2(10, 10, 44, 44));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Edge pixel should have green
		Color edge = img->get_pixel(10, 10);
		CHECK(edge.g > 0.5f);
		CHECK(edge.a > 0.5f);
		// Center should be transparent (outline only)
		Color center = img->get_pixel(32, 32);
		CHECK(center.a < 0.1f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Path building and stroke") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_stroke_color(Color(1, 1, 1, 1));
		c->set_line_width(3.0f);

		c->begin_path();
		c->move_to(Vector2(0, 32));
		c->line_to(Vector2(64, 32));
		c->stroke();

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Middle of the horizontal line should be white
		Color mid = img->get_pixel(32, 32);
		CHECK(mid.r > 0.8f);
		CHECK(mid.a > 0.8f);
		// Far from line should be transparent
		Color off = img->get_pixel(32, 5);
		CHECK(off.a < 0.1f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Path fill") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_fill_color(Color(1, 0, 1, 1)); // Magenta

		c->begin_path();
		c->move_to(Vector2(10, 10));
		c->line_to(Vector2(54, 10));
		c->line_to(Vector2(54, 54));
		c->line_to(Vector2(10, 54));
		c->close_path();
		c->fill();

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color inside = img->get_pixel(32, 32);
		CHECK(inside.r > 0.9f);
		CHECK(inside.b > 0.9f);
		CHECK(inside.a > 0.9f);
		Color outside = img->get_pixel(2, 2);
		CHECK(outside.a < 0.1f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Bezier curve to") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_stroke_color(Color(1, 1, 0, 1));
		c->set_line_width(2.0f);

		c->begin_path();
		c->move_to(Vector2(5, 32));
		c->bezier_curve_to(Vector2(20, 5), Vector2(44, 59), Vector2(59, 32));
		c->stroke();

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		// Just verify something was drawn (non-zero alpha somewhere)
		img->lock();
		bool found_pixel = false;
		for (int y = 0; y < 64 && !found_pixel; y++) {
			for (int x = 0; x < 64 && !found_pixel; x++) {
				if (img->get_pixel(x, y).a > 0.5f) {
					found_pixel = true;
				}
			}
		}
		CHECK(found_pixel);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Arc draws circular path") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_fill_color(Color(0, 0.5, 1, 1));

		c->begin_path();
		c->arc(Vector2(32, 32), 20.0f, 0.0f, Math_PI * 2.0f);
		c->fill();

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Center of circle should be filled
		Color center = img->get_pixel(32, 32);
		CHECK(center.a > 0.9f);
		// Corner should be empty
		Color corner = img->get_pixel(2, 2);
		CHECK(corner.a < 0.1f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Quadratic curve to") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_stroke_color(Color(1, 0.5, 0, 1));
		c->set_line_width(2.0f);

		c->begin_path();
		c->move_to(Vector2(5, 50));
		c->quadratic_curve_to(Vector2(32, 5), Vector2(59, 50));
		c->stroke();

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		bool found_pixel = false;
		for (int y = 0; y < 64 && !found_pixel; y++) {
			for (int x = 0; x < 64 && !found_pixel; x++) {
				if (img->get_pixel(x, y).a > 0.5f) {
					found_pixel = true;
				}
			}
		}
		CHECK(found_pixel);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] is_point_in_path") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->begin_path();
		c->path_rectangle(Rect2(10, 10, 44, 44));

		CHECK(c->is_point_in_path(Vector2(32, 32)) == true);
		CHECK(c->is_point_in_path(Vector2(2, 2)) == false);
	}

	TEST_CASE("[CanvasIty] Global alpha affects opacity") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_global_alpha(0.5f);
		c->set_fill_color(Color(1, 1, 1, 1));
		c->fill_rectangle(Rect2(0, 0, 64, 64));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color pixel = img->get_pixel(32, 32);
		// Alpha should be approximately 0.5
		CHECK(pixel.a > 0.3f);
		CHECK(pixel.a < 0.7f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Line cap and join getters/setters") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(32, 32);

		c->set_line_cap(CanvasIty::CAP_CIRCLE);
		CHECK(c->get_line_cap() == CanvasIty::CAP_CIRCLE);

		c->set_line_join(CanvasIty::JOIN_ROUND);
		CHECK(c->get_line_join() == CanvasIty::JOIN_ROUND);

		c->set_line_cap(CanvasIty::CAP_SQUARE);
		CHECK(c->get_line_cap() == CanvasIty::CAP_SQUARE);

		c->set_line_join(CanvasIty::JOIN_BEVEL);
		CHECK(c->get_line_join() == CanvasIty::JOIN_BEVEL);
	}

	TEST_CASE("[CanvasIty] Composite operation getter/setter") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(32, 32);

		c->set_composite_operation(CanvasIty::COMPOSITE_LIGHTER);
		CHECK(c->get_composite_operation() == CanvasIty::COMPOSITE_LIGHTER);

		c->set_composite_operation(CanvasIty::COMPOSITE_SOURCE_OVER);
		CHECK(c->get_composite_operation() == CanvasIty::COMPOSITE_SOURCE_OVER);
	}

	TEST_CASE("[CanvasIty] Shadow offset getter/setter") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(32, 32);

		c->set_shadow_offset(Vector2(5.0f, -3.0f));
		Vector2 offset = c->get_shadow_offset();
		CHECK(offset.x == doctest::Approx(5.0f));
		CHECK(offset.y == doctest::Approx(-3.0f));
	}

	TEST_CASE("[CanvasIty] Line dash offset getter/setter") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(32, 32);

		c->set_line_dash_offset(7.5f);
		CHECK(c->get_line_dash_offset() == doctest::Approx(7.5f));
	}

	TEST_CASE("[CanvasIty] Text align and baseline getters/setters") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(32, 32);

		c->set_text_align(CanvasIty::ALIGN_CENTER);
		CHECK(c->get_text_align() == CanvasIty::ALIGN_CENTER);

		c->set_text_baseline(CanvasIty::BASELINE_MIDDLE);
		CHECK(c->get_text_baseline() == CanvasIty::BASELINE_MIDDLE);
	}

	TEST_CASE("[CanvasIty] Save/restore preserves state") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_global_alpha(0.5f);
		c->save();
		c->set_global_alpha(1.0f);

		// Draw with full alpha
		c->set_fill_color(Color(1, 0, 0, 1));
		c->fill_rectangle(Rect2(0, 0, 32, 64));

		c->restore();

		// Draw with restored 0.5 alpha
		c->set_fill_color(Color(0, 0, 1, 1));
		c->fill_rectangle(Rect2(32, 0, 32, 64));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color left = img->get_pixel(16, 32);
		Color right = img->get_pixel(48, 32);
		// Left side should be fully opaque (alpha=1.0)
		CHECK(left.a > 0.9f);
		// Right side should be semi-transparent (alpha=0.5)
		CHECK(right.a > 0.3f);
		CHECK(right.a < 0.7f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Transform - translate") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->canvas_translate(Vector2(20, 20));
		c->set_fill_color(Color(1, 1, 0, 1));
		c->fill_rectangle(Rect2(0, 0, 10, 10));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Should be at translated position
		Color at_origin = img->get_pixel(2, 2);
		CHECK(at_origin.a < 0.1f);
		Color at_translated = img->get_pixel(25, 25);
		CHECK(at_translated.a > 0.9f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Transform - reset") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->canvas_translate(Vector2(50, 50));
		c->reset_transform();

		c->set_fill_color(Color(1, 0, 0, 1));
		c->fill_rectangle(Rect2(0, 0, 10, 10));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// After reset, should be at origin
		Color at_origin = img->get_pixel(5, 5);
		CHECK(at_origin.a > 0.9f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Linear gradient fill") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		c->set_fill_linear_gradient(Vector2(0, 0), Vector2(64, 0));
		c->add_fill_color_stop(0.0f, Color(1, 0, 0, 1));
		c->add_fill_color_stop(1.0f, Color(0, 0, 1, 1));
		c->fill_rectangle(Rect2(0, 0, 64, 64));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color left = img->get_pixel(3, 32);
		Color right = img->get_pixel(60, 32);
		// Left should be red-ish
		CHECK(left.r > left.b);
		// Right should be blue-ish
		CHECK(right.b > right.r);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Line dash pattern") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(100, 20);

		c->set_stroke_color(Color(1, 1, 1, 1));
		c->set_line_width(2.0f);

		PoolRealArray dash;
		dash.push_back(10.0f);
		dash.push_back(10.0f);
		c->set_line_dash(dash);

		c->begin_path();
		c->move_to(Vector2(0, 10));
		c->line_to(Vector2(100, 10));
		c->stroke();

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Check that there are both filled and empty segments
		Color dash_pixel = img->get_pixel(5, 10);
		Color gap_pixel = img->get_pixel(15, 10);
		// One should be more opaque than the other
		CHECK(dash_pixel.a != doctest::Approx(gap_pixel.a).epsilon(0.3f));
		img->unlock();
	}

	TEST_CASE("[CanvasIty] put_image_data writes pixels") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(32, 32);

		// Create a small test image
		Ref<Image> src;
		src.instance();
		src->create(8, 8, false, Image::FORMAT_RGBA8);
		src->lock();
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				src->set_pixel(x, y, Color(0, 1, 0, 1));
			}
		}
		src->unlock();

		c->put_image_data(src, Vector2(4, 4));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Pixel at (8, 8) should be green (from put_image_data at offset 4,4)
		Color green = img->get_pixel(8, 8);
		CHECK(green.g > 0.9f);
		CHECK(green.a > 0.9f);
		// Pixel at (0, 0) should be transparent (untouched)
		Color empty = img->get_pixel(0, 0);
		CHECK(empty.a < 0.1f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Multiple fills with different colors") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		// Red left half
		c->set_fill_color(Color(1, 0, 0, 1));
		c->fill_rectangle(Rect2(0, 0, 32, 64));

		// Blue right half
		c->set_fill_color(Color(0, 0, 1, 1));
		c->fill_rectangle(Rect2(32, 0, 32, 64));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color left = img->get_pixel(16, 32);
		Color right = img->get_pixel(48, 32);
		CHECK(left.r > 0.9f);
		CHECK(left.b < 0.1f);
		CHECK(right.b > 0.9f);
		CHECK(right.r < 0.1f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] _ensure_ctx auto-initializes") {
		Ref<CanvasIty> c;
		c.instance();
		// Don't call initialize - methods should auto-init via _ensure_ctx
		c->set_fill_color(Color(1, 1, 1, 1));
		c->fill_rectangle(Rect2(0, 0, 100, 100));

		CHECK(c->get_width() == 256);
		CHECK(c->get_height() == 256);

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color pixel = img->get_pixel(50, 50);
		CHECK(pixel.a > 0.9f);
		img->unlock();
	}

	TEST_CASE("[CanvasIty] Clip restricts drawing") {
		Ref<CanvasIty> c;
		c.instance();
		c->initialize(64, 64);

		// Set clip to left half only
		c->begin_path();
		c->path_rectangle(Rect2(0, 0, 32, 64));
		c->clip();

		// Fill entire canvas with white
		c->set_fill_color(Color(1, 1, 1, 1));
		c->fill_rectangle(Rect2(0, 0, 64, 64));

		Ref<Image> img = c->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color left = img->get_pixel(16, 32);
		Color right = img->get_pixel(48, 32);
		CHECK(left.a > 0.9f);
		CHECK(right.a < 0.1f);
		img->unlock();
	}
}
