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
#include "core/io/ip_address.h"
#include "core/io/json.h"

const int DEFAULT_PORT = 42696;
const int DEFAULT_GAME_PORT = 42699;

//
// LanAdvertiser
//

void LanAdvertiser::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_broadcast_timer = memnew(Timer);
			_broadcast_timer->set_wait_time(broadcast_interval);
			_broadcast_timer->set_one_shot(false);
			_broadcast_timer->set_autostart(true);
			_broadcast_timer->connect("timeout", this, "_broadcast");
			add_child(_broadcast_timer);

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

void LanAdvertiser::_broadcast() {
	if (!peer_info.empty()) {
		_udp_socket->put_var(peer_info);
	}
}

void LanAdvertiser::set_peer_info(const Dictionary &p_dict) {
	peer_info = p_dict;
}

Dictionary LanAdvertiser::get_peer_info() const {
	return peer_info;
}

void LanAdvertiser::set_port(int p_port) {
	broadcast_port = p_port;
}

int LanAdvertiser::get_port() const {
	return broadcast_port;
}

void LanAdvertiser::set_broadcast_interval(real_t p_interval) {
	broadcast_interval = p_interval;
}

int LanAdvertiser::get_broadcast_interval() const {
	return broadcast_interval;
}

void LanAdvertiser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_peer_info"), &LanAdvertiser::set_peer_info);
	ClassDB::bind_method(D_METHOD("get_peer_info"), &LanAdvertiser::get_peer_info);
	ClassDB::bind_method(D_METHOD("set_port"), &LanAdvertiser::set_port);
	ClassDB::bind_method(D_METHOD("get_port"), &LanAdvertiser::get_port);
	ClassDB::bind_method(D_METHOD("set_broadcast_interval"), &LanAdvertiser::set_broadcast_interval);
	ClassDB::bind_method(D_METHOD("get_broadcast_interval"), &LanAdvertiser::get_broadcast_interval);
	ClassDB::bind_method(D_METHOD("_broadcast"), &LanAdvertiser::_broadcast);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "Peer info"), "set_peer_info", "get_peer_info");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "Port"), "set_port", "get_port");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "Broadcast interval"), "set_broadcast_interval", "get_broadcast_interval");
}

LanAdvertiser::LanAdvertiser() {
	broadcast_interval = 1;
	broadcast_port = DEFAULT_PORT;
	_broadcast_timer = nullptr;
}

//
// LanListener
//

void LanListener::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_cleanup_timer = memnew(Timer);
			_cleanup_timer->set_wait_time(server_cleanup_timeout);
			_cleanup_timer->set_one_shot(false);
			_cleanup_timer->set_autostart(true);
			_cleanup_timer->connect("timeout", this, "_cleanup");
			add_child(_cleanup_timer);

			_udp_server = newref(UDPServer);
			if (!Engine::get_singleton()->is_editor_hint()) {
				if (_udp_server->listen(listen_port) != OK) {
					ERR_PRINT(vformat("LAN service: Error listening on port: %d", listen_port));
				} else {
					print_verbose(vformat("LAN service: Listening on port: %d", listen_port));
				}
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_cleanup_timer->stop();
			if (_udp_server) {
				_udp_server->stop();
			}
		} break;
		case NOTIFICATION_PROCESS: {
			if (_udp_server->is_listening()) {
				if (_udp_server->poll() == OK) {
					while (_udp_server->is_connection_available()) {
						Ref<PacketPeerUDP> peer = _udp_server->take_connection();
						IP_Address server_ip = peer->get_packet_address();
						int server_port = peer->get_packet_port();
						if (server_ip.is_valid() && server_port > 0) {
							// new peer!
							if (!_known_peers.has(server_ip)) {
								Variant ret;
								ERR_CONTINUE_MSG(peer->get_var(ret) != OK, "Failed to retrive a var");
								ERR_CONTINUE_MSG(ret.get_type() != Variant::DICTIONARY, "Unsupported content");
								Dictionary peer_info = ret;
								peer_info["lastSeen"] = OS::get_singleton()->get_unix_time();
								peer_info["peer"] = peer;
								_known_peers[server_ip] = peer_info;
								print_verbose(vformat("New server found at address %s:%s - %s", server_ip, server_port, peer_info));
								emit_signal("new_peer", peer_info);
							} else {
								// update heartbeat
								_known_peers[server_ip]["lastSeen"] = OS::get_singleton()->get_unix_time();
							}
						}
					}
				}
			}
		} break;
	}
}

void LanListener::set_port(int p_port) {
	listen_port = p_port;
}

int LanListener::get_port() const {
	return listen_port;
}

void LanListener::set_cleanup_timeout(real_t p_timeout) {
	server_cleanup_timeout = p_timeout;
	if (_cleanup_timer) {
		_cleanup_timer->set_wait_time(server_cleanup_timeout);
	}
}

real_t LanListener::get_cleanup_timeout() const {
	return server_cleanup_timeout;
}

void LanListener::_cleanup() {
	const uint64_t now = OS::get_singleton()->get_unix_time();
	Vector<String> remove;
	for (auto *E = _known_peers.front(); E; E = E->next()) {
		const Dictionary &peer_info = E->value();
		if (now - uint64_t(peer_info["lastSeen"]) > server_cleanup_timeout) {
			remove.push_back(E->key());
		}
	}
	for (const auto &peer_ip : remove) {
		Dictionary peer_info = _known_peers[peer_ip];
		_known_peers.erase(peer_ip);
		print_verbose(vformat("Remove peer: %s", peer_ip));
		emit_signal("remove_peer", peer_info);
	}
}

void LanListener::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_port"), &LanListener::set_port);
	ClassDB::bind_method(D_METHOD("get_port"), &LanListener::get_port);
	ClassDB::bind_method(D_METHOD("set_cleanup_timeout"), &LanListener::set_cleanup_timeout);
	ClassDB::bind_method(D_METHOD("get_cleanup_timeout"), &LanListener::get_cleanup_timeout);

	ClassDB::bind_method(D_METHOD("_cleanup"), &LanListener::_cleanup);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "Listen port"), "set_port", "get_port");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "Cleanup timeout"), "set_cleanup_timeout", "get_cleanup_timeout");

	ADD_SIGNAL(MethodInfo("new_peer", PropertyInfo(Variant::DICTIONARY, "info")));
	ADD_SIGNAL(MethodInfo("remove_peer", PropertyInfo(Variant::DICTIONARY, "info")));
}

LanListener::LanListener() {
	_cleanup_timer = nullptr;
	listen_port = DEFAULT_PORT;
	server_cleanup_timeout = 3;
	if (!Engine::get_singleton()->is_editor_hint()) {
		set_process(true);
	}
}

//
// LanPlayer
//

void LanPlayer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_broadcast_timer = memnew(Timer);
			_broadcast_timer->set_wait_time(broadcast_interval);
			_broadcast_timer->set_one_shot(false);
			_broadcast_timer->set_autostart(true);
			_broadcast_timer->connect("timeout", this, "_broadcast");
			add_child(_broadcast_timer);

			_broadcast_socket = newref(PacketPeerUDP);
			_broadcast_socket->set_broadcast_enabled(true);
			_broadcast_socket->set_dest_address(IP_Address("255.255.255.255"), broadcast_port);

			_cleanup_timer = memnew(Timer);
			_cleanup_timer->set_wait_time(server_cleanup_timeout);
			_cleanup_timer->set_one_shot(false);
			_cleanup_timer->set_autostart(true);
			_cleanup_timer->connect("timeout", this, "_cleanup");
			add_child(_cleanup_timer);

			_broadcast_server = newref(UDPServer);
			if (!Engine::get_singleton()->is_editor_hint()) {
				if (_broadcast_server->listen(broadcast_port) != OK) {
					ERR_PRINT(vformat("LAN service: Error listening on port: %d", broadcast_port));
				} else {
					print_verbose(vformat("LAN service: Listening on port: %d", broadcast_port));
				}
			}

			_game_server = newref(UDPServer);
			if (!Engine::get_singleton()->is_editor_hint()) {
				if (_game_server->listen(game_port) != OK) {
					ERR_PRINT(vformat("GAME service: Error listening on port: %d", game_port));
				} else {
					print_verbose(vformat("GAME service: Listening on port: %d", game_port));
				}
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_broadcast_timer->stop();
			_cleanup_timer->stop();
			_broadcast_server->stop();
			_game_server->stop();
		} break;
		case NOTIFICATION_PROCESS: {
			if (_game_server->is_listening()) {
				if (_game_server->poll() == OK) {
				}
			}
			if (_broadcast_server->is_listening()) {
				if (_broadcast_server->poll() == OK) {
					while (_broadcast_server->is_connection_available()) {
						Ref<PacketPeerUDP> peer = _broadcast_server->take_connection();
						IP_Address server_ip = peer->get_packet_address();
						int server_port = peer->get_packet_port();
						if (server_ip.is_valid() && server_port > 0) {
							// new peer!
							if (!_known_peers.has(server_ip)) {
								Variant ret;
								ERR_CONTINUE_MSG(peer->get_var(ret) != OK, "Failed to retrive a var");
								ERR_CONTINUE_MSG(ret.get_type() != Variant::DICTIONARY, "Unsupported content");
								Dictionary peer_info = ret;
								peer_info["lastSeen"] = OS::get_singleton()->get_unix_time();
								peer_info["peer"] = peer;
								_known_peers[server_ip] = peer_info;
								print_verbose(vformat("New server found at address %s:%s - %s", server_ip, server_port, peer_info));
								emit_signal("new_peer", peer_info);
							} else {
								// update heartbeat
								_known_peers[server_ip]["lastSeen"] = OS::get_singleton()->get_unix_time();
							}
						}
					}
				}
			}
		} break;
	}
}

void LanPlayer::set_peer_info(const Dictionary &p_dict) {
	peer_info = p_dict;
}

Dictionary LanPlayer::get_peer_info() const {
	return peer_info;
}

void LanPlayer::set_game_port(int p_port) {
	game_port = p_port;
}

int LanPlayer::get_game_port() const {
	return game_port;
}

void LanPlayer::set_broadcast_port(int p_port) {
	broadcast_port = p_port;
}

int LanPlayer::get_broadcast_port() const {
	return broadcast_port;
}

void LanPlayer::set_cleanup_timeout(real_t p_timeout) {
	server_cleanup_timeout = p_timeout;
}

real_t LanPlayer::get_cleanup_timeout() const {
	return server_cleanup_timeout;
}

void LanPlayer::_cleanup() {
	const uint64_t now = OS::get_singleton()->get_unix_time();
	Vector<String> remove;
	for (auto *E = _known_peers.front(); E; E = E->next()) {
		const Dictionary &peer_info = E->value();
		if (now - uint64_t(peer_info["lastSeen"]) > server_cleanup_timeout) {
			remove.push_back(E->key());
		}
	}
	for (const auto &peer_ip : remove) {
		Dictionary peer_info = _known_peers[peer_ip];
		_known_peers.erase(peer_ip);
		print_verbose(vformat("Remove peer: %s", peer_ip));
		emit_signal("remove_peer", peer_info);
	}
}

void LanPlayer::_broadcast() {
	if (!peer_info.empty()) {
		_broadcast_socket->put_var(peer_info);
	}
}

void LanPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_peer_info"), &LanPlayer::set_peer_info);
	ClassDB::bind_method(D_METHOD("get_peer_info"), &LanPlayer::get_peer_info);
	ClassDB::bind_method(D_METHOD("set_broadcast_port"), &LanPlayer::set_broadcast_port);
	ClassDB::bind_method(D_METHOD("get_broadcast_port"), &LanPlayer::get_broadcast_port);
	ClassDB::bind_method(D_METHOD("set_game_port"), &LanPlayer::set_game_port);
	ClassDB::bind_method(D_METHOD("get_game_port"), &LanPlayer::get_game_port);
	ClassDB::bind_method(D_METHOD("set_cleanup_timeout"), &LanPlayer::set_cleanup_timeout);
	ClassDB::bind_method(D_METHOD("get_cleanup_timeout"), &LanPlayer::get_cleanup_timeout);

	ClassDB::bind_method(D_METHOD("_broadcast"), &LanPlayer::_broadcast);
	ClassDB::bind_method(D_METHOD("_cleanup"), &LanPlayer::_cleanup);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "Peer info"), "set_peer_info", "get_peer_info");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "Game port"), "set_game_port", "get_game_port");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "Broadcast port"), "set_broadcast_port", "get_broadcast_port");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "Cleanup timeout"), "set_cleanup_timeout", "get_cleanup_timeout");

	ADD_SIGNAL(MethodInfo("new_peer", PropertyInfo(Variant::DICTIONARY, "info")));
	ADD_SIGNAL(MethodInfo("remove_peer", PropertyInfo(Variant::DICTIONARY, "info")));
	ADD_SIGNAL(MethodInfo("new_message", PropertyInfo(Variant::DICTIONARY, "msg")));
}

LanPlayer::LanPlayer() {
	_broadcast_timer = nullptr;
	_cleanup_timer = nullptr;
	broadcast_port = DEFAULT_PORT;
	broadcast_interval = 1;
	server_cleanup_timeout = 3;
	game_port = DEFAULT_GAME_PORT;
	if (!Engine::get_singleton()->is_editor_hint()) {
		set_process(true);
	}
}
