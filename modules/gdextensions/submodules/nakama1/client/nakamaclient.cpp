/*************************************************************************/
/*  nakamaclient.cpp                                                     */
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

#include "nakamaclient.h"
#include "uuid.h"

#include "core/bind/core_bind.h"
#include "core/os/os.h"
#include "core/variant.h"

#include "modules/modules_enabled.gen.h"

#ifndef MODULE_WEBSOCKET_ENABLED
#error WebSocket module is required for network communication.
#endif

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

DefaultSession::DefaultSession(const String p_token) {
	const Vector<String> &decoded = p_token.split("\\.");

	ERR_FAIL_COND_MSG(decoded.size() != 3, "Not a valid token.");

	const Ref<JSONParseResult> json = _JSON::get_singleton()->parse(_Marshalls::get_singleton()->base64_to_utf8((decoded[1])));

	ERR_FAIL_COND_MSG(json.is_valid(), "Failed to parse token.");
	ERR_FAIL_COND_MSG(json->get_error() != OK, "Invalid token content.");

	const Dictionary info = json->get_result();
#ifdef DEBUG_ENABLED
	print_line(vformat("Session token: %s", info));
#endif
	create_time = OS::get_singleton()->get_system_time_secs() * 1000;
	expire_time = info["exp"];
	handle = info["han"];
	user_id = info["uid"];

	token = p_token;
}

bool DefaultSession::is_expired() const {
	return is_expired(OS::get_singleton()->get_system_time_secs() * 1000);
}

bool DefaultSession::is_expired(uint64_t p_now) const {
	return (expire_time - OS::get_singleton()->get_system_time_secs() * 1000) < 0;
}

DefaultSession *DefaultSession::restore(String p_token) {
	return memnew(DefaultSession(p_token));
}

void DefaultClient::_rest_request_completed(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data) {
	String error_text;

	if (p_status == HTTPRequest::RESULT_SUCCESS && p_code < HTTPClient::RESPONSE_BAD_REQUEST) {
		String str = Utils::bytearray_to_string(p_data);
		LOGI(str);

		if (p_data.empty()) {
			emit_signal("request_error", "Error in response body from server");
			return;
		}

		if (const AuthenticateResponse *message = AuthenticateResponse(p_data).get()) {
			const String collation_id = message->get_collation_id();
			ERR_FAIL_COND_MSG(collation_id.empty(), "Unknown response source.");

			ERR_FAIL_COND_MSG(collation_ids.count(collation_id) == 0, "Unregistered response source.");

			ObjectID object_id = collation_ids[message->get_collation_id()];

			if (trace) {
				LOGI(vformat("Authenticate response  received: %s from object: %s", message->get_collation_id(), object_id));
			}

			if (message->is_error()) {
				emit_signal("session_error", message->get_error_code(), message->get_error_message(), collation_id);
			} else if (message->is_session()) {
				emit_signal("session_accepted", message->get_session_token(), collation_id);
			} else {
				emit_signal("session_error", UNRECOGNIZED_PAYLOAD, "Unknown response format from server for message", collation_id);
			}
		} else {
			emit_signal("request_error", "Not an authentication response");
		}
	} else {
		emit_signal("network_error", p_status, p_code);
	}
}

void DefaultClient::_ws_connection_closed() {
	// Graceful socket disconnect is complete, clean up.
	collation_ids.clear();
	emit_signal("session_closed");
}

void DefaultClient::_ws_connection_error() {
	// Socket has failed and is no longer connected, clean up.
	collation_ids.clear();
	emit_signal("session_closed");
}

void DefaultClient::_ws_connection_established() {
	emit_signal("session_connected");
}

void DefaultClient::_ws_server_close_request() {
}

void DefaultClient::_ws_data_received() {
	Ref<WebSocketPeer> peer = ws->get_peer(1);

	if (!peer.is_valid() || !peer->is_connected_to_host())
		return;

	while (peer->get_available_packet_count()) {
		const uint8_t *packet;
		int len;
		Error err = peer->get_packet(&packet, len);
		if (err != OK) {
			ERR_PRINT("Error getting packet!");
			break;
		}

		PoolByteArray data = Utils::data_to_bytearray(packet, len);
		if (data.empty()) {
			emit_signal("request_error", "Error in response body from server");
			return;
		}

		_process_packet(data);
	}
}

#define MakeDeferredMessage(T, data, payload) \
	Ref<DeferredMessage<server::T>>(memnew(DeferredMessage<server::T>(data, payload)))

#define MakeString(str) \
	(str ? CharString(str->c_str(), str->size()) : CharString())

#define _newumsg(T...) \
	Ref<NkUncollatedMessage>(memnew(T))

#define _newmsg(T...) \
	Ref<UncollatedMessage>(memnew(T))

#define _fbloop(T, array) \
	Utils::mkfbloop<server::T>(array)

void DefaultClient::_process_packet(PoolByteArray data) {
	if (const NkMessage *message = NkMessage(data).get()) {
		const String collation_id = message->get_collation_id();
		const server::Envelope_::EnvelopeContent *payload = message->get_envelope_payload();

		if (collation_id.empty()) {
			switch (message->get_payload_case()) {
				case NkMessage::PAYLOAD_NOT_SET: {
					emit_signal("request_error", "No payload in incoming uncollated message from server");
				} break;
				case NkMessage::PAYLOAD_HEARTBEAT: {
					const long new_server_time = payload->heartbeat()->timestamp();
					if (new_server_time > server_time) {
						// Don't let server time go backwards.
						server_time = new_server_time;
					}
				} break;
				case NkMessage::PAYLOAD_TOPIC_MESSAGE: {
					emit_signal("topic_message", payload->topic_message()->user_id(), payload->topic_message()->message_id(), payload->topic_message()->type());
				} break;
				case NkMessage::PAYLOAD_TOPIC_PRESENCE: {
					emit_signal("topic_presence", MakeDeferredMessage(TopicPresence, data, payload));
				} break;
				case NkMessage::PAYLOAD_MATCH_DATA: {
					emit_signal("match_data", MakeDeferredMessage(MatchData, data, payload));
				} break;
				case NkMessage::PAYLOAD_MATCH_PRESENCE: {
					emit_signal("match_presence", MakeDeferredMessage(MatchPresence, data, payload));
				} break;
				case NkMessage::PAYLOAD_MATCHMAKE_MATCHED: {
					emit_signal("matchmake_matched", MakeDeferredMessage(MatchmakeMatched, data, payload));
				} break;
				case NkMessage::PAYLOAD_LIVE_NOTIFICATIONS: {
					Array array;
					for (const auto *item : _fbloop(Notification, payload->live_notifications()->notifications())) {
						array.append(MakeDeferredMessage(Notification, data, item));
					}
					emit_signal("live_notifications", array);
				} break;
				default:
					break;
			}
			return;
		}

		ERR_FAIL_COND_MSG(collation_ids.count(collation_id) == 0, "No matching receiver for incoming collation ID: " + collation_id);

		ObjectID object_id = collation_ids[message->get_collation_id()];
		Object *receiver = ObjectDB::get_instance(object_id);

		ERR_FAIL_NULL_MSG(receiver, "No receiver for incoming collation ID: " + collation_id);

		switch (NkMessage::PayloadCase pc = message->get_payload_case()) {
			case NkMessage::PAYLOAD_NOT_SET: {
				receiver->emit_signal("nk_message", "message", collation_id);
			} break;
			case NkMessage::PAYLOAD_ERROR: {
				receiver->emit_signal("nk_message", "error", payload->error()->message(), payload->error()->code(), collation_id);
			} break;
			case NkMessage::PAYLOAD_SELF: {
				receiver->emit_signal("nk_message", "self", MakeDeferredMessage(Self, data, payload), collation_id);
			} break;
			case NkMessage::PAYLOAD_USERS: {
				Array array;
				for (const auto *item : _fbloop(User, payload->users()->users())) {
					array.append(MakeDeferredMessage(User, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, collation_id);
			} break;
			case NkMessage::PAYLOAD_FRIENDS: {
				Array array;
				for (const auto *item : _fbloop(Friend, payload->friends()->friends())) {
					array.append(MakeDeferredMessage(Friend, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, "", collation_id);
			} break;
			case NkMessage::PAYLOAD_GROUPS: {
				Array array;
				for (const auto *item : _fbloop(Group, payload->groups()->groups())) {
					array.append(MakeDeferredMessage(Group, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, MakeString(payload->groups()->cursor()), collation_id);
			} break;
			case NkMessage::PAYLOAD_GROUPS_SELF: {
				Array array;
				for (const auto *item : _fbloop(TGroupsSelf_::GroupSelf, payload->groups_self()->groups_self())) {
					array.append(MakeDeferredMessage(TGroupsSelf_::GroupSelf, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, collation_id);
			} break;
			case NkMessage::PAYLOAD_GROUP_USERS: {
				Array array;
				for (const auto *item : _fbloop(GroupUser, payload->group_users()->users())) {
					array.append(MakeDeferredMessage(GroupUser, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, collation_id);
			} break;
			case NkMessage::PAYLOAD_STORAGE_DATA: {
				Array array;
				for (const auto *item : _fbloop(TStorageData_::StorageData, payload->storage_data()->data())) {
					array.append(MakeDeferredMessage(TStorageData_::StorageData, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, MakeString(payload->storage_data()->cursor()), collation_id);
			} break;
			case NkMessage::PAYLOAD_STORAGE_KEYS: {
				Array array;
				for (const auto *item : _fbloop(TStorageKeys_::StorageKey, payload->storage_keys()->keys())) {
					array.append(MakeDeferredMessage(TStorageKeys_::StorageKey, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, collation_id);
			} break;
			case NkMessage::PAYLOAD_RPC: {
				receiver->emit_signal("nk_message", pc, MakeDeferredMessage(TRpc, data, payload->rpc()), collation_id);
			} break;
			case NkMessage::PAYLOAD_TOPICS: {
				Array array;
				for (const auto *item : _fbloop(TTopics_::Topic, payload->topics()->topics())) {
					array.append(MakeDeferredMessage(TTopics_::Topic, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, collation_id);
			} break;
			case NkMessage::PAYLOAD_TOPIC_MESSAGE_ACK: {
				receiver->emit_signal("nk_message", pc, MakeDeferredMessage(TTopicMessageAck, data, payload->topic_message_ack()), collation_id);
			} break;
			case NkMessage::PAYLOAD_TOPIC_MESSAGES: {
				Array array;
				for (const auto *item : _fbloop(TopicMessage, payload->topic_messages()->messages())) {
					array.append(MakeDeferredMessage(TopicMessage, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, MakeString(payload->topic_messages()->cursor()), collation_id);
			} break;
			case NkMessage::PAYLOAD_MATCH: {
				receiver->emit_signal("nk_message", pc, MakeDeferredMessage(TMatch, data, payload->match()), collation_id);
			} break;
			case NkMessage::PAYLOAD_MATCHES: {
				Array array;
				for (const auto *item : _fbloop(Match, payload->matches()->matches())) {
					array.append(MakeDeferredMessage(Match, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, collation_id);
			} break;
			case NkMessage::PAYLOAD_MATCHMAKE_TICKET: {
				receiver->emit_signal("nk_message", pc, MakeDeferredMessage(TMatchmakeTicket, data, payload->matchmake_ticket()), collation_id);
			} break;
			case NkMessage::PAYLOAD_LEADERBOARDS: {
				Array array;
				for (const auto *item : _fbloop(Leaderboard, payload->leaderboards()->leaderboards())) {
					array.append(MakeDeferredMessage(Leaderboard, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, MakeString(payload->leaderboards()->cursor()), collation_id);
			} break;
			case NkMessage::PAYLOAD_LEADERBOARD_RECORDS: {
				Array array;
				for (const auto *item : _fbloop(LeaderboardRecord, payload->leaderboard_records()->records())) {
					array.append(MakeDeferredMessage(LeaderboardRecord, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, MakeString(payload->leaderboard_records()->cursor()), collation_id);
			} break;
			case NkMessage::PAYLOAD_NOTIFICATIONS: {
				Array array;
				for (const auto *item : _fbloop(Notification, payload->notifications()->notifications())) {
					array.append(MakeDeferredMessage(Notification, data, item));
				}
				receiver->emit_signal("nk_message", pc, array, MakeString(payload->notifications()->resumable_cursor()), collation_id);
			} break;
			default: {
				if (auto *err = payload->error()) {
					receiver->emit_signal("nk_message", "error", err->message(), err->code(), collation_id);
				} else {
					receiver->emit_signal("nk_message", "error", "Unknow payload content type", collation_id);
				}
			} break;
		}
	} else {
		emit_signal("request_error", "Unrecognized message response", ERR_PARAMETER_RANGE_ERROR);
	}
}

long DefaultClient::get_server_time() {
	return server_time != 0 ? server_time : OS::get_singleton()->get_system_time_msecs();
}

bool DefaultClient::user_login(const Ref<DefaultAuthenticateRequest> &auth) {
	return authenticate(auth, "/user/login");
}

bool DefaultClient::user_register(const Ref<DefaultAuthenticateRequest> &auth) {
	return authenticate(auth, "/user/register");
}

bool DefaultClient::user_logout() {
	return send(_newumsg(LogoutMessage));
}

bool DefaultClient::authenticate(const Ref<DefaultAuthenticateRequest> &p_auth, String p_path) {
	String collation_id = uuid1().hex();
	PoolByteArray payload = p_auth->as_bytes(collation_id);

	String url = Builder::Url()
						 .scheme(ssl ? "https" : "http")
						 .host(host)
						 .port(port)
						 .path(p_path)
						 .build();

	Error ret = Builder::Request()
						.url(url)
						.method(HTTPClient::METHOD_POST, payload)
						.header("Content-Type", "application/octet-stream")
						.header("Authorization", "Basic " + _Marshalls::get_singleton()->utf8_to_base64(server_key + ":"))
						.header("Accept-Language", lang)
						.header("User-Agent", "nakama-godot/0.1.0") // TODO set user-agent based on build version.
						.make(rest, trace);

	if (ret != OK) {
		emit_signal("request_error", "Failed to start a request", ret);
		return false;
	};

	collation_ids[collation_id] = p_auth->get_instance_id();
	return true;
}

bool DefaultClient::connect_session(const Ref<DefaultSession> &session) {
	Ref<WebSocketPeer> peer = ws->get_peer(1);

	if (peer.is_valid() && peer->is_connected_to_host()) {
		emit_signal("request_error", "Client is already connected", FAILED);
		return false;
	}

	String url = Builder::Url()
						 .scheme(ssl ? "wss" : "ws")
						 .host(host)
						 .port(port)
						 .path("/api")
						 .build();

	Error ret = Builder::Request()
						.url(url)
						.query_parameter("token", session->get_auth_token())
						.query_parameter("lang", lang)
						.make(ws, trace);

	if (ret != OK) {
		emit_signal("session_error", "Failed to start a session", ret);
		return false;
	};

	return true;
}

bool DefaultClient::disconnect_session() {
	Ref<WebSocketPeer> peer = ws->get_peer(1);

	if (peer.is_valid()) {
		peer->close(1000);
		return true;
	}

	return false;
}

bool DefaultClient::send(const Ref<NkCollatedMessage> &message) {
	ERR_FAIL_NULL_V(message, false);
	ERR_FAIL_NULL_V(ws, false);

	Ref<WebSocketPeer> peer = ws->get_peer(1);

	if (!peer.is_valid() || !peer->is_connected_to_host()) {
		emit_signal("request_error", "Client is not connected", ERR_UNCONFIGURED);
		return false;
	}

	String collation_id = uuid1().hex();
	PoolByteArray payload = message->as_bytes(collation_id);

	const int len = payload.size();
	if (len == 0) {
		emit_signal("request_error", "Message is empty", ERR_UNCONFIGURED, collation_id);
		return false;
	}
	if (Error ret = ws->put_packet(payload.read().ptr(), len)) {
		emit_signal("request_error", vformat("Failed to send message, make sure the client is connected (Error 0x%x)", ret), FAILED, collation_id);
		return false;
	}

	collation_ids[collation_id] = message->get_instance_id();
	return true;
}

bool DefaultClient::send(const Ref<NkUncollatedMessage> &message) {
	ERR_FAIL_NULL_V(message, false);
	ERR_FAIL_NULL_V(ws, false);

	Ref<WebSocketPeer> peer = ws->get_peer(1);

	if (!peer.is_valid() || !peer->is_connected_to_host()) {
		emit_signal("request_error", "Client is not connected", ERR_UNCONFIGURED);
		return false;
	}

	PoolByteArray payload = message->as_bytes();

	const int len = payload.size();
	if (len == 0) {
		emit_signal("request_error", "Message is empty", ERR_UNCONFIGURED);
		return false;
	}
	if (Error ret = ws->put_packet(payload.read().ptr(), len)) {
		emit_signal("request_error", vformat("Failed to send message, make sure the client is connected (Error 0x%x)", ret), FAILED);
		return false;
	}

	return true;
}

void DefaultClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_rest_request_completed"), &DefaultClient::_rest_request_completed);
	ClassDB::bind_method(D_METHOD("_connection_closed"), &DefaultClient::_ws_connection_closed);
	ClassDB::bind_method(D_METHOD("_connection_error"), &DefaultClient::_ws_connection_error);
	ClassDB::bind_method(D_METHOD("_connection_established"), &DefaultClient::_ws_connection_established);
	ClassDB::bind_method(D_METHOD("_data_received"), &DefaultClient::_ws_data_received);
	ClassDB::bind_method(D_METHOD("_server_close_request"), &DefaultClient::_ws_server_close_request);

	ADD_SIGNAL(MethodInfo("request_error", PropertyInfo(Variant::STRING, "error_message"), PropertyInfo(Variant::INT, "error_code")));

	ADD_SIGNAL(MethodInfo("session_error",
			PropertyInfo(Variant::INT, "error_code"),
			PropertyInfo(Variant::STRING, "error_message"),
			PropertyInfo(Variant::STRING, "collation_id")));
	ADD_SIGNAL(MethodInfo("session_accepted",
			PropertyInfo(Variant::STRING, "token"),
			PropertyInfo(Variant::STRING, "collation_id")));
	ADD_SIGNAL(MethodInfo("session_closed"));

	ADD_SIGNAL(MethodInfo("nk_message",
			PropertyInfo(Variant::INT, "payload_case"),
			PropertyInfo(Variant::OBJECT, "message"),
			PropertyInfo(Variant::STRING, "collation_id")));
}

DefaultClient::DefaultClient(String p_server_key, String p_host, int p_port, bool p_ssl, int p_timeout) {
	host = p_host;
	port = p_port;
	server_key = p_server_key;
	ssl = p_ssl;
	lang = "en";
	trace = false;

	rest = memnew(HTTPRequest);
	rest->set_timeout(p_timeout);
	rest->connect("request_completed", this, "_rest_request_completed");
	ws = WebSocketClient::_create();
	ws->connect("connection_closed", this, "_ws_connection_closed");
	ws->connect("connection_error", this, "_ws_connection_error");
	ws->connect("connection_established", this, "_ws_connection_established");
	ws->connect("data_received", this, "_ws_data_received");
	ws->connect("server_close_request", this, "_ws_server_close_request");
}

DefaultClient::~DefaultClient() {
	if (rest) {
		rest->cancel_request();
		memdelete(rest);
	}

	memdelete(ws);
}

#ifdef DOCTEST
TEST_CASE("Url builder validation") {
	String http_url = Builder::Url()
							  .scheme("http")
							  .host("127.0.0.1")
							  .port(80)
							  .path("/api")
							  .build();
	REQUIRE(http_url == "http://127.0.0.1:80/api");

	String https_url = Builder::Url()
							   .scheme("https")
							   .host("127.0.0.1")
							   .port(80)
							   .path("/api")
							   .build();
	REQUIRE(https_url == "https://127.0.0.1:80/api");
}
#endif
