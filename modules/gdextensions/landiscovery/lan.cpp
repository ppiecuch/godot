/*************************************************************************/
/*  lan.cpp                                                              */
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

#include "lan.h"

#include "common/gd_core.h"
#include "core/io/json.h"

const int DEFAULT_PORT = 42696;

//
// GdLanAdvertiser
//

void GdLanAdvertiser::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_broadcast_timer = memnew(Timer);
			_broadcast_timer->set_wait_time(broadcast_interval);
			_broadcast_timer->set_one_shot(false);
			_broadcast_timer->set_autostart(true);

			add_child(_broadcast_timer);

			_broadcast_timer->connect("timeout", this, "_broadcast");

			_udp_socket = newref(PacketPeerUDP);
			_udp_socket->set_broadcast_enabled(true);
			_udp_socket->set_dest_address(IP_Address("255.255.255.255"), broadcast_port);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_broadcast_timer->stop();
			if (_udp_socket) {
				_udp_socket->close();
			}
		} break;
	}
}

void GdLanAdvertiser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_broadcast"), &GdLanAdvertiser::_broadcast);
}

void GdLanAdvertiser::_broadcast() {
	String msg = JSON::print(server_info);
	CharString packet = msg.ascii();
	_udp_socket->put_packet((const uint8_t *)packet.get_data(), packet.size());
}

GdLanAdvertiser::GdLanAdvertiser() {
	broadcast_interval = 1;
	broadcast_port = DEFAULT_PORT;
}

//
// GdLanListener
//

void GdLanListener::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_cleanup_timer->set_wait_time(server_cleanup_timeout);
			_cleanup_timer->set_one_shot(false);
			_cleanup_timer->set_autostart(true);
			_cleanup_timer->connect("timeout", this, "clean_up");
			add_child(_cleanup_timer);

			_udp_socket = newref(PacketPeerUDP);
			if (_udp_socket->listen(listen_port) != OK) {
				ERR_PRINT(vformat("GameServer LAN service: Error listening on port: %d", listen_port));
			} else {
				print_verbose(vformat("GameServer LAN service: Listening on port: %d", listen_port));
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_cleanup_timer->stop();
			if (_udp_socket) {
				_udp_socket->close();
			}
		} break;
		case NOTIFICATION_PROCESS: {
			while (_udp_socket->get_available_packet_count()) {
				int len;
				const uint8_t *packet;
				Error err = _udp_socket->get_packet(&packet, len);
				if (err != OK) {
					ERR_PRINT("Error getting packet!");
				}
			}
		} break;
	}
}

void GdLanListener::_cleanup() {
}

void GdLanListener::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_cleanup"), &GdLanListener::_cleanup);

	ADD_SIGNAL(MethodInfo("new_server"));
	ADD_SIGNAL(MethodInfo("remove_server"));
}

GdLanListener::GdLanListener() {
	listen_port = DEFAULT_PORT;
	server_cleanup_timeout = 3;
}
