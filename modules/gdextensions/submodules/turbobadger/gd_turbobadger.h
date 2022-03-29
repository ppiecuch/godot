/*************************************************************************/
/*  gd_turbobadger.h                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef GD_TURBOBADGER_H
#define GD_TURBOBADGER_H

#include "core/os/input_event.h"
#include "scene/2d/node_2d.h"

#include "renderers/tb_renderer_gd.h"
#include "tb_widgets.h"

// The root of widgets in a platform backend.

class AppRootWidget : public tb::TBWidget {
	Node2D *_app;
	int mouse_x, mouse_y;

public:
	// For safe typecasting
	TBOBJECT_SUBCLASS(AppRootWidget, tb::TBWidget);

	void OnInvalid() { _app->update(); }

	Node2D *GetApp() { return _app; }
	Point2 ToLocal(const Point2 &pt) const { return _app->to_local(pt); }
	void SetCursorPos(int mx, int my) {
		mouse_x = mx;
		mouse_y = my;
	}
	Point2 GetCursorPos() const { return Point2(mouse_x, mouse_y); }

	AppRootWidget(Node2D *app) :
			_app(app) {}
};

// Godot Node / application interface and renderer

class GdTurboBadgerCore : public Object {
	GDCLASS(GdTurboBadgerCore, Object);

	Ref<tb::TBRendererGD> renderer;

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

	Ref<tb::TBRendererGD> get_renderer() const { return renderer; }
};

class GdTurboBadger : public Node2D {
	GDCLASS(GdTurboBadger, Node2D);

	AppRootWidget root;
	Size2 view_size;

	bool _dirty;

protected:
	static void _bind_methods();
	void notifications(int p_what);

	void _input(const Ref<InputEvent> &p_event);

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
