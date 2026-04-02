/**************************************************************************/
/*  gdimspinner.h                                                         */
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

#ifndef GDSPINNER_H
#define GDSPINNER_H

#include "scene/2d/node_2d.h"

// Forward declarations
struct ImGuiWindow;
struct SpinnerConfig;
struct SpinnerAnimState;

class Spinner : public Node2D {
	GDCLASS(Spinner, Node2D);

	ImGuiWindow *_imgui_wnd;

	int spinner_variant;
	bool spinner_active;

	// Configurable parameters (exposed to GDScript)
	real_t _radius;
	real_t _thickness;
	real_t _speed;
	Color _color;
	Color _bg_color;
	real_t _angle;
	int _dots;
	int _mode;

	// Animation state
	SpinnerAnimState *_anim;

	void _draw_spinner();

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	void set_spinner_active(bool p_active);
	bool get_spinner_active() const;

	void set_spinner_variant(int p_variant);
	int get_spinner_variant() const;

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	void set_thickness(real_t p_thickness);
	real_t get_thickness() const;

	void set_speed(real_t p_speed);
	real_t get_speed() const;

	void set_color(const Color &p_color);
	Color get_color() const;

	void set_bg_color(const Color &p_color);
	Color get_bg_color() const;

	void set_angle(real_t p_angle);
	real_t get_angle() const;

	void set_dots(int p_dots);
	int get_dots() const;

	void set_mode(int p_mode);
	int get_mode() const;

	int get_variant_count() const;
	String get_variant_name(int p_variant) const;
	String get_spinner_name() const;

	Spinner();
	~Spinner();
};

#endif // GDSPINNER_H
