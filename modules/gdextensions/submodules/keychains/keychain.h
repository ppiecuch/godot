/**************************************************************************/
/*  keychain.h                                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Ported from QtKeychain by Frank Osterfeld (BSD license).               */
/**************************************************************************/

#ifndef KEYCHAIN_H
#define KEYCHAIN_H

#include "core/object.h"
#include "core/pool_vector.h"
#include "core/ustring.h"

#define KEYCHAIN_VERSION 0x000200

class KeychainConfigStore;

class Keychain : public Object {
	GDCLASS(Keychain, Object);

public:
	enum Error {
		NO_ERROR = 0,
		ENTRY_NOT_FOUND,
		COULD_NOT_DELETE_ENTRY,
		ACCESS_DENIED_BY_USER,
		ACCESS_DENIED,
		NO_BACKEND_AVAILABLE,
		NOT_IMPLEMENTED,
		OTHER_ERROR
	};

private:
	static Keychain *singleton;

	Error _last_error;
	String _last_error_string;
	String _service;
	bool _insecure_fallback;

	// Platform-specific implementations
	Error _platform_read(const String &p_key, PoolByteArray &r_data);
	Error _platform_write(const String &p_key, const PoolByteArray &p_data);
	Error _platform_delete(const String &p_key);

	// Plaintext fallback
	Error _fallback_read(const String &p_key, PoolByteArray &r_data);
	Error _fallback_write(const String &p_key, const PoolByteArray &p_data);
	Error _fallback_delete(const String &p_key);

	String _get_fallback_path() const;

protected:
	static void _bind_methods();

public:
	static Keychain *get_singleton();

	void set_service(const String &p_service);
	String get_service() const;

	void set_insecure_fallback(bool p_enabled);
	bool get_insecure_fallback() const;

	Error get_last_error() const;
	String get_last_error_string() const;

	// Synchronous API (simpler than Qt's async Job pattern)
	Error write_password(const String &p_key, const String &p_password);
	String read_password(const String &p_key);

	Error write_data(const String &p_key, const PoolByteArray &p_data);
	PoolByteArray read_data(const String &p_key);

	Error delete_entry(const String &p_key);

	bool has_entry(const String &p_key);

	Keychain();
	~Keychain();
};

VARIANT_ENUM_CAST(Keychain::Error);

#endif // KEYCHAIN_H
