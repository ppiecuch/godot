/**************************************************************************/
/*  settings.cpp                                                          */
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

#include "settings.h"

#include "core/io/marshalls.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "core/variant.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

Settings *Settings::instance;

static String get_app_name() {
	String appname = ProjectSettings::get_singleton()->get("application/config/name");
	if (appname.empty()) {
		appname = OS::get_singleton()->get_executable_path().get_basename();
	}
	if (appname.empty()) {
		appname = OS::get_singleton()->get_name();
		if (OS::get_singleton()->get_model_name() != "GenericDevice") {
			appname += " (" + OS::get_singleton()->get_model_name() + ")";
		}
	}
	if (appname.empty()) {
		appname = "n/a";
	}
#ifdef DEBUG_ENABLED
	appname += " (debug)";
#endif
	return appname;
}

#ifndef __linux__
static PoolByteArray encode_var(const Variant &data) {
	PoolByteArray ret;
	int len;
	Error err = encode_variant(data, nullptr, len);
	if (err != OK) {
		WARN_PRINT("Unexpected error encoding variable to bytes");
		return ret;
	}
	ret.resize(len);
	{
		PoolByteArray::Write w = ret.write();
		encode_variant(data, w.ptr(), len);
	}
	return ret;
}

static Variant decode_var(const PoolByteArray &p_data) {
	Variant ret;
	PoolByteArray data = p_data;
	PoolByteArray::Read r = data.read();
	Error err = decode_variant(ret, r.ptr(), data.size(), nullptr);
	if (err != OK) {
		WARN_PRINT("Unexpected error decoding bytes to variable");
		Variant f;
		return f;
	}
	return ret;
}
#endif // __linux__

#if defined(ANDROID_ENABLED)
#include "storage/_sharedpreferences.cpp"
#elif defined(IOS_ENABLED) || defined(OSX_ENABLED)
#include "storage/_userdefaults.cpp"
#elif defined(WINDOWS_ENABLED)
#include "storage/_winreg.cpp"
#else
#include "storage/_configfile.cpp"
#endif

Settings *Settings::get_singleton() {
	return instance;
}

void Settings::setv(const String &p_key, const Variant &p_value) {
	storage->set(p_key, p_value);
}

Variant Settings::getv(const String &p_key, const Variant &p_default) {
	return storage->get(p_key, p_default);
}

bool Settings::has_key(const String &p_key) {
	return storage->has_key(p_key);
}

void Settings::remove(const String &p_key) {
	storage->remove(p_key);
}

void Settings::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setv", "key", "value"), &Settings::setv);
	ClassDB::bind_method(D_METHOD("getv", "key", "default"), &Settings::getv, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("has_key", "key"), &Settings::has_key);
	ClassDB::bind_method(D_METHOD("remove", "key"), &Settings::remove);
}

Settings::Settings() {
	storage = Ref<SettingsStorage>(memnew(SettingsStorage));
	print_verbose(vformat("Open Settings with '%s' name", get_app_name()));
	instance = this;
}

Settings::~Settings() {
	instance = nullptr;
}

#ifdef DOCTEST
TEST_SUITE("settings") {
	// Helper: clean up test keys to avoid cross-test contamination.
	// CFPreferences persists across runs, so we clean known keys before each test.
	static void cleanup_keys(Settings * s, const char *keys[], int count) {
		for (int i = 0; i < count; i++) {
			s->remove(keys[i]);
		}
	}

	// =========================================================================
	// Basic type storage and retrieval
	// =========================================================================

	TEST_CASE("Store and retrieve: int") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_int", "test_int_zero", "test_int_neg", "test_int_large" };
		cleanup_keys(s, keys, 4);

		SUBCASE("positive int") {
			s->setv("test_int", 99);
			CHECK(s->getv("test_int") == Variant(99));
			CHECK(s->getv("test_int").get_type() == Variant::INT);
		}
		SUBCASE("zero") {
			s->setv("test_int_zero", 0);
			CHECK(s->getv("test_int_zero") == Variant(0));
			CHECK(s->getv("test_int_zero").get_type() == Variant::INT);
		}
		SUBCASE("negative int") {
			s->setv("test_int_neg", -42);
			CHECK(s->getv("test_int_neg") == Variant(-42));
		}
		SUBCASE("large int") {
			s->setv("test_int_large", 2147483647); // INT_MAX
			CHECK(s->getv("test_int_large") == Variant(2147483647));
		}
	}

	TEST_CASE("Store and retrieve: float/real") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_float", "test_float_zero", "test_float_neg", "test_float_small" };
		cleanup_keys(s, keys, 4);

		SUBCASE("positive float") {
			s->setv("test_float", 3.14159);
			Variant ret = s->getv("test_float");
			CHECK(ret.get_type() == Variant::REAL);
			CHECK(doctest::Approx((double)ret) == 3.14159);
		}
		SUBCASE("zero float") {
			s->setv("test_float_zero", 0.0);
			CHECK(s->getv("test_float_zero") == Variant(0.0));
		}
		SUBCASE("negative float") {
			s->setv("test_float_neg", -1.5);
			CHECK(s->getv("test_float_neg") == Variant(-1.5));
		}
		SUBCASE("very small float") {
			s->setv("test_float_small", 0.000001);
			Variant ret = s->getv("test_float_small");
			CHECK(doctest::Approx((double)ret) == 0.000001);
		}
	}

	TEST_CASE("Store and retrieve: bool") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_bool_true", "test_bool_false" };
		cleanup_keys(s, keys, 2);

		SUBCASE("true") {
			s->setv("test_bool_true", true);
			CHECK(s->getv("test_bool_true") == Variant(true));
			CHECK(s->getv("test_bool_true").get_type() == Variant::BOOL);
		}
		SUBCASE("false") {
			s->setv("test_bool_false", false);
			CHECK(s->getv("test_bool_false") == Variant(false));
			CHECK(s->getv("test_bool_false").get_type() == Variant::BOOL);
		}
	}

	TEST_CASE("Store and retrieve: String") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_str", "test_str_empty", "test_str_spaces", "test_str_special", "test_str_long" };
		cleanup_keys(s, keys, 5);

		SUBCASE("basic string") {
			s->setv("test_str", "hello world");
			CHECK(s->getv("test_str") == Variant("hello world"));
			CHECK(s->getv("test_str").get_type() == Variant::STRING);
		}
		SUBCASE("empty string") {
			s->setv("test_str_empty", "");
			CHECK(s->getv("test_str_empty") == Variant(""));
		}
		SUBCASE("string with spaces and punctuation") {
			s->setv("test_str_spaces", "  hello, world!  ");
			CHECK(s->getv("test_str_spaces") == Variant("  hello, world!  "));
		}
		SUBCASE("string with special characters") {
			s->setv("test_str_special", "path/to/file.txt@#$%^&*()");
			CHECK(s->getv("test_str_special") == Variant("path/to/file.txt@#$%^&*()"));
		}
		SUBCASE("long string") {
			String long_str;
			for (int i = 0; i < 100; i++) {
				long_str += "ABCDEFGHIJ";
			}
			s->setv("test_str_long", long_str);
			CHECK(s->getv("test_str_long") == Variant(long_str));
		}
	}

	// =========================================================================
	// Complex types (serialized via encode_var / decode_var)
	// =========================================================================

	TEST_CASE("Store and retrieve: Dictionary") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_dict", "test_dict_empty", "test_dict_nested" };
		cleanup_keys(s, keys, 3);

		SUBCASE("mixed-type dictionary") {
			Dictionary d;
			d["int_key"] = 42;
			d["float_key"] = 1.5;
			d["str_key"] = "value";
			d["bool_key"] = true;
			s->setv("test_dict", d);
			Dictionary ret = s->getv("test_dict");
			REQUIRE(ret.size() == 4);
			CHECK(ret["int_key"] == Variant(42));
			CHECK(ret["float_key"] == Variant(1.5));
			CHECK(ret["str_key"] == Variant("value"));
			CHECK(ret["bool_key"] == Variant(true));
		}
		SUBCASE("empty dictionary") {
			Dictionary d;
			s->setv("test_dict_empty", d);
			Dictionary ret = s->getv("test_dict_empty");
			CHECK(ret.size() == 0);
		}
		SUBCASE("nested dictionary") {
			Dictionary inner;
			inner["a"] = 1;
			inner["b"] = 2;
			Dictionary outer;
			outer["inner"] = inner;
			outer["c"] = 3;
			s->setv("test_dict_nested", outer);
			Dictionary ret = s->getv("test_dict_nested");
			REQUIRE(ret.has("inner"));
			Dictionary ret_inner = ret["inner"];
			CHECK(ret_inner["a"] == Variant(1));
			CHECK(ret_inner["b"] == Variant(2));
			CHECK(ret["c"] == Variant(3));
		}
	}

	TEST_CASE("Store and retrieve: Array") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_arr", "test_arr_empty", "test_arr_nested" };
		cleanup_keys(s, keys, 3);

		SUBCASE("mixed-type array") {
			Array a = array(1, 2.5, "three", true);
			s->setv("test_arr", a);
			Array ret = s->getv("test_arr");
			REQUIRE(ret.size() == 4);
			CHECK(ret[0] == Variant(1));
			CHECK(ret[1] == Variant(2.5));
			CHECK(ret[2] == Variant("three"));
			CHECK(ret[3] == Variant(true));
		}
		SUBCASE("empty array") {
			Array a;
			s->setv("test_arr_empty", a);
			Array ret = s->getv("test_arr_empty");
			CHECK(ret.size() == 0);
		}
		SUBCASE("nested array") {
			Array inner = array(10, 20, 30);
			Array outer;
			outer.push_back(inner);
			outer.push_back("after");
			s->setv("test_arr_nested", outer);
			Array ret = s->getv("test_arr_nested");
			REQUIRE(ret.size() == 2);
			Array ret_inner = ret[0];
			CHECK(ret_inner.size() == 3);
			CHECK(ret_inner[0] == Variant(10));
			CHECK(ret_inner[2] == Variant(30));
			CHECK(ret[1] == Variant("after"));
		}
	}

	TEST_CASE("Store and retrieve: Vector2") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_vec2" };
		cleanup_keys(s, keys, 1);

		s->setv("test_vec2", Vector2(1.5, -2.5));
		Variant ret = s->getv("test_vec2");
		CHECK(ret.get_type() == Variant::VECTOR2);
		Vector2 v = ret;
		CHECK(doctest::Approx(v.x) == 1.5);
		CHECK(doctest::Approx(v.y) == -2.5);
	}

	TEST_CASE("Store and retrieve: Color") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_color" };
		cleanup_keys(s, keys, 1);

		s->setv("test_color", Color(0.2, 0.4, 0.6, 0.8));
		Variant ret = s->getv("test_color");
		CHECK(ret.get_type() == Variant::COLOR);
		Color c = ret;
		CHECK(doctest::Approx(c.r) == 0.2f);
		CHECK(doctest::Approx(c.g) == 0.4f);
		CHECK(doctest::Approx(c.b) == 0.6f);
		CHECK(doctest::Approx(c.a) == 0.8f);
	}

	// =========================================================================
	// has_key
	// =========================================================================

	TEST_CASE("has_key") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_has_key" };
		cleanup_keys(s, keys, 1);

		CHECK_FALSE(s->has_key("test_has_key"));
		s->setv("test_has_key", 123);
		CHECK(s->has_key("test_has_key"));
		CHECK_FALSE(s->has_key("test_no_such_key_ever_99999"));
	}

	// =========================================================================
	// remove
	// =========================================================================

	TEST_CASE("remove") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_remove" };
		cleanup_keys(s, keys, 1);

		SUBCASE("remove existing key") {
			s->setv("test_remove", "to_be_deleted");
			REQUIRE(s->has_key("test_remove"));
			s->remove("test_remove");
			CHECK_FALSE(s->has_key("test_remove"));
			CHECK(s->getv("test_remove").get_type() == Variant::NIL);
		}
		SUBCASE("remove nonexistent key is safe") {
			s->remove("test_remove"); // should not crash
			CHECK_FALSE(s->has_key("test_remove"));
		}
	}

	// =========================================================================
	// getv with default value
	// =========================================================================

	TEST_CASE("getv with default value") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_default_exists", "test_default_missing" };
		cleanup_keys(s, keys, 2);

		SUBCASE("returns stored value when key exists") {
			s->setv("test_default_exists", 42);
			CHECK(s->getv("test_default_exists", 999) == Variant(42));
		}
		SUBCASE("returns default when key missing") {
			CHECK(s->getv("test_default_missing", 999) == Variant(999));
		}
		SUBCASE("returns default string when key missing") {
			CHECK(s->getv("test_default_missing", "fallback") == Variant("fallback"));
		}
		SUBCASE("returns nil when no default and key missing") {
			CHECK(s->getv("test_default_missing").get_type() == Variant::NIL);
		}
	}

	// =========================================================================
	// Overwrite behavior
	// =========================================================================

	TEST_CASE("Overwrite: same type") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_ow_same" };
		cleanup_keys(s, keys, 1);

		s->setv("test_ow_same", 10);
		CHECK(s->getv("test_ow_same") == Variant(10));
		s->setv("test_ow_same", 20);
		CHECK(s->getv("test_ow_same") == Variant(20));
		s->setv("test_ow_same", 30);
		CHECK(s->getv("test_ow_same") == Variant(30));
	}

	TEST_CASE("Overwrite: different types") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_ow_diff" };
		cleanup_keys(s, keys, 1);

		s->setv("test_ow_diff", 42);
		CHECK(s->getv("test_ow_diff") == Variant(42));
		CHECK(s->getv("test_ow_diff").get_type() == Variant::INT);

		s->setv("test_ow_diff", "now a string");
		CHECK(s->getv("test_ow_diff") == Variant("now a string"));
		CHECK(s->getv("test_ow_diff").get_type() == Variant::STRING);

		s->setv("test_ow_diff", true);
		CHECK(s->getv("test_ow_diff") == Variant(true));
		CHECK(s->getv("test_ow_diff").get_type() == Variant::BOOL);

		s->setv("test_ow_diff", 3.14);
		Variant ret = s->getv("test_ow_diff");
		CHECK(ret.get_type() == Variant::REAL);
		CHECK(doctest::Approx((double)ret) == 3.14);
	}

	// =========================================================================
	// Multiple independent keys
	// =========================================================================

	TEST_CASE("Multiple independent keys") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_multi_a", "test_multi_b", "test_multi_c" };
		cleanup_keys(s, keys, 3);

		s->setv("test_multi_a", 1);
		s->setv("test_multi_b", "two");
		s->setv("test_multi_c", 3.0);

		// All three should be independently retrievable
		CHECK(s->getv("test_multi_a") == Variant(1));
		CHECK(s->getv("test_multi_b") == Variant("two"));
		CHECK(s->getv("test_multi_c") == Variant(3.0));

		// Modifying one doesn't affect others
		s->setv("test_multi_b", "changed");
		CHECK(s->getv("test_multi_a") == Variant(1));
		CHECK(s->getv("test_multi_b") == Variant("changed"));
		CHECK(s->getv("test_multi_c") == Variant(3.0));
	}

	// =========================================================================
	// Key naming edge cases
	// =========================================================================

	TEST_CASE("Key names") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_key_dots.and.dots", "test_key_with spaces", "test_key/with/slashes", "test_key_CAPS_and_lower" };
		cleanup_keys(s, keys, 4);

		SUBCASE("dotted key") {
			s->setv("test_key_dots.and.dots", 1);
			CHECK(s->getv("test_key_dots.and.dots") == Variant(1));
		}
		SUBCASE("key with spaces") {
			s->setv("test_key_with spaces", 2);
			CHECK(s->getv("test_key_with spaces") == Variant(2));
		}
		SUBCASE("key with slashes") {
			s->setv("test_key/with/slashes", 3);
			CHECK(s->getv("test_key/with/slashes") == Variant(3));
		}
		SUBCASE("case sensitivity") {
			s->setv("test_key_CAPS_and_lower", "mixed");
			CHECK(s->getv("test_key_CAPS_and_lower") == Variant("mixed"));
		}
	}

	// =========================================================================
	// has_key + remove integration
	// =========================================================================

	TEST_CASE("has_key and remove integration") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);
		const char *keys[] = { "test_hr_a", "test_hr_b" };
		cleanup_keys(s, keys, 2);

		// Store two keys
		s->setv("test_hr_a", 100);
		s->setv("test_hr_b", 200);
		CHECK(s->has_key("test_hr_a"));
		CHECK(s->has_key("test_hr_b"));

		// Remove one — other unaffected
		s->remove("test_hr_a");
		CHECK_FALSE(s->has_key("test_hr_a"));
		CHECK(s->has_key("test_hr_b"));
		CHECK(s->getv("test_hr_b") == Variant(200));

		// Re-add removed key with different type
		s->setv("test_hr_a", "reborn");
		CHECK(s->has_key("test_hr_a"));
		CHECK(s->getv("test_hr_a") == Variant("reborn"));
	}

	// =========================================================================
	// Stress: many keys
	// =========================================================================

	TEST_CASE("Many keys") {
		Settings *s = Settings::get_singleton();
		REQUIRE(s != nullptr);

		const int N = 50;
		// Store N keys
		for (int i = 0; i < N; i++) {
			s->setv(vformat("test_batch_%d", i), i * 10);
		}
		// Verify all
		for (int i = 0; i < N; i++) {
			CHECK(s->getv(vformat("test_batch_%d", i)) == Variant(i * 10));
		}
		// Cleanup
		for (int i = 0; i < N; i++) {
			s->remove(vformat("test_batch_%d", i));
		}
		// Verify all removed
		for (int i = 0; i < N; i++) {
			CHECK_FALSE(s->has_key(vformat("test_batch_%d", i)));
		}
	}

} // TEST_SUITE("settings")
#endif
