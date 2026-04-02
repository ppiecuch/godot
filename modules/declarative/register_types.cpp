#include "register_types.h"

#include "gui_loader.h"

void register_declarative_types() {
	ClassDB::register_class<GUILoader>();
}

void unregister_declarative_types() {
}
