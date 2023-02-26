/**************************************************************************/
/*  vgamepad.h                                                            */
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

#ifndef GD_VGAMEPAD_H
#define GD_VGAMEPAD_H

#include "core/math/vector2.h"
#include "scene/2d/node_2d.h"

class VGamePad : public Node2D {
	GDCLASS(VGamePad, Node2D);

public:
	enum VGamePadDesignHint {
		VGAMEPAD_RED_ACCENT,
	};

	enum VGamePadControls {
		VGAMEPAD_BUTTON_ROTATE_RIGHT,
		VGAMEPAD_BUTTON_ROTATE_LEFT,
		VGAMEPAD_BUTTON_ROTATE_A,
		VGAMEPAD_BUTTON_ROTATE_B,
		VGAMEPAD_BUTTON_ROTATE_X,
		VGAMEPAD_BUTTON_ROTATE_Y,
		VGAMEPAD_KEYPAD,
		VGAMEPAD_DPAD,
		VGAMEPAD_DPAD_DECOR,
		VGAMEPAD_ANALOG,
	};

private:
	bool _dirty;
	VGamePadControls controls;
	int device;

	void _debug_draw(CanvasItem *canvas);
	void _debug_draw_option_button(CanvasItem *canvas, const Point2 &position, bool is_pressed);
	void _debug_draw_dpad_arrow(CanvasItem *canvas, const Point2 &position, real_t angle, bool is_pressed);
	void _debug_draw_face_button(CanvasItem *canvas, const Point2 &position, bool is_pressed);
	void _debug_draw_bumper(CanvasItem *canvas, const Point2 &position, bool is_pressed);
	void _debug_draw_trigger(CanvasItem *canvas, const Point2 &position, real_t axis);
	void _debug_draw_joystick(CanvasItem *canvas, const Point2 &position, real_t axis_x, real_t axis_y, bool is_pressed);

protected:
	static void _bind_methods();

	void _notification(int p_what);
	void _input(const Ref<InputEvent> &p_event);

public:
	VGamePad();
	~VGamePad();
};

#endif // GD_VGAMEPAD_H
