/**************************************************************************/
/*  gd_multipeer.cpp                                                      */
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

#include "gd_multipeer.h"

GdMultiPeer *GdMultiPeer::instance = nullptr;

GdMultiPeer *GdMultiPeer::get_singleton() {
	return instance;
}

void GdMultiPeer::start_advertising() {
}

void GdMultiPeer::stop_advertising() {
}

void GdMultiPeer::start_browsing() {
}

void GdMultiPeer::stop_browsing() {
}

void GdMultiPeer::disconnect_self() {
}

Array GdMultiPeer::get_peers() {
	return Array();
}

void GdMultiPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_advertising"), &GdMultiPeer::start_advertising);
	ClassDB::bind_method(D_METHOD("stop_advertising"), &GdMultiPeer::stop_advertising);
	ClassDB::bind_method(D_METHOD("start_browsing"), &GdMultiPeer::start_browsing);
	ClassDB::bind_method(D_METHOD("stop_browsing"), &GdMultiPeer::stop_browsing);
	ClassDB::bind_method(D_METHOD("disconnect_self"), &GdMultiPeer::disconnect_self);
	ClassDB::bind_method(D_METHOD("get_peers"), &GdMultiPeer::get_peers);

	ADD_SIGNAL(MethodInfo("find_peer"));
	ADD_SIGNAL(MethodInfo("loose_peer"));
	ADD_SIGNAL(MethodInfo("receiveInvitation_from_peer"));
	ADD_SIGNAL(MethodInfo("connect_to_peer"));
	ADD_SIGNAL(MethodInfo("disconnect_from_peer"));
	ADD_SIGNAL(MethodInfo("receive_data"));
}

GdMultiPeer::GdMultiPeer() {
	instance = this;
}

GdMultiPeer::~GdMultiPeer() {
	instance = nullptr;
}
