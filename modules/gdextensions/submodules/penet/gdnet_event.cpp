/**************************************************************************/
/*  gdnet_event.cpp                                                       */
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

/* gdnet_server_event.cpp */

#include "gdnet_event.h"

Variant GDNetEvent::get_var() {
	if (_packet.size() > 0) {
		PoolVector<uint8_t>::Read r = _packet.read();
		int len = _packet.size();
		Variant var;

		Error err = decode_variant(var, &r[0], len);

		ERR_FAIL_COND_V(err != OK, Variant());

		return var;
	}

	return Variant();
}

void GDNetEvent::_bind_methods() {
	BIND_ENUM_CONSTANT(NONE);
	BIND_ENUM_CONSTANT(CONNECT);
	BIND_ENUM_CONSTANT(DISCONNECT);
	BIND_ENUM_CONSTANT(RECEIVE);

	ClassDB::bind_method(D_METHOD("get_event_type"), &GDNetEvent::get_event_type);
	ClassDB::bind_method(D_METHOD("get_time"), &GDNetEvent::get_time);
	ClassDB::bind_method(D_METHOD("get_peer_id"), &GDNetEvent::get_peer_id);
	ClassDB::bind_method(D_METHOD("get_channel_id"), &GDNetEvent::get_channel_id);
	ClassDB::bind_method(D_METHOD("get_packet"), &GDNetEvent::get_packet);
	ClassDB::bind_method(D_METHOD("get_var"), &GDNetEvent::get_var);
	ClassDB::bind_method(D_METHOD("get_data"), &GDNetEvent::get_data);
}
