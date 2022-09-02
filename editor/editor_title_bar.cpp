/*************************************************************************/
/*  editor_title_bar.cpp                                                 */
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

#include "editor/editor_title_bar.h"
#include "core/os/os.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"

#if OSX_ENABLED
#include <objc/objc-runtime.h>

bool _is_action_on_double_click(const char *action) {
	id userdefaults = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSUserDefaults"), sel_registerName("standardUserDefaults"));
	id keystr = ((id(*)(id, SEL, id))objc_msgSend)(userdefaults, sel_registerName("stringForKey:"),
			((id(*)(id, SEL, const char *))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "AppleActionOnDoubleClick"));
	const char *ckeystr = ((char *(*)(id, SEL))objc_msgSend)(keystr, sel_registerName("UTF8String"));
	return (ckeystr != nullptr && strcmp(ckeystr, action) == 0);
}

static bool _window_maximize_on_title_dbl_click() {
	return _is_action_on_double_click("Maximize");
}

static bool _window_minimize_on_title_dbl_click() {
	return _is_action_on_double_click("Minimize");
}
#else
#define _window_maximize_on_title_dbl_click() (false)
#define _window_minimize_on_title_dbl_click() (false)
#endif

void EditorTitleBar::input(const Ref<InputEvent> &p_event) {
	if (!can_move) {
		return;
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid() && moving) {
		if (mm->get_button_mask() & BUTTON_LEFT) {
			if (Viewport *v = get_viewport()) {
				Point2 mouse = v->get_mouse_position();
				OS::get_singleton()->set_window_position(mouse - click_pos);
			}
		} else {
			moving = false;
		}
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && has_point(mb->get_position())) {
		if (Viewport *v = get_viewport()) {
			if (mb->get_button_index() == BUTTON_LEFT) {
				if (mb->is_pressed()) {
					click_pos = v->get_mouse_position() - OS::get_singleton()->get_window_position();
					moving = true;
				} else {
					moving = false;
				}
			}
			if (mb->get_button_index() == BUTTON_LEFT && mb->is_doubleclick() && mb->is_pressed()) {
				if (_window_maximize_on_title_dbl_click()) {
					if (!OS::get_singleton()->is_window_maximized() && !OS::get_singleton()->is_window_fullscreen()) {
						OS::get_singleton()->set_window_maximized(true);
					} else if (OS::get_singleton()->is_window_maximized()) {
						OS::get_singleton()->set_window_maximized(false);
					}
				} else if (_window_minimize_on_title_dbl_click()) {
					OS::get_singleton()->set_window_minimized(true);
				}
				moving = false;
			}
		}
	}
}

void EditorTitleBar::set_can_move_window(bool p_enabled) {
	can_move = p_enabled;
	set_process_input(can_move);
}

bool EditorTitleBar::get_can_move_window() const {
	return can_move;
}
