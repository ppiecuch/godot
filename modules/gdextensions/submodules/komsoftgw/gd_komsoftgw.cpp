/**************************************************************************/
/*  gd_komsoftgw.cpp                                                      */
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

#include "gd_komsoftgw.h"

#include "common/gd_core.h"
#include "core/io/json.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

// ── local JSON helpers (mirroring the shared silentwolf helpers) ────────────

static String kgw_string_from_utf8(const PoolByteArray &p_data) {
	String s;
	if (p_data.size() > 0) {
		PoolByteArray::Read r = p_data.read();
		s.parse_utf8((const char *)r.ptr(), p_data.size());
	}
	return s;
}

static Dictionary kgw_parse_json(const String &p_json) {
	Variant data;
	String error_string;
	int error_line = 0;
	if (JSON::parse(p_json, data, error_string, error_line) != OK) {
		return Dictionary();
	}
	if (data.get_type() == Variant::DICTIONARY) {
		return data;
	}
	return Dictionary();
}

// ── pure request-shaping helpers (file-local and side-effect-free so the doctest
//    section at the bottom can exercise them without any networking) ──────────

// Join a base URL and an absolute path, tolerating a trailing slash on the base.
static String kgw_join_url(const String &p_base, const String &p_path) {
	String base = p_base;
	if (base.ends_with("/")) {
		base = base.substr(0, base.length() - 1);
	}
	return base + p_path;
}

// Build the header set every request needs. The gateway rejects any /v1 request
// missing User-Agent + X-Client-Version; the api key authenticates the user.
Vector<String> KomsoftGw::_make_headers(bool p_json) const {
	Vector<String> h;
	h.push_back("User-Agent: " + service + ";Godot (" + client_version + ")");
	h.push_back("X-Client-Version: " + client_version);
	h.push_back("X-Device-Id: " + device_id);
	if (!api_key.empty()) {
		h.push_back("x-api-key: " + api_key);
	}
	if (p_json) {
		h.push_back("Content-Type: application/json");
	}
	return h;
}

// A response is usable only on a transport success with a 2xx status.
static bool kgw_is_ok(int p_result, int p_code) {
	return p_result == BasicHTTPRequest::RESULT_SUCCESS && p_code >= 200 && p_code < 300;
}

// ===========================================================================
// KomsoftGw — singleton holding config, the action queue, and all API calls
// ===========================================================================

KomsoftGw *KomsoftGw::singleton = nullptr;
KomsoftGw *KomsoftGw::get_singleton() { return singleton; }

KomsoftGw::KomsoftGw() {
	if (singleton != nullptr) {
		WARN_PRINT("KomsoftGw singleton is already initialized; overwriting the previous instance.");
	}
	singleton = this;
}
KomsoftGw::~KomsoftGw() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

// ── configuration ─────────────────────────────────────────────────────────

void KomsoftGw::configure(const String &p_base_url, const String &p_service, const String &p_api_key, const String &p_user_id) {
	base_url = p_base_url;
	if (base_url.ends_with("/")) {
		base_url = base_url.substr(0, base_url.length() - 1);
	}
	service = p_service;
	api_key = p_api_key;
	user_id = p_user_id;
}

void KomsoftGw::reset() {
	base_url = String();
	service = String();
	api_key = String();
	user_id = String();
	use_ssl = true;
	timeout_seconds = 15.0;
	queue.clear();
	active.clear();
}

void KomsoftGw::set_use_ssl(bool p_v) { use_ssl = p_v; }
bool KomsoftGw::get_use_ssl() const { return use_ssl; }
void KomsoftGw::set_timeout(double p_seconds) { timeout_seconds = p_seconds; }
double KomsoftGw::get_timeout() const { return timeout_seconds; }
void KomsoftGw::set_client_version(const String &p_v) { client_version = p_v; }
String KomsoftGw::get_client_version() const { return client_version; }
void KomsoftGw::set_device_id(const String &p_v) { device_id = p_v; }
String KomsoftGw::get_device_id() const { return device_id; }

// ── queue + pump ────────────────────────────────────────────────────────────

void KomsoftGw::_enqueue(const String &p_callback, int p_method, const String &p_url, bool p_json, const String &p_body) {
	PendingAction a;
	a.callback = p_callback;
	a.method = p_method;
	a.url = p_url;
	a.headers = _make_headers(p_json);
	a.body = p_body;
	queue.push_back(a);
}

void KomsoftGw::pump() {
	// 1. Issue everything queued since the last pump.
	if (queue.size() > 0) {
		Vector<PendingAction> pending = queue;
		queue.clear();
		for (int i = 0; i < pending.size(); i++) {
			const PendingAction &a = pending[i];
			Ref<BasicHTTPRequest> req = newref(BasicHTTPRequest);
			req->set_timeout(timeout_seconds);
			req->connect("request_completed", this, a.callback);
			req->request(a.url, a.headers, use_ssl, (HTTPClient::Method)a.method, a.body);
			active.push_back(req);
		}
	}
	// 2. Advance in-flight requests; the completion signal fires inside poll().
	for (int i = active.size() - 1; i >= 0; i--) {
		Ref<BasicHTTPRequest> req = active[i];
		if (req.is_null() || req->poll()) {
			active.remove(i);
		}
	}
}

int KomsoftGw::pending_count() const {
	return queue.size() + active.size();
}

void KomsoftGw::_fail(const String &p_endpoint, int p_result, int p_code, const PoolByteArray &p_body) {
	String msg;
	if (p_body.size() > 0) {
		Dictionary d = kgw_parse_json(kgw_string_from_utf8(p_body));
		if (d.has("error")) {
			msg = d["error"];
		}
	}
	if (msg.empty()) {
		msg = "request failed (result " + itos(p_result) + ", http " + itos(p_code) + ")";
	}
	emit_signal("request_failed", p_endpoint, p_code, msg);
}

// ── API: list / token / submit ──────────────────────────────────────────────

void KomsoftGw::list_leaderboards() {
	_enqueue("_on_list_completed", HTTPClient::METHOD_GET,
			kgw_join_url(base_url, "/v1/leaderboards?service=" + service.http_escape() + "&userId=" + user_id.http_escape()),
			false, "");
}

void KomsoftGw::request_submit_token(const String &p_board_key) {
	_enqueue("_on_token_completed", HTTPClient::METHOD_GET,
			kgw_join_url(base_url, "/v1/leaderboards/" + p_board_key.http_escape() + "/token?service=" + service.http_escape() + "&userId=" + user_id.http_escape()),
			false, "");
}

void KomsoftGw::submit_score(const String &p_board_key, int64_t p_score, const Dictionary &p_metadata, const Dictionary &p_context, const String &p_token) {
	Dictionary payload;
	payload["service"] = service;
	payload["userId"] = user_id;
	payload["score"] = p_score;
	if (p_metadata.size() > 0) {
		payload["metadata"] = p_metadata;
	}
	if (p_context.size() > 0) {
		payload["context"] = p_context;
	}
	if (!p_token.empty()) {
		payload["token"] = p_token;
	}
	_enqueue("_on_submit_completed", HTTPClient::METHOD_POST,
			kgw_join_url(base_url, "/v1/leaderboards/" + p_board_key.http_escape() + "/submit"),
			true, JSON::print(payload));
}

// ── API: reads ───────────────────────────────────────────────────────────────

void KomsoftGw::get_rank(const String &p_board_key, int p_around) {
	_enqueue("_on_rank_completed", HTTPClient::METHOD_GET,
			kgw_join_url(base_url, "/v1/leaderboards/" + p_board_key.http_escape() + "/rank?service=" + service.http_escape() + "&userId=" + user_id.http_escape() + "&around=" + itos(p_around)),
			false, "");
}

void KomsoftGw::get_top(const String &p_board_key, const String &p_period_key, int p_limit, const String &p_cursor) {
	String url = kgw_join_url(base_url, "/v1/leaderboards/" + p_board_key.http_escape() + "/top?service=" + service.http_escape() + "&userId=" + user_id.http_escape() + "&limit=" + itos(p_limit));
	if (!p_period_key.empty()) {
		url += "&periodKey=" + p_period_key.http_escape();
	}
	if (!p_cursor.empty()) {
		url += "&cursor=" + p_cursor.http_escape();
	}
	_enqueue("_on_top_completed", HTTPClient::METHOD_GET, url, false, "");
}

void KomsoftGw::get_distribution(const String &p_board_key) {
	_enqueue("_on_distribution_completed", HTTPClient::METHOD_GET,
			kgw_join_url(base_url, "/v1/leaderboards/" + p_board_key.http_escape() + "/distribution?service=" + service.http_escape() + "&userId=" + user_id.http_escape()),
			false, "");
}

void KomsoftGw::get_friends(const String &p_board_key, const Array &p_friend_ids) {
	Dictionary payload;
	payload["service"] = service;
	payload["userId"] = user_id;
	payload["friendIds"] = p_friend_ids;
	_enqueue("_on_friends_completed", HTTPClient::METHOD_POST,
			kgw_join_url(base_url, "/v1/leaderboards/" + p_board_key.http_escape() + "/friends"),
			true, JSON::print(payload));
}

// ── completion callbacks ─────────────────────────────────────────────────────

void KomsoftGw::_on_list_completed(int p_result, int p_code, PoolStringArray, PoolByteArray p_body) {
	if (!kgw_is_ok(p_result, p_code)) {
		_fail("list", p_result, p_code, p_body);
		return;
	}
	Dictionary d = kgw_parse_json(kgw_string_from_utf8(p_body));
	emit_signal("leaderboards_received", d.get("leaderboards", Array()));
}

void KomsoftGw::_on_token_completed(int p_result, int p_code, PoolStringArray, PoolByteArray p_body) {
	if (!kgw_is_ok(p_result, p_code)) {
		_fail("token", p_result, p_code, p_body);
		return;
	}
	Dictionary d = kgw_parse_json(kgw_string_from_utf8(p_body));
	emit_signal("submit_token_received", d.get("token", ""), d.get("expiresIn", 0));
}

void KomsoftGw::_on_submit_completed(int p_result, int p_code, PoolStringArray, PoolByteArray p_body) {
	if (!kgw_is_ok(p_result, p_code)) {
		_fail("submit", p_result, p_code, p_body);
		return;
	}
	emit_signal("score_submitted", kgw_parse_json(kgw_string_from_utf8(p_body)));
}

void KomsoftGw::_on_rank_completed(int p_result, int p_code, PoolStringArray, PoolByteArray p_body) {
	if (!kgw_is_ok(p_result, p_code)) {
		_fail("rank", p_result, p_code, p_body);
		return;
	}
	emit_signal("rank_received", kgw_parse_json(kgw_string_from_utf8(p_body)));
}

void KomsoftGw::_on_top_completed(int p_result, int p_code, PoolStringArray, PoolByteArray p_body) {
	if (!kgw_is_ok(p_result, p_code)) {
		_fail("top", p_result, p_code, p_body);
		return;
	}
	Dictionary d = kgw_parse_json(kgw_string_from_utf8(p_body));
	emit_signal("top_received", d.get("entries", Array()), d.get("nextCursor", ""));
}

void KomsoftGw::_on_distribution_completed(int p_result, int p_code, PoolStringArray, PoolByteArray p_body) {
	if (!kgw_is_ok(p_result, p_code)) {
		_fail("distribution", p_result, p_code, p_body);
		return;
	}
	emit_signal("distribution_received", kgw_parse_json(kgw_string_from_utf8(p_body)));
}

void KomsoftGw::_on_friends_completed(int p_result, int p_code, PoolStringArray, PoolByteArray p_body) {
	if (!kgw_is_ok(p_result, p_code)) {
		_fail("friends", p_result, p_code, p_body);
		return;
	}
	Dictionary d = kgw_parse_json(kgw_string_from_utf8(p_body));
	emit_signal("friends_received", d.get("entries", Array()));
}

// ── bindings ─────────────────────────────────────────────────────────────────

void KomsoftGw::_bind_methods() {
	ClassDB::bind_method(D_METHOD("configure", "base_url", "service", "api_key", "user_id"), &KomsoftGw::configure);
	ClassDB::bind_method(D_METHOD("reset"), &KomsoftGw::reset);
	ClassDB::bind_method(D_METHOD("set_use_ssl", "enabled"), &KomsoftGw::set_use_ssl);
	ClassDB::bind_method(D_METHOD("get_use_ssl"), &KomsoftGw::get_use_ssl);
	ClassDB::bind_method(D_METHOD("set_timeout", "seconds"), &KomsoftGw::set_timeout);
	ClassDB::bind_method(D_METHOD("get_timeout"), &KomsoftGw::get_timeout);
	ClassDB::bind_method(D_METHOD("set_client_version", "version"), &KomsoftGw::set_client_version);
	ClassDB::bind_method(D_METHOD("get_client_version"), &KomsoftGw::get_client_version);
	ClassDB::bind_method(D_METHOD("set_device_id", "id"), &KomsoftGw::set_device_id);
	ClassDB::bind_method(D_METHOD("get_device_id"), &KomsoftGw::get_device_id);

	ClassDB::bind_method(D_METHOD("list_leaderboards"), &KomsoftGw::list_leaderboards);
	ClassDB::bind_method(D_METHOD("request_submit_token", "board_key"), &KomsoftGw::request_submit_token);
	ClassDB::bind_method(D_METHOD("submit_score", "board_key", "score", "metadata", "context", "token"),
			&KomsoftGw::submit_score, DEFVAL(Dictionary()), DEFVAL(Dictionary()), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_rank", "board_key", "around"), &KomsoftGw::get_rank, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_top", "board_key", "period_key", "limit", "cursor"),
			&KomsoftGw::get_top, DEFVAL(""), DEFVAL(50), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_distribution", "board_key"), &KomsoftGw::get_distribution);
	ClassDB::bind_method(D_METHOD("get_friends", "board_key", "friend_ids"),
			&KomsoftGw::get_friends, DEFVAL(Array()));

	ClassDB::bind_method(D_METHOD("pump"), &KomsoftGw::pump);
	ClassDB::bind_method(D_METHOD("pending_count"), &KomsoftGw::pending_count);

	// Internal completion callbacks (bound so connect() can target them).
	ClassDB::bind_method(D_METHOD("_on_list_completed"), &KomsoftGw::_on_list_completed);
	ClassDB::bind_method(D_METHOD("_on_token_completed"), &KomsoftGw::_on_token_completed);
	ClassDB::bind_method(D_METHOD("_on_submit_completed"), &KomsoftGw::_on_submit_completed);
	ClassDB::bind_method(D_METHOD("_on_rank_completed"), &KomsoftGw::_on_rank_completed);
	ClassDB::bind_method(D_METHOD("_on_top_completed"), &KomsoftGw::_on_top_completed);
	ClassDB::bind_method(D_METHOD("_on_distribution_completed"), &KomsoftGw::_on_distribution_completed);
	ClassDB::bind_method(D_METHOD("_on_friends_completed"), &KomsoftGw::_on_friends_completed);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_ssl"), "set_use_ssl", "get_use_ssl");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "timeout"), "set_timeout", "get_timeout");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "client_version"), "set_client_version", "get_client_version");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "device_id"), "set_device_id", "get_device_id");

	ADD_SIGNAL(MethodInfo("leaderboards_received", PropertyInfo(Variant::ARRAY, "leaderboards")));
	ADD_SIGNAL(MethodInfo("submit_token_received",
			PropertyInfo(Variant::STRING, "token"), PropertyInfo(Variant::INT, "expires_in")));
	ADD_SIGNAL(MethodInfo("score_submitted", PropertyInfo(Variant::DICTIONARY, "result")));
	ADD_SIGNAL(MethodInfo("rank_received", PropertyInfo(Variant::DICTIONARY, "result")));
	ADD_SIGNAL(MethodInfo("top_received",
			PropertyInfo(Variant::ARRAY, "entries"), PropertyInfo(Variant::STRING, "next_cursor")));
	ADD_SIGNAL(MethodInfo("distribution_received", PropertyInfo(Variant::DICTIONARY, "result")));
	ADD_SIGNAL(MethodInfo("friends_received", PropertyInfo(Variant::ARRAY, "entries")));
	ADD_SIGNAL(MethodInfo("request_failed",
			PropertyInfo(Variant::STRING, "endpoint"), PropertyInfo(Variant::INT, "code"),
			PropertyInfo(Variant::STRING, "message")));

	BIND_ENUM_CONSTANT(SORT_ASC);
	BIND_ENUM_CONSTANT(SORT_DESC);
	BIND_ENUM_CONSTANT(OP_BEST);
	BIND_ENUM_CONSTANT(OP_SET);
	BIND_ENUM_CONSTANT(OP_INCR);
	BIND_ENUM_CONSTANT(OP_DECR);
	BIND_ENUM_CONSTANT(PERIOD_ALL_TIME);
	BIND_ENUM_CONSTANT(PERIOD_DAILY);
	BIND_ENUM_CONSTANT(PERIOD_WEEKLY);
	BIND_ENUM_CONSTANT(PERIOD_MONTHLY);
}

// ===========================================================================
// KomsoftGwNode — pumps the singleton every frame
// ===========================================================================

KomsoftGwNode::KomsoftGwNode() {}

void KomsoftGwNode::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_process(true);
		} break;

		case NOTIFICATION_PROCESS: {
			if (KomsoftGw::get_singleton()) {
				KomsoftGw::get_singleton()->pump();
			}
		} break;
	}
}

void KomsoftGwNode::_bind_methods() {}

// ===========================================================================
// doctest unit tests (compiled only in DOCTEST builds; see SConstruct)
// ===========================================================================

#ifdef DOCTEST
#include "doctest/doctest_godot.h"

// ---------------------------------------------------------------------------
// URL joining
// ---------------------------------------------------------------------------

TEST_CASE("[komsoftgw] kgw_join_url concatenates base and path") {
	CHECK(kgw_join_url("https://api.example.com", "/v1/leaderboards") == "https://api.example.com/v1/leaderboards");
}

TEST_CASE("[komsoftgw] kgw_join_url trims a single trailing slash on the base") {
	CHECK(kgw_join_url("https://api.example.com/", "/v1/x") == "https://api.example.com/v1/x");
}

TEST_CASE("[komsoftgw] kgw_join_url leaves a slash-free base untouched") {
	CHECK(kgw_join_url("http://localhost:8080", "/health") == "http://localhost:8080/health");
}

// ---------------------------------------------------------------------------
// Header construction (gateway requires UA + X-Client-Version on every /v1 call)
// ---------------------------------------------------------------------------

static bool kgw_has_header(const Vector<String> &h, const String &prefix) {
	for (int i = 0; i < h.size(); i++) {
		if (h[i].begins_with(prefix)) {
			return true;
		}
	}
	return false;
}

TEST_CASE("[komsoftgw] _make_headers always include the required client headers") {
	KomsoftGw *gw = memnew(KomsoftGw);
	gw->configure("http://x", "testsvc", "secret-key", "uid");
	Vector<String> h = gw->_make_headers(false);
	CHECK(kgw_has_header(h, "User-Agent:"));
	CHECK(kgw_has_header(h, "X-Client-Version:"));
	CHECK(kgw_has_header(h, "X-Device-Id:"));
	CHECK(kgw_has_header(h, "User-Agent: testsvc;Godot ("));
	memdelete(gw);
}

TEST_CASE("[komsoftgw] _make_headers include the api key when set") {
	KomsoftGw *gw = memnew(KomsoftGw);
	gw->configure("http://x", "svc", "secret-key", "uid");
	Vector<String> h = gw->_make_headers(false);
	bool found = false;
	for (int i = 0; i < h.size(); i++) {
		if (h[i] == "x-api-key: secret-key") {
			found = true;
		}
	}
	CHECK(found);
	memdelete(gw);
}

TEST_CASE("[komsoftgw] _make_headers omit the api key when empty") {
	KomsoftGw *gw = memnew(KomsoftGw);
	gw->configure("http://x", "svc", "", "uid");
	Vector<String> h = gw->_make_headers(false);
	CHECK_FALSE(kgw_has_header(h, "x-api-key:"));
	memdelete(gw);
}

TEST_CASE("[komsoftgw] _make_headers add Content-Type only for JSON bodies") {
	KomsoftGw *gw = memnew(KomsoftGw);
	gw->configure("http://x", "svc", "k", "uid");
	CHECK_FALSE(kgw_has_header(gw->_make_headers(false), "Content-Type:"));
	CHECK(kgw_has_header(gw->_make_headers(true), "Content-Type: application/json"));
	memdelete(gw);
}

// ---------------------------------------------------------------------------
// Success predicate
// ---------------------------------------------------------------------------

TEST_CASE("[komsoftgw] kgw_is_ok accepts only 2xx on a transport success") {
	CHECK(kgw_is_ok(BasicHTTPRequest::RESULT_SUCCESS, 200));
	CHECK(kgw_is_ok(BasicHTTPRequest::RESULT_SUCCESS, 201));
	CHECK(kgw_is_ok(BasicHTTPRequest::RESULT_SUCCESS, 299));
}

TEST_CASE("[komsoftgw] kgw_is_ok rejects non-2xx status codes") {
	CHECK_FALSE(kgw_is_ok(BasicHTTPRequest::RESULT_SUCCESS, 401));
	CHECK_FALSE(kgw_is_ok(BasicHTTPRequest::RESULT_SUCCESS, 403));
	CHECK_FALSE(kgw_is_ok(BasicHTTPRequest::RESULT_SUCCESS, 503));
}

TEST_CASE("[komsoftgw] kgw_is_ok rejects transport failures regardless of code") {
	CHECK_FALSE(kgw_is_ok(BasicHTTPRequest::RESULT_TIMEOUT, 200));
	CHECK_FALSE(kgw_is_ok(BasicHTTPRequest::RESULT_CANT_CONNECT, 200));
}

// ---------------------------------------------------------------------------
// UTF-8 / JSON helpers
// ---------------------------------------------------------------------------

TEST_CASE("[komsoftgw] kgw_string_from_utf8 decodes ASCII bytes") {
	PoolByteArray b;
	b.push_back('H');
	b.push_back('i');
	CHECK(kgw_string_from_utf8(b) == "Hi");
}

TEST_CASE("[komsoftgw] kgw_string_from_utf8 handles an empty body") {
	PoolByteArray b;
	CHECK(kgw_string_from_utf8(b) == "");
}

TEST_CASE("[komsoftgw] kgw_parse_json parses a JSON object into a Dictionary") {
	Dictionary d = kgw_parse_json("{\"rank\":3,\"total\":42,\"periodKey\":\"2026-W32\"}");
	REQUIRE(d.has("rank"));
	CHECK((int)d["rank"] == 3);
	CHECK((int)d["total"] == 42);
	CHECK(String(d["periodKey"]) == "2026-W32");
}

TEST_CASE("[komsoftgw] kgw_parse_json returns an empty dict for a non-object top level") {
	// The gateway always returns objects; arrays/scalars are treated as no data.
	CHECK(kgw_parse_json("[1,2,3]").empty());
	CHECK(kgw_parse_json("\"just a string\"").empty());
}

TEST_CASE("[komsoftgw] kgw_parse_json returns an empty dict on malformed JSON") {
	CHECK(kgw_parse_json("{not valid").empty());
	CHECK(kgw_parse_json("").empty());
}

// ---------------------------------------------------------------------------
// Singleton: config + action queue (no networking — pump() is not called).
// Tests reuse the singleton and reset() it for isolation rather than churning
// instances (which would clobber the engine-owned singleton).
// ---------------------------------------------------------------------------

// Return the singleton, creating a throwaway one only if the engine hasn't yet.
static KomsoftGw *kgw_test_singleton() {
	if (!KomsoftGw::get_singleton()) {
		memnew(KomsoftGw); // owned by the singleton; leaked intentionally for the test run
	}
	KomsoftGw *gw = KomsoftGw::get_singleton();
	gw->reset();
	return gw;
}

TEST_CASE("[komsoftgw] reset restores sane defaults") {
	KomsoftGw *gw = kgw_test_singleton();
	gw->set_use_ssl(false);
	gw->set_timeout(1.0);
	gw->configure("http://x", "svc", "key", "uid");
	gw->reset();
	CHECK(gw->get_use_ssl() == true);
	CHECK(gw->get_timeout() == 15.0);
	CHECK(gw->pending_count() == 0);
}

TEST_CASE("[komsoftgw] setters round-trip through getters") {
	KomsoftGw *gw = kgw_test_singleton();
	gw->set_use_ssl(false);
	gw->set_timeout(5.0);
	CHECK_FALSE(gw->get_use_ssl());
	CHECK(gw->get_timeout() == 5.0);
}

TEST_CASE("[komsoftgw] actions are queued until pumped") {
	KomsoftGw *gw = kgw_test_singleton();
	gw->configure("http://localhost:8080", "pingpal", "key", "abc123xyz Alice");
	CHECK(gw->pending_count() == 0);
	// Each call enqueues one action; nothing is issued because pump() never runs.
	gw->list_leaderboards();
	gw->submit_score("wk", 100, Dictionary(), Dictionary(), "");
	gw->get_rank("wk", 3);
	CHECK(gw->pending_count() == 3);
	// reset() drops the queue without issuing anything.
	gw->reset();
	CHECK(gw->pending_count() == 0);
}

#endif // DOCTEST
