/**************************************************************************/
/*  thumb_wheel.h                                                         */
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

#ifndef THUMB_WHEEL_H
#define THUMB_WHEEL_H

#include "core/engine.h"
#include "core/os/os.h"
#include "scene/gui/control.h"
#include "scene/main/viewport.h"

enum {
	OrientationHorizontal,
	OrientationVerical,
};

struct ThumbWheelState {
	bool active;
	Vector2 click_pos;
	real_t base_value;
};

class ThumbWheelH : public Control {
	GDCLASS(ThumbWheelH, Control);

	real_t value;
	real_t resolution;
	bool disabled;
	String label;

	ThumbWheelState _state;

	bool _is_point_inside(const Point2 &point) const;
	Point2 _to_local(Point2 global) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);
	virtual void _gui_input(Ref<InputEvent> p_event);

public:
	void set_disabled(bool p_enabled);
	bool is_disabled() const;
	void set_resolution(real_t p_res);
	real_t get_resolution() const;
	void set_label(String p_label);
	String get_label() const;

	ThumbWheelH();
	~ThumbWheelH() {}
};

class ThumbWheelV : public Control {
	GDCLASS(ThumbWheelV, Control);

	real_t value;
	real_t resolution;
	bool disabled;
	String label;

	ThumbWheelState _state;

	bool _is_point_inside(const Point2 &point) const;
	Point2 _to_local(Point2 global) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);
	virtual void _gui_input(Ref<InputEvent> p_event);

public:
	void set_disabled(bool p_enabled);
	bool is_disabled() const;
	void set_resolution(real_t p_res);
	real_t get_resolution() const;
	void set_label(String p_label);
	String get_label() const;

	ThumbWheelV();
	~ThumbWheelV() {}
};

#endif // THUMB_WHEEL_H
