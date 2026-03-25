/**************************************************************************/
/*  gd_discord.cpp                                                        */
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

#include "gd_discord.h"

#include "core/os/os.h"

GdDiscordClient *GdDiscordClient::singleton = nullptr;
const char *GdDiscordClient::REST_BASE_URL = "https://discord.com/api/v10";

// =========================================================================
// Lifecycle
// =========================================================================

GdDiscordClient::GdDiscordClient() {
	singleton = this;
	sequence = -1;
	heartbeat_interval_ms = 0;
	heartbeat_timer_ms = 0;
	heartbeat_ack = true;
	resuming = false;
	intents = 0;
	state = STATE_DISCONNECTED;
	rest_busy = false;
	rest_http = nullptr;
	gateway_http = nullptr;

#ifdef MODULE_WEBSOCKET_ENABLED
	ws_client = Ref<WebSocketClient>(WebSocketClient::create());
#endif
}

GdDiscordClient::~GdDiscordClient() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

GdDiscordClient *GdDiscordClient::get_singleton() {
	return singleton;
}

void GdDiscordClient::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			// Create HTTP request nodes as children
			rest_http = memnew(HTTPRequest);
			rest_http->set_name("RestHTTP");
			rest_http->set_use_threads(true);
			add_child(rest_http);
			rest_http->connect("request_completed", this, "_on_rest_completed");

			gateway_http = memnew(HTTPRequest);
			gateway_http->set_name("GatewayHTTP");
			gateway_http->set_use_threads(true);
			add_child(gateway_http);
			gateway_http->connect("request_completed", this, "_on_gateway_url_received");

#ifdef MODULE_WEBSOCKET_ENABLED
			ws_client->connect("connection_established", this, "_ws_connected");
			ws_client->connect("data_received", this, "_ws_data_received");
			ws_client->connect("connection_closed", this, "_ws_closed");
			ws_client->connect("connection_error", this, "_ws_error");
#endif
			set_process(false);
		} break;

		case NOTIFICATION_PROCESS: {
#ifdef MODULE_WEBSOCKET_ENABLED
			if (ws_client.is_valid()) {
				ws_client->poll();
			}
#endif
			// Heartbeat timer
			if (state == STATE_CONNECTED && heartbeat_interval_ms > 0) {
				int delta_ms = (int)(get_process_delta_time() * 1000.0);
				heartbeat_timer_ms += delta_ms;
				if (heartbeat_timer_ms >= heartbeat_interval_ms) {
					heartbeat_timer_ms = 0;
					if (!heartbeat_ack) {
						// No ACK received — connection is zombie, reconnect
						WARN_PRINT("Discord: heartbeat ACK not received, reconnecting.");
						state = STATE_RECONNECTING;
#ifdef MODULE_WEBSOCKET_ENABLED
						ws_client->disconnect_from_host(4000, "Heartbeat timeout");
#endif
						_connect_to_gateway();
					} else {
						heartbeat_ack = false;
						_send_heartbeat();
					}
				}
			}
		} break;

		case NOTIFICATION_EXIT_TREE: {
			disconnect_bot();
		} break;
	}
}

// =========================================================================
// Connection
// =========================================================================

void GdDiscordClient::connect_bot(const String &p_token, int p_intents) {
	ERR_FAIL_COND_MSG(state != STATE_DISCONNECTED, "Discord: already connected or connecting.");
#ifndef MODULE_WEBSOCKET_ENABLED
	ERR_FAIL_MSG("Discord: WebSocket module is required but not enabled.");
#endif

	bot_token = p_token;
	intents = p_intents;
	state = STATE_CONNECTING;
	sequence = -1;
	session_id = "";
	resuming = false;

	// Fetch gateway URL via REST
	Vector<String> headers = _make_headers();
	String url = String(REST_BASE_URL) + "/gateway/bot";
	gateway_http->request(url, headers, true, HTTPClient::METHOD_GET);
}

void GdDiscordClient::disconnect_bot() {
	if (state == STATE_DISCONNECTED) {
		return;
	}
#ifdef MODULE_WEBSOCKET_ENABLED
	ws_client->disconnect_from_host(1000, "Client disconnect");
#endif
	state = STATE_DISCONNECTED;
	set_process(false);
	emit_signal("disconnected", 1000, "Client disconnect");
}

bool GdDiscordClient::is_discord_connected() const {
	return state == STATE_CONNECTED;
}

GdDiscordClient::ConnectionState GdDiscordClient::get_state() const {
	return state;
}

void GdDiscordClient::_connect_to_gateway() {
#ifdef MODULE_WEBSOCKET_ENABLED
	String url = resuming && !resume_gateway_url.empty() ? resume_gateway_url : gateway_url;
	url += "?v=10&encoding=json";

	Vector<String> protocols;
	Error err = ws_client->connect_to_url(url, protocols, true);
	if (err != OK) {
		state = STATE_DISCONNECTED;
		emit_signal("error", "Discord: failed to connect to gateway WebSocket.");
	}
	set_process(true);
#endif
}

void GdDiscordClient::_on_gateway_url_received(int p_result, int p_code, const PoolStringArray &p_headers, const PoolByteArray &p_body) {
	if (p_result != HTTPRequest::RESULT_SUCCESS || p_code != 200) {
		state = STATE_DISCONNECTED;
		emit_signal("error", vformat("Discord: failed to get gateway URL (HTTP %d).", p_code));
		return;
	}

	String body_str;
	body_str.parse_utf8((const char *)p_body.read().ptr(), p_body.size());
	Variant parsed;
	String err_str;
	int err_line;
	if (JSON::parse(body_str, parsed, err_str, err_line) != OK) {
		state = STATE_DISCONNECTED;
		emit_signal("error", "Discord: failed to parse gateway response.");
		return;
	}

	Dictionary data = parsed;
	gateway_url = data["url"];
	_connect_to_gateway();
}

// =========================================================================
// Gateway WebSocket handlers
// =========================================================================

void GdDiscordClient::_ws_connected(String p_protocol) {
	// Wait for Hello (op 10) — don't send anything yet
}

void GdDiscordClient::_ws_data_received() {
#ifdef MODULE_WEBSOCKET_ENABLED
	Ref<WebSocketPeer> peer = ws_client->get_peer(1);
	while (peer->get_available_packet_count() > 0) {
		const uint8_t *buf;
		int buf_size;
		Error err = peer->get_packet(&buf, buf_size);
		if (err != OK) {
			break;
		}
		String msg;
		msg.parse_utf8((const char *)buf, buf_size);
		_parse_gateway_message(msg);
	}
#endif
}

void GdDiscordClient::_ws_closed(bool p_clean) {
	if (state == STATE_CONNECTED && !p_clean) {
		// Unexpected close — attempt resume
		state = STATE_RECONNECTING;
		resuming = true;
		_connect_to_gateway();
		return;
	}
	if (state != STATE_RECONNECTING) {
		state = STATE_DISCONNECTED;
		set_process(false);
		emit_signal("disconnected", 0, "WebSocket closed");
	}
}

void GdDiscordClient::_ws_error() {
	if (state == STATE_RECONNECTING) {
		// Retry after a delay — for now just report
		state = STATE_DISCONNECTED;
		set_process(false);
	}
	emit_signal("error", "Discord: WebSocket connection error.");
}

// =========================================================================
// Gateway protocol
// =========================================================================

void GdDiscordClient::_send_gateway_message(int p_opcode, const Variant &p_data) {
#ifdef MODULE_WEBSOCKET_ENABLED
	Dictionary msg;
	msg["op"] = p_opcode;
	msg["d"] = p_data;
	String json = JSON::print(msg);

	Ref<WebSocketPeer> peer = ws_client->get_peer(1);
	peer->set_write_mode(WebSocketPeer::WRITE_MODE_TEXT);
	peer->put_packet((const uint8_t *)json.utf8().get_data(), json.utf8().length());
#endif
}

void GdDiscordClient::_send_identify() {
	Dictionary d;
	d["token"] = bot_token;
	d["intents"] = intents;

	Dictionary properties;
	properties["os"] = OS::get_singleton()->get_name();
	properties["browser"] = GODOTDISCORD_USER_AGENT;
	properties["device"] = GODOTDISCORD_USER_AGENT;
	d["properties"] = properties;

	_send_gateway_message(2, d); // Op 2 = Identify
}

void GdDiscordClient::_send_heartbeat() {
	Variant seq = sequence >= 0 ? Variant(sequence) : Variant();
	_send_gateway_message(1, seq); // Op 1 = Heartbeat
}

void GdDiscordClient::_send_resume() {
	Dictionary d;
	d["token"] = bot_token;
	d["session_id"] = session_id;
	d["seq"] = sequence;
	_send_gateway_message(6, d); // Op 6 = Resume
}

void GdDiscordClient::_parse_gateway_message(const String &p_json) {
	Variant parsed;
	String err_str;
	int err_line;
	if (JSON::parse(p_json, parsed, err_str, err_line) != OK) {
		WARN_PRINT("Discord: failed to parse gateway message.");
		return;
	}

	Dictionary msg = parsed;
	int op = msg["op"];

	// Update sequence number
	if (msg.has("s") && msg["s"].get_type() != Variant::NIL) {
		sequence = msg["s"];
	}

	switch (op) {
		case 0: { // Dispatch
			String event = msg["t"];
			Dictionary data = msg.has("d") ? (Dictionary)msg["d"] : Dictionary();
			_handle_dispatch(event, data);
		} break;

		case 1: { // Heartbeat request
			_send_heartbeat();
		} break;

		case 7: { // Reconnect
			state = STATE_RECONNECTING;
			resuming = true;
#ifdef MODULE_WEBSOCKET_ENABLED
			ws_client->disconnect_from_host(4000, "Server requested reconnect");
#endif
			_connect_to_gateway();
		} break;

		case 9: { // Invalid Session
			bool can_resume = msg["d"];
			if (can_resume && !session_id.empty()) {
				resuming = true;
			} else {
				resuming = false;
				session_id = "";
				sequence = -1;
			}
			state = STATE_RECONNECTING;
#ifdef MODULE_WEBSOCKET_ENABLED
			ws_client->disconnect_from_host(4000, "Invalid session");
#endif
			_connect_to_gateway();
		} break;

		case 10: { // Hello
			Dictionary d = msg["d"];
			heartbeat_interval_ms = d["heartbeat_interval"];
			heartbeat_timer_ms = 0;
			heartbeat_ack = true;

			// Send first heartbeat immediately with jitter
			_send_heartbeat();

			if (resuming && !session_id.empty()) {
				_send_resume();
			} else {
				_send_identify();
			}
		} break;

		case 11: { // Heartbeat ACK
			heartbeat_ack = true;
		} break;
	}
}

void GdDiscordClient::_handle_dispatch(const String &p_event, const Dictionary &p_data) {
	if (p_event == "READY") {
		state = STATE_CONNECTED;
		session_id = p_data["session_id"];
		self_user = p_data["user"];
		if (p_data.has("resume_gateway_url")) {
			resume_gateway_url = p_data["resume_gateway_url"];
		}
		resuming = false;
		emit_signal("bot_ready", self_user);
		emit_signal("connected");
	} else if (p_event == "RESUMED") {
		state = STATE_CONNECTED;
		resuming = false;
		emit_signal("connected");
	} else if (p_event == "MESSAGE_CREATE") {
		emit_signal("message_received", p_data);
	} else if (p_event == "MESSAGE_UPDATE") {
		emit_signal("message_updated", p_data);
	} else if (p_event == "MESSAGE_DELETE") {
		emit_signal("message_deleted", p_data);
	} else if (p_event == "MESSAGE_REACTION_ADD") {
		emit_signal("reaction_added", p_data);
	} else if (p_event == "MESSAGE_REACTION_REMOVE") {
		emit_signal("reaction_removed", p_data);
	} else if (p_event == "GUILD_CREATE") {
		emit_signal("guild_joined", p_data);
	} else if (p_event == "PRESENCE_UPDATE") {
		emit_signal("presence_updated", p_data);
	} else if (p_event == "TYPING_START") {
		emit_signal("typing_started", p_data);
	} else {
		emit_signal("dispatch", p_event, p_data);
	}
}

// =========================================================================
// REST API
// =========================================================================

Vector<String> GdDiscordClient::_make_headers() const {
	Vector<String> headers;
	headers.push_back("Authorization: Bot " + bot_token);
	headers.push_back("Content-Type: application/json");
	headers.push_back("User-Agent: " GODOTDISCORD_USER_AGENT);
	return headers;
}

void GdDiscordClient::_rest_request(const String &p_endpoint, HTTPClient::Method p_method, const String &p_body) {
	if (rest_busy) {
		PendingRequest req;
		req.endpoint = p_endpoint;
		req.method = p_method;
		req.body = p_body;
		request_queue.push_back(req);
		return;
	}

	rest_busy = true;
	String url = String(REST_BASE_URL) + p_endpoint;
	Vector<String> headers = _make_headers();
	rest_http->request(url, headers, true, p_method, p_body);
}

void GdDiscordClient::_on_rest_completed(int p_result, int p_code, const PoolStringArray &p_headers, const PoolByteArray &p_body) {
	rest_busy = false;

	Dictionary response;
	response["status"] = p_code;

	if (p_body.size() > 0) {
		String body_str;
		body_str.parse_utf8((const char *)p_body.read().ptr(), p_body.size());
		Variant parsed;
		String err_str;
		int err_line;
		if (JSON::parse(body_str, parsed, err_str, err_line) == OK) {
			response["data"] = parsed;
		} else {
			response["data"] = body_str;
		}
	}

	if (p_code == 429) {
		// Rate limited — could implement retry-after here
		WARN_PRINT("Discord: rate limited.");
	}

	emit_signal("rest_response", p_code, response);
	_process_request_queue();
}

void GdDiscordClient::_process_request_queue() {
	if (request_queue.size() > 0) {
		PendingRequest req = request_queue.front()->get();
		request_queue.pop_front();
		_rest_request(req.endpoint, req.method, req.body);
	}
}

// --- Public REST methods ---

void GdDiscordClient::send_message(const String &p_channel_id, const String &p_content) {
	Dictionary body;
	body["content"] = p_content;
	_rest_request("/channels/" + p_channel_id + "/messages", HTTPClient::METHOD_POST, JSON::print(body));
}

void GdDiscordClient::send_message_embed(const String &p_channel_id, const Dictionary &p_embed) {
	Dictionary body;
	Array embeds;
	embeds.push_back(p_embed);
	body["embeds"] = embeds;
	_rest_request("/channels/" + p_channel_id + "/messages", HTTPClient::METHOD_POST, JSON::print(body));
}

void GdDiscordClient::edit_message(const String &p_channel_id, const String &p_message_id, const String &p_content) {
	Dictionary body;
	body["content"] = p_content;
	_rest_request("/channels/" + p_channel_id + "/messages/" + p_message_id, HTTPClient::METHOD_PATCH, JSON::print(body));
}

void GdDiscordClient::delete_message(const String &p_channel_id, const String &p_message_id) {
	_rest_request("/channels/" + p_channel_id + "/messages/" + p_message_id, HTTPClient::METHOD_DELETE);
}

void GdDiscordClient::add_reaction(const String &p_channel_id, const String &p_message_id, const String &p_emoji) {
	_rest_request("/channels/" + p_channel_id + "/messages/" + p_message_id + "/reactions/" + p_emoji.http_escape() + "/@me", HTTPClient::METHOD_PUT);
}

void GdDiscordClient::typing_indicator(const String &p_channel_id) {
	_rest_request("/channels/" + p_channel_id + "/typing", HTTPClient::METHOD_POST);
}

// --- Getters ---

Dictionary GdDiscordClient::get_self_user() const {
	return self_user;
}

String GdDiscordClient::get_session_id() const {
	return session_id;
}

// =========================================================================
// Bindings
// =========================================================================

void GdDiscordClient::_bind_methods() {
	// Internal callbacks
	ClassDB::bind_method(D_METHOD("_ws_connected", "protocol"), &GdDiscordClient::_ws_connected);
	ClassDB::bind_method(D_METHOD("_ws_data_received"), &GdDiscordClient::_ws_data_received);
	ClassDB::bind_method(D_METHOD("_ws_closed", "clean"), &GdDiscordClient::_ws_closed);
	ClassDB::bind_method(D_METHOD("_ws_error"), &GdDiscordClient::_ws_error);
	ClassDB::bind_method(D_METHOD("_on_rest_completed", "result", "code", "headers", "body"), &GdDiscordClient::_on_rest_completed);
	ClassDB::bind_method(D_METHOD("_on_gateway_url_received", "result", "code", "headers", "body"), &GdDiscordClient::_on_gateway_url_received);

	// Public API
	ClassDB::bind_method(D_METHOD("connect_bot", "token", "intents"), &GdDiscordClient::connect_bot, DEFVAL(INTENT_GUILDS | INTENT_GUILD_MESSAGES | INTENT_MESSAGE_CONTENT));
	ClassDB::bind_method(D_METHOD("disconnect_bot"), &GdDiscordClient::disconnect_bot);
	ClassDB::bind_method(D_METHOD("is_discord_connected"), &GdDiscordClient::is_discord_connected);
	ClassDB::bind_method(D_METHOD("get_state"), &GdDiscordClient::get_state);

	ClassDB::bind_method(D_METHOD("send_message", "channel_id", "content"), &GdDiscordClient::send_message);
	ClassDB::bind_method(D_METHOD("send_message_embed", "channel_id", "embed"), &GdDiscordClient::send_message_embed);
	ClassDB::bind_method(D_METHOD("edit_message", "channel_id", "message_id", "content"), &GdDiscordClient::edit_message);
	ClassDB::bind_method(D_METHOD("delete_message", "channel_id", "message_id"), &GdDiscordClient::delete_message);
	ClassDB::bind_method(D_METHOD("add_reaction", "channel_id", "message_id", "emoji"), &GdDiscordClient::add_reaction);
	ClassDB::bind_method(D_METHOD("typing_indicator", "channel_id"), &GdDiscordClient::typing_indicator);

	ClassDB::bind_method(D_METHOD("get_self_user"), &GdDiscordClient::get_self_user);
	ClassDB::bind_method(D_METHOD("get_session_id"), &GdDiscordClient::get_session_id);

	// Signals
	ADD_SIGNAL(MethodInfo("bot_ready", PropertyInfo(Variant::DICTIONARY, "user")));
	ADD_SIGNAL(MethodInfo("message_received", PropertyInfo(Variant::DICTIONARY, "message")));
	ADD_SIGNAL(MethodInfo("message_updated", PropertyInfo(Variant::DICTIONARY, "message")));
	ADD_SIGNAL(MethodInfo("message_deleted", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("reaction_added", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("reaction_removed", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("guild_joined", PropertyInfo(Variant::DICTIONARY, "guild")));
	ADD_SIGNAL(MethodInfo("presence_updated", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("typing_started", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("dispatch", PropertyInfo(Variant::STRING, "event"), PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connected"));
	ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::INT, "code"), PropertyInfo(Variant::STRING, "reason")));
	ADD_SIGNAL(MethodInfo("error", PropertyInfo(Variant::STRING, "message")));
	ADD_SIGNAL(MethodInfo("rest_response", PropertyInfo(Variant::INT, "status"), PropertyInfo(Variant::DICTIONARY, "data")));

	// Enums
	BIND_ENUM_CONSTANT(STATE_DISCONNECTED);
	BIND_ENUM_CONSTANT(STATE_CONNECTING);
	BIND_ENUM_CONSTANT(STATE_CONNECTED);
	BIND_ENUM_CONSTANT(STATE_RECONNECTING);

	BIND_ENUM_CONSTANT(INTENT_GUILDS);
	BIND_ENUM_CONSTANT(INTENT_GUILD_MEMBERS);
	BIND_ENUM_CONSTANT(INTENT_GUILD_BANS);
	BIND_ENUM_CONSTANT(INTENT_GUILD_EMOJIS);
	BIND_ENUM_CONSTANT(INTENT_GUILD_INTEGRATIONS);
	BIND_ENUM_CONSTANT(INTENT_GUILD_WEBHOOKS);
	BIND_ENUM_CONSTANT(INTENT_GUILD_INVITES);
	BIND_ENUM_CONSTANT(INTENT_GUILD_VOICE_STATES);
	BIND_ENUM_CONSTANT(INTENT_GUILD_PRESENCES);
	BIND_ENUM_CONSTANT(INTENT_GUILD_MESSAGES);
	BIND_ENUM_CONSTANT(INTENT_GUILD_MESSAGE_REACTIONS);
	BIND_ENUM_CONSTANT(INTENT_GUILD_MESSAGE_TYPING);
	BIND_ENUM_CONSTANT(INTENT_DIRECT_MESSAGES);
	BIND_ENUM_CONSTANT(INTENT_DIRECT_MESSAGE_REACTIONS);
	BIND_ENUM_CONSTANT(INTENT_DIRECT_MESSAGE_TYPING);
	BIND_ENUM_CONSTANT(INTENT_MESSAGE_CONTENT);
}
