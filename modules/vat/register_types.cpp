#include "register_types.h"

#include "vat_animation_track.h"
#include "vat_multi_mesh_instance.h"

void register_vat_types() {
#ifndef _3D_DISABLED
	ClassDB::register_class<VATAnimationTrack>();
	ClassDB::register_class<VATMultiMeshInstance>();
#endif
}

void unregister_vat_types() {
}
