/**************************************************************************/
/*  keychain_win.cpp                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "keychain.h"

#ifdef WINDOWS_ENABLED

#include <wincred.h>
#include <windows.h>

Keychain::Error Keychain::_platform_read(const String &p_key, PoolByteArray &r_data) {
	String target = _service + "/" + p_key;
	LPCWSTR name = (LPCWSTR)target.c_str();
	PCREDENTIALW cred;

	if (!CredReadW(name, CRED_TYPE_GENERIC, 0, &cred)) {
		DWORD err = GetLastError();
		if (err == ERROR_NOT_FOUND) {
			_last_error_string = "Entry not found.";
			return ENTRY_NOT_FOUND;
		}
		_last_error_string = vformat("CredReadW failed: Win32 error %d", (int)err);
		return OTHER_ERROR;
	}

	r_data.resize(cred->CredentialBlobSize);
	{
		PoolByteArray::Write w = r_data.write();
		memcpy(w.ptr(), cred->CredentialBlob, cred->CredentialBlobSize);
	}
	CredFree(cred);
	_last_error_string = "";
	return NO_ERROR;
}

Keychain::Error Keychain::_platform_write(const String &p_key, const PoolByteArray &p_data) {
	String target = _service + "/" + p_key;

	CREDENTIALW cred;
	memset(&cred, 0, sizeof(cred));
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = (LPWSTR)target.c_str();
	cred.CredentialBlobSize = p_data.size();

	PoolByteArray::Read r = p_data.read();
	cred.CredentialBlob = (LPBYTE)r.ptr();
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;

	if (!CredWriteW(&cred, 0)) {
		DWORD err = GetLastError();
		_last_error_string = vformat("CredWriteW failed: Win32 error %d", (int)err);
		return OTHER_ERROR;
	}

	_last_error_string = "";
	return NO_ERROR;
}

Keychain::Error Keychain::_platform_delete(const String &p_key) {
	String target = _service + "/" + p_key;
	LPCWSTR name = (LPCWSTR)target.c_str();

	if (!CredDeleteW(name, CRED_TYPE_GENERIC, 0)) {
		DWORD err = GetLastError();
		if (err == ERROR_NOT_FOUND) {
			_last_error_string = "Entry not found.";
			return ENTRY_NOT_FOUND;
		}
		_last_error_string = vformat("CredDeleteW failed: Win32 error %d", (int)err);
		return OTHER_ERROR;
	}

	_last_error_string = "";
	return NO_ERROR;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[keychains]] Windows Credential Manager backend") {
	TEST_CASE("Write and read via Credential Manager") {
		Keychain kc;
		kc.set_service("doctest_win_keychain");

		Keychain::Error err = kc.write_password("win_test_key", "win_secret_123");
		CHECK(err == Keychain::NO_ERROR);

		String result = kc.read_password("win_test_key");
		CHECK(result == "win_secret_123");

		kc.delete_entry("win_test_key");
	}

	TEST_CASE("Read nonexistent key") {
		Keychain kc;
		kc.set_service("doctest_win_keychain");
		String result = kc.read_password("nonexistent_win_key_xyz");
		CHECK(kc.get_last_error() == Keychain::ENTRY_NOT_FOUND);
	}
}

#endif // DOCTEST

#endif // WINDOWS_ENABLED
