/**************************************************************************/
/*  _winreg.cpp                                                           */
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

#include "core/int_types.h"
#include "core/reference.h"
#include "core/variant.h"

#define UNICODE
#include <windows.h>

/// Helpers

static const REGSAM registryPermissions = KEY_READ | KEY_WRITE;

typedef Vector<String> StringList;
typedef PoolVector<wchar_t> PoolCharArray;

static String key_path(const String &key) {
	int idx = key.rfind("\\");
	if (idx == -1) {
		return String();
	}
	return key.substr(0, idx + 1);
}

static String key_name(const String &key) {
	int idx = key.rfind("\\");
	String res;
	if (idx == -1) {
		res = key;
	} else {
		res = key.substr(idx + 1);
	}
	if (res == "Default" || res == ".") {
		res = String();
	}
	return res;
}

static String escaped_key(String key) {
	return key.replace("\\", "/");
}

static String unescaped_key(String key) {
	return key.replace("/", "\\");
}

template <typename T>
static PoolVector<T> from_data(const T *data, size_t data_count) {
	PoolVector<T> pv;
	pv.resize(data_count);
	if (data) {
		memcpy(pv.write().ptr(), data, data_count * sizeof(T));
	}
	return pv;
}

// Open a key with the specified "perms".
// "access" is to explicitly use the 32- or 64-bit branch.
static HKEY open_key(HKEY parentHandle, REGSAM perms, const String &sub_key, REGSAM access = 0) {
	HKEY resultHandle = 0;
	LONG res = RegOpenKeyEx(parentHandle, sub_key.c_str(), 0, perms | access, &resultHandle);
	if (res == ERROR_SUCCESS) {
		return resultHandle;
	}
	return 0;
}

// Open a key with the specified "perms", create it if it does not exist.
// "access" is to explicitly use the 32- or 64-bit branch.
static HKEY create_or_open_key(HKEY parentHandle, REGSAM perms, const String &sub_key, REGSAM access = 0) {
	// try to open it
	HKEY resultHandle = open_key(parentHandle, perms, sub_key, access);
	if (resultHandle != 0) {
		return resultHandle;
	}
	// try to create it
	LONG res = RegCreateKeyEx(parentHandle, sub_key.c_str(), 0, 0, REG_OPTION_NON_VOLATILE, perms | access, 0, &resultHandle, 0);
	if (res == ERROR_SUCCESS) {
		return resultHandle;
	}
	return 0;
}

// Open or create a key in read-write mode if possible, otherwise read-only.
// "access" is to explicitly use the 32- or 64-bit branch.
static HKEY create_or_open_key(HKEY parentHandle, const String &sub_key, bool *readOnly, REGSAM access = 0) {
	// try to open or create it read/write
	HKEY resultHandle = create_or_open_key(parentHandle, registryPermissions, sub_key, access);
	if (resultHandle != 0) {
		if (readOnly != 0) {
			*readOnly = false;
		}
		return resultHandle;
	}
	// try to open or create it read/only
	resultHandle = create_or_open_key(parentHandle, KEY_READ, sub_key, access);
	if (resultHandle != 0) {
		if (readOnly != 0) {
			*readOnly = true;
		}
		return resultHandle;
	}
	return 0;
}

static StringList child_keys_or_groups(HKEY parentHandle, bool child_keys_or_grps) {
	StringList result;
	DWORD numKeys, maxKeySize;
	DWORD numSubgroups, maxSubgroupSize;

	// Find the number of keys and subgroups, as well as the max of their lengths.
	LONG res = RegQueryInfoKey(parentHandle, 0, 0, 0, &numSubgroups, &maxSubgroupSize, 0, &numKeys, &maxKeySize, 0, 0, 0);

	if (res != ERROR_SUCCESS) {
		WARN_PRINT(vformat("Settings: RegQueryInfoKey() failed: code %d", int(res)));
		return result;
	}

	++maxSubgroupSize;
	++maxKeySize;

	int n, m;
	if (child_keys_or_grps) {
		n = numKeys;
		m = maxKeySize;
	} else {
		n = numSubgroups;
		m = maxSubgroupSize;
	}

	++m; // The size does not include the terminating null character.

	// Get the list
	PoolCharArray buff = from_data(static_cast<wchar_t *>(0), m);
	for (int i = 0; i < n; ++i) {
		String item;
		DWORD l = m;
		if (child_keys_or_grps) {
			res = RegEnumValue(parentHandle, i, buff.write().ptr(), &l, 0, 0, 0, 0);
		} else {
			res = RegEnumKeyEx(parentHandle, i, buff.write().ptr(), &l, 0, 0, 0, 0);
		}
		if (res == ERROR_SUCCESS) {
			item = String(buff.read().ptr());
		} else {
			WARN_PRINT(vformat("Settings: RegEnumValue failed: code %d", int(res)));
			continue;
		}
		if (item.empty()) {
			item = ".";
		}
		result.push_back(item);
	}
	return result;
}

static void delete_child_groups(HKEY parentHandle, REGSAM access = 0) {
	StringList childGroups = child_keys_or_groups(parentHandle, false);
	for (const String &group : childGroups) {
		// delete subgroups in group
		HKEY childGroupHandle = open_key(parentHandle, registryPermissions, group, access);
		if (childGroupHandle == 0) {
			continue;
		}
		delete_child_groups(childGroupHandle, access);
		RegCloseKey(childGroupHandle);
		// delete group itself
		LONG res = RegDeleteKey(parentHandle, group.c_str());
		if (res != ERROR_SUCCESS) {
			WARN_PRINT(vformat("Settings: RegDeleteKey failed on subkey \"%s\", code: %d", group, int(res)));
			return;
		}
	}
}

class RegistryKey {
public:
	RegistryKey(HKEY parent_handle = 0, const String &key = String(), bool read_only = true, REGSAM access = 0) :
			_parent_handle(parent_handle), _handle(0), _key(key), _read_only(read_only), _access(access) {}
	String key() const { return _key; }
	HKEY handle() const;
	HKEY parentHandle() const { return _parent_handle; }
	bool read_only() const { return _read_only; }
	void close();

private:
	HKEY _parent_handle;
	mutable HKEY _handle;
	String _key;
	mutable bool _read_only;
	REGSAM _access;
};

HKEY RegistryKey::handle() const {
	if (_handle != 0)
		return _handle;

	if (_read_only)
		_handle = open_key(_parent_handle, KEY_READ, _key, _access);
	else
		_handle = create_or_open_key(_parent_handle, _key, &_read_only, _access);

	return _handle;
}

void RegistryKey::close() {
	if (_handle != 0)
		RegCloseKey(_handle);
	_handle = 0;
}

typedef Vector<RegistryKey> RegistryKeyList;

class SettingsStorage : public Reference {
public:
	SettingsStorage(const String &organization = "GodotSoftware", const String &application = get_app_name(), REGSAM access = 0);
	SettingsStorage(String key, REGSAM access = 0);
	~SettingsStorage();

	bool readKey(HKEY parentHandle, const String &sub_key, Variant *value) const;
	void remove(const String &key);
	void set(const String &key, const Variant &value);
	bool get(const String &key, Variant &value) const;
	Variant get(const String &key) const;
	void clear();
	void sync();
	void flush();
	bool isWritable() const;
	HKEY writeHandle() const;
	String fileName() const;

private:
	RegistryKeyList regList; // list of registry locations to search for keys
	REGSAM access;
	bool fallbacks;
};

SettingsStorage::~SettingsStorage() {
	for (int i = 0; i < regList.size(); ++i) {
		regList.get(i).close();
	}
}

SettingsStorage::SettingsStorage(const String &organization, const String &application, REGSAM access) :
		access(access), fallbacks(true) {
	if (!organization.empty()) {
		String prefix = "Software\\" + organization;
		String org_prefix = prefix + "\\OrganizationDefaults";
		String app_prefix = prefix + "\\" + application;

		if (!application.empty()) {
			regList.push_back(RegistryKey(HKEY_CURRENT_USER, app_prefix, !regList.empty(), access));
		}
		regList.push_back(RegistryKey(HKEY_CURRENT_USER, org_prefix, !regList.empty(), access));
		if (!application.empty()) {
			regList.push_back(RegistryKey(HKEY_LOCAL_MACHINE, app_prefix, !regList.empty(), access));
		}
		regList.push_back(RegistryKey(HKEY_LOCAL_MACHINE, org_prefix, !regList.empty(), access));
	}

	ERR_FAIL_COND_MSG(regList.empty(), "Access error");
}

SettingsStorage::SettingsStorage(String path, REGSAM access) :
		access(access), fallbacks(true) {
	if (path.begins_with("\\")) {
		path.remove(0);
	}

	int keyLength;
	HKEY keyName;

	if (path.begins_with("HKEY_CURRENT_USER")) {
		keyLength = 17;
		keyName = HKEY_CURRENT_USER;
	} else if (path.begins_with("HKCU")) {
		keyLength = 4;
		keyName = HKEY_CURRENT_USER;
	} else if (path.begins_with("HKEY_LOCAL_MACHINE")) {
		keyLength = 18;
		keyName = HKEY_LOCAL_MACHINE;
	} else if (path.begins_with("HKLM")) {
		keyLength = 4;
		keyName = HKEY_LOCAL_MACHINE;
	} else if (path.begins_with("HKEY_CLASSES_ROOT")) {
		keyLength = 17;
		keyName = HKEY_CLASSES_ROOT;
	} else if (path.begins_with("HKCR")) {
		keyLength = 4;
		keyName = HKEY_CLASSES_ROOT;
	} else if (path.begins_with("HKEY_USERS")) {
		keyLength = 10;
		keyName = HKEY_USERS;
	} else if (path.begins_with("HKU")) {
		keyLength = 3;
		keyName = HKEY_USERS;
	} else {
		ERR_FAIL_MSG("Invalid key");
	}

	if (path.length() == keyLength) {
		regList.push_back(RegistryKey(keyName, String(), false, access));
	} else if (path[keyLength] == '\\') {
		regList.push_back(RegistryKey(keyName, path.substr(keyLength + 1), false, access));
	}

	ERR_FAIL_COND_MSG(regList.empty(), "Access error");
}

bool SettingsStorage::readKey(HKEY parentHandle, const String &sub_key, Variant *value) const {
	String subkeyName = key_name(sub_key);
	String subkeyPath = key_path(sub_key);

	// open a handle on the subkey
	HKEY handle = open_key(parentHandle, KEY_READ, subkeyPath, access);
	if (handle == 0) {
		return false;
	}
	// get the size and type of the value
	DWORD dataType;
	DWORD dataSize;
	LONG res = RegQueryValueEx(handle, subkeyName.c_str(), 0, &dataType, 0, &dataSize);
	if (res != ERROR_SUCCESS) {
		RegCloseKey(handle);
		return false;
	}
	// workaround for rare cases where trailing '\0' are missing in registry
	if (dataType == REG_SZ || dataType == REG_EXPAND_SZ) {
		dataSize += 2;
	} else if (dataType == REG_MULTI_SZ) {
		dataSize += 4;
	}
	// get the value
	PoolByteArray data = from_data(static_cast<uint8_t *>(0), dataSize);
	res = RegQueryValueEx(handle, subkeyName.c_str(), 0, 0, data.write().ptr(), &dataSize);
	if (res != ERROR_SUCCESS) {
		RegCloseKey(handle);
		return false;
	}
	switch (dataType) {
		case REG_EXPAND_SZ:
		case REG_SZ: {
			String s;
			if (dataSize) {
				s = String(reinterpret_cast<const wchar_t *>(data.read().ptr()));
			}
			if (value != 0) {
				*value = s;
			}
			break;
		}
		default:
			WARN_PRINT(vformat("Settings: Unknown data %d type in Windows registry", static_cast<int>(dataType)));
			if (value != 0) {
				*value = Variant();
			}
			break;
	}
	RegCloseKey(handle);
	return true;
}

void SettingsStorage::remove(const String &key) {
	ERR_FAIL_COND_MSG(writeHandle() == 0, "Access error");

	String _key = escaped_key(key);
	// try to delete value bar in key foo
	LONG res;
	HKEY handle = open_key(writeHandle(), registryPermissions, key_path(_key), access);
	if (handle != 0) {
		res = RegDeleteValue(handle, reinterpret_cast<const wchar_t *>(key_name(_key).c_str()));
		RegCloseKey(handle);
	}
	// try to delete key foo/bar and all subkeys
	handle = open_key(writeHandle(), registryPermissions, _key, access);
	if (handle != 0) {
		delete_child_groups(handle, access);

		if (_key.empty()) {
			const StringList childKeys = child_keys_or_groups(handle, true);
			for (const String &group : childKeys) {
				LONG res = RegDeleteValue(handle, group.c_str());
				if (res != ERROR_SUCCESS) {
					WARN_PRINT(vformat("Settings: RegDeleteValue failed on subkey \"%s\", code %d", group, int(res)));
				}
			}
		} else {
			res = RegDeleteKey(writeHandle(), _key.c_str());

			if (res != ERROR_SUCCESS) {
				WARN_PRINT(vformat("Settings: RegDeleteKey failed on key \"%ls\", code: %d", _key, int(res)));
			}
		}
		RegCloseKey(handle);
	}
}

void SettingsStorage::set(const String &key, const Variant &value) {
	ERR_FAIL_COND_MSG(writeHandle() == 0, "Access error");

	String _key = escaped_key(key);

	HKEY handle = create_or_open_key(writeHandle(), registryPermissions, key_path(_key), access);
	ERR_FAIL_COND_MSG(handle == 0, "Access error");

	DWORD type;
	PoolByteArray regValueBuff;

	// Determine the type
	switch (value.get_type()) {
		case Variant::POOL_STRING_ARRAY: {
			// If none of the elements contains '\0', we can use REG_MULTI_SZ, the
			// native registry string list type. Otherwise we use REG_BINARY.
			type = REG_MULTI_SZ;
			PoolStringArray l = value;
			for (int i = 0; i < l.size(); i++) {
				if (l[i].length() == 0 || l[i].contains(0)) {
					type = REG_BINARY;
					break;
				}
			}
			if (type == REG_BINARY) {
				String s = value;
				regValueBuff = from_data(reinterpret_cast<const uint8_t *>(s.ptr()), sizeof(CharType) * s.length());
			} else {
				PoolStringArray l = value;
				regValueBuff = from_data(reinterpret_cast<const uint8_t *>(l.read().ptr()), sizeof(CharType) * l.size());
			}
			break;
		}
		case Variant::INT: {
			type = REG_DWORD;
			int32_t i = value;
			regValueBuff = from_data(reinterpret_cast<const uint8_t *>(&i), sizeof(int32_t));
			break;
		}
		default: {
			// If the string does not contain '\0', we can use REG_SZ, the native registry
			// string type. Otherwise we use REG_BINARY.
			String s = value;
			type = s.contains(0) ? REG_BINARY : REG_SZ;
			int length = s.length();
			if (type == REG_SZ) {
				++length;
			}
			regValueBuff = from_data(reinterpret_cast<const uint8_t *>(s.c_str()), int(sizeof(CharType)) * length);
			break;
		}
	}

	// set the value
	LONG res = RegSetValueEx(handle, key_name(_key).c_str(), 0, type, regValueBuff.read().ptr(), regValueBuff.size());
	if (res != ERROR_SUCCESS) {
		WARN_PRINT("Settings: failed to set subkey " + _key);
	}
	RegCloseKey(handle);
}

Variant SettingsStorage::get(const String &key) const {
	String _key = escaped_key(key);
	Variant value;
	for (const RegistryKey &r : regList) {
		HKEY handle = r.handle();
		if (handle != 0 && readKey(handle, _key, &value)) {
			return value;
		}
		if (!fallbacks) {
			return Variant();
		}
	}
	return Variant();
}

bool SettingsStorage::get(const String &key, Variant &value) const {
	String _key = escaped_key(key);
	for (const RegistryKey &r : regList) {
		HKEY handle = r.handle();
		if (handle != 0 && readKey(handle, _key, &value)) {
			return true;
		}
		if (!fallbacks) {
			return false;
		}
	}
	return false;
}

HKEY SettingsStorage::writeHandle() const {
	if (regList.empty()) {
		return 0;
	}
	const RegistryKey &key = regList.get(0);
	if (key.handle() == 0 || key.read_only()) {
		return 0;
	}
	return key.handle();
}
