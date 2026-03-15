/**************************************************************************/
/*  canvas_raster.h                                                       */
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

#ifndef CANVAS_RASTER_H
#define CANVAS_RASTER_H

#include "core/image.h"
#include "core/reference.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

namespace canvas_ity {
class canvas;
}

class CanvasIty : public Reference {
	GDCLASS(CanvasIty, Reference);

public:
	enum CompositeOp {
		COMPOSITE_SOURCE_IN = 1,
		COMPOSITE_SOURCE_COPY = 2,
		COMPOSITE_SOURCE_OUT = 3,
		COMPOSITE_DESTINATION_IN = 4,
		COMPOSITE_DESTINATION_ATOP = 7,
		COMPOSITE_LIGHTER = 10,
		COMPOSITE_DESTINATION_OVER = 11,
		COMPOSITE_DESTINATION_OUT = 12,
		COMPOSITE_SOURCE_ATOP = 13,
		COMPOSITE_SOURCE_OVER = 14,
		COMPOSITE_EXCLUSIVE_OR = 15,
	};

	enum CapStyle {
		CAP_BUTT = 0,
		CAP_SQUARE = 1,
		CAP_CIRCLE = 2,
	};

	enum JoinStyle {
		JOIN_MITER = 0,
		JOIN_BEVEL = 1,
		JOIN_ROUND = 2,
	};

	enum RepetitionStyle {
		REPEAT_BOTH = 0,
		REPEAT_X = 1,
		REPEAT_Y = 2,
		REPEAT_NONE = 3,
	};

	enum TextAlign {
		ALIGN_LEFT = 0,
		ALIGN_RIGHT = 1,
		ALIGN_CENTER = 2,
		ALIGN_START = 0,
		ALIGN_END = 1,
	};

	enum TextBaseline {
		BASELINE_ALPHABETIC = 0,
		BASELINE_TOP = 1,
		BASELINE_MIDDLE = 2,
		BASELINE_BOTTOM = 3,
		BASELINE_HANGING = 4,
		BASELINE_IDEOGRAPHIC = 3,
	};

private:
	canvas_ity::canvas *ctx;
	int canvas_width;
	int canvas_height;
	bool dirty;
	Ref<Image> cached_image;
	Ref<ImageTexture> cached_texture;
	PoolByteArray font_data;

	void _ensure_ctx();

protected:
	static void _bind_methods();

public:
	// Lifecycle
	void initialize(int p_width, int p_height);
	int get_width() const;
	int get_height() const;
	Ref<Image> get_image();
	Ref<ImageTexture> get_texture();

	// State
	void save();
	void restore();

	// Transforms
	void canvas_scale(const Vector2 &p_factor);
	void canvas_rotate(float p_angle);
	void canvas_translate(const Vector2 &p_offset);
	void append_transform(const Transform2D &p_xform);
	void set_canvas_transform(const Transform2D &p_xform);
	void reset_transform();

	// Compositing
	void set_global_alpha(float p_alpha);
	void set_composite_operation(int p_op);
	int get_composite_operation() const;

	// Shadows
	void set_shadow_color(const Color &p_color);
	void set_shadow_offset(const Vector2 &p_offset);
	Vector2 get_shadow_offset() const;
	void set_shadow_blur(float p_level);

	// Line styles
	void set_line_width(float p_width);
	void set_line_cap(int p_cap);
	int get_line_cap() const;
	void set_line_join(int p_join);
	int get_line_join() const;
	void set_miter_limit(float p_limit);
	void set_line_dash_offset(float p_offset);
	float get_line_dash_offset() const;
	void set_line_dash(const PoolRealArray &p_segments);

	// Fill/Stroke styles
	void set_fill_color(const Color &p_color);
	void set_stroke_color(const Color &p_color);
	void set_fill_linear_gradient(const Vector2 &p_start, const Vector2 &p_end);
	void set_stroke_linear_gradient(const Vector2 &p_start, const Vector2 &p_end);
	void set_fill_radial_gradient(const Vector2 &p_start, float p_start_r, const Vector2 &p_end, float p_end_r);
	void set_stroke_radial_gradient(const Vector2 &p_start, float p_start_r, const Vector2 &p_end, float p_end_r);
	void add_fill_color_stop(float p_offset, const Color &p_color);
	void add_stroke_color_stop(float p_offset, const Color &p_color);
	void set_fill_pattern(const Ref<Image> &p_image, int p_repetition);
	void set_stroke_pattern(const Ref<Image> &p_image, int p_repetition);

	// Path building
	void begin_path();
	void move_to(const Vector2 &p_pos);
	void close_path();
	void line_to(const Vector2 &p_pos);
	void quadratic_curve_to(const Vector2 &p_control, const Vector2 &p_end);
	void bezier_curve_to(const Vector2 &p_cp1, const Vector2 &p_cp2, const Vector2 &p_end);
	void arc_to(const Vector2 &p_vertex, const Vector2 &p_point, float p_radius);
	void arc(const Vector2 &p_center, float p_radius, float p_start_angle, float p_end_angle, bool p_ccw = false);
	void path_rectangle(const Rect2 &p_rect);

	// Drawing
	void fill();
	void stroke();
	void clip();
	bool is_point_in_path(const Vector2 &p_point);

	// Rectangle drawing
	void clear_rectangle(const Rect2 &p_rect);
	void fill_rectangle(const Rect2 &p_rect);
	void stroke_rectangle(const Rect2 &p_rect);

	// Text
	void set_text_align(int p_align);
	int get_text_align() const;
	void set_text_baseline(int p_baseline);
	int get_text_baseline() const;
	bool set_font_from_data(const PoolByteArray &p_data, float p_size);
	bool set_font_size(float p_size);
	void fill_text(const String &p_text, const Vector2 &p_pos, float p_max_width = -1.0f);
	void stroke_text(const String &p_text, const Vector2 &p_pos, float p_max_width = -1.0f);
	float measure_text(const String &p_text);

	// Images
	void draw_canvas_image(const Ref<Image> &p_image, const Vector2 &p_pos, const Vector2 &p_size);
	void put_image_data(const Ref<Image> &p_image, const Vector2 &p_pos);

	CanvasIty();
	~CanvasIty();
};

VARIANT_ENUM_CAST(CanvasIty::CompositeOp);
VARIANT_ENUM_CAST(CanvasIty::CapStyle);
VARIANT_ENUM_CAST(CanvasIty::JoinStyle);
VARIANT_ENUM_CAST(CanvasIty::RepetitionStyle);
VARIANT_ENUM_CAST(CanvasIty::TextAlign);
VARIANT_ENUM_CAST(CanvasIty::TextBaseline);

class CanvasIty2D : public Node2D {
	GDCLASS(CanvasIty2D, Node2D);

	Size2 canvas_size;
	bool centered;
	Ref<CanvasIty> canvas_ity;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;
#endif

	void set_canvas_size(const Size2 &p_size);
	Size2 get_canvas_size() const;
	void set_centered(bool p_centered);
	bool is_centered() const;
	Ref<CanvasIty> get_canvas_ity();
	void refresh();

	CanvasIty2D();
};

#endif // CANVAS_RASTER_H
