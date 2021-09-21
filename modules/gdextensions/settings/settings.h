
#include "core/object.h"
#include "core/reference.h"
#include "core/variant.h"

class SettingsStorage;

class Settings: public Object {
	GDCLASS(Settings, Object);

private:
	Ref<SettingsStorage> storage;

protected:
	static Settings *instance;
	static void _bind_methods();

public:
	static Settings *get_singleton();

	void set(const String &p_key, const Variant &p_value);
	Variant get(const String &p_key);

	Settings();
	~Settings();
};
