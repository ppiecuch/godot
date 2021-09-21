#include "settings.h"

#include "core/variant.h"
#include "core/project_settings.h"
#include "core/os/os.h"

Settings *Settings::instance;

_FORCE_INLINE_ String get_app_name() {
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

#if defined(ANDROID_ENABLED)
#include "storage/_sharedpreferences.cpp"
#elif defined(IOS_ENABLED) || defined(OSX_ENABLED)
#include "storage/_userdefaults.cpp"
#elif defined(WIN32_ENABLED)
#include "storage/_winreg.cpp"
#else
#include "storage/_configfile.cpp"
#endif

Settings *Settings::get_singleton() {
	return instance;
}

void Settings::set(const String &p_key, const Variant &p_value) {
	storage->set(p_key, p_value);
}

Variant Settings::get(const String &p_key) {
	return storage->get(p_key);
}

void Settings::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set", "key", "value"), &Settings::set);
	ClassDB::bind_method(D_METHOD("get", "key"), &Settings::get);
}

Settings::Settings() {
	storage = Ref<SettingsStorage>(memnew(SettingsStorage));
	instance = this;
}

Settings::~Settings() {
	instance = nullptr;
}
