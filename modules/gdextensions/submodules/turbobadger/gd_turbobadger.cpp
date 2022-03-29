/*************************************************************************/
/*  gd_turbobadger.cpp                                                   */
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

#include "gd_turbobadger.h"
#include "animation/tb_widget_animation.h"
#include "tb_editfield.h"
#include "tb_msg.h"
#include "tb_system.h"

#include "core/os/input.h"
#include "core/os/keyboard.h"
#include "scene/main/scene_tree.h"

// Reference:
// ----------
//   - https://github.com/floooh/oryol-tbui/tree/dd0afc300629efc6bd1c90e829895e3148916ad6/src/TBUI/tb

using namespace tb;

MODIFIER_KEYS GetModifierKeys(Ref<InputEventWithModifiers> event) {
	MODIFIER_KEYS code = TB_MODIFIER_NONE;
	if (event) {
		if (event->get_alt())
			code |= TB_ALT;
		if (event->get_command())
			code |= TB_CTRL;
		if (event->get_shift())
			code |= TB_SHIFT;
		if (event->get_metakey())
			code |= TB_SUPER;
	}
	return code;
}

static int toupr_ascii(int ascii) {
	if (ascii >= 'a' && ascii <= 'z') {
		return ascii + 'A' - 'a';
	}
	return ascii;
}

static bool InvokeShortcut(int key, SPECIAL_KEY special_key, MODIFIER_KEYS modifierkeys, bool down) {
#ifdef TB_TARGET_MACOSX
	bool shortcut_key = (modifierkeys & TB_SUPER) ? true : false;
#else
	bool shortcut_key = (modifierkeys & TB_CTRL) ? true : false;
#endif
	if (!TBWidget::focused_widget || !down || !shortcut_key) {
		return false;
	}
	bool reverse_key = (modifierkeys & TB_SHIFT) ? true : false;
	int upper_key = toupr_ascii(key);
	TBID id;
	if (upper_key == 'X') {
		id = TBIDC("cut");
	} else if (upper_key == 'C' || special_key == TB_KEY_INSERT) {
		id = TBIDC("copy");
	} else if (upper_key == 'V' || (special_key == TB_KEY_INSERT && reverse_key)) {
		id = TBIDC("paste");
	} else if (upper_key == 'A') {
		id = TBIDC("selectall");
	} else if (upper_key == 'Z' || upper_key == 'Y') {
		bool undo = upper_key == 'Z';
		if (reverse_key)
			undo = !undo;
		id = undo ? TBIDC("undo") : TBIDC("redo");
	} else if (upper_key == 'N') {
		id = TBIDC("new");
	} else if (upper_key == 'O') {
		id = TBIDC("open");
	} else if (upper_key == 'S') {
		id = TBIDC("save");
	} else if (upper_key == 'W') {
		id = TBIDC("close");
	} else if (special_key == TB_KEY_PAGE_UP) {
		id = TBIDC("prev_doc");
	} else if (special_key == TB_KEY_PAGE_DOWN) {
		id = TBIDC("next_doc");
	} else {
		return false;
	}

	TBWidgetEvent ev(EVENT_TYPE_SHORTCUT);
	ev.modifierkeys = modifierkeys;
	ev.ref_id = id;
	return TBWidget::focused_widget->InvokeEvent(ev);
}

static bool ShouldEmulateTouchEvent(Ref<InputEventWithModifiers> event) {
	// Used to emulate that mouse events are touch events when alt, ctrl and shift are pressed.
	// This makes testing a lot easier when there is no touch screen around :)
	return (GetModifierKeys(event) & (TB_ALT | TB_CTRL | TB_SHIFT)) ? true : false;
}

static bool InvokeKey(AppRootWidget *root, unsigned int key, SPECIAL_KEY special_key, MODIFIER_KEYS modifierkeys, bool down) {
	if (InvokeShortcut(key, special_key, modifierkeys, down)) {
		return true;
	}
	root->InvokeKey(key, special_key, modifierkeys, down);
	return true;
}

static void key_callback(AppRootWidget *root, Ref<InputEventKey> event) {
	MODIFIER_KEYS modifier = GetModifierKeys(event);
	bool down = event->is_pressed() || event->is_echo();
	switch (event->get_scancode()) {
		case KEY_F1:
			InvokeKey(root, 0, TB_KEY_F1, modifier, down);
			break;
		case KEY_F2:
			InvokeKey(root, 0, TB_KEY_F2, modifier, down);
			break;
		case KEY_F3:
			InvokeKey(root, 0, TB_KEY_F3, modifier, down);
			break;
		case KEY_F4:
			InvokeKey(root, 0, TB_KEY_F4, modifier, down);
			break;
		case KEY_F5:
			InvokeKey(root, 0, TB_KEY_F5, modifier, down);
			break;
		case KEY_F6:
			InvokeKey(root, 0, TB_KEY_F6, modifier, down);
			break;
		case KEY_F7:
			InvokeKey(root, 0, TB_KEY_F7, modifier, down);
			break;
		case KEY_F8:
			InvokeKey(root, 0, TB_KEY_F8, modifier, down);
			break;
		case KEY_F9:
			InvokeKey(root, 0, TB_KEY_F9, modifier, down);
			break;
		case KEY_F10:
			InvokeKey(root, 0, TB_KEY_F10, modifier, down);
			break;
		case KEY_F11:
			InvokeKey(root, 0, TB_KEY_F11, modifier, down);
			break;
		case KEY_F12:
			InvokeKey(root, 0, TB_KEY_F12, modifier, down);
			break;
		case KEY_LEFT:
			InvokeKey(root, 0, TB_KEY_LEFT, modifier, down);
			break;
		case KEY_UP:
			InvokeKey(root, 0, TB_KEY_UP, modifier, down);
			break;
		case KEY_RIGHT:
			InvokeKey(root, 0, TB_KEY_RIGHT, modifier, down);
			break;
		case KEY_DOWN:
			InvokeKey(root, 0, TB_KEY_DOWN, modifier, down);
			break;
		case KEY_PAGEUP:
			InvokeKey(root, 0, TB_KEY_PAGE_UP, modifier, down);
			break;
		case KEY_PAGEDOWN:
			InvokeKey(root, 0, TB_KEY_PAGE_DOWN, modifier, down);
			break;
		case KEY_HOME:
			InvokeKey(root, 0, TB_KEY_HOME, modifier, down);
			break;
		case KEY_END:
			InvokeKey(root, 0, TB_KEY_END, modifier, down);
			break;
		case KEY_INSERT:
			InvokeKey(root, 0, TB_KEY_INSERT, modifier, down);
			break;
		case KEY_TAB:
			InvokeKey(root, 0, TB_KEY_TAB, modifier, down);
			break;
		case KEY_DELETE:
			InvokeKey(root, 0, TB_KEY_DELETE, modifier, down);
			break;
		case KEY_BACKSPACE:
			InvokeKey(root, 0, TB_KEY_BACKSPACE, modifier, down);
			break;
		case KEY_ENTER:
		case KEY_KP_ENTER:
			InvokeKey(root, 0, TB_KEY_ENTER, modifier, down);
			break;
		case KEY_ESCAPE:
			InvokeKey(root, 0, TB_KEY_ESC, modifier, down);
			break;
		case KEY_MENU: {
			if (TBWidget::focused_widget && !down) {
				TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU);
				ev.modifierkeys = modifier;
				TBWidget::focused_widget->InvokeEvent(ev);
			}
		} break;
	}
}

static void mouse_button_callback(AppRootWidget *root, Ref<InputEventMouseButton> event) {
	MODIFIER_KEYS modifier = GetModifierKeys(event);
	const Point2 pt = root->ToLocal(event->get_position());
	const int button = event->get_button_index();
	int x = pt.x;
	int y = pt.y;
	if (button == BUTTON_LEFT) {
		if (event->is_pressed()) {
			// This is a quick fix with n-click support :)
			static double last_time = 0;
			static int last_x = 0;
			static int last_y = 0;
			static int counter = 1;

			double time = TBSystem::GetTimeMS();
			if (time < last_time + 600 && last_x == x && last_y == y) {
				counter++;
			} else {
				counter = 1;
			}
			last_x = x;
			last_y = y;
			last_time = time;

			root->InvokePointerDown(x, y, counter, modifier, ShouldEmulateTouchEvent(event));
		} else {
			root->InvokePointerUp(x, y, modifier, ShouldEmulateTouchEvent(event));
		}
	} else if (button == BUTTON_RIGHT && !event->is_pressed()) {
		root->InvokePointerMove(x, y, modifier, ShouldEmulateTouchEvent(event));
		if (TBWidget::hovered_widget) {
			TBWidget::hovered_widget->ConvertFromRoot(x, y);
			TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU, x, y, false, modifier);
			TBWidget::hovered_widget->InvokeEvent(ev);
		}
	}
}

static void cursor_position_callback(AppRootWidget *root, Ref<InputEventMouse> event) {
	const Point2 pt = root->ToLocal(event->get_position());
	root->SetCursorPos(pt.x, pt.y);
	if (root && !(ShouldEmulateTouchEvent(event) && !TBWidget::captured_widget)) {
		root->InvokePointerMove(pt.x, pt.y, GetModifierKeys(event), ShouldEmulateTouchEvent(event));

		// Update cursor.
		TBWidget *active_widget = TBWidget::captured_widget ? TBWidget::captured_widget : TBWidget::hovered_widget;
		if (TBSafeCast<TBEditField>(active_widget)) {
			Input::get_singleton()->set_default_cursor_shape(Input::CURSOR_IBEAM);
		} else {
			Input::get_singleton()->set_default_cursor_shape(Input::CURSOR_ARROW);
		}
	}
}

static void scroll_callback(AppRootWidget *root, Ref<InputEventMouseButton> event) {
	if (root) {
		const Point2 pt = root->ToLocal(event->get_position());
		const Point2 cr = root->GetCursorPos();
		root->InvokeWheel(cr.x, cr.y, (int)pt.x, -(int)pt.y, GetModifierKeys(event));
	}
}

// Reschedule the platform timer, or cancel it if fire_time is TB_NOT_SOON.
// If fire_time is 0, it should be fired ASAP.
// If force is true, it will ask the platform to schedule it again, even if
// the fire_time is the same as last time.
static void ReschedulePlatformTimer(real_t fire_time, bool force) {
	static real_t set_fire_time = -1;
	static Ref<SceneTreeTimer> _sys_timer;
	if (fire_time == TB_NOT_SOON) {
		set_fire_time = -1;
	} else if (fire_time != set_fire_time || force || fire_time == 0) {
		if (!_sys_timer) {
			if (SceneTree *st = SceneTree::get_singleton()) {
				_sys_timer = st->create_timer(fire_time);
				_sys_timer->connect("timeout", GdTurboBadgerCore::get_singleton(), "_timer_callback");
			}
		}
		set_fire_time = fire_time;
		real_t delay = fire_time - tb::TBSystem::GetTimeMS();
		unsigned int idelay = (unsigned int)MAX(delay, 0.0);
		_sys_timer->set_time_left(idelay);
	}
}

// This doesn't really belong here (it belongs in tb_system_[linux/windows].cpp.
// This is here since the proper implementations has not yet been done.
void TBSystem::RescheduleTimer(double fire_time) {
	ReschedulePlatformTimer(fire_time, false);
}

static void drop_callback(tb::TBWidget *root, int count, const char **files_utf8) {
	TBWidget *target = TBWidget::hovered_widget;
	if (!target) {
		target = TBWidget::focused_widget;
	}
	if (!target) {
		target = root;
	}
	if (target) {
		TBWidgetEventFileDrop ev;
		for (int i = 0; i < count; i++) {
			ev.files.Add(new TBStr(files_utf8[i]));
		}
		target->InvokeEvent(ev);
	}
}

// Godot node

#define get_rect() Rect2(Point2(0, 0), view_size)

#ifdef TOOLS_ENABLED
bool GdTurboBadger::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return get_rect().has_point(p_point);
}

Rect2 GdTurboBadger::_edit_get_rect() const {
	return get_rect();
}

bool GdTurboBadger::_edit_use_rect() const {
	return true;
}
#endif

void GdTurboBadger::_input(const Ref<InputEvent> &p_event) {
	if (!p_event.is_valid()) {
		return;
	}
	if (!get_tree()) {
		return;
	}

	ERR_FAIL_COND(!is_visible_in_tree());
}

void GdTurboBadger::notifications(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			GdTurboBadgerCore::get_singleton()->init();
		} break;
		case NOTIFICATION_ENTER_TREE: {
			// Set initial size which suggest to the backend which size we want the window to be.
			root.SetRect(TBRect(0, 0, view_size.width, view_size.height));
		} break;
		case NOTIFICATION_DRAW: {
			if (_dirty) {
				root.SetRect(TBRect(0, 0, view_size.width, view_size.height));
				_dirty = false;
			}
			auto *renderer = GdTurboBadgerCore::get_singleton()->get_renderer();
			renderer->BeginPaint(this, root.GetRect().w, root.GetRect().h);
			root.InvokePaint(TBWidget::PaintProps());
			renderer->EndPaint();
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			TBAnimationManager::Update();
			root.InvokeProcessStates();
			root.InvokeProcess();
			// If animations are running, reinvalidate immediately
			if (TBAnimationManager::HasAnimationsRunning()) {
				update();
			}
		} break;
	}
}

void GdTurboBadger::set_view_size(const Size2 &p_size) {
	view_size = p_size;
	_dirty = true;
	update();
}

Size2 GdTurboBadger::get_view_size() const {
	return view_size;
}

void GdTurboBadger::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_input"), &GdTurboBadger::_input);
}

GdTurboBadger::GdTurboBadger() :
		root(this) {
	_dirty = false;
	view_size = Size2(640, 480);
}

GdTurboBadger::~GdTurboBadger() {
	GdTurboBadgerCore::get_singleton()->release();
}

// Core singleton

GdTurboBadgerCore *_tb_core = nullptr;

void GdTurboBadgerCore::_timer_callback() {
	double next_fire_time = TBMessageHandler::GetNextMessageFireTime();
	double now = tb::TBSystem::GetTimeMS();
	if (now < next_fire_time) {
		// We timed out *before* we were supposed to (the OS is not playing nice).
		// Calling ProcessMessages now won't achieve a thing so force a reschedule
		// of the platform timer again with the same time.
		ReschedulePlatformTimer(next_fire_time, true);
		return;
	}

	TBMessageHandler::ProcessMessages();

	// If we still have things to do (because we didn't process all messages,
	// or because there are new messages), we need to reschedule, so call RescheduleTimer.
	TBSystem::RescheduleTimer(TBMessageHandler::GetNextMessageFireTime());
}

void GdTurboBadgerCore::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_timer_callback"), &GdTurboBadgerCore::_timer_callback);
}

GdTurboBadgerCore *GdTurboBadgerCore::get_singleton() {
	if (!_tb_core) {
		_tb_core = memnew(GdTurboBadgerCore);
	}
	return _tb_core;
}

void GdTurboBadgerCore::init() {
	if (_ref == 0) {
		TBWidgetsAnimationManager::Init();
		tb_core_init(&renderer);
	}
	_ref++;
}

void GdTurboBadgerCore::release() {
	_ref--;

	ERR_FAIL_COND(_ref < 0);

	if (_ref == 0) {
		delete _tb_core;
		_tb_core = nullptr;
	}
}

GdTurboBadgerCore::GdTurboBadgerCore() {
	_ref = 0;
	// register default theme files
#include "turbobadger/resources/default_theme.inc"
}

GdTurboBadgerCore::~GdTurboBadgerCore() {
	tb_core_shutdown();
	TBWidgetsAnimationManager::Shutdown();
}
