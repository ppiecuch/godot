/**************************************************************************/
/*  nakamaclient.h                                                        */
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

#ifndef NAKAMACLIENT_H
#define NAKAMACLIENT_H

#include "core/reference.h"
#include "modules/websocket/websocket_client.h"
#include "scene/main/http_request.h"

#include "nakama1_api.h"

#include <map>

namespace Builder {
struct Url {
	String _scheme = "http";
	String _host;
	String _port;
	String _path;
	Url &scheme(const String &p_scheme) {
		_scheme = _scheme.empty() ? "http" : p_scheme;
		return *this;
	}
	Url &host(const String &p_host) {
		_host = p_host;
		return *this;
	}
	Url &port(int p_port) {
		_port = p_port == -1 ? "" : String::num(p_port);
		return *this;
	}
	Url &path(const String &p_path) {
		_path = p_path;
		return *this;
	}
	String build() { return vformat("%s://%s%s%s", _scheme, _host, _port.empty() ? "" : ":" + _port, _path.begins_with("/") ? _path : "/" + _path); }
};
struct Request {
	String _url;
	HTTPClient::Method _method = HTTPClient::METHOD_POST;
	Vector<String> _headers;
	PoolStringArray _query;
	PoolByteArray _payload;
	Request &url(const String &p_url) {
		_url = p_url;
		return *this;
	}
	Request &header(const String &p_header) {
		_headers.push_back(p_header);
		return *this;
	}
	Request &header(const String &p_name, const String &p_value) {
		_headers.push_back(p_name + ":" + p_value);
		return *this;
	}
	Request &query_parameter(const String &p_name, const String &p_value) {
		_query.push_back(p_name + "=" + p_value);
		return *this;
	}
	Request method(HTTPClient::Method p_method, const PoolByteArray &p_payload) {
		_method = p_method;
		_payload = p_payload;
		return *this;
	}
	Error make(HTTPRequest *p_client, bool trace = false) {
		if (trace) {
			LOGI("Sending request: " + _url);
		}
		p_client->cancel_request();
		return p_client->request(_url, _headers, true, _method, Utils::bytearray_to_string(_payload));
	}
	Error make(WebSocketClient *p_client, bool trace = false) {
		if (trace) {
			LOGI("Sending request: " + _url);
		}
		String url = _url;
		if (!_query.empty()) {
			String query = _query.join("&");
			if (!query.empty()) {
				url = url + "?" + query;
			}
		}
		return p_client->connect_to_url(_url);
	}
};
} //namespace Builder

class DefaultSession {
private:
	String token;
	String handle;
	String user_id;
	uint64_t create_time = 0; // milliseconds
	uint64_t expire_time = 0; // milliseconds

public:
	String get_auth_token() const { return token; }
	String get_user_id() const { return user_id; }
	uint64_t get_create_time() const { return create_time; }
	uint64_t get_expire_time() const { return expire_time; }
	bool is_expired() const;
	bool is_expired(uint64_t p_now) const;

	static DefaultSession *restore(String p_token);

	DefaultSession(String p_token);
};

class DefaultClient : public Object {
	GDCLASS(DefaultClient, Object)

private:
	String server_key; // The key used to authenticate with the server without a session. Defaults to "defaultkey".
	String host;
	int port; // The port number of the server: 7349 - gRPC API, 7350 - HTTP API, 443  - gRPC & HTTP API if SSL is enabled
	/// Set connection strings to use the secure mode with the server. Defaults to false.
	/// The server must be configured to make use of this option. With HTTP, GRPC, and WebSockets the server must
	/// be configured with an SSL certificate or use a load balancer which performs SSL termination.
	/// For rUDP you must configure the server to expose it's IP address so it can be bundled within session tokens.
	/// See the server documentation for more information.
	bool ssl;
	int connect_timeout;
	int timeout;
	String lang;

	bool trace;

	long server_time;

	HTTPRequest *rest;
	WebSocketClient *ws;
	std::map<String, ObjectID> collation_ids;

	// network callbacks:

	void _rest_request_completed(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &p_headers, const PoolByteArray &p_data);

	void _ws_connection_closed();
	void _ws_connection_error();
	void _ws_connection_established();
	void _ws_data_received();
	void _ws_server_close_request();

protected:
	static void _bind_methods();

	void _process_packet(PoolByteArray data);

public:
	void set_lang(String p_lang) { lang = p_lang; }
	String get_lang() const { return lang; }

	void set_trace(bool p_trace) { trace = p_trace; }
	bool get_trace() const { return trace; }

	long get_server_time();

	bool authenticate(const Ref<DefaultAuthenticateRequest> &p_auth, String p_path);

	bool user_login(const Ref<DefaultAuthenticateRequest> &p_auth);
	bool user_register(const Ref<DefaultAuthenticateRequest> &p_auth);
	bool user_logout();

	bool connect_session(const Ref<DefaultSession> &p_session);
	bool disconnect_session();

	bool send(const Ref<NkCollatedMessage> &p_message);
	bool send(const Ref<NkUncollatedMessage> &p_message);

	DefaultClient(String p_server_key, String p_host, int p_port, bool p_ssl = false, int p_timeout = 60);
	~DefaultClient();
};

#endif // NAKAMACLIENT_H
