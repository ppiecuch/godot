#ifndef INPUT_HELPER_H
#define INPUT_HELPER_H

#include "core/object.h"

class InputHelper : public Object {
	GDCLASS(InputHelper, Object)

	String device;
	int device_index;

protected:
	static void _bind_methods();

public:
	String get_simplified_device_name(const String &raw_name);
	String get_key_label(const String &action_name);
	int get_button_index(const String &action_name);

	InputHelper();
	~InputHelper();
};

#endif // INPUT_HELPER_H
