/**************************************************************************/
/*  gdnet_host.h                                                          */
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

/* gdnet_host.h */

#ifndef GDNET_HOST_H
#define GDNET_HOST_H

#include <string.h>

#include "core/os/mutex.h"
#include "core/os/os.h"
#include "core/os/thread.h"
#include "core/reference.h"

#include "penet/penet.h"

#include "gdnet_address.h"
#include "gdnet_event.h"
#include "gdnet_message.h"
#include "gdnet_peer.h"
#include "gdnet_queue.h"

class GDNetEvent;
class GDNetPeer;

class GDNetHost : public Reference {
	GDCLASS(GDNetHost, Reference);

	friend class GDNetPeer;

	enum {
		DEFAULT_EVENT_WAIT = 1,
		DEFAULT_MAX_PEERS = 32,
		DEFAULT_MAX_CHANNELS = 1,
	};

	PENetHost *_host;
	volatile bool _running;
	Thread _thread;
	Mutex _accessMutex;
	Mutex _hostMutex;

	int _event_wait;
	int _max_peers;
	int _max_channels;
	int _max_bandwidth_in;
	int _max_bandwidth_out;

	GDNetQueue<GDNetEvent> _event_queue;
	GDNetQueue<GDNetMessage> _message_queue;

	void send_messages();
	void poll_events();

	static void thread_callback(void *instance);
	void thread_start();
	void thread_loop();
	void thread_stop();

	void acquireMutex();
	void releaseMutex();

	int get_peer_id(PENetPeer *peer);
	GDNetEvent *new_event(const PENetEvent &penet_event);

protected:
	static void _bind_methods();

public:
	GDNetHost();

	Ref<GDNetPeer> get_peer(unsigned id);

	void set_event_wait(int wait) { _event_wait = wait; }
	void set_max_peers(int max) { _max_peers = max; }
	void set_max_channels(int max) { _max_channels = max; }
	void set_max_bandwidth_in(int max) { _max_bandwidth_in = max; }
	void set_max_bandwidth_out(int max) { _max_bandwidth_out = max; }

	Error bind(Ref<GDNetAddress> addr);
	void unbind();

	Ref<GDNetPeer> host_connect(Ref<GDNetAddress> addr = NULL, int data = 0);

	void broadcast_packet(const PoolByteArray &packet, int channel_id = 0, int type = GDNetMessage::UNSEQUENCED);
	void broadcast_var(const Variant &var, int channel_id = 0, int type = GDNetMessage::UNSEQUENCED);

	bool is_event_available();
	int get_event_count();
	Ref<GDNetEvent> get_event();
};

#endif
