/**************************************************************************/
/*  keychain_win.cpp                                                      */
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
