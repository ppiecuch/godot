/**************************************************************************/
/*  keychain_unix.cpp                                                     */
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

#if defined(X11_ENABLED) || defined(SERVER_ENABLED)

#include "core/os/os.h"
#include "core/print_string.h"

#include <dlfcn.h>
#include <cstring>

// --- libsecret types (minimal subset) ---

typedef void GCancellable;
typedef void GError;
typedef unsigned int guint;
typedef int gint;
typedef gint gboolean;
typedef char gchar;
typedef unsigned int GQuark;

enum SecretSchemaAttributeType {
	SECRET_SCHEMA_ATTRIBUTE_STRING = 0,
	SECRET_SCHEMA_ATTRIBUTE_INTEGER = 1,
	SECRET_SCHEMA_ATTRIBUTE_BOOLEAN = 2,
};

enum SecretSchemaFlags {
	SECRET_SCHEMA_NONE = 0,
	SECRET_SCHEMA_DONT_MATCH_NAME = 1 << 1,
};

struct SecretSchemaAttribute {
	const gchar *name;
	SecretSchemaAttributeType type;
};

struct SecretSchema {
	const gchar *name;
	SecretSchemaFlags flags;
	SecretSchemaAttribute attributes[32];
};

#define SECRET_COLLECTION_DEFAULT "default"

// --- Function pointer types ---

typedef gchar *(*secret_password_lookup_sync_fn)(const SecretSchema *, GCancellable *, GError **, ...);
typedef gboolean (*secret_password_store_sync_fn)(const SecretSchema *, const gchar *, const gchar *, const gchar *, GCancellable *, GError **, ...);
typedef gboolean (*secret_password_clear_sync_fn)(const SecretSchema *, GCancellable *, GError **, ...);
typedef void (*secret_password_free_fn)(gchar *);

typedef void (*g_error_free_fn)(GError *);

// --- Schema ---

static const SecretSchema _godot_schema = {
	"org.godotengine.keychain",
	SECRET_SCHEMA_DONT_MATCH_NAME,
	{
			{ "user", SECRET_SCHEMA_ATTRIBUTE_STRING },
			{ "server", SECRET_SCHEMA_ATTRIBUTE_STRING },
			{ "type", SECRET_SCHEMA_ATTRIBUTE_STRING },
			{ NULL, (SecretSchemaAttributeType)0 },
	}
};

// --- Library state ---

struct LibSecretState {
	void *lib;
	secret_password_lookup_sync_fn lookup;
	secret_password_store_sync_fn store;
	secret_password_clear_sync_fn clear;
	secret_password_free_fn pw_free;
	g_error_free_fn err_free;
	bool available;

	LibSecretState() :
			lib(nullptr), lookup(nullptr), store(nullptr), clear(nullptr), pw_free(nullptr), err_free(nullptr), available(false) {}

	bool init() {
		if (lib) {
			return available;
		}
		lib = dlopen("libsecret-1.so.0", RTLD_LAZY);
		if (!lib) {
			lib = dlopen("libsecret-1.so", RTLD_LAZY);
		}
		if (!lib) {
			return false;
		}

		lookup = (secret_password_lookup_sync_fn)dlsym(lib, "secret_password_lookup_sync");
		store = (secret_password_store_sync_fn)dlsym(lib, "secret_password_store_sync");
		clear = (secret_password_clear_sync_fn)dlsym(lib, "secret_password_clear_sync");
		pw_free = (secret_password_free_fn)dlsym(lib, "secret_password_free");

		// g_error_free from GLib
		void *glib = dlopen("libglib-2.0.so.0", RTLD_LAZY);
		if (glib) {
			err_free = (g_error_free_fn)dlsym(glib, "g_error_free");
		}

		available = (lookup && store && clear && pw_free);
		if (!available) {
			print_verbose("Keychain: libsecret loaded but required symbols not found.");
		}
		return available;
	}
};

static LibSecretState _libsecret;

// --- Extract GError message (GError is: { GQuark domain; gint code; gchar *message; }) ---

static String _gerror_message(GError *err) {
	if (!err) {
		return String();
	}
	// GError layout: guint32 domain, gint code, gchar* message
	struct GErrorCompat {
		GQuark domain;
		gint code;
		gchar *message;
	};
	GErrorCompat *e = (GErrorCompat *)err;
	String msg = e->message ? String::utf8(e->message) : "Unknown error";
	if (_libsecret.err_free) {
		_libsecret.err_free(err);
	}
	return msg;
}

// --- Platform implementation ---

Keychain::Error Keychain::_platform_read(const String &p_key, PoolByteArray &r_data) {
	if (!_libsecret.init()) {
		_last_error_string = "libsecret not available.";
		return NO_BACKEND_AVAILABLE;
	}

	CharString service_utf8 = _service.utf8();
	CharString key_utf8 = p_key.utf8();

	GError *error = nullptr;
	gchar *password = _libsecret.lookup(
			&_godot_schema, nullptr, &error,
			"user", key_utf8.get_data(),
			"server", service_utf8.get_data(),
			"type", "plaintext",
			NULL);

	if (error) {
		_last_error_string = _gerror_message(error);
		return OTHER_ERROR;
	}

	if (!password) {
		_last_error_string = "Entry not found.";
		return ENTRY_NOT_FOUND;
	}

	int len = strlen(password);
	r_data.resize(len);
	{
		PoolByteArray::Write w = r_data.write();
		memcpy(w.ptr(), password, len);
	}

	_libsecret.pw_free(password);
	_last_error_string = "";
	return NO_ERROR;
}

Keychain::Error Keychain::_platform_write(const String &p_key, const PoolByteArray &p_data) {
	if (!_libsecret.init()) {
		_last_error_string = "libsecret not available.";
		return NO_BACKEND_AVAILABLE;
	}

	CharString service_utf8 = _service.utf8();
	CharString key_utf8 = p_key.utf8();

	// Store as plaintext string (null-terminated copy)
	PoolByteArray::Read r = p_data.read();
	CharString data_str;
	data_str.resize(p_data.size() + 1);
	memcpy(data_str.ptrw(), r.ptr(), p_data.size());
	data_str.ptrw()[p_data.size()] = '\0';

	String label = _service + "/" + p_key;
	CharString label_utf8 = label.utf8();

	GError *error = nullptr;
	gboolean ok = _libsecret.store(
			&_godot_schema,
			SECRET_COLLECTION_DEFAULT,
			label_utf8.get_data(),
			data_str.get_data(),
			nullptr, &error,
			"user", key_utf8.get_data(),
			"server", service_utf8.get_data(),
			"type", "plaintext",
			NULL);

	if (error) {
		_last_error_string = _gerror_message(error);
		return OTHER_ERROR;
	}
	if (!ok) {
		_last_error_string = "secret_password_store_sync returned false.";
		return OTHER_ERROR;
	}

	_last_error_string = "";
	return NO_ERROR;
}

Keychain::Error Keychain::_platform_delete(const String &p_key) {
	if (!_libsecret.init()) {
		_last_error_string = "libsecret not available.";
		return NO_BACKEND_AVAILABLE;
	}

	CharString service_utf8 = _service.utf8();
	CharString key_utf8 = p_key.utf8();

	GError *error = nullptr;
	gboolean ok = _libsecret.clear(
			&_godot_schema, nullptr, &error,
			"user", key_utf8.get_data(),
			"server", service_utf8.get_data(),
			NULL);

	if (error) {
		_last_error_string = _gerror_message(error);
		return OTHER_ERROR;
	}
	if (!ok) {
		_last_error_string = "Entry not found.";
		return ENTRY_NOT_FOUND;
	}

	_last_error_string = "";
	return NO_ERROR;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[keychains]] Linux libsecret backend") {
	TEST_CASE("Schema has correct attributes") {
		CHECK(String::utf8(_godot_schema.name) == "org.godotengine.keychain");
		CHECK(_godot_schema.flags == SECRET_SCHEMA_DONT_MATCH_NAME);
		CHECK(String::utf8(_godot_schema.attributes[0].name) == "user");
		CHECK(String::utf8(_godot_schema.attributes[1].name) == "server");
		CHECK(String::utf8(_godot_schema.attributes[2].name) == "type");
		CHECK(_godot_schema.attributes[3].name == nullptr);
	}

	TEST_CASE("LibSecretState init loads library or fails gracefully") {
		LibSecretState state;
		// On macOS this will fail (no libsecret), on Linux it may succeed
		// Either way it should not crash
		bool result = state.init();
		// If libsecret is not available, available should be false
		if (!result) {
			CHECK_FALSE(state.available);
		}
	}

	TEST_CASE("GError message extraction handles null") {
		String msg = _gerror_message(nullptr);
		CHECK(msg.empty());
	}
}

#endif // DOCTEST

#endif // X11_ENABLED || SERVER_ENABLED
