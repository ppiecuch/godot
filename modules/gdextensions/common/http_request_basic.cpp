/*************************************************************************/
/*  http_request.cpp                                                     */
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

#include "http_request_basic.h"

void HTTPRequestBasic::_redirect_request(const String &p_new_url) {
}

Error HTTPRequestBasic::_request() {
	return client->connect_to_host(url, port, use_ssl, validate_ssl);
}

Error HTTPRequestBasic::_parse_url(const String &p_url) {
	use_ssl = false;
	request_string = "";
	port = 80;
	request_sent = false;
	got_response = false;
	body_len = -1;
	body.resize(0);
	downloaded.set(0);
	redirections = 0;

	String scheme;
	Error err = p_url.parse_url(scheme, url, port, request_string);
	ERR_FAIL_COND_V_MSG(err != OK, err, "Error parsing URL: " + p_url + ".");
	if (scheme == "https://") {
		use_ssl = true;
	} else if (scheme != "http://") {
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Invalid URL scheme: " + scheme + ".");
	}
	if (port == 0) {
		port = use_ssl ? 443 : 80;
	}
	if (request_string.empty()) {
		request_string = "/";
	}
	return OK;
}

Error HTTPRequestBasic::request(const String &p_url, const Vector<String> &p_custom_headers, bool p_ssl_validate_domain, HTTPClient::Method p_method, const String &p_request_data) {
	// Copy the string into a raw buffer
	PoolVector<uint8_t> raw_data;

	CharString charstr = p_request_data.utf8();
	size_t len = charstr.length();
	raw_data.resize(len);
	memcpy(raw_data.write().ptr(), charstr.ptr(), len);

	return request_raw(p_url, p_custom_headers, p_ssl_validate_domain, p_method, raw_data);
}

Error HTTPRequestBasic::request_raw(const String &p_url, const Vector<String> &p_custom_headers, bool p_ssl_validate_domain, HTTPClient::Method p_method, const PoolVector<uint8_t> &p_request_data_raw) {
	ERR_FAIL_COND_V_MSG(requesting, ERR_BUSY, "HTTPRequestBasic is processing a request. Wait for completion or cancel it before attempting a new one.");

	if (timeout > 0.0) {
		timer->stop();
		timer->start(timeout);
	}

	method = p_method;

	Error err = _parse_url(p_url);
	if (err) {
		return err;
	}

	validate_ssl = p_ssl_validate_domain;

	headers = p_custom_headers;

	request_data = p_request_data_raw;

	requesting = true;

	client->set_blocking_mode(false);
	err = _request();
	if (err != OK) {
		call_deferred("_request_done", RESULT_CANT_CONNECT, 0, PoolStringArray(), PoolByteArray());
		return ERR_CANT_CONNECT;
	}

	return OK;
}

bool HTTPRequestBasic::_process() {
	Error err = _request();

	if (err != OK) {
		call_deferred("_request_done", RESULT_CANT_CONNECT, 0, PoolStringArray(), PoolByteArray());
	} else {
		return _update_connection();
	}
	return false;
}

void HTTPRequestBasic::cancel_request() {
	timer->stop();

	if (!requesting) {
		return;
	}

	if (file) {
		memdelete(file);
		file = nullptr;
	}
	client->close();
	body.resize(0);
	got_response = false;
	response_code = -1;
	request_sent = false;
	requesting = false;
}

bool HTTPRequestBasic::_handle_response(bool *ret_value) {
	if (!client->has_response()) {
		call_deferred("_request_done", RESULT_NO_RESPONSE, 0, PoolStringArray(), PoolByteArray());
		*ret_value = true;
		return true;
	}

	got_response = true;
	response_code = client->get_response_code();
	List<String> rheaders;
	client->get_response_headers(&rheaders);
	response_headers.resize(0);
	downloaded.set(0);
	for (List<String>::Element *E = rheaders.front(); E; E = E->next()) {
		response_headers.push_back(E->get());
	}

	if (response_code == 301 || response_code == 302) {
		// Handle redirect

		if (max_redirects >= 0 && redirections >= max_redirects) {
			call_deferred("_request_done", RESULT_REDIRECT_LIMIT_REACHED, response_code, response_headers, PoolByteArray());
			*ret_value = true;
			return true;
		}

		String new_request;

		for (List<String>::Element *E = rheaders.front(); E; E = E->next()) {
			if (E->get().findn("Location: ") != -1) {
				new_request = E->get().substr(9, E->get().length()).strip_edges();
			}
		}

		if (new_request != "") {
			// Process redirect
			client->close();
			int new_redirs = redirections + 1; // Because _request() will clear it
			Error err;
			if (new_request.begins_with("http")) {
				// New url, request all again
				_parse_url(new_request);
			} else {
				request_string = new_request;
			}

			err = _request();
			if (err == OK) {
				request_sent = false;
				got_response = false;
				body_len = -1;
				body.resize(0);
				downloaded.set(0);
				redirections = new_redirs;
				*ret_value = false;
				return true;
			}
		}
	}

	return false;
}

bool HTTPRequestBasic::_update_connection() {
	switch (client->get_status()) {
		case HTTPClient::STATUS_DISCONNECTED: {
			call_deferred("_request_done", RESULT_CANT_CONNECT, 0, PoolStringArray(), PoolByteArray());
			return true; // End it, since it's doing something
		} break;
		case HTTPClient::STATUS_RESOLVING: {
			client->poll();
			// Must wait
			return false;
		} break;
		case HTTPClient::STATUS_CANT_RESOLVE: {
			call_deferred("_request_done", RESULT_CANT_RESOLVE, 0, PoolStringArray(), PoolByteArray());
			return true;

		} break;
		case HTTPClient::STATUS_CONNECTING: {
			client->poll();
			// Must wait
			return false;
		} break; // Connecting to IP
		case HTTPClient::STATUS_CANT_CONNECT: {
			call_deferred("_request_done", RESULT_CANT_CONNECT, 0, PoolStringArray(), PoolByteArray());
			return true;

		} break;
		case HTTPClient::STATUS_CONNECTED: {
			if (request_sent) {
				if (!got_response) {
					// No body

					bool ret_value;

					if (_handle_response(&ret_value)) {
						return ret_value;
					}

					call_deferred("_request_done", RESULT_SUCCESS, response_code, response_headers, PoolByteArray());
					return true;
				}
				if (body_len < 0) {
					// Chunked transfer is done
					call_deferred("_request_done", RESULT_SUCCESS, response_code, response_headers, body);
					return true;
				}

				call_deferred("_request_done", RESULT_CHUNKED_BODY_SIZE_MISMATCH, response_code, response_headers, PoolByteArray());
				return true;
				// Request migh have been done
			} else {
				// Did not request yet, do request

				Error err = client->request_raw(method, request_string, headers, request_data);
				if (err != OK) {
					call_deferred("_request_done", RESULT_CONNECTION_ERROR, 0, PoolStringArray(), PoolByteArray());
					return true;
				}

				request_sent = true;
				return false;
			}
		} break; // Connected: break requests only accepted here
		case HTTPClient::STATUS_REQUESTING: {
			// Must wait, still requesting
			client->poll();
			return false;

		} break; // Request in progress
		case HTTPClient::STATUS_BODY: {
			if (!got_response) {
				bool ret_value;

				if (_handle_response(&ret_value)) {
					return ret_value;
				}

				if (!client->is_response_chunked() && client->get_response_body_length() == 0) {
					call_deferred("_request_done", RESULT_SUCCESS, response_code, response_headers, PoolByteArray());
					return true;
				}

				// No body len (-1) if chunked or no content-length header was provided.
				// Change your webserver configuration if you want body len.
				body_len = client->get_response_body_length();

				if (body_size_limit >= 0 && body_len > body_size_limit) {
					call_deferred("_request_done", RESULT_BODY_SIZE_LIMIT_EXCEEDED, response_code, response_headers, PoolByteArray());
					return true;
				}

				if (download_to_file != String()) {
					file = FileAccess::open(download_to_file, FileAccess::WRITE);
					if (!file) {
						call_deferred("_request_done", RESULT_DOWNLOAD_FILE_CANT_OPEN, response_code, response_headers, PoolByteArray());
						return true;
					}
				}
			}

			client->poll();
			if (client->get_status() != HTTPClient::STATUS_BODY) {
				return false;
			}

			PoolByteArray chunk = client->read_response_body_chunk();

			if (chunk.size()) {
				downloaded.add(chunk.size());
				if (file) {
					PoolByteArray::Read r = chunk.read();
					file->store_buffer(r.ptr(), chunk.size());
					if (file->get_error() != OK) {
						call_deferred("_request_done", RESULT_DOWNLOAD_FILE_WRITE_ERROR, response_code, response_headers, PoolByteArray());
						return true;
					}
				} else {
					body.append_array(chunk);
				}
			}

			if (body_size_limit >= 0 && downloaded.get() > body_size_limit) {
				call_deferred("_request_done", RESULT_BODY_SIZE_LIMIT_EXCEEDED, response_code, response_headers, PoolByteArray());
				return true;
			}

			if (body_len >= 0) {
				if (downloaded.get() == body_len) {
					call_deferred("_request_done", RESULT_SUCCESS, response_code, response_headers, body);
					return true;
				}
			} else if (client->get_status() == HTTPClient::STATUS_DISCONNECTED) {
				// We read till EOF, with no errors. Request is done.
				call_deferred("_request_done", RESULT_SUCCESS, response_code, response_headers, body);
				return true;
			}

			return false;

		} break; // Request resulted in body: break which must be read
		case HTTPClient::STATUS_CONNECTION_ERROR: {
			call_deferred("_request_done", RESULT_CONNECTION_ERROR, 0, PoolStringArray(), PoolByteArray());
			return true;
		} break;
		case HTTPClient::STATUS_SSL_HANDSHAKE_ERROR: {
			call_deferred("_request_done", RESULT_SSL_HANDSHAKE_ERROR, 0, PoolStringArray(), PoolByteArray());
			return true;
		} break;
	}

	ERR_FAIL_V(false);
}

void HTTPRequestBasic::_request_done(int p_status, int p_code, const PoolStringArray &p_headers, const PoolByteArray &p_data) {
	cancel_request();
	emit_signal("request_completed", p_status, p_code, p_headers, p_data);
}

void HTTPRequestBasic::set_body_size_limit(int p_bytes) {
	ERR_FAIL_COND(get_http_client_status() != HTTPClient::STATUS_DISCONNECTED);

	body_size_limit = p_bytes;
}

int HTTPRequestBasic::get_body_size_limit() const {
	return body_size_limit;
}

void HTTPRequestBasic::set_download_file(const String &p_file) {
	ERR_FAIL_COND(get_http_client_status() != HTTPClient::STATUS_DISCONNECTED);

	download_to_file = p_file;
}

String HTTPRequestBasic::get_download_file() const {
	return download_to_file;
}

void HTTPRequestBasic::set_download_chunk_size(int p_chunk_size) {
	ERR_FAIL_COND(get_http_client_status() != HTTPClient::STATUS_DISCONNECTED);

	client->set_read_chunk_size(p_chunk_size);
}

int HTTPRequestBasic::get_download_chunk_size() const {
	return client->get_read_chunk_size();
}

HTTPClient::Status HTTPRequestBasic::get_http_client_status() const {
	return client->get_status();
}

void HTTPRequestBasic::set_max_redirects(int p_max) {
	max_redirects = p_max;
}

int HTTPRequestBasic::get_max_redirects() const {
	return max_redirects;
}

int HTTPRequestBasic::get_downloaded_bytes() const {
	return downloaded.get();
}
int HTTPRequestBasic::get_body_size() const {
	return body_len;
}

void HTTPRequestBasic::set_http_proxy(const String &p_host, int p_port) {
	client->set_http_proxy(p_host, p_port);
}

void HTTPRequestBasic::set_https_proxy(const String &p_host, int p_port) {
	client->set_https_proxy(p_host, p_port);
}

void HTTPRequestBasic::set_timeout(double p_timeout) {
	if (Math::is_zero_approx(p_timeout)) {
		timeout = 0.0;
	} else {
		ERR_FAIL_COND(p_timeout < 0.0);
		timeout = p_timeout;
	}
}

double HTTPRequestBasic::get_timeout() {
	return timeout;
}

void HTTPRequestBasic::_timeout() {
	cancel_request();
	call_deferred("_request_done", RESULT_TIMEOUT, 0, PoolStringArray(), PoolByteArray());
}

void HTTPRequestBasic::_bind_methods() {
	ClassDB::bind_method(D_METHOD("request_raw", "url", "custom_headers", "ssl_validate_domain", "method", "request_data_raw"), &HTTPRequestBasic::request_raw, DEFVAL(PoolStringArray()), DEFVAL(true), DEFVAL(HTTPClient::METHOD_GET), DEFVAL(PoolVector<uint8_t>()));
	ClassDB::bind_method(D_METHOD("request", "url", "custom_headers", "ssl_validate_domain", "method", "request_data"), &HTTPRequestBasic::request, DEFVAL(PoolStringArray()), DEFVAL(true), DEFVAL(HTTPClient::METHOD_GET), DEFVAL(String()));
	ClassDB::bind_method(D_METHOD("cancel_request"), &HTTPRequestBasic::cancel_request);

	ClassDB::bind_method(D_METHOD("get_http_client_status"), &HTTPRequestBasic::get_http_client_status);

	ClassDB::bind_method(D_METHOD("set_body_size_limit", "bytes"), &HTTPRequestBasic::set_body_size_limit);
	ClassDB::bind_method(D_METHOD("get_body_size_limit"), &HTTPRequestBasic::get_body_size_limit);

	ClassDB::bind_method(D_METHOD("set_max_redirects", "amount"), &HTTPRequestBasic::set_max_redirects);
	ClassDB::bind_method(D_METHOD("get_max_redirects"), &HTTPRequestBasic::get_max_redirects);

	ClassDB::bind_method(D_METHOD("set_download_file", "path"), &HTTPRequestBasic::set_download_file);
	ClassDB::bind_method(D_METHOD("get_download_file"), &HTTPRequestBasic::get_download_file);

	ClassDB::bind_method(D_METHOD("get_downloaded_bytes"), &HTTPRequestBasic::get_downloaded_bytes);
	ClassDB::bind_method(D_METHOD("get_body_size"), &HTTPRequestBasic::get_body_size);

	ClassDB::bind_method(D_METHOD("_redirect_request"), &HTTPRequestBasic::_redirect_request);
	ClassDB::bind_method(D_METHOD("_request_done"), &HTTPRequestBasic::_request_done);

	ClassDB::bind_method(D_METHOD("set_timeout", "timeout"), &HTTPRequestBasic::set_timeout);
	ClassDB::bind_method(D_METHOD("get_timeout"), &HTTPRequestBasic::get_timeout);

	ClassDB::bind_method(D_METHOD("set_download_chunk_size", "chunk_size"), &HTTPRequestBasic::set_download_chunk_size);
	ClassDB::bind_method(D_METHOD("get_download_chunk_size"), &HTTPRequestBasic::get_download_chunk_size);

	ClassDB::bind_method(D_METHOD("set_http_proxy", "host", "port"), &HTTPRequestBasic::set_http_proxy);
	ClassDB::bind_method(D_METHOD("set_https_proxy", "host", "port"), &HTTPRequestBasic::set_https_proxy);

	ClassDB::bind_method(D_METHOD("_timeout"), &HTTPRequestBasic::_timeout);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "download_file", PROPERTY_HINT_FILE), "set_download_file", "get_download_file");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "download_chunk_size", PROPERTY_HINT_RANGE, "256,16777216"), "set_download_chunk_size", "get_download_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "body_size_limit", PROPERTY_HINT_RANGE, "-1,2000000000"), "set_body_size_limit", "get_body_size_limit");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_redirects", PROPERTY_HINT_RANGE, "-1,64"), "set_max_redirects", "get_max_redirects");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "timeout", PROPERTY_HINT_RANGE, "0,3600,0.1,or_greater"), "set_timeout", "get_timeout");

	ADD_SIGNAL(MethodInfo("request_completed", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "response_code"), PropertyInfo(Variant::POOL_STRING_ARRAY, "headers"), PropertyInfo(Variant::POOL_BYTE_ARRAY, "body")));

	BIND_ENUM_CONSTANT(RESULT_SUCCESS);
	//BIND_ENUM_CONSTANT( RESULT_NO_BODY );
	BIND_ENUM_CONSTANT(RESULT_CHUNKED_BODY_SIZE_MISMATCH);
	BIND_ENUM_CONSTANT(RESULT_CANT_CONNECT);
	BIND_ENUM_CONSTANT(RESULT_CANT_RESOLVE);
	BIND_ENUM_CONSTANT(RESULT_CONNECTION_ERROR);
	BIND_ENUM_CONSTANT(RESULT_SSL_HANDSHAKE_ERROR);
	BIND_ENUM_CONSTANT(RESULT_NO_RESPONSE);
	BIND_ENUM_CONSTANT(RESULT_BODY_SIZE_LIMIT_EXCEEDED);
	BIND_ENUM_CONSTANT(RESULT_REQUEST_FAILED);
	BIND_ENUM_CONSTANT(RESULT_DOWNLOAD_FILE_CANT_OPEN);
	BIND_ENUM_CONSTANT(RESULT_DOWNLOAD_FILE_WRITE_ERROR);
	BIND_ENUM_CONSTANT(RESULT_REDIRECT_LIMIT_REACHED);
	BIND_ENUM_CONSTANT(RESULT_TIMEOUT);
}

HTTPRequestBasic::HTTPRequestBasic() {
	port = 80;
	redirections = 0;
	max_redirects = 8;
	body_len = -1;
	got_response = false;
	validate_ssl = false;
	use_ssl = false;
	response_code = 0;
	request_sent = false;
	requesting = false;
	client.instance();
	body_size_limit = -1;
	file = nullptr;

	timer = memnew(Timer);
	timer->set_one_shot(true);
	timer->connect("timeout", this, "_timeout");
	timeout = 0.0;
}

HTTPRequestBasic::~HTTPRequestBasic() {
	if (requesting) {
		cancel_request();
	}
	if (file) {
		memdelete(file);
	}
	if (timer) {
		memdelete(timer);
	}
}
