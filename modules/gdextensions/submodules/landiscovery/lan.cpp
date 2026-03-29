/**************************************************************************/
/*  lan.cpp                                                               */
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

#include "lan.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

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
			if (_broadcast_timer) {
				_broadcast_timer->stop();
			}
			if (_udp_socket.is_valid()) {
				_udp_socket->close();
			}
		} break;
	}
}

void LanAdvertiser::_broadcast() {
	if (!peer_info.empty() && _udp_socket.is_valid()) {
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
	if (_broadcast_timer) {
		_broadcast_timer->set_wait_time(broadcast_interval);
	}
}

real_t LanAdvertiser::get_broadcast_interval() const {
	return broadcast_interval;
}

void LanAdvertiser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_peer_info", "info"), &LanAdvertiser::set_peer_info);
	ClassDB::bind_method(D_METHOD("get_peer_info"), &LanAdvertiser::get_peer_info);
	ClassDB::bind_method(D_METHOD("set_port", "port"), &LanAdvertiser::set_port);
	ClassDB::bind_method(D_METHOD("get_port"), &LanAdvertiser::get_port);
	ClassDB::bind_method(D_METHOD("set_broadcast_interval", "interval"), &LanAdvertiser::set_broadcast_interval);
	ClassDB::bind_method(D_METHOD("get_broadcast_interval"), &LanAdvertiser::get_broadcast_interval);
	ClassDB::bind_method(D_METHOD("_broadcast"), &LanAdvertiser::_broadcast);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "peer_info"), "set_peer_info", "get_peer_info");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "port"), "set_port", "get_port");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "broadcast_interval"), "set_broadcast_interval", "get_broadcast_interval");
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
			if (_cleanup_timer) {
				_cleanup_timer->stop();
			}
			if (_udp_server.is_valid()) {
				_udp_server->stop();
			}
		} break;
		case NOTIFICATION_PROCESS: {
			if (_udp_server.is_valid() && _udp_server->is_listening()) {
				if (_udp_server->poll() == OK) {
					while (_udp_server->is_connection_available()) {
						Ref<PacketPeerUDP> peer = _udp_server->take_connection();
						IP_Address server_ip = peer->get_packet_address();
						int server_port = peer->get_packet_port();
						if (server_ip.is_valid() && server_port > 0) {
							if (!_known_peers.has(server_ip)) {
								Variant ret;
								ERR_CONTINUE_MSG(peer->get_var(ret) != OK, "Failed to retrieve a var");
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

Map<String, Dictionary> LanListener::get_known_peers() const {
	return _known_peers;
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
	ClassDB::bind_method(D_METHOD("set_port", "port"), &LanListener::set_port);
	ClassDB::bind_method(D_METHOD("get_port"), &LanListener::get_port);
	ClassDB::bind_method(D_METHOD("set_cleanup_timeout", "timeout"), &LanListener::set_cleanup_timeout);
	ClassDB::bind_method(D_METHOD("get_cleanup_timeout"), &LanListener::get_cleanup_timeout);

	ClassDB::bind_method(D_METHOD("_cleanup"), &LanListener::_cleanup);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "listen_port"), "set_port", "get_port");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "cleanup_timeout"), "set_cleanup_timeout", "get_cleanup_timeout");

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
			if (_broadcast_timer) {
				_broadcast_timer->stop();
			}
			if (_cleanup_timer) {
				_cleanup_timer->stop();
			}
			if (_broadcast_server.is_valid()) {
				_broadcast_server->stop();
			}
			if (_game_server.is_valid()) {
				_game_server->stop();
			}
			if (_broadcast_socket.is_valid()) {
				_broadcast_socket->close();
			}
		} break;
		case NOTIFICATION_PROCESS: {
			if (_game_server.is_valid() && _game_server->is_listening()) {
				_game_server->poll();
			}
			if (_broadcast_server.is_valid() && _broadcast_server->is_listening()) {
				if (_broadcast_server->poll() == OK) {
					while (_broadcast_server->is_connection_available()) {
						Ref<PacketPeerUDP> peer = _broadcast_server->take_connection();
						IP_Address server_ip = peer->get_packet_address();
						int server_port = peer->get_packet_port();
						if (server_ip.is_valid() && server_port > 0) {
							if (!_known_peers.has(server_ip)) {
								Variant ret;
								ERR_CONTINUE_MSG(peer->get_var(ret) != OK, "Failed to retrieve a var");
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
	if (_cleanup_timer) {
		_cleanup_timer->set_wait_time(server_cleanup_timeout);
	}
}

real_t LanPlayer::get_cleanup_timeout() const {
	return server_cleanup_timeout;
}

void LanPlayer::set_broadcast_interval(real_t p_interval) {
	broadcast_interval = p_interval;
	if (_broadcast_timer) {
		_broadcast_timer->set_wait_time(broadcast_interval);
	}
}

real_t LanPlayer::get_broadcast_interval() const {
	return broadcast_interval;
}

Map<String, Dictionary> LanPlayer::get_known_peers() const {
	return _known_peers;
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
	if (!peer_info.empty() && _broadcast_socket.is_valid()) {
		_broadcast_socket->put_var(peer_info);
	}
}

void LanPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_peer_info", "info"), &LanPlayer::set_peer_info);
	ClassDB::bind_method(D_METHOD("get_peer_info"), &LanPlayer::get_peer_info);
	ClassDB::bind_method(D_METHOD("set_broadcast_port", "port"), &LanPlayer::set_broadcast_port);
	ClassDB::bind_method(D_METHOD("get_broadcast_port"), &LanPlayer::get_broadcast_port);
	ClassDB::bind_method(D_METHOD("set_game_port", "port"), &LanPlayer::set_game_port);
	ClassDB::bind_method(D_METHOD("get_game_port"), &LanPlayer::get_game_port);
	ClassDB::bind_method(D_METHOD("set_cleanup_timeout", "timeout"), &LanPlayer::set_cleanup_timeout);
	ClassDB::bind_method(D_METHOD("get_cleanup_timeout"), &LanPlayer::get_cleanup_timeout);
	ClassDB::bind_method(D_METHOD("set_broadcast_interval", "interval"), &LanPlayer::set_broadcast_interval);
	ClassDB::bind_method(D_METHOD("get_broadcast_interval"), &LanPlayer::get_broadcast_interval);

	ClassDB::bind_method(D_METHOD("_broadcast"), &LanPlayer::_broadcast);
	ClassDB::bind_method(D_METHOD("_cleanup"), &LanPlayer::_cleanup);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "peer_info"), "set_peer_info", "get_peer_info");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "game_port"), "set_game_port", "get_game_port");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "broadcast_port"), "set_broadcast_port", "get_broadcast_port");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "broadcast_interval"), "set_broadcast_interval", "get_broadcast_interval");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "cleanup_timeout"), "set_cleanup_timeout", "get_cleanup_timeout");

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

// --- Doctest Tests ---

#ifdef DOCTEST
#include "doctest/doctest_godot.h"

TEST_SUITE("[[landiscovery]] LanAdvertiser") {
	TEST_CASE("[landiscovery] default constructor values") {
		LanAdvertiser *adv = memnew(LanAdvertiser);

		CHECK(adv->get_port() == DEFAULT_PORT);
		CHECK(adv->get_broadcast_interval() == doctest::Approx(1.0));
		CHECK(adv->get_peer_info().empty());

		memdelete(adv);
	}

	TEST_CASE("[landiscovery] set and get port") {
		LanAdvertiser *adv = memnew(LanAdvertiser);

		adv->set_port(12345);
		CHECK(adv->get_port() == 12345);

		adv->set_port(0);
		CHECK(adv->get_port() == 0);

		adv->set_port(65535);
		CHECK(adv->get_port() == 65535);

		memdelete(adv);
	}

	TEST_CASE("[landiscovery] set and get broadcast interval") {
		LanAdvertiser *adv = memnew(LanAdvertiser);

		adv->set_broadcast_interval(0.5);
		CHECK(adv->get_broadcast_interval() == doctest::Approx(0.5));

		adv->set_broadcast_interval(5.0);
		CHECK(adv->get_broadcast_interval() == doctest::Approx(5.0));

		adv->set_broadcast_interval(0.1);
		CHECK(adv->get_broadcast_interval() == doctest::Approx(0.1));

		memdelete(adv);
	}

	TEST_CASE("[landiscovery] set and get peer info") {
		LanAdvertiser *adv = memnew(LanAdvertiser);

		CHECK(adv->get_peer_info().empty());

		Dictionary info;
		info["name"] = "TestGame";
		info["players"] = 4;
		info["map"] = "desert";
		adv->set_peer_info(info);

		Dictionary result = adv->get_peer_info();
		CHECK(result.size() == 3);
		CHECK(String(result["name"]) == "TestGame");
		CHECK(int(result["players"]) == 4);
		CHECK(String(result["map"]) == "desert");

		memdelete(adv);
	}

	TEST_CASE("[landiscovery] peer info can be overwritten") {
		LanAdvertiser *adv = memnew(LanAdvertiser);

		Dictionary info1;
		info1["name"] = "Game1";
		adv->set_peer_info(info1);
		CHECK(String(adv->get_peer_info()["name"]) == "Game1");

		Dictionary info2;
		info2["name"] = "Game2";
		info2["version"] = "1.0";
		adv->set_peer_info(info2);
		CHECK(String(adv->get_peer_info()["name"]) == "Game2");
		CHECK(adv->get_peer_info().size() == 2);

		memdelete(adv);
	}

	TEST_CASE("[landiscovery] peer info can be cleared") {
		LanAdvertiser *adv = memnew(LanAdvertiser);

		Dictionary info;
		info["name"] = "TestGame";
		adv->set_peer_info(info);
		CHECK_FALSE(adv->get_peer_info().empty());

		adv->set_peer_info(Dictionary());
		CHECK(adv->get_peer_info().empty());

		memdelete(adv);
	}
}

TEST_SUITE("[[landiscovery]] LanListener") {
	TEST_CASE("[landiscovery] default constructor values") {
		LanListener *lst = memnew(LanListener);

		CHECK(lst->get_port() == DEFAULT_PORT);
		CHECK(lst->get_cleanup_timeout() == doctest::Approx(3.0));
		CHECK(lst->get_known_peers().empty());

		memdelete(lst);
	}

	TEST_CASE("[landiscovery] set and get port") {
		LanListener *lst = memnew(LanListener);

		lst->set_port(54321);
		CHECK(lst->get_port() == 54321);

		lst->set_port(DEFAULT_PORT);
		CHECK(lst->get_port() == DEFAULT_PORT);

		memdelete(lst);
	}

	TEST_CASE("[landiscovery] set and get cleanup timeout") {
		LanListener *lst = memnew(LanListener);

		lst->set_cleanup_timeout(10.0);
		CHECK(lst->get_cleanup_timeout() == doctest::Approx(10.0));

		lst->set_cleanup_timeout(0.5);
		CHECK(lst->get_cleanup_timeout() == doctest::Approx(0.5));

		memdelete(lst);
	}

	TEST_CASE("[landiscovery] known peers initially empty") {
		LanListener *lst = memnew(LanListener);

		Map<String, Dictionary> peers = lst->get_known_peers();
		CHECK(peers.empty());

		memdelete(lst);
	}
}

TEST_SUITE("[[landiscovery]] LanPlayer") {
	TEST_CASE("[landiscovery] default constructor values") {
		LanPlayer *player = memnew(LanPlayer);

		CHECK(player->get_broadcast_port() == DEFAULT_PORT);
		CHECK(player->get_game_port() == DEFAULT_GAME_PORT);
		CHECK(player->get_cleanup_timeout() == doctest::Approx(3.0));
		CHECK(player->get_broadcast_interval() == doctest::Approx(1.0));
		CHECK(player->get_peer_info().empty());
		CHECK(player->get_known_peers().empty());

		memdelete(player);
	}

	TEST_CASE("[landiscovery] set and get broadcast port") {
		LanPlayer *player = memnew(LanPlayer);

		player->set_broadcast_port(11111);
		CHECK(player->get_broadcast_port() == 11111);

		memdelete(player);
	}

	TEST_CASE("[landiscovery] set and get game port") {
		LanPlayer *player = memnew(LanPlayer);

		player->set_game_port(22222);
		CHECK(player->get_game_port() == 22222);

		memdelete(player);
	}

	TEST_CASE("[landiscovery] set and get cleanup timeout") {
		LanPlayer *player = memnew(LanPlayer);

		player->set_cleanup_timeout(5.0);
		CHECK(player->get_cleanup_timeout() == doctest::Approx(5.0));

		memdelete(player);
	}

	TEST_CASE("[landiscovery] set and get broadcast interval") {
		LanPlayer *player = memnew(LanPlayer);

		player->set_broadcast_interval(2.5);
		CHECK(player->get_broadcast_interval() == doctest::Approx(2.5));

		memdelete(player);
	}

	TEST_CASE("[landiscovery] set and get peer info") {
		LanPlayer *player = memnew(LanPlayer);

		Dictionary info;
		info["game"] = "MyGame";
		info["level"] = 5;
		info["mode"] = "coop";
		player->set_peer_info(info);

		Dictionary result = player->get_peer_info();
		CHECK(result.size() == 3);
		CHECK(String(result["game"]) == "MyGame");
		CHECK(int(result["level"]) == 5);
		CHECK(String(result["mode"]) == "coop");

		memdelete(player);
	}

	TEST_CASE("[landiscovery] peer info with nested data") {
		LanPlayer *player = memnew(LanPlayer);

		Dictionary info;
		info["name"] = "Server1";
		Array player_list;
		player_list.push_back("Alice");
		player_list.push_back("Bob");
		info["players"] = player_list;
		player->set_peer_info(info);

		Dictionary result = player->get_peer_info();
		CHECK(result.has("players"));
		Array players = result["players"];
		CHECK(players.size() == 2);
		CHECK(String(players[0]) == "Alice");
		CHECK(String(players[1]) == "Bob");

		memdelete(player);
	}

	TEST_CASE("[landiscovery] ports are independent") {
		LanPlayer *player = memnew(LanPlayer);

		player->set_broadcast_port(10000);
		player->set_game_port(20000);

		CHECK(player->get_broadcast_port() == 10000);
		CHECK(player->get_game_port() == 20000);
		CHECK(player->get_broadcast_port() != player->get_game_port());

		memdelete(player);
	}

	TEST_CASE("[landiscovery] known peers initially empty") {
		LanPlayer *player = memnew(LanPlayer);

		Map<String, Dictionary> peers = player->get_known_peers();
		CHECK(peers.empty());

		memdelete(player);
	}
}

TEST_SUITE("[[landiscovery]] Constants") {
	TEST_CASE("[landiscovery] default port values") {
		CHECK(DEFAULT_PORT == 42696);
		CHECK(DEFAULT_GAME_PORT == 42699);
		CHECK(DEFAULT_PORT != DEFAULT_GAME_PORT);
	}

	TEST_CASE("[landiscovery] all classes share default broadcast port") {
		LanAdvertiser *adv = memnew(LanAdvertiser);
		LanListener *lst = memnew(LanListener);
		LanPlayer *player = memnew(LanPlayer);

		CHECK(adv->get_port() == lst->get_port());
		CHECK(lst->get_port() == player->get_broadcast_port());

		memdelete(adv);
		memdelete(lst);
		memdelete(player);
	}
}

TEST_SUITE("[[landiscovery]] ClassDB registration") {
	TEST_CASE("[landiscovery] LanAdvertiser is registered") {
		CHECK(ClassDB::class_exists("LanAdvertiser"));
		CHECK(ClassDB::is_parent_class("LanAdvertiser", "Node"));
	}

	TEST_CASE("[landiscovery] LanListener is registered") {
		CHECK(ClassDB::class_exists("LanListener"));
		CHECK(ClassDB::is_parent_class("LanListener", "Node"));
	}

	TEST_CASE("[landiscovery] LanPlayer is registered") {
		CHECK(ClassDB::class_exists("LanPlayer"));
		CHECK(ClassDB::is_parent_class("LanPlayer", "Node"));
	}

	TEST_CASE("[landiscovery] LanAdvertiser properties are bound") {
		CHECK(ClassDB::has_method("LanAdvertiser", "set_peer_info"));
		CHECK(ClassDB::has_method("LanAdvertiser", "get_peer_info"));
		CHECK(ClassDB::has_method("LanAdvertiser", "set_port"));
		CHECK(ClassDB::has_method("LanAdvertiser", "get_port"));
		CHECK(ClassDB::has_method("LanAdvertiser", "set_broadcast_interval"));
		CHECK(ClassDB::has_method("LanAdvertiser", "get_broadcast_interval"));
	}

	TEST_CASE("[landiscovery] LanListener properties are bound") {
		CHECK(ClassDB::has_method("LanListener", "set_port"));
		CHECK(ClassDB::has_method("LanListener", "get_port"));
		CHECK(ClassDB::has_method("LanListener", "set_cleanup_timeout"));
		CHECK(ClassDB::has_method("LanListener", "get_cleanup_timeout"));
	}

	TEST_CASE("[landiscovery] LanListener has signals") {
		CHECK(ClassDB::has_signal("LanListener", "new_peer"));
		CHECK(ClassDB::has_signal("LanListener", "remove_peer"));
	}

	TEST_CASE("[landiscovery] LanPlayer properties are bound") {
		CHECK(ClassDB::has_method("LanPlayer", "set_peer_info"));
		CHECK(ClassDB::has_method("LanPlayer", "get_peer_info"));
		CHECK(ClassDB::has_method("LanPlayer", "set_broadcast_port"));
		CHECK(ClassDB::has_method("LanPlayer", "get_broadcast_port"));
		CHECK(ClassDB::has_method("LanPlayer", "set_game_port"));
		CHECK(ClassDB::has_method("LanPlayer", "get_game_port"));
		CHECK(ClassDB::has_method("LanPlayer", "set_cleanup_timeout"));
		CHECK(ClassDB::has_method("LanPlayer", "get_cleanup_timeout"));
		CHECK(ClassDB::has_method("LanPlayer", "set_broadcast_interval"));
		CHECK(ClassDB::has_method("LanPlayer", "get_broadcast_interval"));
	}

	TEST_CASE("[landiscovery] LanPlayer has signals") {
		CHECK(ClassDB::has_signal("LanPlayer", "new_peer"));
		CHECK(ClassDB::has_signal("LanPlayer", "remove_peer"));
		CHECK(ClassDB::has_signal("LanPlayer", "new_message"));
	}
}

#endif // DOCTEST
