/**************************************************************************/
/*  gd_nakama1.cpp                                                        */
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

#include "gd_nakama1.h"

// ── GdNakama1 (internal singleton) ──────────────────────────────────────────

GdNakama1 *GdNakama1::singleton = nullptr;

GdNakama1 *GdNakama1::get_singleton() { return singleton; }

void GdNakama1::set_session(DefaultSession *p_session) {
	if (nk_session) {
		memdelete(nk_session);
	}
	nk_session = p_session;
}

bool GdNakama1::is_session_expired() const {
	return !(nk_session && !nk_session->is_expired());
}

GdNakama1::GdNakama1() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "GdNakama1 singleton already exists");
	singleton = this;
}

GdNakama1::~GdNakama1() {
	if (nk_session) {
		memdelete(nk_session);
	}
	singleton = nullptr;
}

// ── GdNakama1Node ────────────────────────────────────────────────────────────

void GdNakama1Node::_client_request_error(String error_message) {
	ERR_PRINT("Network request unsuccessful. Check the logs!");
}

void GdNakama1Node::_client_network_error(int status, int code) {
	ERR_PRINT("Network request unsuccessful. Check the logs!");
}

void GdNakama1Node::_client_session_error(NkErrorCode error_code, String error_message, String collation_id) {
	ERR_PRINT("Session failed!");
}

void GdNakama1Node::_client_session_accepted(String session_token, String collation_id) {
	_authenticated(session_token);
}

void GdNakama1Node::_authenticated(const String &p_session_token) {
	ERR_FAIL_NULL(nk_client);
	GdNakama1::get_singleton()->set_session(memnew(DefaultSession(p_session_token)));
	LOGI("Authenticated successfully. User ID: " + GdNakama1::get_singleton()->get_session()->get_user_id());
	emit_signal("authenticated");
}

void GdNakama1Node::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_client", "server_key", "server_host", "port", "ssl", "timeout"), &GdNakama1Node::create_client, DEFVAL(7349), DEFVAL(false), DEFVAL(60));
	ClassDB::bind_method(D_METHOD("authenticate_device", "device", "create"), &GdNakama1Node::authenticate_device, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("authenticate_email", "email", "password", "create"), &GdNakama1Node::authenticate_email, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_session_expired"), &GdNakama1Node::is_session_expired);
	ClassDB::bind_method(D_METHOD("logout"), &GdNakama1Node::logout);

	ClassDB::bind_method(D_METHOD("set_lang", "lang"), &GdNakama1Node::set_lang);
	ClassDB::bind_method(D_METHOD("get_lang"), &GdNakama1Node::get_lang);
	ClassDB::bind_method(D_METHOD("set_trace", "trace"), &GdNakama1Node::set_trace);
	ClassDB::bind_method(D_METHOD("get_trace"), &GdNakama1Node::get_trace);

	ClassDB::bind_method(D_METHOD("join_chat_room", "room_name"), &GdNakama1Node::join_chat_room);
	ClassDB::bind_method(D_METHOD("write_chat_message", "channel_id", "content"), &GdNakama1Node::write_chat_message);
	ClassDB::bind_method(D_METHOD("submit_score", "leaderboard_id", "op", "score"), &GdNakama1Node::submit_score);

	ClassDB::bind_method(D_METHOD("_client_request_error"), &GdNakama1Node::_client_request_error);
	ClassDB::bind_method(D_METHOD("_client_network_error"), &GdNakama1Node::_client_network_error);
	ClassDB::bind_method(D_METHOD("_client_session_error"), &GdNakama1Node::_client_session_error);
	ClassDB::bind_method(D_METHOD("_client_session_accepted"), &GdNakama1Node::_client_session_accepted);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "lang"), "set_lang", "get_lang");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "trace"), "set_trace", "get_trace");

	ADD_SIGNAL(MethodInfo("authenticated"));
	ADD_SIGNAL(MethodInfo("chat_message_received",
			PropertyInfo(Variant::STRING, "channel_id"),
			PropertyInfo(Variant::STRING, "message_id"),
			PropertyInfo(Variant::STRING, "message_code"),
			PropertyInfo(Variant::INT, "sender_id"),
			PropertyInfo(Variant::STRING, "username"),
			PropertyInfo(Variant::STRING, "content")));
}

void GdNakama1Node::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_process(true);
		} break;

		case NOTIFICATION_PROCESS: {
			if (nk_client) {
				nk_client->poll();
			}
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (nk_client) {
				memdelete(nk_client);
				nk_client = nullptr;
			}
		} break;
	}
}

void GdNakama1Node::create_client(String p_server_key, String p_server_host, int p_port, bool p_ssl, int p_timeout) {
	if (nk_client) {
		memdelete(nk_client);
		nk_client = nullptr;
	}

	nk_client = memnew(DefaultClient(p_server_key, p_server_host, p_port, p_ssl, p_timeout));
	nk_client->connect("session_accepted", this, "_client_session_accepted");
}

void GdNakama1Node::authenticate_device(String p_device, bool create) {
	ERR_FAIL_NULL(nk_client);
	if (create)
		nk_client->user_register(DefaultAuthenticateRequest::Builder::device(p_device));
	else
		nk_client->user_login(DefaultAuthenticateRequest::Builder::device(p_device));
}

void GdNakama1Node::authenticate_email(String p_email, String p_password, bool create) {
	ERR_FAIL_NULL(nk_client);
	if (create)
		nk_client->user_register(DefaultAuthenticateRequest::Builder::email(p_email, p_password));
	else
		nk_client->user_login(DefaultAuthenticateRequest::Builder::email(p_email, p_password));
}

bool GdNakama1Node::is_session_expired() {
	GdNakama1 *s = GdNakama1::get_singleton();
	return s ? s->is_session_expired() : true;
}

void GdNakama1Node::join_chat_room(String p_room_name) {
	ERR_FAIL_NULL(nk_client);
	nk_client->send(Ref<NkCollatedMessage>(memnew(TopicsJoinMessage(p_room_name))));
}

void GdNakama1Node::write_chat_message(String p_channel_id, String p_content) {
	ERR_FAIL_NULL(nk_client);
	nk_client->send(Ref<NkCollatedMessage>(memnew(TopicMessageSendMessage(p_channel_id, p_content))));
}

void GdNakama1Node::logout() {
	ERR_FAIL_NULL(nk_client);
	nk_client->user_logout();
	GdNakama1 *s = GdNakama1::get_singleton();
	if (s) {
		s->set_session(nullptr);
	}
}

void GdNakama1Node::set_lang(String p_lang) {
	ERR_FAIL_NULL(nk_client);
	nk_client->set_lang(p_lang);
}

String GdNakama1Node::get_lang() const {
	ERR_FAIL_NULL_V(nk_client, "");
	return nk_client->get_lang();
}

void GdNakama1Node::set_trace(bool p_trace) {
	ERR_FAIL_NULL(nk_client);
	nk_client->set_trace(p_trace);
}

bool GdNakama1Node::get_trace() const {
	ERR_FAIL_NULL_V(nk_client, false);
	return nk_client->get_trace();
}

void GdNakama1Node::submit_score(String p_leaderboard_id, NkMessage::ScoreOperator p_op, int64_t p_score) {
	ERR_FAIL_NULL(nk_client);
	nk_client->send(Ref<NkCollatedMessage>(memnew(LeaderboardRecordWriteMessage(p_leaderboard_id, p_op, p_score))));
}

GdNakama1Node::GdNakama1Node() {}

GdNakama1Node::~GdNakama1Node() {
	// NOTIFICATION_EXIT_TREE handles teardown when in scene.
	// This is a safety fallback for nodes destroyed without entering the tree.
	if (nk_client) {
		memdelete(nk_client);
	}
}
