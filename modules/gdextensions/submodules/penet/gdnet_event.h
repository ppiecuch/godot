/**************************************************************************/
/*  gdnet_event.h                                                         */
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

/* gdnet_event.h */

#ifndef GDNET_EVENT_H
#define GDNET_EVENT_H

#include "core/io/marshalls.h"
#include "core/reference.h"
#include "core/variant.h"

#include "penet/penet.h"

class GDNetEvent : public Reference {
	GDCLASS(GDNetEvent, Reference);

public:
	enum Type {
		NONE,
		CONNECT,
		DISCONNECT,
		RECEIVE
	};

private:
	Type _type;
	int _time;
	int _peer_id;
	int _channel_id;
	PoolByteArray _packet;
	int _data;

protected:
	static void _bind_methods();

public:
	GDNetEvent() :
			_type(NONE), _time(0), _peer_id(0), _channel_id(0), _data(0) {}

	void set_event_type(Type type) { _type = type; }
	void set_time(int ms) { _time = ms; }
	void set_peer_id(int peer_id) { _peer_id = peer_id; }
	void set_channel_id(int channel_id) { _channel_id = channel_id; }
	void set_packet(const PoolByteArray &packet) { _packet = packet; }
	void set_data(int data) { _data = data; }

	Type get_event_type() { return _type; }
	int get_time() { return _time; }
	int get_peer_id() { return _peer_id; }
	int get_channel_id() { return _channel_id; }
	const PoolByteArray &get_packet() { return _packet; }
	Variant get_var();
	int get_data() { return _data; }
};

VARIANT_ENUM_CAST(GDNetEvent::Type);

#endif
