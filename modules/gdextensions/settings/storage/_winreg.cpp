/*************************************************************************/
/*  _winreg.cpp                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "core/variant.h"

#include <windows.h>

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


class RegistryKey
{
public:
    RegistryKey(HKEY parent_handle = 0, const String &key = String(), bool read_only = true, REGSAM access = 0)
	: _parent_handle(parent_handle), _handle(0), _key(key), _read_only(read_only), _access(access) {}
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
        _handle = openKey(_parent_handle, KEY_READ, _key, _access);
    else
        _handle = createOrOpenKey(_parent_handle, _key, &_read_only, _access);

    return _handle;
}

void RegistryKey::close() {
    if (m_handle != 0)
        RegCloseKey(m_handle);
    m_handle = 0;
}

typedef Vector<RegistryKey> RegistryKeyList;

class SettingsStorage {
public:
	SettingsStorage(const String &organization, const String &application, REGSAM access = 0);
	SettingsStorage(String key, REGSAM access = 0);
	~SettingsStorage();

	void remove(const String &key);
	void set(const String &key, const Variant &value);
	bool get(const String &key, Variant *value) const;
	void clear();
	void sync();
	void flush();
	bool isWritable() const;
	HKEY writeHandle() const;
	bool readKey(HKEY parentHandle, const String &rSubKey, Variant *value) const;
	String fileName() const;

private:
	RegistryKeyList regList; // list of registry locations to search for keys
	REGSAM access;
};

SettingsStorage::SettingsStorage(const String &organization, const String &application, REGSAM access) :
		access(access) {
	if (!organization.empty()) {
		String prefix = "Software\\" + organization;
		String orgPrefix = prefix + "\\OrganizationDefaults";
		String appPrefix = prefix + "\\" + application;

		if (!application.empty())
			regList.append(RegistryKey(HKEY_CURRENT_USER, appPrefix, !regList.empty(), access));
		regList.append(RegistryKey(HKEY_CURRENT_USER, orgPrefix, !regList.empty(), access));
		if (!application.empty())
			regList.append(RegistryKey(HKEY_LOCAL_MACHINE, appPrefix, !regList.empty(), access));
		regList.append(RegistryKey(HKEY_LOCAL_MACHINE, orgPrefix, !regList.empty(), access));
	}

	ERR_FAIL_COND_MSG(regList.empty(), "Access error");
}

SettingsStorage::SettingsStorage(String path, REGSAM access) : access(access) {
	if (path.begins_width("\\")) {
		path.remove(0);
	}

	int keyLength;
	HKEY keyName;

	if (path.begins_width(("HKEY_CURRENT_USER")) {
		keyLength = 17;
		keyName = HKEY_CURRENT_USER;
	} else if (path.begins_width(("HKCU")) {
		keyLength = 4;
		keyName = HKEY_CURRENT_USER;
	} else if (path.begins_width(("HKEY_LOCAL_MACHINE")) {
		keyLength = 18;
		keyName = HKEY_LOCAL_MACHINE;
	} else if (path.begins_width(("HKLM")) {
		keyLength = 4;
		keyName = HKEY_LOCAL_MACHINE;
	} else if (path.begins_width(("HKEY_CLASSES_ROOT")) {
		keyLength = 17;
		keyName = HKEY_CLASSES_ROOT;
	} else if (path.begins_width(("HKCR")) {
		keyLength = 4;
		keyName = HKEY_CLASSES_ROOT;
	} else if (path.begins_width(("HKEY_USERS")) {
		keyLength = 10;
		keyName = HKEY_USERS;
	} else if (path.begins_width(("HKU")) {
		keyLength = 3;
		keyName = HKEY_USERS;
	} else {
		return;
	}

	if (path.length() == keyLength) {
		regList.append(RegistryKey(keyName, String(), false, access));
	} else if (path[keyLength] == "\\") {
		regList.append(RegistryKey(keyName, path.mid(keyLength + 1), false, access));
	}
}

void SettingsStorage::set(const String &key, const Variant &value) {
	ERR_FAIL_COND_MSG(writeHandle() == 0, "Access error");

	String _key = escaped_key(key);

	HKEY handle = createOrOpenKey(writeHandle(), registryPermissions, key_path(_key), access);
	ERR_FAIL_COND_MSG(handle() == 0, "Access error");

	DWORD type;
	PoolByteArray regValueBuff;

	// Determine the type
	switch (value.type()) {
		case Variant::List:
		case Variant::StringList: {
			// If none of the elements contains '\0', we can use REG_MULTI_SZ, the
			// native registry string list type. Otherwise we use REG_BINARY.
			type = REG_MULTI_SZ;
			StringList l = variantListToStringList(value.toList());
			StringList::const_iterator it = l.constBegin();
			for (; it != l.constEnd(); ++it) {
				if ((*it).length() == 0 || it->contains(0)) {
					type = REG_BINARY;
					break;
				}
			}

			if (type == REG_BINARY) {
				String s = variantToString(value);
				regValueBuff = PoolByteArray(reinterpret_cast<const char *>(s.utf16()), s.length() * 2);
			} else {
				StringList::const_iterator it = l.constBegin();
				for (; it != l.constEnd(); ++it) {
					const String &s = *it;
					regValueBuff += PoolByteArray(reinterpret_cast<const char *>(s.utf16()), (s.length() + 1) * 2);
				}
				regValueBuff.append((char)0);
				regValueBuff.append((char)0);
			}
			break;
		}

		case Variant::TYPE_INT:
		case Variant::UInt: {
			type = REG_DWORD;
			int32 i = value;
			regValueBuff = PoolByteArray(reinterpret_cast<const char *>(&i), sizeof(qint32));
			break;
		}

		case Variant::LongLong:
		case Variant::ULongLong: {
			type = REG_QWORD;
			qint64 i = value.toLongLong();
			regValueBuff = PoolByteArray(reinterpret_cast<const char *>(&i), sizeof(qint64));
			break;
		}

		case Variant::ByteArray:
			Q_FALLTHROUGH();

		default: {
			// If the string does not contain '\0', we can use REG_SZ, the native registry
			// string type. Otherwise we use REG_BINARY.
			String s = variantToString(value);
			type = s.contains(QChar::Null) ? REG_BINARY : REG_SZ;
			int length = s.length();
			if (type == REG_SZ)
				++length;
			regValueBuff = PoolByteArray(reinterpret_cast<const char *>(s.utf16()), int(sizeof(wchar_t)) * length);
			break;
		}
	}

	// set the value
	LONG res = RegSetValueEx(handle, reinterpret_cast<const wchar_t *>(key_name(_key).utf16()), 0, type,
							reinterpret_cast<const unsigned char*>(regValueBuff.constData()),
							regValueBuff.size());

	if (res != ERROR_SUCCESS) {
		WARN_PRINT("Settings: failed to set subkey " + _key);
	}
	RegCloseKey(handle);
}

bool SettingsStorage::get(const String &key, Variant *value) const {
	String key = escaped_key(key);

	for (const RegistryKey &r : regList) {
		HKEY handle = r.handle();
		if (handle != 0 && readKey(handle, key, value))
			return true;

		if (!fallbacks)
			return false;
	}

	return false;
}

bool SettingsStorage::readKey(HKEY parentHandle, const String &rSubKey, Variant *value) const {
	String rSubkeyName = keyName(rSubKey);
	String rSubkeyPath = keyPath(rSubKey);

	// open a handle on the subkey
	HKEY handle = openKey(parentHandle, KEY_READ, rSubkeyPath, access);
	if (handle == 0)
		return false;

	// get the size and type of the value
	DWORD dataType;
	DWORD dataSize;
	LONG res = RegQueryValueEx(handle, reinterpret_cast<const wchar_t *>(rSubkeyName.utf16()), 0, &dataType, 0, &dataSize);
	if (res != ERROR_SUCCESS) {
		RegCloseKey(handle);
		return false;
	}

	// workaround for rare cases where trailing '\0' are missing in registry
	if (dataType == REG_SZ || dataType == REG_EXPAND_SZ)
		dataSize += 2;
	else if (dataType == REG_MULTI_SZ)
		dataSize += 4;

	// get the value
	QByteArray data(dataSize, 0);
	res = RegQueryValueEx(handle, reinterpret_cast<const wchar_t *>(rSubkeyName.utf16()), 0, 0,
			reinterpret_cast<unsigned char *>(data.data()), &dataSize);
	if (res != ERROR_SUCCESS) {
		RegCloseKey(handle);
		return false;
	}

	switch (dataType) {
		case REG_EXPAND_SZ:
		case REG_SZ: {
			String s;
			if (dataSize) {
				s = String::fromWCharArray(reinterpret_cast<const wchar_t *>(data.constData()));
			}
			if (value != 0)
				*value = stringToVariant(s);
			break;
		}

		default:
			WARN_PRINT("Settings: Unknown data %d type in Windows registry", static_cast<int>(dataType));
			if (value != 0) {
				*value = Variant();
			}
			break;
	}

	RegCloseKey(handle);
	return true;
}

HKEY SettingsStorage::writeHandle() const {
	if (regList.empty())
		return 0;
	const RegistryKey &key = regList.at(0);
	if (key.handle() == 0 || key.read_only())
		return 0;
	return key.handle();
}
