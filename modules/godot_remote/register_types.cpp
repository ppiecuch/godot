/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

/* register_types.cpp */

#include "register_types.h"

#include "GRClient.h"
#include "GRCodec.h"
#include "GRDevice.h"
#include "GRNotifications.h"
#include "GRPacket.h"
#include "GRServer.h"
#include "GRUtils.h"
#include "GodotRemote.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "core/project_settings.h"

// clumsy settings to test
// outbound and ip.DstAddr >= 127.0.0.1 and ip.DstAddr <= 127.255.255.255 and (tcp.DstPort == 51341 or tcp.SrcPort == 51341)

// Force-link GRCodecManager (ensures doctest registration survives linker)
static GRCodecManager *_codec_mgr_instance = nullptr;

void register_godot_remote_types() {
	_codec_mgr_instance = memnew(GRCodecManager);

	ClassDB::register_class<GodotRemote>();
	ClassDB::register_class<GRUtilsData>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("GodotRemote", memnew(GodotRemote)));

	ClassDB::register_class<GRNotifications>();
	ClassDB::register_class<GRNotificationPanel>();
	ClassDB::register_class<GRNotificationPanelUpdatable>();
	ClassDB::register_class<GRNotificationStyle>();

	ClassDB::register_virtual_class<GRDevice>();

#ifndef NO_GODOTREMOTE_SERVER
	ClassDB::register_class<GRServer>();
	ClassDB::register_class<GRSViewport>();
	ClassDB::register_class<GRSViewportRenderer>();
#endif

#ifndef NO_GODOTREMOTE_CLIENT
	ClassDB::register_class<GRClient>();
	ClassDB::register_class<GRInputCollector>();
	ClassDB::register_class<GRTextureRect>();
#endif

	// Packets
	ClassDB::register_virtual_class<GRPacket>();
	ClassDB::register_class<GRPacketClientStreamAspect>();
	ClassDB::register_class<GRPacketClientStreamOrientation>();
	ClassDB::register_class<GRPacketCustomInputScene>();
	ClassDB::register_class<GRPacketImageData>();
	ClassDB::register_class<GRPacketInputData>();
	ClassDB::register_class<GRPacketMouseModeSync>();
	ClassDB::register_class<GRPacketServerSettings>();
	ClassDB::register_class<GRPacketSyncTime>();
	ClassDB::register_class<GRPacketCustomUserData>();

	ClassDB::register_class<GRPacketPing>();
	ClassDB::register_class<GRPacketPong>();

	// Input Data
	ClassDB::register_virtual_class<GRInputData>();
	ClassDB::register_class<GRInputDeviceSensorsData>();
	ClassDB::register_class<GRInputDataEvent>();

	ClassDB::register_class<GRIEDataWithModifiers>();
	ClassDB::register_class<GRIEDataMouse>();
	ClassDB::register_class<GRIEDataGesture>();

	ClassDB::register_class<GRIEDataAction>();
	ClassDB::register_class<GRIEDataJoypadButton>();
	ClassDB::register_class<GRIEDataJoypadMotion>();
	ClassDB::register_class<GRIEDataKey>();
	ClassDB::register_class<GRIEDataMagnifyGesture>();
	ClassDB::register_class<GRIEDataMIDI>();
	ClassDB::register_class<GRIEDataMouseButton>();
	ClassDB::register_class<GRIEDataMouseMotion>();
	ClassDB::register_class<GRIEDataPanGesture>();
	ClassDB::register_class<GRIEDataScreenDrag>();
	ClassDB::register_class<GRIEDataScreenTouch>();
}

void unregister_godot_remote_types() {
	if (_codec_mgr_instance) {
		_codec_mgr_instance->deinit();
		memdelete(_codec_mgr_instance);
		_codec_mgr_instance = nullptr;
	}
	if (GodotRemote *instance = GodotRemote::get_singleton())
		memdelete(instance);
}
