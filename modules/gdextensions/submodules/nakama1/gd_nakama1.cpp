/*************************************************************************/
/*  gd_nakama1.cpp                                                       */
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

#include "gd_nakama1.h"

static GdNakama1 *singleton = nullptr;

void GdNakama1::_client_request_error(String error_message) {
	ERR_PRINT("Network request unsuccesful. Check the logs!");
}

void GdNakama1::_client_network_error(HTTPRequest::Result status, HTTPClient::ResponseCode code) {
	ERR_PRINT("Network request unsuccesful. Check the logs!");
};

void GdNakama1::_client_session_error(NkErrorCode error_code, String error_message, String collation_id) {
	ERR_PRINT("Session failed!");
}

void GdNakama1::_client_session_accepted(String session_token, String collation_id) {
	_authenticated(session_token);
}

void GdNakama1::_authenticated(String p_session_token) {
	ERR_FAIL_NULL(nk_client);

	if (nk_session) {
		memdelete(nk_session);
		nk_session = nullptr;
	}

	nk_session = memnew(DefaultSession(p_session_token));
	LOGI("Session token: " + nk_session->get_auth_token());
	LOGI("Authenticated succesfully. User ID: " + nk_session->get_user_id());
	emit_signal("authenticated");
}

void GdNakama1::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_client"), &GdNakama1::create_client);
	ClassDB::bind_method(D_METHOD("authenticate_device"), &GdNakama1::authenticate_device);
	ClassDB::bind_method(D_METHOD("authenticate_email"), &GdNakama1::authenticate_email);
	ClassDB::bind_method(D_METHOD("is_session_expired"), &GdNakama1::is_session_expired);
	ClassDB::bind_method(D_METHOD("logout"), &GdNakama1::logout);

	ClassDB::bind_method(D_METHOD("set_lang"), &GdNakama1::set_lang);
	ClassDB::bind_method(D_METHOD("get_lang"), &GdNakama1::get_lang);
	ClassDB::bind_method(D_METHOD("set_trace"), &GdNakama1::set_trace);
	ClassDB::bind_method(D_METHOD("get_trace"), &GdNakama1::get_trace);

	ClassDB::bind_method(D_METHOD("join_chat_room"), &GdNakama1::join_chat_room);
	ClassDB::bind_method(D_METHOD("write_chat_message"), &GdNakama1::write_chat_message);

	// Private notifications:
	ClassDB::bind_method(D_METHOD("_client_request_error"), &GdNakama1::_client_request_error);
	ClassDB::bind_method(D_METHOD("_client_network_error"), &GdNakama1::_client_network_error);
	ClassDB::bind_method(D_METHOD("_client_session_error"), &GdNakama1::_client_session_error);
	ClassDB::bind_method(D_METHOD("_client_session_accepted"), &GdNakama1::_client_session_accepted);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "lang"), "set_lang", "get_lang");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "trace"), "set_trace", "get_trace");

	ADD_SIGNAL(MethodInfo("authenticated"));
	ADD_SIGNAL(MethodInfo("chat_message_recieved",
			PropertyInfo(Variant::STRING, "channel_id"),
			PropertyInfo(Variant::STRING, "message_id"),
			PropertyInfo(Variant::STRING, "message_code"),
			PropertyInfo(Variant::INT, "sender_id"),
			PropertyInfo(Variant::STRING, "username"),
			PropertyInfo(Variant::STRING, "content")));
}

void GdNakama1::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;

		case NOTIFICATION_PROCESS: {
			const real_t delta = get_process_delta_time();
			time_since_last_tick += delta;
			if (time_since_last_tick > 0.05) {
				time_since_last_tick = 0;
			}
		} break;
	}
}

void GdNakama1::create_client(String p_server_key, String p_server_host, int p_port, bool p_ssl, int p_timeout) {
	if (nk_client) {
		memdelete(nk_client);
		nk_client = nullptr;
	}

	nk_client = memnew(DefaultClient(p_server_key, p_server_host, p_port, p_ssl, p_timeout));
	nk_client->connect("session_accepted", this, "_client_session_accepted");
}

void GdNakama1::authenticate_device(String p_device, bool create) {
	ERR_FAIL_NULL(nk_client);

	if (create)
		nk_client->user_register(DefaultAuthenticateRequest::Builder::device(p_device));
	else
		nk_client->user_login(DefaultAuthenticateRequest::Builder::device(p_device));
}

void GdNakama1::authenticate_email(String p_email, String p_password, bool create) {
	ERR_FAIL_NULL(nk_client);

	if (create)
		nk_client->user_register(DefaultAuthenticateRequest::Builder::email(p_email, p_password));
	else
		nk_client->user_login(DefaultAuthenticateRequest::Builder::email(p_email, p_password));
}

bool GdNakama1::is_session_expired() {
	return !(nk_session && !nk_session->is_expired());
}

void GdNakama1::join_chat_room(String p_room_name) {
}

void GdNakama1::write_chat_message(String p_channel_id, String p_content) {
	ERR_FAIL_NULL(nk_client);
}

void GdNakama1::logout() {
	ERR_FAIL_NULL(nk_client);
}

void GdNakama1::set_lang(String p_lang) {
	ERR_FAIL_NULL(nk_client);

	nk_client->set_lang(p_lang);
}

String GdNakama1::get_lang() const {
	ERR_FAIL_NULL_V(nk_client, "");

	return nk_client->get_lang();
}

void GdNakama1::set_trace(bool p_trace) {
	ERR_FAIL_NULL(nk_client);

	nk_client->set_trace(p_trace);
}

bool GdNakama1::get_trace() const {
	ERR_FAIL_NULL_V(nk_client, false);

	return nk_client->get_trace();
}

GdNakama1 *GdNakama1::get_singleton() {
	return singleton;
}

GdNakama1::GdNakama1() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "Singleton already exists");
	singleton = this;
}

GdNakama1::~GdNakama1() {
	singleton = nullptr;
}
