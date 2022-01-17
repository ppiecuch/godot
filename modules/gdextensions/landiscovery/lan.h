/*************************************************************************/
/*  lan.h                                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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
	Dictionary server_info;

	Ref<PacketPeerUDP> _udp_socket;
	Timer *_broadcast_timer;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _broadcast();

public:
	void set_service_info(const Dictionary &p_dict);
	Dictionary get_service_info() const;

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
	void set_listen_port(int p_port);
	int get_listen_port() const;
	void set_cleanup_timeout(real_t p_timeout);
	real_t get_cleanup_timeout() const;

	LanListener();
};
