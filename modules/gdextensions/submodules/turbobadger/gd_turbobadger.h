/**************************************************************************/
/*  gd_turbobadger.h                                                      */
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

#ifndef GD_TURBOBADGER_H
#define GD_TURBOBADGER_H

#include "core/os/input_event.h"
#include "scene/2d/node_2d.h"
#include "scene/main/node.h"

#include "renderers/tb_renderer_gd.h"
#include "tb_widgets.h"

// The root of widgets in a platform backend.

class AppRootWidget : public tb::TBWidget {
	CanvasItem *_canvas;
	int mouse_x, mouse_y;

public:
	// For safe typecasting
	TBOBJECT_SUBCLASS(AppRootWidget, tb::TBWidget);

	void OnInvalid() { _canvas->update(); }

	Point2 ToLocal(const Point2 &pt) const { return _canvas->get_global_transform().affine_inverse().xform(pt); }
	void SetCursorPos(int mx, int my) {
		mouse_x = mx;
		mouse_y = my;
	}
	Point2 GetCursorPos() const { return Point2(mouse_x, mouse_y); }

	AppRootWidget(CanvasItem *canvas) :
			_canvas(canvas) {}
};

// Godot Node / application interface and renderer

class GdTurboBadgerCore : public Object {
	GDCLASS(GdTurboBadgerCore, Object);

	tb::TBRendererGD renderer;

	void _timer_callback();

	GdTurboBadgerCore();
	~GdTurboBadgerCore();

	int _ref;

protected:
	static void _bind_methods();

public:
	static GdTurboBadgerCore *get_singleton();

	void init();
	void release();

	tb::TBRendererGD *get_renderer() { return &renderer; }
};

class TBRootWidget : public tb::TBWidget, public Node2D {
	GDCLASS(TBRootWidget, Node2D);

	Size2 view_size;
	int mouse_x, mouse_y;

	void set_cursor_pos(int mx, int my) {
		mouse_x = mx;
		mouse_y = my;
	}
	Point2 get_cursor_pos() const { return Point2(mouse_x, mouse_y); }

	bool _dirty;

protected:
	static void _bind_methods();
	void notifications(int p_what);

	void _input(const Ref<InputEvent> &p_event);

public:
	TBOBJECT_SUBCLASS(AppRootWidget, tb::TBWidget); // For safe typecasting

	void OnInvalid() { update(); }

	void set_view_size(const Size2 &p_size);
	Size2 get_view_size() const;

	TBRootWidget();
};

class GdTurboBadger : public Node {
	GDCLASS(GdTurboBadger, Node);

	TBRootWidget *root;

protected:
	static void _bind_methods();
	void notifications(int p_what);

public:
#ifdef TOOLS_ENABLED
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
#endif

	void set_view_size(const Size2 &p_size);
	Size2 get_view_size() const;

	GdTurboBadger();
	~GdTurboBadger();
};

#endif // GD_TURBOBADGER_H
