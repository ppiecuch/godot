/**************************************************************************/
/*  http_pool.h                                                           */
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

#ifndef HTTP_POOL_H
#define HTTP_POOL_H

#include "core/error_macros.h"
#include "core/hash_map.h"
#include "core/io/http_client.h"
#include "core/io/stream_peer_ssl.h"
#include "core/io/stream_peer_tcp.h"
#include "core/local_vector.h"
#include "core/object.h"
#include "core/os/os.h"
#include "core/ref_ptr.h"
#include "scene/main/http_request.h"
#include "scene/main/node.h"

class HTTPPoolFuture : public Reference {
	GDCLASS(HTTPPoolFuture, Reference);

protected:
	static void _bind_methods() {
		ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::OBJECT, "http", PROPERTY_HINT_RESOURCE_TYPE, "HTTPClient")));
	}
};

class HTTPPool;

class HTTPState : public Reference {
	GDCLASS(HTTPState, Reference);

	HTTPPool *http_pool = nullptr;
	Ref<HTTPClient> http_client;

public:
	static const int YIELD_PERIOD_MS = 50;

	String out_path;

	Ref<HTTPClient> http;
	bool busy = false;
	bool cancelled = false;
	bool terminated = false;

	bool sent_request = false;
	int status = 0;
	int connect_err = OK;

	int response_code = 0;
	PackedByteArray response_body;
	Dictionary response_headers;
	int bytes = 0;
	int total_bytes = 0;

protected:
	static void _bind_methods();

public:
	void set_output_path(String p_out_path);
	void cancel();
	void terminate();
	void http_tick();
	Ref<HTTPClient> connect_http(String p_hostname, int p_port, bool p_use_ssl);
	Variant wait_for_request();
	void release();
	void initialize(Ref<HTTPPool> pool, Ref<HTTPClient> client) {
		this->http = client;
	}
};

class HTTPPool : public Node {
	GDCLASS(HTTPPool, Node);

	int next_request = 0;
	HashMap<int, Ref<HTTPPoolFuture>> pending_requests;

	LocalVector<Ref<HTTPClient>> http_client_pool;
	int total_http_clients = 5;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	HTTPPool() {
		for (int client_i = 0; client_i < total_http_clients; client_i++) {
			Ref<HTTPClient> client = memnew(HTTPClient);
			http_client_pool.push_back(client);
		}
	}
	void set_total_clients(int p_total) {
		total_http_clients = p_total;
	}

	int get_total_clients() const {
		return total_http_clients;
	}
	Ref<HTTPClient> _acquire_client();
	void _release_client(Ref<HTTPClient> p_http);
	Ref<HTTPState> new_http_state();
};

#endif // HTTP_POOL_H
