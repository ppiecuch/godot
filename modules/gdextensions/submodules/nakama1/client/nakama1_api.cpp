/**************************************************************************/
/*  nakama1_api.cpp                                                       */
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

#include "core/variant.h"

#include "nakama1_api.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

/// AuthenticateMessage

// Note: donot mix new with memnew (delete/memdelete)

#define _make_auth_request(what, value) [&]() {                                    \
	using AuthenticateMethodT = server::AuthenticateRequest_::AuthenticateMethodT; \
	using AuthenticateRequestT = server::AuthenticateRequestT;                     \
	AuthenticateMethodT *method = new AuthenticateMethodT;                         \
	method->what = value;                                                          \
	AuthenticateRequestT *request = memnew(AuthenticateRequestT);                  \
	request->id = std::unique_ptr<AuthenticateMethodT>(method);                    \
	return request;                                                                \
}()

#define _make_payload(type, message) [&]() {                                     \
	flatbuffers::FlatBufferBuilder builder(1024);                                \
	builder.Finish(Create##type(builder, message));                              \
	return Utils::create_payload(builder.GetBufferPointer(), builder.GetSize()); \
}()

#define _newauthref(...) \
	memnew(DefaultAuthenticateRequest(_make_auth_request(__VA_ARGS__)))

/// DefaultAuthenticateRequest

Ref<DefaultAuthenticateRequest> DefaultAuthenticateRequest::Builder::custom(String p_custom) {
	return _newauthref(custom, p_custom.ascii());
}

Ref<DefaultAuthenticateRequest> DefaultAuthenticateRequest::Builder::device(String p_device) {
	return _newauthref(device, p_device.ascii());
}

Ref<DefaultAuthenticateRequest> DefaultAuthenticateRequest::Builder::email(String p_email, String p_password) {
	using EmailT = server::AuthenticateRequest_::EmailT;

	EmailT *mail_account = new EmailT;
	mail_account->email = p_email.ascii();
	mail_account->password = p_password.ascii();

	return _newauthref(email, std::unique_ptr<EmailT>(mail_account));
}

Ref<DefaultAuthenticateRequest> DefaultAuthenticateRequest::Builder::facebook(String p_oauth_token) {
	return _newauthref(facebook, p_oauth_token.ascii());
}

Ref<DefaultAuthenticateRequest> DefaultAuthenticateRequest::Builder::game_center(String p_player_id, String p_bundle_id, long p_timestamp, String p_salt, String p_signature, String p_public_key_url) {
	using GameCenterT = server::AuthenticateRequest_::GameCenterT;

	GameCenterT *gamecenter_info = new GameCenterT;
	gamecenter_info->player_id = p_player_id.ascii();
	gamecenter_info->bundle_id = p_bundle_id.ascii();
	gamecenter_info->timestamp = p_timestamp;
	gamecenter_info->signature = p_signature.ascii();
	gamecenter_info->public_key_url = p_public_key_url.ascii();

	return _newauthref(game_center, std::unique_ptr<GameCenterT>(gamecenter_info));
}

Ref<DefaultAuthenticateRequest> DefaultAuthenticateRequest::Builder::google(String p_oauth_token) {
	return _newauthref(google, p_oauth_token.ascii());
}

Ref<DefaultAuthenticateRequest> DefaultAuthenticateRequest::Builder::steam(String p_session_token) {
	return _newauthref(google, p_session_token.ascii());
}

PoolByteArray DefaultAuthenticateRequest::as_bytes(String p_collation_id) const {
	ERR_FAIL_NULL_V(message, PoolByteArray());

	message->collation_id = p_collation_id.ascii();
	return _make_payload(AuthenticateRequest, message);
}

DefaultAuthenticateRequest::DefaultAuthenticateRequest(server::AuthenticateRequestT *p_message) {
	message = p_message;
}

DefaultAuthenticateRequest::~DefaultAuthenticateRequest() {
	if (message) {
		memdelete(message);
	}
}

/// AuthenticateResponse

const AuthenticateResponse *AuthenticateResponse::get() const {
	return message ? this : nullptr;
}

String AuthenticateResponse::get_collation_id() const {
	return message->collation_id()->c_str();
}
bool AuthenticateResponse::is_valid() const {
	return message != nullptr;
}
bool AuthenticateResponse::is_error() const {
	return message->id()->error() != nullptr;
}
NkErrorCode AuthenticateResponse::get_error_code() const {
	return message->id()->error()->code();
}
String AuthenticateResponse::get_error_message() const {
	return message->id()->error()->message()->c_str();
}
bool AuthenticateResponse::is_session() const {
	return message->id()->session() != nullptr;
}
String AuthenticateResponse::get_session_token() const {
	return message->id()->session()->token()->c_str();
}

AuthenticateResponse::AuthenticateResponse(PoolByteArray p_payload) :
		payload(p_payload), message(flatbuffers::GetRoot<server::AuthenticateResponse>(payload.read().ptr())) {
}

/// NkMessage

const NkMessage *NkMessage::get() const {
	return message ? this : nullptr;
}

NkMessage::PayloadCase NkMessage::get_payload_case() const {
	if (const server::Envelope_::EnvelopeContent *payload = get_envelope_payload()) {
		if (payload->heartbeat())
			return PAYLOAD_HEARTBEAT;
		if (payload->topic_message())
			return PAYLOAD_TOPIC_MESSAGE;
		if (payload->topic_presence())
			return PAYLOAD_TOPIC_PRESENCE;
		if (payload->match_data())
			return PAYLOAD_MATCH_DATA;
		if (payload->match_presence())
			return PAYLOAD_MATCH_PRESENCE;
		if (payload->matchmake_matched())
			return PAYLOAD_MATCHMAKE_MATCHED;
		if (payload->live_notifications())
			return PAYLOAD_LIVE_NOTIFICATIONS;
		if (payload->error())
			return PAYLOAD_ERROR;
		if (payload->self())
			return PAYLOAD_SELF;
		if (payload->users())
			return PAYLOAD_USERS;
		if (payload->friends())
			return PAYLOAD_FRIENDS;
		if (payload->groups())
			return PAYLOAD_GROUPS;
		if (payload->groups_self())
			return PAYLOAD_GROUPS_SELF;
		if (payload->group_users())
			return PAYLOAD_GROUP_USERS;
		if (payload->storage_data())
			return PAYLOAD_STORAGE_DATA;
		if (payload->storage_keys())
			return PAYLOAD_STORAGE_KEYS;
		if (payload->rpc())
			return PAYLOAD_RPC;
		if (payload->topics())
			return PAYLOAD_TOPICS;
		if (payload->topic_message_ack())
			return PAYLOAD_TOPIC_MESSAGE_ACK;
		if (payload->topic_messages())
			return PAYLOAD_TOPIC_MESSAGES;
		if (payload->match())
			return PAYLOAD_MATCH;
		if (payload->matches())
			return PAYLOAD_MATCHES;
		if (payload->matchmake_ticket())
			return PAYLOAD_MATCHMAKE_TICKET;
		if (payload->leaderboards())
			return PAYLOAD_LEADERBOARDS;
		if (payload->leaderboard_records())
			return PAYLOAD_LEADERBOARD_RECORDS;
		if (payload->notifications())
			return PAYLOAD_NOTIFICATIONS;
	}
	return PAYLOAD_NOT_SET;
}

String NkMessage::get_collation_id() const {
	ERR_FAIL_NULL_V(message, String());
	ERR_FAIL_NULL_V(message->collation_id(), String());

	return message->collation_id()->c_str();
}

const server::Envelope_::EnvelopeContent *NkMessage::get_envelope_payload() const {
	ERR_FAIL_NULL_V(message, nullptr);
	ERR_FAIL_NULL_V(message->payload(), nullptr);

	return flatbuffers::GetRoot<server::Envelope_::EnvelopeContent>(message);
}

void NkMessage::_bind_methods() {
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_NOT_SET);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_HEARTBEAT);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_TOPIC_MESSAGE);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_TOPIC_PRESENCE);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_MATCH_DATA);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_MATCH_PRESENCE);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_MATCHMAKE_MATCHED);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_LIVE_NOTIFICATIONS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_ERROR);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_SELF);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_USERS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_FRIENDS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_GROUPS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_GROUPS_SELF);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_GROUP_USERS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_STORAGE_DATA);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_STORAGE_KEYS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_RPC);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_TOPICS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_TOPIC_MESSAGE_ACK);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_TOPIC_MESSAGES);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_MATCH);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_MATCHES);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_MATCHMAKE_TICKET);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_LEADERBOARDS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_LEADERBOARD_RECORDS);
	BIND_ENUM_CONSTANT(NkMessage::PAYLOAD_NOTIFICATIONS);

	BIND_ENUM_CONSTANT(NkMessage::TOPIC_DIRECTMESSAGE);
	BIND_ENUM_CONSTANT(NkMessage::TOPIC_ROOM);
	BIND_ENUM_CONSTANT(NkMessage::TOPIC_GROUP);

	BIND_ENUM_CONSTANT(NkMessage::SUBMITOP_INCR);
	BIND_ENUM_CONSTANT(NkMessage::SUBMITOP_DECR);
	BIND_ENUM_CONSTANT(NkMessage::SUBMITOP_SET);
	BIND_ENUM_CONSTANT(NkMessage::SUBMITOP_BEST);
}

NkMessage::NkMessage(PoolByteArray p_payload) :
		payload(p_payload), message(flatbuffers::GetRoot<server::Envelope>(payload.read().ptr())) {
	ERR_FAIL_NULL(message);
}

/// LogoutMessage

LogoutMessage::LogoutMessage() {
	server::LogoutT logout;
	payload = _make_payload(Logout, &logout);
}

#ifdef DOCTEST
TEST_CASE("Check authenticate request messages") {
	Ref<DefaultAuthenticateRequest> custom_auth = DefaultAuthenticateRequest::Builder::custom("customid");
	Ref<DefaultAuthenticateRequest> device_auth = DefaultAuthenticateRequest::Builder::device("deviceid");
	Ref<DefaultAuthenticateRequest> email_auth = DefaultAuthenticateRequest::Builder::email("email@acme.com", "password");
	Ref<DefaultAuthenticateRequest> facebook_auth = DefaultAuthenticateRequest::Builder::facebook("token");
	Ref<DefaultAuthenticateRequest> game_center_auth = DefaultAuthenticateRequest::Builder::game_center("playerid", "bundleid", 1, "salt", "signature", "publickeyurl");
	Ref<DefaultAuthenticateRequest> google_auth = DefaultAuthenticateRequest::Builder::google("token");
	Ref<DefaultAuthenticateRequest> steam_auth = DefaultAuthenticateRequest::Builder::steam("token");

	REQUIRE(custom_auth != nullptr);
	REQUIRE(device_auth != nullptr);
	REQUIRE(email_auth != nullptr);
	REQUIRE(facebook_auth != nullptr);
	REQUIRE(game_center_auth != nullptr);
	REQUIRE(google_auth != nullptr);
	REQUIRE(steam_auth != nullptr);

	SUBCASE("Message is not empty") {
		REQUIRE(custom_auth->as_bytes("msgid").size() > 0);
		REQUIRE(device_auth->as_bytes("msgid").size() > 0);
		REQUIRE(email_auth->as_bytes("msgid").size() > 0);
		REQUIRE(facebook_auth->as_bytes("msgid").size() > 0);
		REQUIRE(game_center_auth->as_bytes("msgid").size() > 0);
		REQUIRE(google_auth->as_bytes("msgid").size() > 0);
		REQUIRE(steam_auth->as_bytes("msgid").size() > 0);
	}

	SUBCASE("Message is valid") {
		REQUIRE(flatbuffers::GetRoot<server::AuthenticateRequest>(custom_auth->as_bytes("msgid").read().ptr()) != nullptr);
		REQUIRE(flatbuffers::GetRoot<server::AuthenticateRequest>(device_auth->as_bytes("msgid").read().ptr()) != nullptr);
		REQUIRE(flatbuffers::GetRoot<server::AuthenticateRequest>(email_auth->as_bytes("msgid").read().ptr()) != nullptr);
		REQUIRE(flatbuffers::GetRoot<server::AuthenticateRequest>(facebook_auth->as_bytes("msgid").read().ptr()) != nullptr);
		REQUIRE(flatbuffers::GetRoot<server::AuthenticateRequest>(game_center_auth->as_bytes("msgid").read().ptr()) != nullptr);
		REQUIRE(flatbuffers::GetRoot<server::AuthenticateRequest>(google_auth->as_bytes("msgid").read().ptr()) != nullptr);
		REQUIRE(flatbuffers::GetRoot<server::AuthenticateRequest>(steam_auth->as_bytes("msgid").read().ptr()) != nullptr);
	}
}

TEST_CASE("Check outgoing messages") {
	SUBCASE("Logout message is valid") {
		PoolByteArray payload = LogoutMessage().as_bytes();
		REQUIRE(payload.size() > 0);
		REQUIRE(flatbuffers::GetRoot<server::Logout>(payload.read().ptr()) != nullptr);
	}
}
#endif // DOCTEST
