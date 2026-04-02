/**************************************************************************/
/*  keychain_stub.cpp                                                     */
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

#include "keychain.h"

#if !defined(OSX_ENABLED) && !defined(IPHONE_ENABLED) && !defined(WINDOWS_ENABLED) && !defined(X11_ENABLED) && !defined(SERVER_ENABLED) && !defined(ANDROID_ENABLED)

Keychain::Error Keychain::_platform_read(const String &p_key, PoolByteArray &r_data) {
	_last_error_string = "No secure keychain backend available on this platform.";
	return NO_BACKEND_AVAILABLE;
}

Keychain::Error Keychain::_platform_write(const String &p_key, const PoolByteArray &p_data) {
	_last_error_string = "No secure keychain backend available on this platform.";
	return NO_BACKEND_AVAILABLE;
}

Keychain::Error Keychain::_platform_delete(const String &p_key) {
	_last_error_string = "No secure keychain backend available on this platform.";
	return NO_BACKEND_AVAILABLE;
}

#endif
