/*************************************************************************/
/*  enet_node.h                                                          */
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

#ifndef NETWORKED_MULTIPLAYER_NODE_H
#define NETWORKED_MULTIPLAYER_NODE_H

#include "core/io/networked_multiplayer_peer.h"
#include "enet_packet_peer.h"
#include "scene/main/node.h"

class ENetNode : public Node {
	GDCLASS(ENetNode, Node);

public:
	enum NetProcessMode {
		MODE_IDLE,
		MODE_PHYSICS
	};

	enum NetworkCommands {
		NETWORK_COMMAND_REMOTE_CALL,
		NETWORK_COMMAND_REMOTE_SET,
		NETWORK_COMMAND_SIMPLIFY_PATH,
		NETWORK_COMMAND_CONFIRM_PATH,
	};

private:
	//path sent caches
	struct PathSentCache {
		Map<int, bool> confirmed_peers;
		int id;
	};

	//path get caches
	struct PathGetCache {
		struct NodeInfo {
			NodePath path;
			ObjectID instance;
		};

		Map<int, NodeInfo> nodes;
	};

	HashMap<NodePath, PathSentCache> path_send_cache;
	Map<int, PathGetCache> path_get_cache;

	Ref<ENetPacketPeer> network_peer;
	Set<int> connected_peers;
	NetProcessMode poll_mode;
	NetProcessMode signal_mode;
	Node *root_node;

	void _network_poll();
	void _network_process();
	void _network_process_packet(int p_from, const uint8_t *p_packet, int p_packet_len);

	void _network_peer_connected(int p_id);
	void _network_peer_disconnected(int p_id);

	void _connected_to_server();
	void _connection_failed();
	void _server_disconnected();
	void _update_process_mode();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_network_peer(const Ref<ENetPacketPeer> &p_network_peer);

	void set_signal_mode(NetProcessMode p_mode);
	NetProcessMode get_signal_mode() const;
	void set_poll_mode(NetProcessMode p_mode);
	NetProcessMode get_poll_mode() const;

	bool is_network_server() const;
	int get_network_unique_id() const;
	Error kick_client(int p_id);

	Error broadcast(const PoolVector<uint8_t> &p_packet, int p_channel = 0);
	Error send(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel = 0);
	Error broadcast_unreliable(const PoolVector<uint8_t> &p_packet, int p_channel = 0);
	Error send_unreliable(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel = 0);
	Error broadcast_ordered(const PoolVector<uint8_t> &p_packet, int p_channel = 0);
	Error send_ordered(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel = 0);
	Error put_packet(NetworkedMultiplayerPeer::TransferMode p_mode, int p_target, const PoolVector<uint8_t> &p_packet, int p_channel = 0);

	ENetNode();
	~ENetNode();
};

VARIANT_ENUM_CAST(ENetNode::NetProcessMode);

#endif
