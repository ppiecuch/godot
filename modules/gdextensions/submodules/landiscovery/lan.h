/**************************************************************************/
/*  lan.h                                                                 */
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

#include "core/io/packet_peer_udp.h"
#include "core/io/udp_server.h"
#include "core/map.h"
#include "core/object.h"
#include "scene/main/node.h"
#include "scene/main/timer.h"

class LanAdvertiser : public Node {
	GDCLASS(LanAdvertiser, Node);

	real_t broadcast_interval;
	int broadcast_port;
	Dictionary peer_info;

	Ref<PacketPeerUDP> _udp_socket;
	Timer *_broadcast_timer;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _broadcast();

public:
	void set_peer_info(const Dictionary &p_dict);
	Dictionary get_peer_info() const;
	void set_broadcast_interval(real_t p_interval);
	int get_broadcast_interval() const;
	void set_port(int p_port);
	int get_port() const;

	LanAdvertiser();
};

class LanListener : public Node {
	GDCLASS(LanListener, Node);

	int listen_port;
	int server_cleanup_timeout; // Number of seconds to wait when a server
								// hasn't been heard from before remove

	Ref<UDPServer> _udp_server;
	Timer *_cleanup_timer;
	Map<String, Dictionary> _known_peers;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _cleanup();

public:
	void set_port(int p_port);
	int get_port() const;
	void set_cleanup_timeout(real_t p_timeout);
	real_t get_cleanup_timeout() const;

	LanListener();
};

class LanPlayer : public Node {
	GDCLASS(LanPlayer, Node);

	int broadcast_port, game_port;
	real_t server_cleanup_timeout, broadcast_interval;

	// advertiser
	Ref<PacketPeerUDP> _broadcast_socket;
	Timer *_broadcast_timer;
	Dictionary peer_info;
	// listener
	Ref<UDPServer> _broadcast_server;
	Timer *_cleanup_timer;
	Map<String, Dictionary> _known_peers;
	// game server
	Ref<UDPServer> _game_server;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _broadcast();
	void _cleanup();

public:
	void set_peer_info(const Dictionary &p_dict);
	Dictionary get_peer_info() const;
	void set_broadcast_port(int p_port);
	int get_broadcast_port() const;
	void set_game_port(int p_port);
	int get_game_port() const;
	void set_cleanup_timeout(real_t p_timeout);
	real_t get_cleanup_timeout() const;

	LanPlayer();
};
