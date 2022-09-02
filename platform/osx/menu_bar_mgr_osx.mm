/*************************************************************************/
/*  menu_bar_mgr_osx.mm                                                  */
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

#include "menu_bar_mgr_osx.h"

#include "core/image.h"
#include "platform/osx/key_mapping_macos.h"

#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

@interface GodotMenuItem : NSObject {
@public
	MenuItemCallback callback;
	Variant meta;
	GlobalMenuCheckType checkable_type;
	int max_states;
	int state;
	Ref<Image> img;
}

@end

@implementation GodotMenuItem
@end

NSImage *MenuBarDisplayMgrOSX::_convert_to_nsimg(Ref<Image> &p_image) const {
	p_image->convert(Image::FORMAT_RGBA8);
	NSBitmapImageRep *imgrep = [[NSBitmapImageRep alloc]
			initWithBitmapDataPlanes:NULL
						  pixelsWide:p_image->get_width()
						  pixelsHigh:p_image->get_height()
					   bitsPerSample:8
					 samplesPerPixel:4
							hasAlpha:YES
							isPlanar:NO
					  colorSpaceName:NSDeviceRGBColorSpace
						 bytesPerRow:int(p_image->get_width()) * 4
						bitsPerPixel:32];
	ERR_FAIL_COND_V(imgrep == nil, nil);
	uint8_t *pixels = [imgrep bitmapData];

	int len = p_image->get_width() * p_image->get_height();
	const uint8_t *r = p_image->get_data().read().ptr();

	// Premultiply the alpha channel
	for (int i = 0; i < len; i++) {
		uint8_t alpha = r[i * 4 + 3];
		pixels[i * 4 + 0] = (uint8_t)(((uint16_t)r[i * 4 + 0] * alpha) / 255);
		pixels[i * 4 + 1] = (uint8_t)(((uint16_t)r[i * 4 + 1] * alpha) / 255);
		pixels[i * 4 + 2] = (uint8_t)(((uint16_t)r[i * 4 + 2] * alpha) / 255);
		pixels[i * 4 + 3] = alpha;
	}

	NSImage *nsimg = [[NSImage alloc] initWithSize:NSMakeSize(p_image->get_width(), p_image->get_height())];
	ERR_FAIL_COND_V(nsimg == nil, nil);
	[nsimg addRepresentation:imgrep];
	return nsimg;
}

const NSMenu *MenuBarDisplayMgrOSX::_get_menu_root(const String &p_menu_root) const {
	const NSMenu *menu = nullptr;
	if (p_menu_root == "" || p_menu_root.to_lower() == "_main") {
		// Main menu.
		menu = [NSApp mainMenu];
	} else if (p_menu_root.to_lower() == "_dock") {
		// macOS dock menu.
		menu = dock_menu;
	} else {
		// Submenu.
		if (submenu.has(p_menu_root)) {
			menu = submenu[p_menu_root];
		}
	}
	if (menu == apple_menu) {
		// Do not allow to change Apple menu.
		return nullptr;
	}
	return menu;
}

NSMenu *MenuBarDisplayMgrOSX::_get_menu_root(const String &p_menu_root) {
	NSMenu *menu = nullptr;
	if (p_menu_root == "" || p_menu_root.to_lower() == "_main") {
		// Main menu.
		menu = [NSApp mainMenu];
	} else if (p_menu_root.to_lower() == "_dock") {
		// macOS dock menu.
		menu = dock_menu;
	} else {
		// Submenu.
		if (!submenu.has(p_menu_root)) {
			NSMenu *n_menu = [[NSMenu alloc] initWithTitle:[NSString stringWithUTF8String:p_menu_root.utf8().get_data()]];
			[n_menu setAutoenablesItems:NO];
			submenu[p_menu_root] = n_menu;
		}
		menu = submenu[p_menu_root];
	}
	if (menu == apple_menu) {
		// Do not allow to change Apple menu.
		return nullptr;
	}
	return menu;
}

bool MenuBarDisplayMgrOSX::_has_help_menu() const {
	if ([NSApp helpMenu]) {
		return true;
	} else {
		NSMenu *menu = [NSApp mainMenu];
		const NSMenuItem *menu_item = [menu itemAtIndex:[menu numberOfItems] - 1];
		if (menu_item) {
			String menu_name = String::utf8([[menu_item title] UTF8String]);
			if (menu_name == "Help" || menu_name == RTR("Help")) {
				return true;
			}
		}
		return false;
	}
}

NSMenuItem *MenuBarDisplayMgrOSX::_menu_add_item(const String &p_menu_root, const String &p_label, uint32_t p_accel, int p_index, int *r_out) {
	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		String keycode = KeyMappingMacOS::keycode_get_native_string(p_accel & KEY_CODE_MASK);
		NSMenuItem *menu_item;
		int item_count = ((menu == [NSApp mainMenu]) && _has_help_menu()) ? [menu numberOfItems] - 1 : [menu numberOfItems];
		if ((menu == [NSApp mainMenu]) && (p_label == "Help" || p_label == RTR("Help"))) {
			p_index = [menu numberOfItems];
		} else if (p_index < 0) {
			p_index = item_count;
		} else {
			if (menu == [NSApp mainMenu]) { // Skip Apple menu.
				p_index++;
			}
			p_index = CLAMP(p_index, 0, item_count);
		}
		menu_item = [menu insertItemWithTitle:[NSString stringWithUTF8String:p_label.utf8().get_data()] action:@selector(globalMenuCallback:) keyEquivalent:[NSString stringWithUTF8String:keycode.utf8().get_data()] atIndex:p_index];
		*r_out = (menu == [NSApp mainMenu]) ? p_index - 1 : p_index;
		return menu_item;
	}
	return nullptr;
}

int MenuBarDisplayMgrOSX::global_menu_add_item(const String &p_menu_root, const String &p_label, const MenuItemCallback &p_callback, const Variant &p_tag, uint32_t p_accel, int p_index) {
	_THREAD_SAFE_METHOD_

	int out = -1;
	NSMenuItem *menu_item = _menu_add_item(p_menu_root, p_label, p_accel, p_index, &out);
	if (menu_item) {
		GodotMenuItem *obj = [[GodotMenuItem alloc] init];
		obj->callback = p_callback;
		obj->meta = p_tag;
		obj->checkable_type = CHECKABLE_TYPE_NONE;
		obj->max_states = 0;
		obj->state = 0;
		[menu_item setKeyEquivalentModifierMask:KeyMappingMacOS::keycode_get_native_mask(p_accel)];
		[menu_item setRepresentedObject:obj];
	}
	return out;
}

int MenuBarDisplayMgrOSX::global_menu_add_submenu_item(const String &p_menu_root, const String &p_label, const String &p_submenu, int p_index) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	NSMenu *sub_menu = _get_menu_root(p_submenu);
	int out = -1;
	if (menu && sub_menu) {
		if (sub_menu == menu) {
			ERR_PRINT("Can't set submenu to self!");
			return -1;
		}
		if ([sub_menu supermenu]) {
			ERR_PRINT("Can't set submenu to menu that is already a submenu of some other menu!");
			return -1;
		}
		NSMenuItem *menu_item;
		int item_count = ((menu == [NSApp mainMenu]) && _has_help_menu()) ? [menu numberOfItems] - 1 : [menu numberOfItems];
		if ((menu == [NSApp mainMenu]) && (p_label == "Help" || p_label == RTR("Help"))) {
			p_index = [menu numberOfItems];
		} else if (p_index < 0) {
			p_index = item_count;
		} else {
			if (menu == [NSApp mainMenu]) { // Skip Apple menu.
				p_index++;
			}
			p_index = CLAMP(p_index, 0, item_count);
		}
		menu_item = [menu insertItemWithTitle:[NSString stringWithUTF8String:p_label.utf8().get_data()] action:nil keyEquivalent:@"" atIndex:p_index];
		out = (menu == [NSApp mainMenu]) ? p_index - 1 : p_index;

		GodotMenuItem *obj = [[GodotMenuItem alloc] init];
		obj->callback = MenuItemCallback();
		obj->checkable_type = CHECKABLE_TYPE_NONE;
		obj->max_states = 0;
		obj->state = 0;
		[menu_item setRepresentedObject:obj];

		[sub_menu setTitle:[NSString stringWithUTF8String:p_label.utf8().get_data()]];
		[menu setSubmenu:sub_menu forItem:menu_item];
	}
	return out;
}

int MenuBarDisplayMgrOSX::global_menu_add_separator(const String &p_menu_root, int p_index) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		if (menu == [NSApp mainMenu]) { // Do not add separators into main menu.
			return -1;
		}
		if (p_index < 0) {
			p_index = [menu numberOfItems];
		} else {
			p_index = CLAMP(p_index, 0, [menu numberOfItems]);
		}
		[menu insertItem:[NSMenuItem separatorItem] atIndex:p_index];
		return p_index;
	}
	return -1;
}

int MenuBarDisplayMgrOSX::global_menu_add_check_item(const String &p_menu_root, const String &p_label, const MenuItemCallback &p_callback, const Variant &p_tag, uint32_t p_accel, int p_index) {
	_THREAD_SAFE_METHOD_

	int out = -1;
	NSMenuItem *menu_item = _menu_add_item(p_menu_root, p_label, p_accel, p_index, &out);
	if (menu_item) {
		GodotMenuItem *obj = [[GodotMenuItem alloc] init];
		obj->callback = p_callback;
		obj->meta = p_tag;
		obj->checkable_type = CHECKABLE_TYPE_CHECK_BOX;
		obj->max_states = 0;
		obj->state = 0;
		[menu_item setKeyEquivalentModifierMask:KeyMappingMacOS::keycode_get_native_mask(p_accel)];
		[menu_item setRepresentedObject:obj];
	}
	return out;
}

String MenuBarDisplayMgrOSX::global_menu_get_item_submenu(const String &p_menu_root, int p_idx) const {
	_THREAD_SAFE_METHOD_

	const NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND_V(p_idx < 0, String());
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND_V(p_idx >= [menu numberOfItems], String());
		const NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			const NSMenu *sub_menu = [menu_item submenu];
			if (sub_menu) {
				for (const KeyValue<String, NSMenu *> &E : submenu) {
					if (E.value == sub_menu) {
						return E.key;
					}
				}
			}
		}
	}
	return String();
}

void MenuBarDisplayMgrOSX::global_menu_set_item_checked(const String &p_menu_root, int p_idx, bool p_checked) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			if (p_checked) {
				[menu_item setState:NSControlStateValueOn];
			} else {
				[menu_item setState:NSControlStateValueOff];
			}
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_checkable(const String &p_menu_root, int p_idx, bool p_checkable) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			GodotMenuItem *obj = [menu_item representedObject];
			ERR_FAIL_COND(!obj);
			obj->checkable_type = (p_checkable) ? CHECKABLE_TYPE_CHECK_BOX : CHECKABLE_TYPE_NONE;
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_radio_checkable(const String &p_menu_root, int p_idx, bool p_checkable) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			GodotMenuItem *obj = [menu_item representedObject];
			ERR_FAIL_COND(!obj);
			obj->checkable_type = (p_checkable) ? CHECKABLE_TYPE_RADIO_BUTTON : CHECKABLE_TYPE_NONE;
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_accelerator(const String &p_menu_root, int p_idx, uint32_t p_keycode) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			[menu_item setKeyEquivalentModifierMask:KeyMappingMacOS::keycode_get_native_mask(p_keycode)];
			String keycode = KeyMappingMacOS::keycode_get_native_string(p_keycode & KEY_CODE_MASK);
			[menu_item setKeyEquivalent:[NSString stringWithUTF8String:keycode.utf8().get_data()]];
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_disabled(const String &p_menu_root, int p_idx, bool p_disabled) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			[menu_item setEnabled:(!p_disabled)];
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_tooltip(const String &p_menu_root, int p_idx, const String &p_tooltip) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			[menu_item setToolTip:[NSString stringWithUTF8String:p_tooltip.utf8().get_data()]];
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_max_states(const String &p_menu_root, int p_idx, int p_max_states) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			GodotMenuItem *obj = [menu_item representedObject];
			ERR_FAIL_COND(!obj);
			obj->max_states = p_max_states;
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_icon(const String &p_menu_root, int p_idx, const Ref<Texture> &p_icon) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			GodotMenuItem *obj = [menu_item representedObject];
			ERR_FAIL_COND(!obj);
			if (p_icon.is_valid()) {
				obj->img = p_icon->get_data();
				obj->img = obj->img->duplicate();
				if (obj->img->is_compressed()) {
					obj->img->decompress();
				}
				obj->img->resize(16, 16, Image::INTERPOLATE_LANCZOS);
				[menu_item setImage:_convert_to_nsimg(obj->img)];
			} else {
				obj->img = Ref<Image>();
				[menu_item setImage:nil];
			}
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_state(const String &p_menu_root, int p_idx, int p_state) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			GodotMenuItem *obj = [menu_item representedObject];
			ERR_FAIL_COND(!obj);
			obj->state = p_state;
		}
	}
}

void MenuBarDisplayMgrOSX::global_menu_set_item_indentation_level(const String &p_menu_root, int p_idx, int p_level) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		NSMenuItem *menu_item = [menu itemAtIndex:p_idx];
		if (menu_item) {
			[menu_item setIndentationLevel:p_level];
		}
	}
}

int MenuBarDisplayMgrOSX::global_menu_get_item_count(const String &p_menu_root) const {
	_THREAD_SAFE_METHOD_

	const NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			return [menu numberOfItems] - 1;
		} else {
			return [menu numberOfItems];
		}
	} else {
		return 0;
	}
}

void MenuBarDisplayMgrOSX::global_menu_remove_item(const String &p_menu_root, int p_idx) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		ERR_FAIL_COND(p_idx < 0);
		if (menu == [NSApp mainMenu]) { // Skip Apple menu.
			p_idx++;
		}
		ERR_FAIL_COND(p_idx >= [menu numberOfItems]);
		[menu removeItemAtIndex:p_idx];
	}
}

void MenuBarDisplayMgrOSX::global_menu_clear(const String &p_menu_root) {
	_THREAD_SAFE_METHOD_

	NSMenu *menu = _get_menu_root(p_menu_root);
	if (menu) {
		[menu removeAllItems];
		// Restore Apple menu.
		if (menu == [NSApp mainMenu]) {
			NSMenuItem *menu_item = [menu addItemWithTitle:@"" action:nil keyEquivalent:@""];
			[menu setSubmenu:apple_menu forItem:menu_item];
		}
		if (submenu.has(p_menu_root)) {
			submenu.erase(p_menu_root);
		}
	}
}
