/**************************************************************************/
/*  keychain_stub.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Platform stub for unsupported platforms — falls back to insecure       */
/* storage or returns NOT_IMPLEMENTED.                                    */
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
