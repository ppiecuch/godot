/**************************************************************************/
/*  basic_http_request.h                                                  */
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

#ifndef BASIC_HTTP_REQUEST_H
#define BASIC_HTTP_REQUEST_H

#include "core/io/http_client.h"
#include "core/io/json.h"
#include "core/os/file_access.h"
#include "core/os/os.h"
#include "core/safe_refcount.h"

// BasicHTTPRequest — tree-independent HTTP client for use in Reference-based objects.
// Drive it each frame by calling poll(); it returns true when done.
//
// Example 1 — simple GET with default headers (e.g. auth token):
//
//   Ref<BasicHTTPRequest> http = memnew(BasicHTTPRequest);
//   http->set_timeout(10.0);
//   http->add_default_header("Authorization", "Bearer " + token);
//   http->connect("request_completed", this, "_on_response");
//   http->request("https://api.example.com/users");
//   // In _process():  http->poll();
//
//   void _on_response(int result, int code, PoolStringArray headers, PoolByteArray body) {
//       if (result == BasicHTTPRequest::RESULT_SUCCESS && code == 200) {
//           // body.get_string_from_utf8() to read raw text
//       }
//   }
//
// Example 2 — POST JSON and parse the JSON response synchronously:
//
//   Ref<BasicHTTPRequest> http = memnew(BasicHTTPRequest);
//   http->connect("request_completed", this, "_on_login_response");
//   Dictionary payload;
//   payload["email"] = "user@example.com";
//   payload["password"] = "s3cr3t";
//   http->post_json("https://api.example.com/auth/login", payload);
//   // In _process():  if (http->poll()) { /* done */ }
//
//   void _on_login_response(int result, int code, PoolStringArray headers, PoolByteArray body) {
//       if (result == BasicHTTPRequest::RESULT_SUCCESS) {
//           Variant json = http->get_last_response_json(); // parsed Dictionary
//           String token = json.operator Dictionary()["access_token"];
//       }
//   }

class BasicHTTPRequest : public Reference {
	GDCLASS(BasicHTTPRequest, Reference);

public:
	enum Result {
		RESULT_SUCCESS,
		RESULT_CHUNKED_BODY_SIZE_MISMATCH,
		RESULT_CANT_CONNECT,
		RESULT_CANT_RESOLVE,
		RESULT_CONNECTION_ERROR,
		RESULT_SSL_HANDSHAKE_ERROR,
		RESULT_NO_RESPONSE,
		RESULT_BODY_SIZE_LIMIT_EXCEEDED,
		RESULT_REQUEST_FAILED,
		RESULT_DOWNLOAD_FILE_CANT_OPEN,
		RESULT_DOWNLOAD_FILE_WRITE_ERROR,
		RESULT_REDIRECT_LIMIT_REACHED,
		RESULT_TIMEOUT,
		RESULT_MAX,
	};

private:
	// ── request state ──────────────────────────────────────────────────────────
	bool requesting = false;
	bool request_sent = false;
	bool got_response = false;
	bool validate_ssl = false;
	bool use_ssl = false;

	String request_string;
	String url;
	int port = 80;
	Vector<String> headers;
	HTTPClient::Method method = HTTPClient::METHOD_GET;
	PoolVector<uint8_t> request_data;

	Ref<HTTPClient> client;
	PoolByteArray body;

	int response_code = 0;
	PoolVector<String> response_headers;

	int body_len = -1;
	SafeNumeric<int> downloaded;
	int body_size_limit = -1;
	int redirections = 0;
	int max_redirects = 8;

	// ── download ───────────────────────────────────────────────────────────────
	String download_to_file;
	FileAccess *file = nullptr;

	// ── timeout ────────────────────────────────────────────────────────────────
	double timeout = 0.0;
	uint64_t request_start_ms = 0;

	// ── default headers (merged into every request before custom headers) ──────
	Vector<String> default_headers;

	// ── last response (valid after request_completed signal fires) ─────────────
	Result last_result = RESULT_SUCCESS;
	int last_response_code = 0;
	PoolVector<String> last_response_headers;
	PoolByteArray last_response_body;

	bool _update_connection();
	void _redirect_request(const String &p_new_url);
	bool _handle_response(bool *ret_value);
	Error _parse_url(const String &p_url);
	Error _request();
	void _request_done(int p_status, int p_code, const PoolStringArray &p_headers, const PoolByteArray &p_data);
	void _timeout();

protected:
	static void _bind_methods();

public:
	bool poll(); // returns true when the request is done

	Error request(const String &p_url,
			const Vector<String> &p_custom_headers = Vector<String>(),
			bool p_ssl_validate_domain = true,
			HTTPClient::Method p_method = HTTPClient::METHOD_GET,
			const String &p_request_data = "");

	Error request_raw(const String &p_url,
			const Vector<String> &p_custom_headers = Vector<String>(),
			bool p_ssl_validate_domain = true,
			HTTPClient::Method p_method = HTTPClient::METHOD_GET,
			const PoolVector<uint8_t> &p_request_data_raw = PoolVector<uint8_t>());

	void cancel_request();
	_FORCE_INLINE_ bool is_requesting() const { return requesting; }
	_FORCE_INLINE_ bool is_active_request() const { return requesting; } // compat alias
	HTTPClient::Status get_http_client_status() const;

	// ── convenience wrappers ───────────────────────────────────────────────────
	_FORCE_INLINE_ Error get_url(const String &p_url,
			const Vector<String> &p_headers = Vector<String>(),
			bool p_ssl_validate = true) {
		return request(p_url, p_headers, p_ssl_validate, HTTPClient::METHOD_GET);
	}
	_FORCE_INLINE_ Error post(const String &p_url,
			const String &p_body,
			const Vector<String> &p_headers = Vector<String>(),
			bool p_ssl_validate = true) {
		return request(p_url, p_headers, p_ssl_validate, HTTPClient::METHOD_POST, p_body);
	}
	_FORCE_INLINE_ Error post_raw(const String &p_url,
			const PoolVector<uint8_t> &p_body,
			const Vector<String> &p_headers = Vector<String>(),
			bool p_ssl_validate = true) {
		return request_raw(p_url, p_headers, p_ssl_validate, HTTPClient::METHOD_POST, p_body);
	}
	// Serializes p_data as JSON, sets Content-Type/Accept: application/json.
	Error post_json(const String &p_url,
			const Variant &p_data,
			const Vector<String> &p_extra_headers = Vector<String>(),
			bool p_ssl_validate = true);

	// ── default headers ────────────────────────────────────────────────────────
	void set_default_headers(const Vector<String> &p_headers);
	Vector<String> get_default_headers() const;
	void add_default_header(const String &p_name, const String &p_value);

	// ── last response accessors ────────────────────────────────────────────────
	_FORCE_INLINE_ Result get_last_result() const { return last_result; }
	_FORCE_INLINE_ int get_last_response_code() const { return last_response_code; }
	_FORCE_INLINE_ PoolVector<String> get_last_response_headers() const { return last_response_headers; }
	_FORCE_INLINE_ PoolByteArray get_last_response_body() const { return last_response_body; }
	// Parses the last response body as JSON. Returns null Variant on failure.
	Variant get_last_response_json() const;

	// ── download ───────────────────────────────────────────────────────────────
	void set_download_file(const String &p_file);
	String get_download_file() const;
	void set_download_chunk_size(int p_chunk_size);
	int get_download_chunk_size() const;

	// ── limits ─────────────────────────────────────────────────────────────────
	void set_body_size_limit(int p_bytes);
	int get_body_size_limit() const;
	void set_max_redirects(int p_max);
	int get_max_redirects() const;

	// ── timeout ────────────────────────────────────────────────────────────────
	void set_timeout(double p_timeout);
	double get_timeout() const;

	// ── progress ───────────────────────────────────────────────────────────────
	int get_downloaded_bytes() const;
	int get_body_size() const;

	// ── proxy ──────────────────────────────────────────────────────────────────
	void set_http_proxy(const String &p_host, int p_port);
	void set_https_proxy(const String &p_host, int p_port);

	BasicHTTPRequest();
	~BasicHTTPRequest();
};

VARIANT_ENUM_CAST(BasicHTTPRequest::Result);

#endif // BASIC_HTTP_REQUEST_H
