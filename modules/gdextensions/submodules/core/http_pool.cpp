/**************************************************************************/
/*  http_pool.cpp                                                         */
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

#include "http_pool.h"

/// HTTPState

void HTTPState::_bind_methods() {
	ADD_SIGNAL(MethodInfo("_connection_finished", PropertyInfo(Variant::OBJECT, "http_client", PROPERTY_HINT_RESOURCE_TYPE, "HTTPClient")));
	ADD_SIGNAL(MethodInfo("_request_finished", PropertyInfo(Variant::BOOL, "success")));
	ADD_SIGNAL(MethodInfo("download_progressed", PropertyInfo(Variant::INT, "bytes"), PropertyInfo(Variant::INT, "total_bytes")));

	ClassDB::bind_method(D_METHOD("set_output_path", "out_path"), &HTTPState::set_output_path);
	ClassDB::bind_method(D_METHOD("cancel"), &HTTPState::cancel);
	ClassDB::bind_method(D_METHOD("term"), &HTTPState::terminate);
	ClassDB::bind_method(D_METHOD("http_tick"), &HTTPState::http_tick);
	ClassDB::bind_method(D_METHOD("connect_http", "hostname", "port", "use_ssl"), &HTTPState::connect_http);
	ClassDB::bind_method(D_METHOD("wait_for_request"), &HTTPState::wait_for_request);
	ClassDB::bind_method(D_METHOD("release"), &HTTPState::release);
}

void HTTPState::set_output_path(String p_out_path) {
	// Set output path code here.
}

void HTTPState::cancel() {
	// Cancel code here.
}

void HTTPState::terminate() {
	terminated = true;
	if (http.is_valid()) {
		http->close();
	}
	http.instance();
}

void HTTPState::http_tick() {
}

Ref<HTTPClient> HTTPState::connect_http(String p_hostname, int p_port, bool p_use_ssl) {
	return Ref<HTTPClient>(); // Connect to HTTP code here.
}

Variant HTTPState::wait_for_request() {
	sent_request = true;
	http_pool->connect("http_tick", this, "http_tick");
	Variant ret;
	// ret = _request_finished.wait(); // Assuming _request_finished is a Thread or similar
	call_deferred("release");
	return ret;
}

/// HTTPPool

void HTTPPool::_bind_methods() {
	ADD_SIGNAL(MethodInfo("http_tick"));
	ClassDB::bind_method(D_METHOD("set_total_clients", "out_path"), &HTTPPool::set_total_clients);
	ClassDB::bind_method(D_METHOD("get_total_clients"), &HTTPPool::get_total_clients);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "total_http_clients"), "set_total_clients", "get_total_clients");
	ClassDB::bind_method(D_METHOD("_acquire_client"), &HTTPPool::_acquire_client);
	ClassDB::bind_method(D_METHOD("_release_client", "http"), &HTTPPool::_release_client);
	ClassDB::bind_method(D_METHOD("new_http_state"), &HTTPPool::new_http_state);
}

Ref<HTTPClient> HTTPPool::_acquire_client() {
	if (!http_client_pool.empty()) {
		Ref<HTTPClient> client = http_client_pool.top();
		http_client_pool.pop();
		return client;
	}

	// If no available client, create a new one.
	Ref<HTTPClient> client = memnew(HTTPClient);
	return client;
}

void HTTPPool::_release_client(Ref<HTTPClient> http) {
	// Check if there are any pending requests.
	HashMap<int, Ref<HTTPPoolFuture>>::Iterator E = pending_requests.begin();

	if (E) {
		Ref<HTTPPoolFuture> f = (*E).value;
		pending_requests.erase((*E).key);
		f->emit_signal("completed", http);
	} else {
		// If no pending requests, add the client back to the pool.
		http_client_pool.push_back(http);
	}
}

Ref<HTTPState> HTTPPool::new_http_state() {
	Ref<HTTPClient> http_client = _acquire_client();
	Ref<HTTPState> state = memnew(HTTPState);
	// state->initialize(this, http_client);
	return state;
}

void HTTPPool::_notification(int p_what) {
	switch (p_what) {
		case Node::NOTIFICATION_INTERNAL_PROCESS: {
			emit_signal("http_tick");
		} break;
		case Node::NOTIFICATION_POSTINITIALIZE: {
		} break;
	}
}
void HTTPState::release() {
	if (!http_pool) {
		return;
	}
	// if (http_pool->is_connected("http_tick", "http_tick")) {
	//   http_pool->disconnect("http_tick","http_tick");
	// }
	if (http_pool && http) {
		http_pool->_release_client(this->http);
		memdelete(http_pool);
		http.unref();
	}
}
