#ifndef MENU_BAR_MGR_H
#define MENU_BAR_MGR_H

#include "core/callable.h"
#include "core/hash_map.h"
#include "core/variant.h"
#include "scene/resources/texture.h"

enum GlobalMenuCheckType {
	CHECKABLE_TYPE_NONE,
	CHECKABLE_TYPE_CHECK_BOX,
	CHECKABLE_TYPE_RADIO_BUTTON,
};

typedef Callable<void(int)> MenuItemCallback;

class MenuBarDisplayMgr {

public:
	virtual bool has_global_menu() const { return false; }

	virtual int global_menu_add_item(const String &p_menu_root, const String &p_label, const MenuItemCallback &p_callback = nullptr, const Variant &p_tag = Variant(), uint32_t p_accel = 0, int p_index = -1) { return -1; }
	virtual int global_menu_add_check_item(const String &p_menu_root, const String &p_label, const MenuItemCallback &p_callback = MenuItemCallback(), const Variant &p_tag = Variant(), uint32_t p_accel = 0, int p_index = -1) { return -1; }
	virtual int global_menu_add_submenu_item(const String &p_menu_root, const String &p_label, const String &p_submenu, int p_index = -1) { return -1; }
	virtual int global_menu_add_separator(const String &p_menu_root, int p_index = -1) { return -1; }

	virtual String global_menu_get_item_submenu(const String &p_menu_root, int p_idx) const { return ""; }

	virtual void global_menu_set_item_checked(const String &p_menu_root, int p_idx, bool p_checked) {}
	virtual void global_menu_set_item_checkable(const String &p_menu_root, int p_idx, bool p_checkable) {}
	virtual void global_menu_set_item_radio_checkable(const String &p_menu_root, int p_idx, bool p_checkable) {}
	virtual void global_menu_set_item_accelerator(const String &p_menu_root, int p_idx, uint32_t p_keycode) {}
	virtual void global_menu_set_item_disabled(const String &p_menu_root, int p_idx, bool p_disabled) {}
	virtual void global_menu_set_item_tooltip(const String &p_menu_root, int p_idx, const String &p_tooltip) {}
	virtual void global_menu_set_item_max_states(const String &p_menu_root, int p_idx, int p_max_states) {}
	virtual void global_menu_set_item_icon(const String &p_menu_root, int p_idx, const Ref<Texture> &p_icon) {}
	virtual void global_menu_set_item_state(const String &p_menu_root, int p_idx, int p_state) {}
	virtual void global_menu_set_item_indentation_level(const String &p_menu_root, int p_idx, int p_level) {}

	virtual int global_menu_get_item_count(const String &p_menu_root) const { return 0; }

	virtual void global_menu_remove_item(const String &p_menu_root, int p_idx) {}
	virtual void global_menu_clear(const String &p_menu_root) {}

	static MenuBarDisplayMgr *get_instance();
};

#endif // MENU_BAR_MGR_H
