/*************************************************************************/
/*  menu_bar_mgr_osx.h                                                   */
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

#ifndef MENU_BAR_MGR_OSX_H
#define MENU_BAR_MGR_OSX_H

#include "core/os/thread_safe.h"
#include "scene/gui/menu_bar.h"

#include <objc/objc.h>

#ifdef __OBJC__
#define DECL_OBJC(cls) @class cls
#else
#define DECL_OBJC(cls) typedef struct objc_object cls
#endif

DECL_OBJC(NSMenu);
DECL_OBJC(NSMenuItem);
DECL_OBJC(NSImage);

class MenuBarDisplayMgrOSX : public MenuBarDisplayMgr {
	_THREAD_SAFE_CLASS_

	NSMenu *apple_menu = nullptr;
	NSMenu *dock_menu = nullptr;
	HashMap<String, NSMenu *> submenu;

	NSImage *_convert_to_nsimg(Ref<Image> &p_image) const;

	bool _has_help_menu() const;

	const NSMenu *_get_menu_root(const String &p_menu_root) const;
	NSMenu *_get_menu_root(const String &p_menu_root);
	NSMenuItem *_menu_add_item(const String &p_menu_root, const String &p_label, uint32_t p_accel, int p_index, int *r_out);

public:
	virtual bool has_global_menu() const override { return true; }

	virtual int global_menu_add_item(const String &p_menu_root, const String &p_label, const MenuItemCallback &p_callback = nullptr, const Variant &p_tag = Variant(), uint32_t p_accel = 0, int p_index = -1) override;
	virtual int global_menu_add_check_item(const String &p_menu_root, const String &p_label, const MenuItemCallback &p_callback = MenuItemCallback(), const Variant &p_tag = Variant(), uint32_t p_accel = 0, int p_index = -1) override;
	virtual int global_menu_add_submenu_item(const String &p_menu_root, const String &p_label, const String &p_submenu, int p_index = -1) override;
	virtual int global_menu_add_separator(const String &p_menu_root, int p_index = -1) override;

	virtual String global_menu_get_item_submenu(const String &p_menu_root, int p_idx) const override;

	virtual void global_menu_set_item_checked(const String &p_menu_root, int p_idx, bool p_checked) override;
	virtual void global_menu_set_item_checkable(const String &p_menu_root, int p_idx, bool p_checkable) override;
	virtual void global_menu_set_item_radio_checkable(const String &p_menu_root, int p_idx, bool p_checkable) override;
	virtual void global_menu_set_item_accelerator(const String &p_menu_root, int p_idx, uint32_t p_keycode) override;
	virtual void global_menu_set_item_disabled(const String &p_menu_root, int p_idx, bool p_disabled) override;
	virtual void global_menu_set_item_tooltip(const String &p_menu_root, int p_idx, const String &p_tooltip) override;
	virtual void global_menu_set_item_max_states(const String &p_menu_root, int p_idx, int p_max_states) override;
	virtual void global_menu_set_item_icon(const String &p_menu_root, int p_idx, const Ref<Texture> &p_icon) override;
	virtual void global_menu_set_item_state(const String &p_menu_root, int p_idx, int p_state) override;
	virtual void global_menu_set_item_indentation_level(const String &p_menu_root, int p_idx, int p_level) override;

	virtual int global_menu_get_item_count(const String &p_menu_root) const override;

	virtual void global_menu_remove_item(const String &p_menu_root, int p_idx) override;
	virtual void global_menu_clear(const String &p_menu_root) override;

	MenuBarDisplayMgrOSX() {}
};

#endif // MENU_BAR_MGR_OSX_H
