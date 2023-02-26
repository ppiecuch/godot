/**************************************************************************/
/*  test_font.cpp                                                         */
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

#ifndef _3D_DISABLED

#include "test_gui.h"

#include "core/os/os.h"
#include "core/print_string.h"
#include "scene/2d/line_2d.h"
#include "scene/gui/control.h"
#include "scene/gui/label.h"
#include "scene/gui/panel.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#undef private

namespace TestFont {

Line2D *get_bounds_rect(const Rect2 &rc) {
	Line2D *bounds = memnew(Line2D);
	bounds->set_width(1.0);
	bounds->add_point(Vector2(rc.position.x, rc.position.y));
	bounds->add_point(Vector2(rc.position.x + rc.size.x, rc.position.y));
	bounds->add_point(Vector2(rc.position.x + rc.size.x, rc.position.y + rc.size.y));
	bounds->add_point(Vector2(rc.position.x, rc.position.y + rc.size.y));
	bounds->add_point(Vector2(rc.position.x, rc.position.y));
	return bounds;
}

class TestMainLoop : public SceneTree {
public:
	virtual void request_quit() {
		quit();
	}
	virtual void init() {
		SceneTree::init();

		Panel *frame = memnew(Panel);
		frame->set_anchor(MARGIN_RIGHT, Control::ANCHOR_END);
		frame->set_anchor(MARGIN_BOTTOM, Control::ANCHOR_END);
		frame->set_end(Point2(0, 0));

		Ref<Theme> t = memnew(Theme);
		frame->set_theme(t);

		get_root()->add_child(frame);

		int x_pos = 80, y_pos = 90, x_sz = 179, y_sz = 80, spc = 5;
		String text("There was once  upon a time a beautiful   \nunicorn that loved to play with little   \ngirls ...   ");

		Label *label1 = memnew(Label);
		label1->set_position(Point2(x_pos, y_pos));
		label1->set_size(Point2(x_sz, y_sz));
		label1->set_align(Label::ALIGN_LEFT);
		label1->set_text(text);

		Label *label2 = memnew(Label);
		label2->set_position(Point2(x_pos, y_pos + y_sz + spc));
		label2->set_size(Point2(x_sz, y_sz));
		label2->set_align(Label::ALIGN_RIGHT);
		label2->set_text(text);

		Label *label3 = memnew(Label);
		label3->set_position(Point2(x_pos, y_pos + (y_sz + spc) * 2));
		label3->set_size(Point2(x_sz, y_sz));
		label3->set_align(Label::ALIGN_CENTER);
		label3->set_text(text);

		frame->add_child(label1);
		frame->add_child(get_bounds_rect(Rect2(x_pos, y_pos, x_sz, y_sz)));
		frame->add_child(label2);
		frame->add_child(get_bounds_rect(Rect2(x_pos, y_pos + y_sz + spc, x_sz, y_sz)));
		frame->add_child(label3);
		frame->add_child(get_bounds_rect(Rect2(x_pos, y_pos + (y_sz + spc) * 2, x_sz, y_sz)));

		OS::get_singleton()->print("Running tests:\n");

		// 1:
		{
			int num = label1->get_line_size(label1->word_cache, 0);
			OS::get_singleton()->print("\t 9: %s\n", num == 39 ? "PASS" : "FAILED");
		}
		// 2:
		{
			int num = label1->get_line_size(label1->word_cache, 2);
			OS::get_singleton()->print("\t10: %s\n", num == 9 ? "PASS" : "FAILED");
		}
		// 3:
		{
			int num = label1->get_line_size(label1->word_cache, 10);
			OS::get_singleton()->print("\t11: %s\n", num == 0 ? "PASS" : "FAILED");
		}

		OS::get_singleton()->print("Running animation:\n");

		label1->set_transition_effect(Label::TRANSITIONEFFECT_ROTATE_V);
		label1->set_transition_duration(5);
		label1->set_text("New\nanimation transition");

		OS::get_singleton()->print("\nDONE\n");
	}
};

MainLoop *test() {
	return memnew(TestMainLoop);
}
} // namespace TestFont

#endif
