
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPreferences.h>

#include "core/ustring.h"
#include "core/vector.h"
#include "core/os/os.h"

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
};

String SettingsStorage::_to_godot_string(CFStringRef cf_string) {
	String ret;
	if (cf_string) {
		CFIndex length = sizeof(UniChar) * CFStringGetLength( cf_string ) + 1;
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
	CFStringRef nom = CFStringCreateWithCString (nullptr, key.utf8().c_str(), CFStringGetSystemEncoding());
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
			CFIndex length = sizeof(UniChar) * CFStringGetLength(CFStringRef(value) ) + 1;
			char *result = (char*)calloc(1, length);
			if (result) {
				if (!CFStringGetCString(CFStringRef(value), result, length, kCFStringEncodingASCII)) {
					if (CFStringGetCString(CFStringRef(value), result, length, kCFStringEncodingUTF8 ) ) {
						ret = String::utf8(result);
					}
				} else {
					ret = String(result);
				}
				free(result);
			}
		}
		CFRelease(value);
	}
	return ret;
}

void SettingsStorage::set(const String &key, const Variant &value) {
	CFStringRef nom = CFStringCreateWithCString (nullptr, key.utf8().c_str(), CFStringGetSystemEncoding());
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
				CFPreferencesSetAppValue (nom, cftype, kCFPreferencesCurrentApplication);
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
		default:
			break;
	}
	CFRelease(nom);
}

SettingsStorage::SettingsStorage() {
	_last_sync_time = OS::get_singleton()->get_ticks_msec();
}
