#include "core/map.h"
#include "core/variant.h"
#include "core/reference.h"

class SignalMapper : public Reference {

	void _forward(Array args);

protected:
	static void _bind_methods();

public:
	void forward(Object *p_obj_from, const String &p_sig_from, Object *p_obj_to, const String &p_sig_to = "");
};
