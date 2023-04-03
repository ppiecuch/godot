/**************************************************************************/
/*  sw_wsclient.cpp                                                       */
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
#include "core/io/json.h"
#include <_types/_uint8_t.h>

void SW_WSClient::_ready() {
	sw_debug("Entering SW_WSClient _ready function");
	_client = WebSocketClient::_create();
	// Connect base signals to get notified of connection open, close, and errors.
	_client->connect("connection_closed", this, "_on_closed");
	_client->connect("connection_error", this, "_on_closed");
	_client->connect("connection_established", this, "_on_connected");
	// This signal is emitted when not using the Multiplayer API every time
	// a full packet is received.
	_client->connect("data_received", this, "_on_data");

	// Initiate connection to the given URL.
	const Error err = _client->connect_to_url(websocket_url);
	if (err != OK) {
		ERR_PRINT("Unable to connect to WS server");
	}
	emit_signal("ws_client_ready", err);
}

void SW_WSClient::_terminate() {
	if (_client) {
		_client->disconnect_from_host();
		_client->disconnect("connection_closed", this, "_on_closed");
		_client->disconnect("connection_error", this, "_on_closed");
		_client->disconnect("connection_established", this, "_on_connected");
		_client->disconnect("data_received", this, "_on_data");
	}
}

void SW_WSClient::_process() {
	// Call this in _process or _physics_process. Data transfer, and signals
	// emission will only happen when calling this function.
	_client->poll();
}

void SW_WSClient::_on_closed(bool p_was_clean) {
	// was_clean will tell you if the disconnection was correctly notified
	// by the remote peer before closing the socket.
	sw_debug("WS connection closed, clean: ", p_was_clean);
}

void SW_WSClient::_on_connected(const String &p_proto) {
	// This is called on connection, "proto" will be the selected WebSocket
	// sub-protocol (which is optional)
	if (p_proto.empty()) {
		sw_debug("WS connected");
	} else {
		sw_debug("WS connected with protocol: ", p_proto);
	}
	// You MUST always use get_peer(1)->put_packet to send data to server,
	// and not put_packet directly when not using the MultiplayerAPI.
	//Dictionary test_packet = helper::dict("data", "Test packet");
	//send_to_server(test_packet);
	//_client->get_peer(1)->put_packet(String("Test packet").utf8());
}

void SW_WSClient::_on_data() {
	Ref<WebSocketPeer> peer = _client->get_peer(1);

	if (!peer.is_valid() || !peer->is_connected_to_host()) {
		return;
	}

	String data;
	while (peer->get_available_packet_count()) {
		const uint8_t *packet;
		int len;
		Error err = peer->get_packet(&packet, len);
		if (err != OK) {
			ERR_PRINT("Error getting packet!");
			break;
		}
		String s;
		if (len > 0) {
			s.parse_utf8(reinterpret_cast<const char *>(packet), len);
		}
		if (!s.empty()) {
			data += s;
		}
	}

	sw_debug("Got data from WS server: ", data);
}

// send arbitrary data to backend
void SW_WSClient::send_to_server(const String &p_category, const Dictionary &p_data) {
	Ref<WebSocketPeer> peer = _client->get_peer(1);

	if (!peer.is_valid() || !peer->is_connected_to_host()) {
		ERR_PRINT("Connection lost");
		return;
	}

	Dictionary data = p_data;
	data["message_type"] = p_category;
	sw_debug("Sending data to server: ", data);
	const CharString msg = JSON::print(data).utf8();
	peer->put_packet(reinterpret_cast<const uint8_t *>(msg.c_str()), msg.length());
}

void SW_WSClient::init_mp_session(const String &p_player_name) {
	sw_debug("WSClient init_mp_session, sending initialisation packet to server");
	Dictionary init_packet = helper::dict("player_name", p_player_name);
	return send_to_server("init", init_packet);
}

void SW_WSClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_data"), &SW_WSClient::_on_data);
	ClassDB::bind_method(D_METHOD("_on_closed", "clean"), &SW_WSClient::_on_closed);
	ClassDB::bind_method(D_METHOD("_on_connected", "proto"), &SW_WSClient::_on_connected);
	ClassDB::bind_method(D_METHOD("send_to_server", "message_type", "data"), &SW_WSClient::send_to_server);

	ADD_SIGNAL(MethodInfo("ws_client_ready", PropertyInfo(Variant::INT, "status")));
}

SW_WSClient::SW_WSClient() {
}
