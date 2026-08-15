/**************************************************************************/
/*  gd_komsoftgw.h                                                        */
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

#ifndef GD_KOMSOFTGW_H
#define GD_KOMSOFTGW_H

#include "common/basic_http_request.h"
#include "core/dictionary.h"
#include "core/object.h"
#include "core/ustring.h"
#include "core/vector.h"
#include "scene/main/node.h"

// The leaderboard client is split in two, mirroring the SilentWolf design:
//
//   * [KomsoftGw]     — a global singleton that holds the config and exposes every
//                       action (submit, rank, top, …). Calls made anywhere are
//                       queued and only issued/polled while something pumps it, so
//                       the API is usable from any script without a node reference.
//   * [KomsoftGwNode] — a lightweight scene-tree node whose only job is to pump the
//                       singleton every frame. Drop one into your scene once.
//
//   func _ready():
//       add_child(KomsoftGwNode.new())          # drives processing
//       KomsoftGw.configure(url, svc, key, uid)  # singleton holds all actions
//       KomsoftGw.connect("score_submitted", self, "_on_submitted")
//       KomsoftGw.submit_score("weekly_highscore", 4200)
//
class KomsoftGw : public Object {
	GDCLASS(KomsoftGw, Object);

public:
	// Informational enums mirroring the server's board config vocabulary.
	enum SortOrder { SORT_ASC,
		SORT_DESC };
	enum ScoreOperator { OP_BEST,
		OP_SET,
		OP_INCR,
		OP_DECR };
	enum ResetPeriod { PERIOD_ALL_TIME,
		PERIOD_DAILY,
		PERIOD_WEEKLY,
		PERIOD_MONTHLY };

private:
	static KomsoftGw *singleton;

	String base_url;
	String service;
	String api_key;
	String user_id;
	String client_version = "1.0";
	String device_id = "komsoftgw-client";
	bool use_ssl = true;
	double timeout_seconds = 15.0;

	// A queued call: created by an API method, issued by the next pump().
	struct PendingAction {
		String callback;
		String url;
		Vector<String> headers;
		int method = 0; // HTTPClient::Method
		String body;
	};
	Vector<PendingAction> queue; // not yet issued
	Vector<Ref<BasicHTTPRequest>> active; // issued, being polled

	void _enqueue(const String &p_callback, int p_method, const String &p_url, bool p_json, const String &p_body);
	void _fail(const String &p_endpoint, int p_result, int p_code, const PoolByteArray &p_body);

	// Per-endpoint completion callbacks (bound so BasicHTTPRequest can target them).
	void _on_list_completed(int p_result, int p_code, PoolStringArray p_headers, PoolByteArray p_body);
	void _on_token_completed(int p_result, int p_code, PoolStringArray p_headers, PoolByteArray p_body);
	void _on_submit_completed(int p_result, int p_code, PoolStringArray p_headers, PoolByteArray p_body);
	void _on_rank_completed(int p_result, int p_code, PoolStringArray p_headers, PoolByteArray p_body);
	void _on_top_completed(int p_result, int p_code, PoolStringArray p_headers, PoolByteArray p_body);
	void _on_distribution_completed(int p_result, int p_code, PoolStringArray p_headers, PoolByteArray p_body);
	void _on_friends_completed(int p_result, int p_code, PoolStringArray p_headers, PoolByteArray p_body);

protected:
	static void _bind_methods();

public:
	static KomsoftGw *get_singleton();

	Vector<String> _make_headers(bool p_json) const;

	void configure(const String &p_base_url, const String &p_service, const String &p_api_key, const String &p_user_id);
	// Clear config back to defaults and drop any queued/in-flight requests.
	void reset();
	void set_use_ssl(bool p_v);
	bool get_use_ssl() const;
	void set_timeout(double p_seconds);
	double get_timeout() const;
	void set_client_version(const String &p_v);
	String get_client_version() const;
	void set_device_id(const String &p_v);
	String get_device_id() const;

	// Leaderboard API — each call is queued and processed on the next pump().
	void list_leaderboards();
	void request_submit_token(const String &p_board_key);
	void submit_score(const String &p_board_key, int64_t p_score, const Dictionary &p_metadata, const Dictionary &p_context, const String &p_token);
	void get_rank(const String &p_board_key, int p_around);
	void get_top(const String &p_board_key, const String &p_period_key, int p_limit, const String &p_cursor);
	void get_distribution(const String &p_board_key);
	void get_friends(const String &p_board_key, const Array &p_friend_ids);

	// Issue queued calls and poll in-flight ones. Called by KomsoftGwNode each frame;
	// safe to call manually if you drive processing yourself.
	void pump();

	// Number of queued + in-flight requests (0 when idle).
	int pending_count() const;

	KomsoftGw();
	~KomsoftGw();
};

VARIANT_ENUM_CAST(KomsoftGw::SortOrder);
VARIANT_ENUM_CAST(KomsoftGw::ScoreOperator);
VARIANT_ENUM_CAST(KomsoftGw::ResetPeriod);

// Pump driver: add one instance to the scene tree and it advances the singleton's
// queued/in-flight requests every frame. Holds no state of its own.
class KomsoftGwNode : public Node {
	GDCLASS(KomsoftGwNode, Node);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	KomsoftGwNode();
};

#endif // GD_KOMSOFTGW_H
