#include "register_types.h"

#include "core/class_db.h"
#include "gdbulletml.h"

void register_gd_bullet_hell_types() {
    ClassDB::register_class<GdBulletml>();
}

void unregister_gd_bullet_hell_types() {
   // Nothing to do here in this example.
}
