/* register_types.cpp */

#include "register_types.h"
#include "core/class_db.h"
#include "core/error_macros.h"

#include "penet/penet.h"

#include "gdnet_address.h"
#include "gdnet_event.h"
#include "gdnet_message.h"
#include "gdnet_peer.h"

void register_penet_types() {
	ClassDB::register_virtual_class<GDNetPeer>();
	ClassDB::register_virtual_class<GDNetEvent>();
	ClassDB::register_virtual_class<GDNetMessage>();
	ClassDB::register_class<GDNetHost>();
	ClassDB::register_class<GDNetAddress>();

	ERR_FAIL_COND_MSG(penet_initialize() != 0, "Unable to initialize PENet");
}

void unregister_penet_types() {
	penet_deinitialize();
}
