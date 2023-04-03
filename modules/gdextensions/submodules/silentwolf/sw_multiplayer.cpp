/**************************************************************************/
/*  sw_multiplayer.cpp                                                    */
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

#include "silent_wolf.h"

#include "common/gd_core.h"

void SW_Multiplayer::_send_init_message() {
	WSClient->init_mp_session(mp_player_name);
	mp_ws_ready = true;
	mp_session_started = true;
}

void SW_Multiplayer::_on_ws_data(const String &data) {
	Dictionary msg;
	emit_signal("sw_data_received", msg);
}

void SW_Multiplayer::sw_notification(int what) {
	if (what == SW_NOTIFICATION_READY) {
		mp_ws_ready = false;
		mp_session_started = false;
		WSClient->_ready();
	} else if (what == SW_NOTIFICATION_PROCESS) {
		WSClient->_process();
	}
}

bool SW_Multiplayer::sw_requesting() const {
	return OS::get_singleton()->get_system_time_secs() - _check < 10; // 10 sec after last send
}

void SW_Multiplayer::init_mp_session(const String &player_name) {
	WSClient->init_mp_session(player_name);
}

void SW_Multiplayer::send(const Dictionary &data) {
	sw_debug("Sending data to WS server...");
	WSClient->send_to_server("update", data);
	_check = OS::get_singleton()->get_system_time_secs();
	emit_signal("sw_data_requested");
}

void SW_Multiplayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init_mp_session", "player_name"), &SW_Multiplayer::init_mp_session);
	ClassDB::bind_method(D_METHOD("send", "data"), &SW_Multiplayer::send);

	ADD_SIGNAL(MethodInfo("sw_data_requested"));
	ADD_SIGNAL(MethodInfo("sw_data_received", PropertyInfo(Variant::DICTIONARY, "msg")));
}

SW_Multiplayer::SW_Multiplayer() {
	WSClient = newref(SW_WSClient);
	mp_ws_ready = false;
	mp_session_started = false;
	mp_player_name = "";
	poll_timer = nullptr;
	_check = 0;
}
