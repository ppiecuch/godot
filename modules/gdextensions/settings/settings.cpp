/*************************************************************************/
/*  settings.cpp                                                         */
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

Variant Settings::getv(const String &p_key) {
	return storage->get(p_key);
}

void Settings::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setv", "key", "value"), &Settings::setv);
	ClassDB::bind_method(D_METHOD("getv", "key"), &Settings::getv);
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
TEST_CASE("Storing") {
	Settings *settings = Settings::get_singleton();
	SUBCASE("int") {
		settings->setv("int", 99);
		CHECK(settings->getv("int") == 99);
	}
	SUBCASE("float") {
		settings->setv("float", 1.25);
		CHECK(settings->getv("float") == 1.25);
	}
	SUBCASE("Dictionary") {
		Dictionary map1;
		map1["1"] = 1;
		map1["2"] = 2.5;
		map1["3"] = "3";
		settings->setv("dict", map1);
		Dictionary ret1 = settings->getv("dict");
		CHECK(ret1.has_all(array("1", "2", "3")));
		CHECK(ret1["1"] == 1 and ret1["2"] == 2.5 and ret1["3"] == "3");
	}
	SUBCASE("Array") {
		Array arr1 = array(1, 2.5, "3");
		settings->setv("array", arr1);
		Array ret1 = settings->getv("array");
		CHECK(ret1[0] == 1 and ret1[1] == 2.5 and ret1[2] == "3");
	}
}
#endif
