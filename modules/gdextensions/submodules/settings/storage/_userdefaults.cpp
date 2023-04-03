/**************************************************************************/
/*  _userdefaults.cpp                                                     */
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

#include <CoreFoundation/CFPreferences.h>
#include <CoreFoundation/CoreFoundation.h>

#include "core/os/os.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "core/vector.h"

#include <memory>

class SettingsStorage : public Reference {
private:
	String _to_godot_string(CFStringRef cf_string);

	real_t _last_sync_time;
	void _sync();

public:
	void set(const String &key, const Variant &value);
	Variant get(const String &key, const Variant &default_val = Variant()) const;

	SettingsStorage();
	~SettingsStorage();
};

String SettingsStorage::_to_godot_string(CFStringRef cf_string) {
	String ret;
	if (cf_string) {
		CFIndex length = sizeof(UniChar) * CFStringGetLength(cf_string) + 1;
		char *result = (char *)calloc(1, length);
		if (result) {
			if (!CFStringGetCString(cf_string, result, length, kCFStringEncodingASCII)) {
				if (!CFStringGetCString(cf_string, result, length, kCFStringEncodingUTF8)) {
					free(result);
					result = nullptr;
				} else {
					ret = String::utf8(result);
				}
			} else {
				ret = String(result);
			}
		}
	}
	return ret;
}

void SettingsStorage::_sync() {
	if (OS::get_singleton()->get_ticks_msec() - _last_sync_time > 1000) {
		CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
		_last_sync_time = OS::get_singleton()->get_ticks_msec();
	}
}

Variant SettingsStorage::get(const String &key, const Variant &default_val) const {
	Variant ret = default_val;
	CFStringRef nom = CFStringCreateWithCString(nullptr, key.utf8().c_str(), CFStringGetSystemEncoding());
	ERR_FAIL_COND_V(nom == nullptr, ret);

	CFTypeRef value = CFPreferencesCopyAppValue(nom, kCFPreferencesCurrentApplication);
	if (value != nullptr) {
		if (CFGetTypeID(value) == CFBooleanGetTypeID()) {
			Boolean keyExistsAndHasValidFormat = false;
			Boolean val = CFPreferencesGetAppBooleanValue(nom, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat);
			if (keyExistsAndHasValidFormat) {
				ret = bool(val);
			}
		} else if (CFGetTypeID(value) == CFNumberGetTypeID()) {
			switch (CFNumberGetType(CFNumberRef(value))) {
				case kCFNumberIntType: {
					int val;
					if (CFNumberGetValue(CFNumberRef(value), kCFNumberIntType, &val)) {
						ret = val;
					}
					break;
				}
				case kCFNumberFloatType: {
					float val;
					if (CFNumberGetValue(CFNumberRef(value), kCFNumberFloatType, &val)) {
						ret = val;
					}
					break;
				}
				case kCFNumberDoubleType:
				default: {
					double val;
					if (CFNumberGetValue(CFNumberRef(value), kCFNumberDoubleType, &val)) {
						ret = val;
					}
				}
			}
		} else if (CFGetTypeID(value) == CFStringGetTypeID()) {
			CFIndex length = sizeof(UniChar) * CFStringGetLength(CFStringRef(value)) + 1;
			char *result = (char *)calloc(1, length);
			if (result) {
				if (!CFStringGetCString(CFStringRef(value), result, length, kCFStringEncodingASCII)) {
					if (CFStringGetCString(CFStringRef(value), result, length, kCFStringEncodingUTF8)) {
						ret = String::utf8(result);
					}
				} else {
					ret = String(result);
				}
				free(result);
			}
		} else if (CFGetTypeID(value) == CFDataGetTypeID()) {
			PoolByteArray data;
			data.resize(CFDataGetLength(CFDataRef(value)));
			memcpy(data.write().ptr(), CFDataGetBytePtr(CFDataRef(value)), data.size());
			ret = decode_var(data);
		}
		CFRelease(value);
	}
	return ret;
}

void SettingsStorage::set(const String &key, const Variant &value) {
	CFStringRef nom = CFStringCreateWithCString(nullptr, key.utf8().c_str(), CFStringGetSystemEncoding());
	ERR_FAIL_COND(nom == nullptr);
	switch (value.get_type()) {
		case Variant::BOOL: {
			CFPreferencesSetAppValue(nom, value ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication);
			_sync();
		}
		case Variant::INT: {
			const int val = value;
			CFNumberRef cftype = CFNumberCreate(nullptr, kCFNumberIntType, &val);
			if (cftype != nullptr) {
				CFPreferencesSetAppValue(nom, cftype, kCFPreferencesCurrentApplication);
				CFRelease(cftype);
				_sync();
			}
		}
		case Variant::REAL: {
			const double val = value;
			CFNumberRef cftype = CFNumberCreate(nullptr, kCFNumberDoubleType, &val);
			if (cftype != nullptr) {
				CFPreferencesSetAppValue(nom, cftype, kCFPreferencesCurrentApplication);
				CFRelease(cftype);
				_sync();
			}
		}
		case Variant::STRING: {
			const String val = value;
			CFStringRef cftype = CFStringCreateWithCString(nullptr, val.utf8().c_str(), CFStringGetSystemEncoding());
			if (cftype != nullptr) {
				CFPreferencesSetAppValue(nom, cftype, kCFPreferencesCurrentApplication);
				CFRelease(cftype);
				_sync();
			}
		}
		default: {
			PoolByteArray val = encode_var(value);
			CFDataRef cftype = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(val.read().ptr()), CFIndex(val.size()));
			if (cftype != nullptr) {
				CFPreferencesSetAppValue(nom, cftype, kCFPreferencesCurrentApplication);
				CFRelease(cftype);
				_sync();
			}
		}
	}
	CFRelease(nom);
}

SettingsStorage::SettingsStorage() {
	_last_sync_time = OS::get_singleton()->get_ticks_msec();
}

SettingsStorage::~SettingsStorage() {
	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}
