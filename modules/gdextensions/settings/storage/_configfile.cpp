/*************************************************************************/
/*  _configfile.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "core/io/config_file.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "core/variant.h"

const String config_file = ".sharedpreferences.ini";

class SettingsStorage : public Reference {
private:
	ConfigFile cache;
	String section;

	real_t _last_sync_time;
	void _sync();
	String _config_location();

public:
	void set(const String &key, const Variant &value);
	Variant get(const String &key) const;

	SettingsStorage();
	~SettingsStorage();
};

void SettingsStorage::set(const String &key, const Variant &value) {
	cache.set_value(section, key, value);
	_sync();
}

Variant SettingsStorage::get(const String &key) const {
	return cache.get_value(section, key);
}

String SettingsStorage::_config_location() {
	return "userdata://" + config_file;
}

void SettingsStorage::_sync() {
	if (OS::get_singleton()->get_ticks_msec() - _last_sync_time > 1000) {
		cache.save(_config_location());
		_last_sync_time = OS::get_singleton()->get_ticks_msec();
	}
}

SettingsStorage::SettingsStorage() {
	section = get_app_name();
	cache.load(_config_location());
	_last_sync_time = OS::get_singleton()->get_ticks_msec();
}

SettingsStorage::~SettingsStorage() {
	cache.save(_config_location());
}
