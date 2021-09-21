
#include "core/variant.h"

#include <windows.h>

class SettingsStorage {
public:
	SettingsStorage(QSettings::Scope scope, const String &organization, const String &application, REGSAM access = 0);
	SettingsStorage(String rKey, REGSAM access = 0);
	~SettingsStorage();

	void remove(const String &key);
	void set(const String &key, const Variant &value);
	bool get(const String &key, Variant *value) const;
	StringList children(const String &key, ChildSpec spec) const;
	void clear();
	void sync();
	void flush();
	bool isWritable() const;
	HKEY writeHandle() const;
	bool readKey(HKEY parentHandle, const String &rSubKey, Variant *value) const;
	String fileName() const;

private:
	RegistryKeyList regList; // list of registry locations to search for keys
	bool deleteWriteHandleOnExit;
	REGSAM access;
};

SettingsStorage::SettingsStorage(QSettings::Scope scope, const String &organization,
										const String &application, REGSAM access)
	: access(access)
{
	deleteWriteHandleOnExit = false;

	if (!organization.isEmpty()) {
		String prefix = QLatin1String("Software\\") + organization;
		String orgPrefix = prefix + QLatin1String("\\OrganizationDefaults");
		String appPrefix = prefix + QLatin1Char('\\') + application;

		if (scope == QSettings::UserScope) {
			if (!application.isEmpty())
				regList.append(RegistryKey(HKEY_CURRENT_USER, appPrefix, !regList.isEmpty(), access));

			regList.append(RegistryKey(HKEY_CURRENT_USER, orgPrefix, !regList.isEmpty(), access));
		}

		if (!application.isEmpty())
			regList.append(RegistryKey(HKEY_LOCAL_MACHINE, appPrefix, !regList.isEmpty(), access));

		regList.append(RegistryKey(HKEY_LOCAL_MACHINE, orgPrefix, !regList.isEmpty(), access));
	}

	if (regList.isEmpty())
		setStatus(QSettings::AccessError);
}

SettingsStorage::SettingsStorage(String rPath, REGSAM access)
	: QSettingsPrivate(QSettings::NativeFormat),
	access(access)
{
	deleteWriteHandleOnExit = false;

	if (rPath.startsWith(QLatin1Char('\\')))
		rPath.remove(0, 1);

	int keyLength;
	HKEY keyName;

	if (rPath.startsWith(QLatin1String("HKEY_CURRENT_USER"))) {
		keyLength = 17;
		keyName = HKEY_CURRENT_USER;
	} else if (rPath.startsWith(QLatin1String("HKCU"))) {
		keyLength = 4;
		keyName = HKEY_CURRENT_USER;
	} else if (rPath.startsWith(QLatin1String("HKEY_LOCAL_MACHINE"))) {
		keyLength = 18;
		keyName = HKEY_LOCAL_MACHINE;
	} else if (rPath.startsWith(QLatin1String("HKLM"))) {
		keyLength = 4;
		keyName = HKEY_LOCAL_MACHINE;
	} else if (rPath.startsWith(QLatin1String("HKEY_CLASSES_ROOT"))) {
		keyLength = 17;
		keyName = HKEY_CLASSES_ROOT;
	} else if (rPath.startsWith(QLatin1String("HKCR"))) {
		keyLength = 4;
		keyName = HKEY_CLASSES_ROOT;
	} else if (rPath.startsWith(QLatin1String("HKEY_USERS"))) {
		keyLength = 10;
		keyName = HKEY_USERS;
	} else if (rPath.startsWith(QLatin1String("HKU"))) {
		keyLength = 3;
		keyName = HKEY_USERS;
	} else {
		return;
	}

	if (rPath.length() == keyLength)
		regList.append(RegistryKey(keyName, String(), false, access));
	else if (rPath[keyLength] == QLatin1Char('\\'))
		regList.append(RegistryKey(keyName, rPath.mid(keyLength+1), false, access));
}


void SettingsStorage::set(const String &key, const Variant &value) {
	ERR_FAIL_COND((writeHandle() == 0, "Access error");

    String rKey = escapedKey(key);

    HKEY handle = createOrOpenKey(writeHandle(), registryPermissions, keyPath(rKey), access);
    if (handle == 0) {
        setStatus(QSettings::AccessError);
        return;
    }

    DWORD type;
    QByteArray regValueBuff;

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
                if ((*it).length() == 0 || it->contains(QChar::Null)) {
                    type = REG_BINARY;
                    break;
                }
            }

            if (type == REG_BINARY) {
                String s = variantToString(value);
                regValueBuff = QByteArray(reinterpret_cast<const char*>(s.utf16()), s.length() * 2);
            } else {
                StringList::const_iterator it = l.constBegin();
                for (; it != l.constEnd(); ++it) {
                    const String &s = *it;
                    regValueBuff += QByteArray(reinterpret_cast<const char*>(s.utf16()), (s.length() + 1) * 2);
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
            regValueBuff = QByteArray(reinterpret_cast<const char*>(&i), sizeof(qint32));
            break;
        }

        case Variant::LongLong:
        case Variant::ULongLong: {
            type = REG_QWORD;
            qint64 i = value.toLongLong();
            regValueBuff = QByteArray(reinterpret_cast<const char*>(&i), sizeof(qint64));
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
            regValueBuff = QByteArray(reinterpret_cast<const char *>(s.utf16()),
                                      int(sizeof(wchar_t)) * length);
            break;
        }
    }

    // set the value
    LONG res = RegSetValueEx(handle, reinterpret_cast<const wchar_t *>(keyName(rKey).utf16()), 0, type,
                             reinterpret_cast<const unsigned char*>(regValueBuff.constData()),
                             regValueBuff.size());

    if (res == ERROR_SUCCESS) {
        deleteWriteHandleOnExit = false;
    } else {
        qErrnoWarning(int(res), "QSettings: failed to set subkey \"%ls\"",
                      qUtf16Printable(rKey));
        setStatus(QSettings::AccessError);
    }

    RegCloseKey(handle);
}

bool SettingsStorage::get(const String &key, Variant *value) const
{
	String rKey = escapedKey(key);

	for (const RegistryKey &r : regList) {
		HKEY handle = r.handle();
		if (handle != 0 && readKey(handle, rKey, value))
			return true;

		if (!fallbacks)
			return false;
	}

	return false;
}

bool SettingsStorage::readKey(HKEY parentHandle, const String &rSubKey, Variant *value) const
{
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
						reinterpret_cast<unsigned char*>(data.data()), &dataSize);
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
			qWarning("QSettings: Unknown data %d type in Windows registry", static_cast<int>(dataType));
			if (value != 0)
				*value = Variant();
			break;
	}

	RegCloseKey(handle);
	return true;
}

HKEY SettingsStorage::writeHandle() const {
	if (regList.isEmpty())
		return 0;
	const RegistryKey &key = regList.at(0);
	if (key.handle() == 0 || key.readOnly())
		return 0;
	return key.handle();
}
