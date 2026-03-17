/**************************************************************************/
/*  keychain.cpp                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "keychain.h"

#include "core/crypto/crypto_core.h"
#include "core/io/config_file.h"
#include "core/os/os.h"

Keychain *Keychain::singleton = nullptr;

Keychain *Keychain::get_singleton() {
	return singleton;
}

void Keychain::set_service(const String &p_service) {
	_service = p_service;
}

String Keychain::get_service() const {
	return _service;
}

void Keychain::set_insecure_fallback(bool p_enabled) {
	_insecure_fallback = p_enabled;
}

bool Keychain::get_insecure_fallback() const {
	return _insecure_fallback;
}

Keychain::Error Keychain::get_last_error() const {
	return _last_error;
}

String Keychain::get_last_error_string() const {
	return _last_error_string;
}

// --- Base64 helpers ---

static String _b64_encode(const PoolByteArray &p_data) {
	if (p_data.size() == 0) {
		return String();
	}
	PoolByteArray::Read r = p_data.read();
	return CryptoCore::b64_encode_str(r.ptr(), p_data.size());
}

static PoolByteArray _b64_decode(const String &p_str) {
	PoolByteArray result;
	if (p_str.empty()) {
		return result;
	}
	CharString cs = p_str.ascii();
	int out_len = cs.length(); // overestimate
	result.resize(out_len);
	{
		PoolByteArray::Write w = result.write();
		size_t actual = 0;
		if (CryptoCore::b64_decode(w.ptr(), out_len, &actual, (const unsigned char *)cs.get_data(), cs.length()) == OK) {
			result.resize(actual);
		} else {
			result.resize(0);
		}
	}
	return result;
}

// --- Public API ---

Keychain::Error Keychain::write_password(const String &p_key, const String &p_password) {
	PoolByteArray data;
	CharString utf8 = p_password.utf8();
	data.resize(utf8.length());
	{
		PoolByteArray::Write w = data.write();
		memcpy(w.ptr(), utf8.get_data(), utf8.length());
	}
	return write_data(p_key, data);
}

String Keychain::read_password(const String &p_key) {
	PoolByteArray data = read_data(p_key);
	if (_last_error != NO_ERROR || data.size() == 0) {
		return String();
	}
	PoolByteArray::Read r = data.read();
	return String::utf8((const char *)r.ptr(), data.size());
}

Keychain::Error Keychain::write_data(const String &p_key, const PoolByteArray &p_data) {
	ERR_FAIL_COND_V_MSG(_service.empty(), OTHER_ERROR, "Keychain: service name not set.");
	ERR_FAIL_COND_V_MSG(p_key.empty(), OTHER_ERROR, "Keychain: key is empty.");

	_last_error = _platform_write(p_key, p_data);
	if (_last_error == NO_ERROR) {
		return NO_ERROR;
	}

	if (_insecure_fallback) {
		_last_error = _fallback_write(p_key, p_data);
	}
	return _last_error;
}

PoolByteArray Keychain::read_data(const String &p_key) {
	ERR_FAIL_COND_V_MSG(_service.empty(), PoolByteArray(), "Keychain: service name not set.");
	ERR_FAIL_COND_V_MSG(p_key.empty(), PoolByteArray(), "Keychain: key is empty.");

	PoolByteArray data;
	_last_error = _platform_read(p_key, data);
	if (_last_error == NO_ERROR) {
		return data;
	}

	if (_insecure_fallback) {
		_last_error = _fallback_read(p_key, data);
		if (_last_error == NO_ERROR) {
			return data;
		}
	}
	return PoolByteArray();
}

Keychain::Error Keychain::delete_entry(const String &p_key) {
	ERR_FAIL_COND_V_MSG(_service.empty(), OTHER_ERROR, "Keychain: service name not set.");
	ERR_FAIL_COND_V_MSG(p_key.empty(), OTHER_ERROR, "Keychain: key is empty.");

	_last_error = _platform_delete(p_key);
	if (_last_error == NO_ERROR) {
		return NO_ERROR;
	}

	if (_insecure_fallback) {
		_last_error = _fallback_delete(p_key);
	}
	return _last_error;
}

bool Keychain::has_entry(const String &p_key) {
	PoolByteArray data;
	Keychain::Error err = _platform_read(p_key, data);
	if (err == NO_ERROR) {
		return true;
	}
	if (_insecure_fallback) {
		err = _fallback_read(p_key, data);
		return err == NO_ERROR;
	}
	return false;
}

// --- Plaintext fallback using ConfigFile ---

String Keychain::_get_fallback_path() const {
	return "user://keychain_" + _service.to_lower().replace(" ", "_") + ".cfg";
}

Keychain::Error Keychain::_fallback_read(const String &p_key, PoolByteArray &r_data) {
	Ref<ConfigFile> cfg;
	cfg.instance();
	if (cfg->load(_get_fallback_path()) != OK) {
		_last_error_string = "Fallback storage not found.";
		return ENTRY_NOT_FOUND;
	}
	if (!cfg->has_section_key("keys", p_key)) {
		_last_error_string = "Entry not found in fallback storage.";
		return ENTRY_NOT_FOUND;
	}
	String encoded = cfg->get_value("keys", p_key);
	r_data = _b64_decode(encoded);
	_last_error_string = "";
	return NO_ERROR;
}

Keychain::Error Keychain::_fallback_write(const String &p_key, const PoolByteArray &p_data) {
	Ref<ConfigFile> cfg;
	cfg.instance();
	cfg->load(_get_fallback_path()); // OK if doesn't exist yet
	String encoded = _b64_encode(p_data);
	cfg->set_value("keys", p_key, encoded);
	if (cfg->save(_get_fallback_path()) != OK) {
		_last_error_string = "Could not write to fallback storage.";
		return ACCESS_DENIED;
	}
	_last_error_string = "";
	return NO_ERROR;
}

Keychain::Error Keychain::_fallback_delete(const String &p_key) {
	Ref<ConfigFile> cfg;
	cfg.instance();
	if (cfg->load(_get_fallback_path()) != OK) {
		_last_error_string = "Fallback storage not found.";
		return ENTRY_NOT_FOUND;
	}
	if (!cfg->has_section_key("keys", p_key)) {
		_last_error_string = "Entry not found in fallback storage.";
		return ENTRY_NOT_FOUND;
	}
	cfg->erase_section_key("keys", p_key);
	cfg->save(_get_fallback_path());
	_last_error_string = "";
	return NO_ERROR;
}

// --- Bind methods ---

void Keychain::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_service", "service"), &Keychain::set_service);
	ClassDB::bind_method(D_METHOD("get_service"), &Keychain::get_service);

	ClassDB::bind_method(D_METHOD("set_insecure_fallback", "enabled"), &Keychain::set_insecure_fallback);
	ClassDB::bind_method(D_METHOD("get_insecure_fallback"), &Keychain::get_insecure_fallback);

	ClassDB::bind_method(D_METHOD("get_last_error"), &Keychain::get_last_error);
	ClassDB::bind_method(D_METHOD("get_last_error_string"), &Keychain::get_last_error_string);

	ClassDB::bind_method(D_METHOD("write_password", "key", "password"), &Keychain::write_password);
	ClassDB::bind_method(D_METHOD("read_password", "key"), &Keychain::read_password);

	ClassDB::bind_method(D_METHOD("write_data", "key", "data"), &Keychain::write_data);
	ClassDB::bind_method(D_METHOD("read_data", "key"), &Keychain::read_data);

	ClassDB::bind_method(D_METHOD("delete_entry", "key"), &Keychain::delete_entry);
	ClassDB::bind_method(D_METHOD("has_entry", "key"), &Keychain::has_entry);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "service"), "set_service", "get_service");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "insecure_fallback"), "set_insecure_fallback", "get_insecure_fallback");

	BIND_ENUM_CONSTANT(NO_ERROR);
	BIND_ENUM_CONSTANT(ENTRY_NOT_FOUND);
	BIND_ENUM_CONSTANT(COULD_NOT_DELETE_ENTRY);
	BIND_ENUM_CONSTANT(ACCESS_DENIED_BY_USER);
	BIND_ENUM_CONSTANT(ACCESS_DENIED);
	BIND_ENUM_CONSTANT(NO_BACKEND_AVAILABLE);
	BIND_ENUM_CONSTANT(NOT_IMPLEMENTED);
	BIND_ENUM_CONSTANT(OTHER_ERROR);
}

Keychain::Keychain() :
		_last_error(NO_ERROR),
		_service("godot"),
		_insecure_fallback(false) {
	singleton = this;
}

Keychain::~Keychain() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[keychains]] base64 helpers") {
	TEST_CASE("Encode and decode roundtrip") {
		PoolByteArray data;
		data.resize(5);
		{
			PoolByteArray::Write w = data.write();
			w[0] = 'H';
			w[1] = 'e';
			w[2] = 'l';
			w[3] = 'l';
			w[4] = 'o';
		}
		String encoded = _b64_encode(data);
		CHECK(!encoded.empty());

		PoolByteArray decoded = _b64_decode(encoded);
		REQUIRE(decoded.size() == 5);
		{
			PoolByteArray::Read r = decoded.read();
			CHECK(r[0] == 'H');
			CHECK(r[1] == 'e');
			CHECK(r[2] == 'l');
			CHECK(r[3] == 'l');
			CHECK(r[4] == 'o');
		}
	}

	TEST_CASE("Empty data") {
		PoolByteArray empty;
		String encoded = _b64_encode(empty);
		CHECK(encoded.empty());

		PoolByteArray decoded = _b64_decode("");
		CHECK(decoded.size() == 0);
	}

	TEST_CASE("Binary data roundtrip") {
		PoolByteArray data;
		data.resize(256);
		{
			PoolByteArray::Write w = data.write();
			for (int i = 0; i < 256; i++) {
				w[i] = (uint8_t)i;
			}
		}
		String encoded = _b64_encode(data);
		PoolByteArray decoded = _b64_decode(encoded);
		REQUIRE(decoded.size() == 256);
		{
			PoolByteArray::Read r = decoded.read();
			for (int i = 0; i < 256; i++) {
				CHECK(r[i] == (uint8_t)i);
			}
		}
	}
}

TEST_SUITE("[[keychains]] Keychain fallback storage") {
	TEST_CASE("Write and read password via fallback") {
		Keychain kc;
		kc.set_service("doctest_keychain");
		kc.set_insecure_fallback(true);

		Keychain::Error err = kc.write_password("test_pw", "secret123");
		CHECK(err == Keychain::NO_ERROR);

		String result = kc.read_password("test_pw");
		CHECK(result == "secret123");
		CHECK(kc.get_last_error() == Keychain::NO_ERROR);

		kc.delete_entry("test_pw");
	}

	TEST_CASE("Write and read binary data via fallback") {
		Keychain kc;
		kc.set_service("doctest_keychain");
		kc.set_insecure_fallback(true);

		PoolByteArray data;
		data.resize(4);
		{
			PoolByteArray::Write w = data.write();
			w[0] = 0xDE;
			w[1] = 0xAD;
			w[2] = 0xBE;
			w[3] = 0xEF;
		}

		Keychain::Error err = kc.write_data("bin_key", data);
		CHECK(err == Keychain::NO_ERROR);

		PoolByteArray result = kc.read_data("bin_key");
		CHECK(kc.get_last_error() == Keychain::NO_ERROR);
		REQUIRE(result.size() == 4);
		{
			PoolByteArray::Read r = result.read();
			CHECK(r[0] == 0xDE);
			CHECK(r[1] == 0xAD);
			CHECK(r[2] == 0xBE);
			CHECK(r[3] == 0xEF);
		}

		kc.delete_entry("bin_key");
	}

	TEST_CASE("Read nonexistent key returns error") {
		Keychain kc;
		kc.set_service("doctest_keychain");
		kc.set_insecure_fallback(true);

		String result = kc.read_password("nonexistent_key_xyz123");
		CHECK(kc.get_last_error() == Keychain::ENTRY_NOT_FOUND);
		CHECK(result.empty());
	}

	TEST_CASE("Delete entry") {
		Keychain kc;
		kc.set_service("doctest_keychain");
		kc.set_insecure_fallback(true);

		kc.write_password("delete_me", "temp");
		CHECK(kc.has_entry("delete_me"));

		Keychain::Error err = kc.delete_entry("delete_me");
		CHECK(err == Keychain::NO_ERROR);
		CHECK_FALSE(kc.has_entry("delete_me"));
	}

	TEST_CASE("has_entry") {
		Keychain kc;
		kc.set_service("doctest_keychain");
		kc.set_insecure_fallback(true);

		CHECK_FALSE(kc.has_entry("has_test_abc"));
		kc.write_password("has_test_abc", "val");
		CHECK(kc.has_entry("has_test_abc"));
		kc.delete_entry("has_test_abc");
		CHECK_FALSE(kc.has_entry("has_test_abc"));
	}

	TEST_CASE("Overwrite existing key") {
		Keychain kc;
		kc.set_service("doctest_keychain");
		kc.set_insecure_fallback(true);

		kc.write_password("ow_key", "first");
		CHECK(kc.read_password("ow_key") == "first");

		kc.write_password("ow_key", "second");
		CHECK(kc.read_password("ow_key") == "second");

		kc.delete_entry("ow_key");
	}

	TEST_CASE("Empty key rejected") {
		Keychain kc;
		kc.set_service("doctest_keychain");
		kc.set_insecure_fallback(true);

		Keychain::Error err = kc.write_password("", "val");
		CHECK(err == Keychain::OTHER_ERROR);
	}

	TEST_CASE("Empty service rejected") {
		Keychain kc;
		kc.set_service("");
		kc.set_insecure_fallback(true);

		Keychain::Error err = kc.write_password("key", "val");
		CHECK(err == Keychain::OTHER_ERROR);
	}

	TEST_CASE("Properties") {
		Keychain kc;
		kc.set_service("my_app");
		CHECK(kc.get_service() == "my_app");

		kc.set_insecure_fallback(true);
		CHECK(kc.get_insecure_fallback());
		kc.set_insecure_fallback(false);
		CHECK_FALSE(kc.get_insecure_fallback());
	}
}

#endif // DOCTEST
