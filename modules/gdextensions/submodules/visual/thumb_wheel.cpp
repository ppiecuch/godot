#include "thumb_wheel.h"

#include "core/math/math_funcs.h"
#include "servers/visual_server.h"

static void _draw_wheel(CanvasItem *c, const Rect2 &r, int offset, bool disabled, int orientation) {
	static int GRAY33 = 84;
	static int GRAY60 = 153;
	static int GRAY75 = 192;
	static int GRAY85 = 217;
	static int GRAY90 = 229;

	const real_t ARC = 1.5; // 1/2 the number of radians visible
	const real_t delta = .2; // radians per knurl

#define _G(v) {(v)/255.f,(v)/255.f,(v)/255.f,1.f}

	if (orientation == OrientationHorizontal) {
		// draw shaded ends of wheel:
		int h1 = r.size.width/4 + 1; // distance from end that shading starts
		c->draw_rect(Rect2(r.left()+h1, r.top(), r.size.width-2*h1, r.size.height), GLOBAL_GET("rendering/environment/default_clear_color"), true);
		for (int i = 0; h1; i++) {
			int h2 = GRAY75-i-1 > GRAY33 ? 2 * h1/3 + 1 : 0;
			c->draw_rect(Rect2(r.left()+h2, r.top(), h1-h2, r.size.height), _G(GRAY75-i-1), true);
			c->draw_rect(Rect2(r.right()-h1, r.top(), h1-h2, r.size.height), _G(GRAY75-i-1), true);
			h1 = h2;
		}
		if (!disabled) {
			// draw ridges:
			real_t junk;
			for (real_t y = -ARC+Math::modf(offset*Math::sin(ARC)/(r.size.width/2)/delta, &junk)*delta;; y += delta) {
				int y1 = int((Math::sin(y)/Math::sin(ARC)+1) * r.size.width/2);
				if (y1 <= 0) continue; else if (y1 >= r.size.width-1) break;
				c->draw_line(Point2(r.left()+y1, r.top()+1), Point2(r.left()+y1, r.bottom()-1), _G(GRAY33));
				if (y < 0) y1--; else y1++;
				c->draw_line(Point2(r.left()+y1, r.top()+1), Point2(r.left()+y1, r.bottom()-1), _G(GRAY85));
			}
			// draw edges:
			h1 = r.size.width/8 + 1; // distance from end the color inverts
			c->draw_line(Point2(r.left()+h1, r.bottom()-1), Point2(r.right()-h1, r.bottom()-1), _G(GRAY60));
			c->draw_line(Point2(r.left(), r.bottom()), Point2(r.left(), r.top()), _G(GRAY33));
			c->draw_line(Point2(r.left(), r.top()), Point2(r.left()+h1, r.top()), _G(GRAY33));
			c->draw_line(Point2(r.bottom()-h1, r.top()), Point2(r.right(), r.top()), _G(GRAY33));
			c->draw_line(Point2(r.left()+h1, r.top()), Point2(r.right()-h1, r.top()), _G(GRAY90));
			c->draw_line(Point2(r.right(), r.top()), Point2(r.right(), r.bottom()), _G(GRAY90));
			c->draw_line(Point2(r.right(), r.bottom()), Point2(r.right()-h1, r.bottom()), _G(GRAY90));
			c->draw_line(Point2(r.left()+h1, r.bottom()), Point2(r.left(), r.bottom()), _G(GRAY90));
		}
	} else if (orientation == OrientationVerical) {
	} else {
		WARN_PRINT("Invalid value");
	}
}

void ThumbWheelH::_notification(int p_what) {
	switch(p_what) {
		case NOTIFICATION_DRAW: {
			const Rect2 r = get_rect();
			const int offset = int(value/resolution);
			_draw_wheel(this, r, offset, disabled, _orientation);
		}
	}
}

void ThumbWheelH::_input(Ref<InputEvent> p_event) {
}

void ThumbWheelH::_gui_input(Ref<InputEvent> p_event) {
}

void ThumbWheelH::_unhandled_input(Ref<InputEvent> p_event) {
}

void ThumbWheelH::set_disabled(bool p_disabled) {
	disabled = p_disabled;
	update();
}

bool ThumbWheelH::is_disabled() const {
	return disabled;
}

void ThumbWheelH::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_disabled"), &ThumbWheelH::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &ThumbWheelH::is_disabled);

	ClassDB::bind_method(D_METHOD("_input"), &ThumbWheelH::_input);
	ClassDB::bind_method(D_METHOD("_gui_input"), &ThumbWheelH::_gui_input);
	ClassDB::bind_method(D_METHOD("_unhandled_input"), &ThumbWheelH::_unhandled_input);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_disabled", "is_disabled");

	ADD_SIGNAL(MethodInfo("transformation_changed", PropertyInfo(Variant::TRANSFORM, "tr")));
}

ThumbWheelH::ThumbWheelH() {
	disabled = false;
}
