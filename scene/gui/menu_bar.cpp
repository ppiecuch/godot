/*************************************************************************/
/*  menu_bar.cpp                                                         */
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

#include "menu_bar.h"

#include "core/os/keyboard.h"
#include "core/os/os.h"
#include "menu_bar_mgr.h"
#include "scene/main/viewport.h"

Point2 MenuBar::_get_screen_position() const {
	ERR_FAIL_COND_V(!is_inside_tree(), Point2());
	Point2 global_pos = get_global_transform_with_canvas().get_origin();
	global_pos += OS::get_singleton()->get_window_position();

	return global_pos;
}

void MenuBar::_gui_input(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());
	if (is_native_menu()) {
		// Handled by OS.
		return;
	}

	MutexLock lock(mutex);
	if (p_event->is_action("ui_left") && p_event->is_pressed()) {
		int new_sel = selected_menu;
		int old_sel = (selected_menu < 0) ? 0 : selected_menu;
		do {
			new_sel--;
			if (new_sel < 0) {
				new_sel = menu_cache.size() - 1;
			}
			if (old_sel == new_sel) {
				return;
			}
		} while (menu_cache[new_sel].hidden || menu_cache[new_sel].disabled);

		if (selected_menu != new_sel) {
			selected_menu = new_sel;
			focused_menu = selected_menu;
			if (active_menu >= 0) {
				get_menu_popup(active_menu)->hide();
			}
			_open_popup(selected_menu);
		}
		return;
	} else if (p_event->is_action("ui_right") && p_event->is_pressed()) {
		int new_sel = selected_menu;
		int old_sel = (selected_menu < 0) ? menu_cache.size() - 1 : selected_menu;
		do {
			new_sel++;
			if (new_sel >= menu_cache.size()) {
				new_sel = 0;
			}
			if (old_sel == new_sel) {
				return;
			}
		} while (menu_cache[new_sel].hidden || menu_cache[new_sel].disabled);

		if (selected_menu != new_sel) {
			selected_menu = new_sel;
			focused_menu = selected_menu;
			if (active_menu >= 0) {
				get_menu_popup(active_menu)->hide();
			}
			_open_popup(selected_menu);
		}
		return;
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		int old_sel = selected_menu;
		focused_menu = _get_index_at_point(mm->get_position());
		if (focused_menu >= 0) {
			selected_menu = focused_menu;
		}
		if (selected_menu != old_sel) {
			update();
		}
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		if (mb->is_pressed() && (mb->get_button_index() == BUTTON_LEFT || mb->get_button_index() == BUTTON_RIGHT)) {
			int index = _get_index_at_point(mb->get_position());
			if (index >= 0) {
				_open_popup(index);
			}
		}
	}
}

void MenuBar::_open_popup(int p_index) {
	ERR_FAIL_INDEX(p_index, menu_cache.size());

	PopupMenu *pm = get_menu_popup(p_index);
	if (pm->is_visible()) {
		pm->hide();
		return;
	}

	Rect2 item_rect = _get_menu_item_rect(p_index);
	Point2 screen_pos = _get_screen_position() + item_rect.position * get_viewport()->get_canvas_transform().get_scale();
	Size2 screen_size = item_rect.size * get_viewport()->get_canvas_transform().get_scale();

	active_menu = p_index;

	pm->set_size(Size2(screen_size.x, 0));
	screen_pos.y += screen_size.y;
	pm->set_position(screen_pos);
	pm->set_parent_rect(Rect2(Point2(screen_pos - pm->get_position()), Size2(screen_size.x, screen_pos.y)));
	pm->popup();

	update();
}

void MenuBar::_popup_visibility_changed(bool p_visible) {
	if (!p_visible) {
		active_menu = -1;
		focused_menu = -1;
		set_process_internal(false);
		update();
		return;
	}

	if (switch_on_hover) {
		mouse_pos_adjusted = OS::get_singleton()->get_window_position();
		set_process_internal(true);
	}
}

void MenuBar::_update_submenu(const String &p_menu_name, PopupMenu *p_child) {
	int count = p_child->get_item_count();
	global_menus.insert(p_menu_name);
	for (int i = 0; i < count; i++) {
		if (p_child->is_item_separator(i)) {
			_mgr->global_menu_add_separator(p_menu_name);
		} else if (!p_child->get_item_submenu(i).empty()) {
			Node *n = p_child->get_node(p_child->get_item_submenu(i));
			ERR_FAIL_COND_MSG(!n, "Item subnode does not exist: " + p_child->get_item_submenu(i) + ".");
			PopupMenu *pm = Object::cast_to<PopupMenu>(n);
			ERR_FAIL_COND_MSG(!pm, "Item subnode is not a PopupMenu: " + p_child->get_item_submenu(i) + ".");

			_mgr->global_menu_add_submenu_item(p_menu_name, p_child->get_item_text(i), p_menu_name + "/" + itos(i));
			_update_submenu(p_menu_name + "/" + itos(i), pm);
		} else {
			int index = _mgr->global_menu_add_item(p_menu_name, p_child->get_item_text(i), callable_mp(p_child, &PopupMenu::activate_item), i);

			if (p_child->is_item_checkable(i)) {
				_mgr->global_menu_set_item_checkable(p_menu_name, index, true);
			}
			if (p_child->is_item_radio_checkable(i)) {
				_mgr->global_menu_set_item_radio_checkable(p_menu_name, index, true);
			}
			_mgr->global_menu_set_item_checked(p_menu_name, index, p_child->is_item_checked(i));
			_mgr->global_menu_set_item_disabled(p_menu_name, index, p_child->is_item_disabled(i));
			_mgr->global_menu_set_item_max_states(p_menu_name, index, p_child->get_item_max_states(i));
			_mgr->global_menu_set_item_icon(p_menu_name, index, p_child->get_item_icon(i));
			_mgr->global_menu_set_item_state(p_menu_name, index, p_child->get_item_state(i));
			_mgr->global_menu_set_item_indentation_level(p_menu_name, index, p_child->get_item_indent(i));
			_mgr->global_menu_set_item_tooltip(p_menu_name, index, p_child->get_item_tooltip(i));
			if (!p_child->is_item_shortcut_disabled(i) && p_child->get_item_shortcut(i).is_valid() && p_child->get_item_shortcut(i)->is_valid()) {
				Array events = array(p_child->get_item_shortcut(i)->get_shortcut());
				for (int j = 0; j < events.size(); j++) {
					Ref<InputEventKey> ie = events[j];
					if (ie.is_valid()) {
						_mgr->global_menu_set_item_accelerator(p_menu_name, index, ie->get_scancode_with_modifiers());
						break;
					}
				}
			} else if (p_child->get_item_accelerator(i) != 0) {
				_mgr->global_menu_set_item_accelerator(p_menu_name, i, p_child->get_item_accelerator(i));
			}
		}
	}
}

bool MenuBar::is_native_menu() const {
	if (Engine::get_singleton()->is_editor_hint() && is_inside_tree() && get_tree()->get_edited_scene_root() && (get_tree()->get_edited_scene_root()->is_a_parent_of(this) || get_tree()->get_edited_scene_root() == this)) {
		return false;
	}

	return (_mgr->has_global_menu() && is_native);
}

void MenuBar::_clear_menu() {
	if (!_mgr->has_global_menu()) {
		return;
	}

	// Remove root menu items.
	int count = _mgr->global_menu_get_item_count("_main");
	for (int i = count - 1; i >= 0; i--) {
		if (global_menus.has(_mgr->global_menu_get_item_submenu("_main", i))) {
			_mgr->global_menu_remove_item("_main", i);
		}
	}
	// Erase submenu contents.
	for (const String &E : global_menus) {
		_mgr->global_menu_clear(E);
	}
	global_menus.clear();
}

void MenuBar::_update_menu() {
	_clear_menu();

	if (!is_visible_in_tree()) {
		return;
	}

	int index = start_index;
	if (is_native_menu()) {
		Vector<PopupMenu *> popups = _get_popups();
		String root_name = "MenuBar<" + String::num_int64((uint64_t)this, 16) + ">";
		for (int i = 0; i < popups.size(); i++) {
			if (menu_cache[i].hidden) {
				continue;
			}
			String menu_name = String(popups[i]->get_meta("_menu_name", popups[i]->get_name()));

			index = _mgr->global_menu_add_submenu_item("_main", menu_name, root_name + "/" + itos(i), index);
			if (menu_cache[i].disabled) {
				_mgr->global_menu_set_item_disabled("_main", index, true);
			}
			_update_submenu(root_name + "/" + itos(i), popups[i]);
			index++;
		}
	}
	minimum_size_changed();
	update();
}

void MenuBar::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (get_menu_count() > 0) {
				_refresh_menu_names();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_clear_menu();
		} break;
		case NOTIFICATION_MOUSE_EXIT: {
			focused_menu = -1;
			update();
		} break;
		case NOTIFICATION_TRANSLATION_CHANGED:
		case NOTIFICATION_THEME_CHANGED: {
			_update_menu();
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			_update_menu();
		} break;
		case NOTIFICATION_DRAW: {
			if (is_native_menu()) {
				return;
			}
			for (int i = 0; i < menu_cache.size(); i++) {
				_draw_menu_item(i);
			}
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			MutexLock lock(mutex);

			Vector2 pos = get_viewport()->get_mouse_position() - mouse_pos_adjusted - get_global_position();
			int index = _get_index_at_point(pos);
			if (index >= 0 && index != active_menu) {
				selected_menu = index;
				focused_menu = selected_menu;
				get_menu_popup(active_menu)->hide();
				_open_popup(index);
			}
		} break;
	}
}

int MenuBar::_get_index_at_point(const Point2 &p_point) const {
	Ref<StyleBox> style = get_stylebox("normal");
	Ref<Font> font = get_font("font");
	int hsep = get_constant("h_separation");
	int offset = 0;
	for (int i = 0; i < menu_cache.size(); i++) {
		if (menu_cache[i].hidden) {
			continue;
		}
		Size2 size = font->get_string_size(menu_cache[i].name) + style->get_minimum_size();
		if (p_point.x > offset && p_point.x < offset + size.x) {
			if (p_point.y > 0 && p_point.y < size.y) {
				return i;
			}
		}
		offset += size.x + hsep;
	}
	return -1;
}

Rect2 MenuBar::_get_menu_item_rect(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, menu_cache.size(), Rect2());

	Ref<StyleBox> style = get_stylebox("normal");
	Ref<Font> font = get_stylebox("font");
	int hsep = get_constant("h_separation");

	int offset = 0;
	for (int i = 0; i < p_index; i++) {
		if (menu_cache[i].hidden) {
			continue;
		}
		Size2 size = font->get_string_size(menu_cache[i].name) + style->get_minimum_size();
		offset += size.x + hsep;
	}

	return Rect2(Point2(offset, 0), font->get_string_size(menu_cache[p_index].name) + style->get_minimum_size());
}

void MenuBar::_draw_menu_item(int p_index) {
	ERR_FAIL_INDEX(p_index, menu_cache.size());

	RID ci = get_canvas_item();
	bool hovered = (focused_menu == p_index);
	bool pressed = (active_menu == p_index);

	if (menu_cache[p_index].hidden) {
		return;
	}

	Color color;
	Ref<StyleBox> style = get_stylebox("normal");
	Ref<Font> font = get_stylebox("font");
	Rect2 item_rect = _get_menu_item_rect(p_index);

	if (menu_cache[p_index].disabled) {
		style = get_stylebox("disabled");
		if (!flat) {
			style->draw(ci, item_rect);
		}
		color = get_color("font_disabled_color");
	} else if (hovered && pressed && has_stylebox("hover_pressed")) {
		style = get_stylebox("hover_pressed");
		if (!flat) {
			style->draw(ci, item_rect);
		}
		if (has_color("font_hover_pressed_color")) {
			color = get_color("font_hover_pressed_color");
		}
	} else if (pressed) {
		style = get_stylebox("pressed");
		if (!flat) {
			style->draw(ci, item_rect);
		}
		if (has_color("font_pressed_color")) {
			color = get_color("font_pressed_color");
		} else {
			color = get_color("font_color");
		}
	} else if (hovered) {
		style = get_stylebox("hover");
		if (!flat) {
			style->draw(ci, item_rect);
		}
		color = get_color("font_hover_color");
	} else {
		style = get_stylebox("normal");
		if (!flat) {
			style->draw(ci, item_rect);
		}
		// Focus colors only take precedence over normal state.
		if (has_focus()) {
			color = get_color("font_focus_color");
		} else {
			color = get_color("font_color");
		}
	}

	Point2 text_ofs = item_rect.position + Point2(style->get_margin(MARGIN_LEFT), style->get_margin(MARGIN_TOP));

	font->draw(ci, text_ofs, menu_cache[p_index].name, color);
}

void MenuBar::_refresh_menu_names() {
	Vector<PopupMenu *> popups = _get_popups();
	for (int i = 0; i < popups.size(); i++) {
		if (!popups[i]->has_meta("_menu_name") && String(popups[i]->get_name()) != get_menu_title(i)) {
			menu_cache.write[i].name = popups[i]->get_name();
		}
	}
	_update_menu();
}

Vector<PopupMenu *> MenuBar::_get_popups() const {
	Vector<PopupMenu *> popups;
	for (int i = 0; i < get_child_count(); i++) {
		PopupMenu *pm = Object::cast_to<PopupMenu>(get_child(i));
		if (!pm) {
			continue;
		}
		popups.push_back(pm);
	}
	return popups;
}

int MenuBar::get_menu_idx_from_control(PopupMenu *p_child) const {
	ERR_FAIL_NULL_V(p_child, -1);
	ERR_FAIL_COND_V(p_child->get_parent() != this, -1);

	Vector<PopupMenu *> popups = _get_popups();
	for (int i = 0; i < popups.size(); i++) {
		if (popups[i] == p_child) {
			return i;
		}
	}

	return -1;
}

void MenuBar::add_child_notify(Node *p_child) {
	Control::add_child_notify(p_child);

	PopupMenu *pm = Object::cast_to<PopupMenu>(p_child);
	if (!pm) {
		return;
	}
	Menu menu = Menu(p_child->get_name());
	menu_cache.push_back(menu);
	p_child->connect("renamed", this, "_refresh_menu_names");
	p_child->connect("menu_changed", this, "_update_menu");
	p_child->connect("about_to_popup", this, "_popup_visibility_changed", varray(true));
	p_child->connect("popup_hide", this, "_popup_visibility_changed", varray(false));

	_update_menu();
}

void MenuBar::move_child_notify(Node *p_child) {
	Control::move_child_notify(p_child);

	PopupMenu *pm = Object::cast_to<PopupMenu>(p_child);
	if (!pm) {
		return;
	}

	int old_idx = -1;
	String menu_name = String(pm->get_meta("_menu_name", pm->get_name()));
	// Find the previous menu index of the control.
	for (int i = 0; i < get_menu_count(); i++) {
		if (get_menu_title(i) == menu_name) {
			old_idx = i;
			break;
		}
	}
	Menu menu = menu_cache[old_idx];
	menu_cache.remove(old_idx);
	menu_cache.insert(get_menu_idx_from_control(pm), menu);

	_update_menu();
}

void MenuBar::remove_child_notify(Node *p_child) {
	Control::remove_child_notify(p_child);

	PopupMenu *pm = Object::cast_to<PopupMenu>(p_child);
	if (!pm) {
		return;
	}

	int idx = get_menu_idx_from_control(pm);

	menu_cache.remove(idx);

	p_child->remove_meta("_menu_name");
	p_child->remove_meta("_menu_tooltip");

	p_child->disconnect("renamed", this, "_refresh_menu_names");
	p_child->disconnect("menu_changed", this, "_update_menu");
	p_child->disconnect("about_to_popup", this, "_popup_visibility_changed");
	p_child->disconnect("popup_hide", this, "_popup_visibility_changed");

	_update_menu();
}

void MenuBar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_switch_on_hover", "enable"), &MenuBar::set_switch_on_hover);
	ClassDB::bind_method(D_METHOD("is_switch_on_hover"), &MenuBar::is_switch_on_hover);
	ClassDB::bind_method(D_METHOD("set_disable_shortcuts", "disabled"), &MenuBar::set_disable_shortcuts);

	ClassDB::bind_method(D_METHOD("set_prefer_global_menu", "enabled"), &MenuBar::set_prefer_global_menu);
	ClassDB::bind_method(D_METHOD("is_prefer_global_menu"), &MenuBar::is_prefer_global_menu);
	ClassDB::bind_method(D_METHOD("is_native_menu"), &MenuBar::is_native_menu);

	ClassDB::bind_method(D_METHOD("get_menu_count"), &MenuBar::get_menu_count);

	ClassDB::bind_method(D_METHOD("set_flat", "enabled"), &MenuBar::set_flat);
	ClassDB::bind_method(D_METHOD("is_flat"), &MenuBar::is_flat);
	ClassDB::bind_method(D_METHOD("set_start_index", "enabled"), &MenuBar::set_start_index);
	ClassDB::bind_method(D_METHOD("get_start_index"), &MenuBar::get_start_index);

	ClassDB::bind_method(D_METHOD("set_menu_title", "menu", "title"), &MenuBar::set_menu_title);
	ClassDB::bind_method(D_METHOD("get_menu_title", "menu"), &MenuBar::get_menu_title);

	ClassDB::bind_method(D_METHOD("set_menu_tooltip", "menu", "tooltip"), &MenuBar::set_menu_tooltip);
	ClassDB::bind_method(D_METHOD("get_menu_tooltip", "menu"), &MenuBar::get_menu_tooltip);

	ClassDB::bind_method(D_METHOD("set_menu_disabled", "menu", "disabled"), &MenuBar::set_menu_disabled);
	ClassDB::bind_method(D_METHOD("is_menu_disabled", "menu"), &MenuBar::is_menu_disabled);

	ClassDB::bind_method(D_METHOD("set_menu_hidden", "menu", "hidden"), &MenuBar::set_menu_hidden);
	ClassDB::bind_method(D_METHOD("is_menu_hidden", "menu"), &MenuBar::is_menu_hidden);

	ClassDB::bind_method(D_METHOD("set_shortcut_context", "node"), &MenuBar::set_shortcut_context);
	ClassDB::bind_method(D_METHOD("get_shortcut_context"), &MenuBar::get_shortcut_context);

	ClassDB::bind_method(D_METHOD("get_menu_popup", "menu"), &MenuBar::get_menu_popup);

	ClassDB::bind_method(D_METHOD("_gui_input"), &MenuBar::_gui_input);
	ClassDB::bind_method(D_METHOD("_refresh_menu_names"), &MenuBar::_refresh_menu_names);
	ClassDB::bind_method(D_METHOD("_update_menu"), &MenuBar::_update_menu);
	ClassDB::bind_method(D_METHOD("_popup_visibility_changed", "state"), &MenuBar::_popup_visibility_changed);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flat"), "set_flat", "is_flat");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "start_index"), "set_start_index", "get_start_index");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "switch_on_hover"), "set_switch_on_hover", "is_switch_on_hover");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "prefer_global_menu"), "set_prefer_global_menu", "is_prefer_global_menu");
}

void MenuBar::set_switch_on_hover(bool p_enabled) {
	switch_on_hover = p_enabled;
}

bool MenuBar::is_switch_on_hover() {
	return switch_on_hover;
}

void MenuBar::set_disable_shortcuts(bool p_disabled) {
	disable_shortcuts = p_disabled;
}

void MenuBar::set_flat(bool p_enabled) {
	if (flat != p_enabled) {
		flat = p_enabled;
		update();
	}
}

bool MenuBar::is_flat() const {
	return flat;
}

void MenuBar::set_start_index(int p_index) {
	if (start_index != p_index) {
		start_index = p_index;
		_update_menu();
	}
}

int MenuBar::get_start_index() const {
	return start_index;
}

void MenuBar::set_prefer_global_menu(bool p_enabled) {
	if (is_native != p_enabled) {
		if (is_native) {
			_clear_menu();
		}
		is_native = p_enabled;
		_update_menu();
	}
}

bool MenuBar::is_prefer_global_menu() const {
	return is_native;
}

Size2 MenuBar::get_minimum_size() const {
	if (is_native_menu()) {
		return Size2();
	}

	Ref<StyleBox> style = get_stylebox("normal");
	Ref<Font> font = get_font("font");

	Vector2 size;
	for (int i = 0; i < menu_cache.size(); i++) {
		if (menu_cache[i].hidden) {
			continue;
		}
		Size2 sz = font->get_string_size(menu_cache[i].name) + style->get_minimum_size();
		size.y = MAX(size.y, sz.y);
		size.x += sz.x;
	}
	if (menu_cache.size() > 1) {
		size.x += get_constant("h_separation") * (menu_cache.size() - 1);
	}
	return size;
}

int MenuBar::get_menu_count() const {
	return menu_cache.size();
}

void MenuBar::set_menu_title(int p_menu, const String &p_title) {
	ERR_FAIL_INDEX(p_menu, menu_cache.size());
	PopupMenu *pm = get_menu_popup(p_menu);
	if (p_title == pm->get_name()) {
		pm->remove_meta("_menu_name");
	} else {
		pm->set_meta("_menu_name", p_title);
	}
	menu_cache.write[p_menu].name = p_title;
	_update_menu();
}

String MenuBar::get_menu_title(int p_menu) const {
	ERR_FAIL_INDEX_V(p_menu, menu_cache.size(), String());
	return menu_cache[p_menu].name;
}

void MenuBar::set_menu_tooltip(int p_menu, const String &p_tooltip) {
	ERR_FAIL_INDEX(p_menu, menu_cache.size());
	PopupMenu *pm = get_menu_popup(p_menu);
	pm->set_meta("_menu_tooltip", p_tooltip);
	menu_cache.write[p_menu].name = p_tooltip;
}

String MenuBar::get_menu_tooltip(int p_menu) const {
	ERR_FAIL_INDEX_V(p_menu, menu_cache.size(), String());
	return menu_cache[p_menu].tooltip;
}

void MenuBar::set_menu_disabled(int p_menu, bool p_disabled) {
	ERR_FAIL_INDEX(p_menu, menu_cache.size());
	menu_cache.write[p_menu].disabled = p_disabled;
	_update_menu();
}

bool MenuBar::is_menu_disabled(int p_menu) const {
	ERR_FAIL_INDEX_V(p_menu, menu_cache.size(), false);
	return menu_cache[p_menu].disabled;
}

void MenuBar::set_menu_hidden(int p_menu, bool p_hidden) {
	ERR_FAIL_INDEX(p_menu, menu_cache.size());
	menu_cache.write[p_menu].hidden = p_hidden;
	_update_menu();
}

bool MenuBar::is_menu_hidden(int p_menu) const {
	ERR_FAIL_INDEX_V(p_menu, menu_cache.size(), false);
	return menu_cache[p_menu].hidden;
}

PopupMenu *MenuBar::get_menu_popup(int p_idx) const {
	Vector<PopupMenu *> controls = _get_popups();
	if (p_idx >= 0 && p_idx < controls.size()) {
		return controls[p_idx];
	} else {
		return nullptr;
	}
}

String MenuBar::get_tooltip(const Point2 &p_pos) const {
	int index = _get_index_at_point(p_pos);
	if (index >= 0 && index < menu_cache.size()) {
		return menu_cache[index].tooltip;
	} else {
		return String();
	}
}

void MenuBar::set_shortcut_context(Node *p_node) {
	if (p_node != nullptr) {
		shortcut_context = p_node->get_instance_id();
	} else {
		shortcut_context = ObjectID();
	}
}

Node *MenuBar::get_shortcut_context() const {
	Object *ctx_obj = ObjectDB::get_instance(shortcut_context);
	Node *ctx_node = Object::cast_to<Node>(ctx_obj);

	return ctx_node;
}

void MenuBar::get_translatable_strings(List<String> *p_strings) const {
	Vector<PopupMenu *> popups = _get_popups();
	for (int i = 0; i < popups.size(); i++) {
		PopupMenu *pm = popups[i];

		if (!pm->has_meta("_menu_name") && !pm->has_meta("_menu_tooltip")) {
			continue;
		}

		String name = pm->get_meta("_menu_name");
		if (!name.empty()) {
			p_strings->push_back(name);
		}

		String tooltip = pm->get_meta("_menu_tooltip");
		if (!tooltip.empty()) {
			p_strings->push_back(tooltip);
		}
	}
}

MenuBar::MenuBar() {
	_mgr = MenuBarDisplayMgr::get_instance();
}

MenuBar::~MenuBar() {
}
