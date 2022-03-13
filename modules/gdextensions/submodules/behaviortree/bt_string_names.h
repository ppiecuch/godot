#ifndef BT_STRING_NAMES_H
#define BT_STRING_NAMES_H

#include "core/string_name.h"
#include "core/os/memory.h"

class BTStringNames {

	friend void register_gdextensions_types();
	friend void unregister_gdextensions_types();

	static BTStringNames* singleton;

	static void create() { singleton = memnew(BTStringNames); }
	static void free() { memdelete( singleton); singleton = nullptr; }

	BTStringNames();

public:
	_FORCE_INLINE_ static BTStringNames* get_singleton() { return singleton; }

	StringName _continue;
	StringName _prepare;
	StringName _self_update;
	StringName _child_update;
	StringName _update;
	StringName _pre_update;
	StringName _post_update;
	StringName _abort;
};

#endif // BT_STRING_NAMES_H
