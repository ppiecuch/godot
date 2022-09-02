/*************************************************************************/
/*  key_mapping_macos.mm                                                 */
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

#include "key_mapping_macos.h"
#include "core/os/keyboard.h"

#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

bool KeyMappingMacOS::is_numpad_key(unsigned int key) {
	static const unsigned int table[] = {
		0x41, /* kVK_ANSI_KeypadDecimal */
		0x43, /* kVK_ANSI_KeypadMultiply */
		0x45, /* kVK_ANSI_KeypadPlus */
		0x47, /* kVK_ANSI_KeypadClear */
		0x4b, /* kVK_ANSI_KeypadDivide */
		0x4c, /* kVK_ANSI_KeypadEnter */
		0x4e, /* kVK_ANSI_KeypadMinus */
		0x51, /* kVK_ANSI_KeypadEquals */
		0x52, /* kVK_ANSI_Keypad0 */
		0x53, /* kVK_ANSI_Keypad1 */
		0x54, /* kVK_ANSI_Keypad2 */
		0x55, /* kVK_ANSI_Keypad3 */
		0x56, /* kVK_ANSI_Keypad4 */
		0x57, /* kVK_ANSI_Keypad5 */
		0x58, /* kVK_ANSI_Keypad6 */
		0x59, /* kVK_ANSI_Keypad7 */
		0x5b, /* kVK_ANSI_Keypad8 */
		0x5c, /* kVK_ANSI_Keypad9 */
		0x5f, /* kVK_JIS_KeypadComma */
		0x00
	};
	for (int i = 0; table[i] != 0; i++) {
		if (key == table[i]) {
			return true;
		}
	}
	return false;
}

// Keyboard symbol translation table.
static const uint32_t _macos_to_godot_table[128] = {
	/* 00 */ KEY_A,
	/* 01 */ KEY_S,
	/* 02 */ KEY_D,
	/* 03 */ KEY_F,
	/* 04 */ KEY_H,
	/* 05 */ KEY_G,
	/* 06 */ KEY_Z,
	/* 07 */ KEY_X,
	/* 08 */ KEY_C,
	/* 09 */ KEY_V,
	/* 0a */ KEY_SECTION, /* ISO Section */
	/* 0b */ KEY_B,
	/* 0c */ KEY_Q,
	/* 0d */ KEY_W,
	/* 0e */ KEY_E,
	/* 0f */ KEY_R,
	/* 10 */ KEY_Y,
	/* 11 */ KEY_T,
	/* 12 */ KEY_1,
	/* 13 */ KEY_2,
	/* 14 */ KEY_3,
	/* 15 */ KEY_4,
	/* 16 */ KEY_6,
	/* 17 */ KEY_5,
	/* 18 */ KEY_EQUAL,
	/* 19 */ KEY_9,
	/* 1a */ KEY_7,
	/* 1b */ KEY_MINUS,
	/* 1c */ KEY_8,
	/* 1d */ KEY_0,
	/* 1e */ KEY_BRACERIGHT,
	/* 1f */ KEY_O,
	/* 20 */ KEY_U,
	/* 21 */ KEY_BRACELEFT,
	/* 22 */ KEY_I,
	/* 23 */ KEY_P,
	/* 24 */ KEY_ENTER,
	/* 25 */ KEY_L,
	/* 26 */ KEY_J,
	/* 27 */ KEY_APOSTROPHE,
	/* 28 */ KEY_K,
	/* 29 */ KEY_SEMICOLON,
	/* 2a */ KEY_BACKSLASH,
	/* 2b */ KEY_COMMA,
	/* 2c */ KEY_SLASH,
	/* 2d */ KEY_N,
	/* 2e */ KEY_M,
	/* 2f */ KEY_PERIOD,
	/* 30 */ KEY_TAB,
	/* 31 */ KEY_SPACE,
	/* 32 */ KEY_QUOTELEFT,
	/* 33 */ KEY_BACKSPACE,
	/* 34 */ KEY_UNKNOWN,
	/* 35 */ KEY_ESCAPE,
	/* 36 */ KEY_META,
	/* 37 */ KEY_META,
	/* 38 */ KEY_SHIFT,
	/* 39 */ KEY_CAPSLOCK,
	/* 3a */ KEY_ALT,
	/* 3b */ KEY_CONTROL,
	/* 3c */ KEY_SHIFT,
	/* 3d */ KEY_ALT,
	/* 3e */ KEY_CONTROL,
	/* 3f */ KEY_UNKNOWN, /* Function */
	/* 40 */ KEY_F17,
	/* 41 */ KEY_KP_PERIOD,
	/* 42 */ KEY_UNKNOWN,
	/* 43 */ KEY_KP_MULTIPLY,
	/* 44 */ KEY_UNKNOWN,
	/* 45 */ KEY_KP_ADD,
	/* 46 */ KEY_UNKNOWN,
	/* 47 */ KEY_NUMLOCK, /* Really KeypadClear... */
	/* 48 */ KEY_VOLUMEUP, /* VolumeUp */
	/* 49 */ KEY_VOLUMEDOWN, /* VolumeDown */
	/* 4a */ KEY_VOLUMEMUTE, /* Mute */
	/* 4b */ KEY_KP_DIVIDE,
	/* 4c */ KEY_KP_ENTER,
	/* 4d */ KEY_UNKNOWN,
	/* 4e */ KEY_KP_SUBTRACT,
	/* 4f */ KEY_F18,
	/* 50 */ KEY_F19,
	/* 51 */ KEY_EQUAL, /* KeypadEqual */
	/* 52 */ KEY_KP_0,
	/* 53 */ KEY_KP_1,
	/* 54 */ KEY_KP_2,
	/* 55 */ KEY_KP_3,
	/* 56 */ KEY_KP_4,
	/* 57 */ KEY_KP_5,
	/* 58 */ KEY_KP_6,
	/* 59 */ KEY_KP_7,
	/* 5a */ KEY_F20,
	/* 5b */ KEY_KP_8,
	/* 5c */ KEY_KP_9,
	/* 5d */ KEY_YEN, /* JIS Yen */
	/* 5e */ KEY_UNDERSCORE, /* JIS Underscore */
	/* 5f */ KEY_COMMA, /* JIS KeypadComma */
	/* 60 */ KEY_F5,
	/* 61 */ KEY_F6,
	/* 62 */ KEY_F7,
	/* 63 */ KEY_F3,
	/* 64 */ KEY_F8,
	/* 65 */ KEY_F9,
	/* 66 */ KEY_UNKNOWN, /* JIS Eisu */
	/* 67 */ KEY_F11,
	/* 68 */ KEY_UNKNOWN, /* JIS Kana */
	/* 69 */ KEY_F13,
	/* 6a */ KEY_F16,
	/* 6b */ KEY_F14,
	/* 6c */ KEY_UNKNOWN,
	/* 6d */ KEY_F10,
	/* 6e */ KEY_MENU,
	/* 6f */ KEY_F12,
	/* 70 */ KEY_UNKNOWN,
	/* 71 */ KEY_F15,
	/* 72 */ KEY_INSERT, /* Really Help... */
	/* 73 */ KEY_HOME,
	/* 74 */ KEY_PAGEUP,
	/* 75 */ KEY_DELETE,
	/* 76 */ KEY_F4,
	/* 77 */ KEY_END,
	/* 78 */ KEY_F2,
	/* 79 */ KEY_PAGEDOWN,
	/* 7a */ KEY_F1,
	/* 7b */ KEY_LEFT,
	/* 7c */ KEY_RIGHT,
	/* 7d */ KEY_DOWN,
	/* 7e */ KEY_UP,
	/* 7f */ KEY_UNKNOWN,
};

// Translates a OS X keycode to a Godot keycode.
uint32_t KeyMappingMacOS::translate_key(unsigned int key) {
	if (key >= 128) {
		return KEY_UNKNOWN;
	}

	return _macos_to_godot_table[key];
}

// Translates a Godot keycode back to a macOS keycode.
unsigned int KeyMappingMacOS::unmap_key(uint32_t key) {
	for (int i = 0; i <= 126; i++) {
		if (_macos_to_godot_table[i] == key) {
			return i;
		}
	}
	return 127;
}

struct _KeyCodeMap {
	uint32_t kchar;
	uint32_t kcode;
};

static const _KeyCodeMap _keycodes[55] = {
	{ '`', KEY_QUOTELEFT },
	{ '~', KEY_ASCIITILDE },
	{ '0', KEY_0 },
	{ '1', KEY_1 },
	{ '2', KEY_2 },
	{ '3', KEY_3 },
	{ '4', KEY_4 },
	{ '5', KEY_5 },
	{ '6', KEY_6 },
	{ '7', KEY_7 },
	{ '8', KEY_8 },
	{ '9', KEY_9 },
	{ '-', KEY_MINUS },
	{ '_', KEY_UNDERSCORE },
	{ '=', KEY_EQUAL },
	{ '+', KEY_PLUS },
	{ 'q', KEY_Q },
	{ 'w', KEY_W },
	{ 'e', KEY_E },
	{ 'r', KEY_R },
	{ 't', KEY_T },
	{ 'y', KEY_Y },
	{ 'u', KEY_U },
	{ 'i', KEY_I },
	{ 'o', KEY_O },
	{ 'p', KEY_P },
	{ '[', KEY_BRACELEFT },
	{ ']', KEY_BRACERIGHT },
	{ '{', KEY_BRACELEFT },
	{ '}', KEY_BRACERIGHT },
	{ 'a', KEY_A },
	{ 's', KEY_S },
	{ 'd', KEY_D },
	{ 'f', KEY_F },
	{ 'g', KEY_G },
	{ 'h', KEY_H },
	{ 'j', KEY_J },
	{ 'k', KEY_K },
	{ 'l', KEY_L },
	{ ';', KEY_SEMICOLON },
	{ ':', KEY_COLON },
	{ '\'', KEY_APOSTROPHE },
	{ '\"', KEY_QUOTEDBL },
	{ '\\', KEY_BACKSLASH },
	{ '#', KEY_NUMBERSIGN },
	{ 'z', KEY_Z },
	{ 'x', KEY_X },
	{ 'c', KEY_C },
	{ 'v', KEY_V },
	{ 'b', KEY_B },
	{ 'n', KEY_N },
	{ 'm', KEY_M },
	{ ',', KEY_COMMA },
	{ '.', KEY_PERIOD },
	{ '/', KEY_SLASH }
};

// Remap key according to current keyboard layout.
uint32_t KeyMappingMacOS::remap_key(unsigned int key, unsigned int state) {
	if (is_numpad_key(key)) {
		return translate_key(key);
	}

	TISInputSourceRef current_keyboard = TISCopyCurrentKeyboardInputSource();
	if (!current_keyboard) {
		return translate_key(key);
	}

	CFDataRef layout_data = (CFDataRef)TISGetInputSourceProperty(current_keyboard, kTISPropertyUnicodeKeyLayoutData);
	if (!layout_data) {
		return translate_key(key);
	}

	const UCKeyboardLayout *keyboard_layout = (const UCKeyboardLayout *)CFDataGetBytePtr(layout_data);

	UInt32 keys_down = 0;
	UniChar chars[4];
	UniCharCount real_length;

	OSStatus err = UCKeyTranslate(keyboard_layout,
			key,
			kUCKeyActionDisplay,
			(state >> 8) & 0xFF,
			LMGetKbdType(),
			kUCKeyTranslateNoDeadKeysBit,
			&keys_down,
			sizeof(chars) / sizeof(chars[0]),
			&real_length,
			chars);

	if (err != noErr) {
		return translate_key(key);
	}

	for (unsigned int i = 0; i < 55; i++) {
		if (_keycodes[i].kchar == chars[0]) {
			return _keycodes[i].kcode;
		}
	}
	return translate_key(key);
}

struct _KeyCodeText {
	uint32_t code;
	char32_t text;
};

static const _KeyCodeText _native_keycodes[] = {
	/* clang-format off */
		{KEY_ESCAPE                        ,0x001B},
		{KEY_TAB                           ,0x0009},
		{KEY_BACKTAB                       ,0x007F},
		{KEY_BACKSPACE                     ,0x0008},
		{KEY_ENTER                         ,0x000D},
		{KEY_INSERT                        ,NSInsertFunctionKey},
		{KEY_DELETE                    ,0x007F},
		{KEY_PAUSE                         ,NSPauseFunctionKey},
		{KEY_PRINT                         ,NSPrintScreenFunctionKey},
		{KEY_SYSREQ                        ,NSSysReqFunctionKey},
		{KEY_CLEAR                         ,NSClearLineFunctionKey},
		{KEY_HOME                          ,0x2196},
		{KEY_END                           ,0x2198},
		{KEY_LEFT                          ,0x001C},
		{KEY_UP                            ,0x001E},
		{KEY_RIGHT                         ,0x001D},
		{KEY_DOWN                          ,0x001F},
		{KEY_PAGEUP                        ,0x21DE},
		{KEY_PAGEDOWN                      ,0x21DF},
		{KEY_NUMLOCK                       ,NSClearLineFunctionKey},
		{KEY_SCROLLLOCK                    ,NSScrollLockFunctionKey},
		{KEY_F1                            ,NSF1FunctionKey},
		{KEY_F2                            ,NSF2FunctionKey},
		{KEY_F3                            ,NSF3FunctionKey},
		{KEY_F4                            ,NSF4FunctionKey},
		{KEY_F5                            ,NSF5FunctionKey},
		{KEY_F6                            ,NSF6FunctionKey},
		{KEY_F7                            ,NSF7FunctionKey},
		{KEY_F8                            ,NSF8FunctionKey},
		{KEY_F9                            ,NSF9FunctionKey},
		{KEY_F10                           ,NSF10FunctionKey},
		{KEY_F11                           ,NSF11FunctionKey},
		{KEY_F12                           ,NSF12FunctionKey},
		{KEY_F13                           ,NSF13FunctionKey},
		{KEY_F14                           ,NSF14FunctionKey},
		{KEY_F15                           ,NSF15FunctionKey},
		{KEY_F16                           ,NSF16FunctionKey},
		{KEY_F17                           ,NSF17FunctionKey},
		{KEY_F18                           ,NSF18FunctionKey},
		{KEY_F19                           ,NSF19FunctionKey},
		{KEY_F20                           ,NSF20FunctionKey},
		{KEY_F21                           ,NSF21FunctionKey},
		{KEY_F22                           ,NSF22FunctionKey},
		{KEY_F23                           ,NSF23FunctionKey},
		{KEY_F24                           ,NSF24FunctionKey},
		{KEY_F25                           ,NSF25FunctionKey},
		{KEY_F26                           ,NSF26FunctionKey},
		{KEY_F27                           ,NSF27FunctionKey},
		{KEY_F28                           ,NSF28FunctionKey},
		{KEY_F29                           ,NSF29FunctionKey},
		{KEY_F30                           ,NSF30FunctionKey},
		{KEY_F31                           ,NSF31FunctionKey},
		{KEY_F32                           ,NSF32FunctionKey},
		{KEY_F33                           ,NSF33FunctionKey},
		{KEY_F34                           ,NSF34FunctionKey},
		{KEY_F35                           ,NSF35FunctionKey},
		{KEY_MENU                          ,NSMenuFunctionKey},
		{KEY_HELP                          ,NSHelpFunctionKey},
		{KEY_STOP                          ,NSStopFunctionKey},
		{KEY_LAUNCH0                       ,NSUserFunctionKey},
		{KEY_SPACE                         ,0x0020},
		{KEY_EXCLAM                        ,'!'},
		{KEY_QUOTEDBL                      ,'\"'},
		{KEY_NUMBERSIGN                    ,'#'},
		{KEY_DOLLAR                        ,'$'},
		{KEY_PERCENT                       ,'\%'},
		{KEY_AMPERSAND                     ,'&'},
		{KEY_APOSTROPHE                    ,'\''},
		{KEY_PARENLEFT                     ,'('},
		{KEY_PARENRIGHT                    ,')'},
		{KEY_ASTERISK                      ,'*'},
		{KEY_PLUS                          ,'+'},
		{KEY_COMMA                         ,','},
		{KEY_MINUS                         ,'-'},
		{KEY_PERIOD                        ,'.'},
		{KEY_SLASH                         ,'/'},
		{KEY_0                         ,'0'},
		{KEY_1                         ,'1'},
		{KEY_2                         ,'2'},
		{KEY_3                         ,'3'},
		{KEY_4                         ,'4'},
		{KEY_5                         ,'5'},
		{KEY_6                         ,'6'},
		{KEY_7                         ,'7'},
		{KEY_8                         ,'8'},
		{KEY_9                         ,'9'},
		{KEY_COLON                         ,':'},
		{KEY_SEMICOLON                     ,';'},
		{KEY_LESS                          ,'<'},
		{KEY_EQUAL                         ,'='},
		{KEY_GREATER                       ,'>'},
		{KEY_QUESTION                      ,'?'},
		{KEY_AT                            ,'@'},
		{KEY_A                             ,'a'},
		{KEY_B                             ,'b'},
		{KEY_C                             ,'c'},
		{KEY_D                             ,'d'},
		{KEY_E                             ,'e'},
		{KEY_F                             ,'f'},
		{KEY_G                             ,'g'},
		{KEY_H                             ,'h'},
		{KEY_I                             ,'i'},
		{KEY_J                             ,'j'},
		{KEY_K                             ,'k'},
		{KEY_L                             ,'l'},
		{KEY_M                             ,'m'},
		{KEY_N                             ,'n'},
		{KEY_O                             ,'o'},
		{KEY_P                             ,'p'},
		{KEY_Q                             ,'q'},
		{KEY_R                             ,'r'},
		{KEY_S                             ,'s'},
		{KEY_T                             ,'t'},
		{KEY_U                             ,'u'},
		{KEY_V                             ,'v'},
		{KEY_W                             ,'w'},
		{KEY_X                             ,'x'},
		{KEY_Y                             ,'y'},
		{KEY_Z                             ,'z'},
		{KEY_BRACKETLEFT                   ,'['},
		{KEY_BACKSLASH                     ,'\\'},
		{KEY_BRACKETRIGHT                  ,']'},
		{KEY_ASCIICIRCUM                   ,'^'},
		{KEY_UNDERSCORE                    ,'_'},
		{KEY_QUOTELEFT                     ,'`'},
		{KEY_BRACELEFT                     ,'{'},
		{KEY_BAR                           ,'|'},
		{KEY_BRACERIGHT                    ,'}'},
		{KEY_ASCIITILDE                    ,'~'},
		{KEY_NONE                          ,0x0000}
	/* clang-format on */
};

String KeyMappingMacOS::keycode_get_native_string(uint32_t p_keycode) {
	const _KeyCodeText *kct = &_native_keycodes[0];

	while (kct->text) {
		if (kct->code == p_keycode) {
			return String::chr(kct->text);
		}
		kct++;
	}
	return String();
}

unsigned int KeyMappingMacOS::keycode_get_native_mask(uint32_t p_keycode) {
	unsigned int mask = 0;
	if ((p_keycode & KEY_MASK_CTRL) != KEY_NONE) {
		mask |= NSEventModifierFlagControl;
	}
	if ((p_keycode & KEY_MASK_ALT) != KEY_NONE) {
		mask |= NSEventModifierFlagOption;
	}
	if ((p_keycode & KEY_MASK_SHIFT) != KEY_NONE) {
		mask |= NSEventModifierFlagShift;
	}
	if ((p_keycode & KEY_MASK_META) != KEY_NONE) {
		mask |= NSEventModifierFlagCommand;
	}
	if ((p_keycode & KEY_MASK_KPAD) != KEY_NONE) {
		mask |= NSEventModifierFlagNumericPad;
	}
	return mask;
}
