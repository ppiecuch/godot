
#include "core/ustring.h"
#include "core/reference.h"
#include "core/variant.h"
#include "core/os/os.h"
#include "core/io/config_file.h"

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
