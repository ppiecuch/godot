/**************************************************************************/
/*  widget_controls.h                                                     */
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

#ifndef WIDGET_CONTROLS_H
#define WIDGET_CONTROLS_H

#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"
#include "scene/resources/theme.h"

enum WidgetType {
	WIDGET_TRANSLATION_XY,
	WIDGET_TRANSLATION_X,
	WIDGET_TRANSLATION_Y,
	WIDGET_TRANSLATION_Z,
	WIDGET_ROTATION_SPHERE,
};

constexpr int WIDGET_TYPES = WIDGET_ROTATION_SPHERE + 1;

class ControlWidget : public Node2D {
	GDCLASS(ControlWidget, Node2D);

	WidgetType control_type;
	Rect2 control_rect;
	bool flat, disabled;
	real_t resolution;

	Ref<StyleBoxFlat> _style;
	struct StyleInfo {
		real_t width;
		real_t radius;
		Color bg_color;
		real_t shadow_size;
		Color shadow_color;
		Vector2 shadow_offset;
	} _style_info;
	Ref<ArrayMesh> _mesh;
	Ref<Texture> _checker;

	struct {
		bool active;
		Point2 initial_pos;
		char locked;
		Vector3 from_vector, to_vector, locked_axis;
		Transform tr, base_tr;
		void rotate(const Vector3 &axis, real_t angle) { tr.rotate(axis, angle); }
		void swap() { from_vector = to_vector; }
	} _state;

	Rect2 _get_global_rect() const;
	bool _is_point_inside(const Vector2 &vec) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _input(const Ref<InputEvent> &p_event);
	void _unhandled_input(const Ref<InputEvent> &p_event);

public:
#ifdef TOOLS_ENABLED
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
#endif

	void set_control_type(WidgetType p_type);
	WidgetType get_control_type() const;

	void set_control_flat(bool p_themed);
	bool is_control_flat() const;

	void set_control_disabled(bool p_disabled);
	bool is_control_disabled() const;

	void set_control_resolution(real_t p_resolution);
	real_t get_control_resolution() const;

	ControlWidget();
};

VARIANT_ENUM_CAST(WidgetType);

#endif // WIDGET_CONTROLS_H
