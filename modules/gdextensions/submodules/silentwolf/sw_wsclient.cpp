/*************************************************************************/
/*  sw_wsclient.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "silent_wolf.h"

void SW_WSClient::_ready() {
	sw_debug("Entering MPClient _ready function");
	// Connect base signals to get notified of connection open, close, and errors.
	_client->connect("connection_closed", this, "_closed");
	_client->connect("connection_error", this, "_closed");
	_client->connect("connection_established", this, "_connected");
	// This signal is emitted when not using the Multiplayer API every time
	// a full packet is received.
	// Alternatively, you could check get_peer(1).get_available_packets() in a loop.
	_client->connect("data_received", this, "_on_data");

	// Initiate connection to the given URL.
	Error err = _client->connect_to_url(websocket_url);
	if (err != OK) {
		ERR_PRINT("Unable to connect to WS server");
		set_process(false);
	}
	emit_signal("ws_client_ready");
}

void SW_WSClient::_closed(bool p_was_clean) {
	// was_clean will tell you if the disconnection was correctly notified
	// by the remote peer before closing the socket.
	sw_debug("WS connection closed, clean: ", p_was_clean);
	set_process(false);
}

void SW_WSClient::_connected(const String &p_proto) {
	// This is called on connection, "proto" will be the selected WebSocket
	// sub-protocol (which is optional)
	//sw_debug("Connected with protocol: ", proto);
	print_debug("Connected with protocol: ", proto);
	// You MUST always use get_peer(1).put_packet to send data to server,
	// and not put_packet directly when not using the MultiplayerAPI.
	//Dictionary test_packet = helper::dict( "data", "Test packet" );
	//send_to_server(test_packet);
	//_client->get_peer(1)->put_packet(String("Test packet").utf8());
}

void SW_WSClient::_on_data() {
	// Print the received packet, you MUST always use get_peer(1)->get_packet()
	// to receive data from server, and not get_packet directly when not
	// using the MultiplayerAPI.
	print_debug("Got data from WS server: ", _client->get_peer(1)->get_packet()->get_string_from_utf8());
}

void SW_WSClient::_process(float p_delta) {
	// Call this in _process or _physics_process. Data transfer, and signals
	// emission will only happen when calling this function.
	_client->poll();
}

// send arbitrary data to backend
void SW_WSClient::send_to_server(int p_message_type, Dictionary p_data) {
	data["message_type"] = p_message_type;
	print_debug("Sending data to server: ", p_ data);
	_client->get_peer(1)->put_packet(JSON.print(data).to_utf8());
}

void SW_WSClient::init_mp_session(const String &p_player_name) {
	print_debug("WSClient init_mp_session, sending initialisation packet to server");
	Dictionary init_packet = helper::dict(
			"player_name", p_player_name, );
	return send_to_server("init", init_packet);
}

void SW_WSClient::create_room() {
}

void SW_WSClient::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_ready();
		} break;
		case NOTIFICATION_PROCESS: {
			float delta = get_process_delta_time();
			_process(delta);
		} break;
	}
}

void SW_WSClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_close"), &SW_WSClient::_close);
	ClassDB::bind_method(D_METHOD("_connected"), &SW_WSClient::_connected);

	ADD_SIGNAL(ws_client_ready);
}

void SW_WSClient::SW_WSClient() {
	_client = newref(WebSocketClient);
}
