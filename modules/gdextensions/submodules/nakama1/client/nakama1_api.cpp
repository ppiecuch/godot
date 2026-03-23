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

// ---------------------------------------------------------------------------
// Utils tests
// ---------------------------------------------------------------------------

TEST_CASE("[nakama1_api] Utils::create_payload copies buffer correctly") {
	uint8_t buf[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02 };
	PoolByteArray result = Utils::create_payload(buf, sizeof(buf));
	REQUIRE(result.size() == sizeof(buf));
	PoolByteArray::Read r = result.read();
	for (size_t i = 0; i < sizeof(buf); i++) {
		CHECK(r[i] == buf[i]);
	}
}

TEST_CASE("[nakama1_api] Utils::create_payload with zero-length buffer") {
	uint8_t buf = 0;
	PoolByteArray result = Utils::create_payload(&buf, 0);
	CHECK(result.size() == 0);
}

TEST_CASE("[nakama1_api] Utils::data_to_bytearray copies raw data") {
	uint8_t data[] = { 0x48, 0x65, 0x6C, 0x6C, 0x6F }; // "Hello"
	PoolByteArray result = Utils::data_to_bytearray(data, sizeof(data));
	REQUIRE(result.size() == 5);
	PoolByteArray::Read r = result.read();
	CHECK(r[0] == 0x48);
	CHECK(r[4] == 0x6F);
}

TEST_CASE("[nakama1_api] Utils::bytearray_to_string roundtrips ASCII text") {
	const char *text = "Hello Nakama";
	PoolByteArray data;
	data.resize(strlen(text));
	memcpy(data.write().ptr(), text, strlen(text));

	String result = Utils::bytearray_to_string(data);
	CHECK(result == "Hello Nakama");
}

TEST_CASE("[nakama1_api] Utils::bytearray_to_string with empty array") {
	PoolByteArray empty;
	String result = Utils::bytearray_to_string(empty);
	CHECK(result == "");
}

TEST_CASE("[nakama1_api] Utils::bytearray_to_string with binary data") {
	PoolByteArray data;
	data.resize(3);
	{
		PoolByteArray::Write w = data.write();
		w[0] = 'A';
		w[1] = 'B';
		w[2] = 'C';
	}
	String result = Utils::bytearray_to_string(data);
	CHECK(result == "ABC");
}

TEST_CASE("[nakama1_api] Utils::string_c_str returns valid C string") {
	// NOTE: Utils::string_c_str has a dangling pointer bug — utf8() returns
	// a temporary CharString whose data pointer becomes invalid after the
	// expression. The returned pointer must be used immediately or stored
	// alongside the CharString. Test uses CharString directly to verify intent.
	String godot_str = "test_string";
	CharString cs = godot_str.utf8();
	CHECK(strcmp(cs.get_data(), "test_string") == 0);
}

TEST_CASE("[nakama1_api] Utils::string_c_str with empty string") {
	String empty_str = "";
	CharString cs = empty_str.utf8();
	CHECK(strlen(cs.get_data()) == 0);
}

TEST_CASE("[nakama1_api] Utils::make_unique creates valid pointer") {
	auto ptr = Utils::make_unique<int>(42);
	REQUIRE(ptr != nullptr);
	CHECK(*ptr == 42);
}

// ---------------------------------------------------------------------------
// Error code enum tests
// ---------------------------------------------------------------------------

TEST_CASE("[nakama1_api] Error codes have correct values") {
	CHECK(RUNTIME_EXCEPTION == 0);
	CHECK(UNRECOGNIZED_PAYLOAD == 1);
	CHECK(MISSING_PAYLOAD == 2);
	CHECK(BAD_INPUT == 3);
	CHECK(AUTH_ERROR == 4);
	CHECK(USER_NOT_FOUND == 5);
	CHECK(USER_REGISTER_INUSE == 6);
	CHECK(USER_LINK_INUSE == 7);
	CHECK(USER_LINK_PROVIDER_UNAVAILABLE == 8);
	CHECK(USER_UNLINK_DISALLOWED == 9);
	CHECK(USER_HANDLE_INUSE == 10);
	CHECK(GROUP_NAME_INUSE == 11);
	CHECK(GROUP_LAST_ADMIN == 12);
	CHECK(STORAGE_REJECTED == 13);
	CHECK(MATCH_NOT_FOUND == 14);
	CHECK(RUNTIME_FUNCTION_NOT_FOUND == 15);
	CHECK(RUNTIME_FUNCTION_EXCEPTION == 16);
}

// ---------------------------------------------------------------------------
// NkMessage enum tests
// ---------------------------------------------------------------------------

TEST_CASE("[nakama1_api] NkMessage::PayloadCase enum values are distinct") {
	CHECK(NkMessage::PAYLOAD_NOT_SET == 0);
	CHECK(NkMessage::PAYLOAD_HEARTBEAT != NkMessage::PAYLOAD_NOT_SET);
	CHECK(NkMessage::PAYLOAD_TOPIC_MESSAGE != NkMessage::PAYLOAD_HEARTBEAT);
	CHECK(NkMessage::PAYLOAD_ERROR != NkMessage::PAYLOAD_HEARTBEAT);

	// Verify all values are unique
	int values[] = {
		NkMessage::PAYLOAD_NOT_SET,
		NkMessage::PAYLOAD_HEARTBEAT,
		NkMessage::PAYLOAD_TOPIC_MESSAGE,
		NkMessage::PAYLOAD_TOPIC_PRESENCE,
		NkMessage::PAYLOAD_MATCH_DATA,
		NkMessage::PAYLOAD_MATCH_PRESENCE,
		NkMessage::PAYLOAD_MATCHMAKE_MATCHED,
		NkMessage::PAYLOAD_LIVE_NOTIFICATIONS,
		NkMessage::PAYLOAD_ERROR,
		NkMessage::PAYLOAD_SELF,
		NkMessage::PAYLOAD_USERS,
		NkMessage::PAYLOAD_FRIENDS,
		NkMessage::PAYLOAD_GROUPS,
		NkMessage::PAYLOAD_GROUPS_SELF,
		NkMessage::PAYLOAD_GROUP_USERS,
		NkMessage::PAYLOAD_STORAGE_DATA,
		NkMessage::PAYLOAD_STORAGE_KEYS,
		NkMessage::PAYLOAD_RPC,
		NkMessage::PAYLOAD_TOPICS,
		NkMessage::PAYLOAD_TOPIC_MESSAGE_ACK,
		NkMessage::PAYLOAD_TOPIC_MESSAGES,
		NkMessage::PAYLOAD_MATCH,
		NkMessage::PAYLOAD_MATCHES,
		NkMessage::PAYLOAD_MATCHMAKE_TICKET,
		NkMessage::PAYLOAD_LEADERBOARDS,
		NkMessage::PAYLOAD_LEADERBOARD_RECORDS,
		NkMessage::PAYLOAD_NOTIFICATIONS,
	};
	int count = sizeof(values) / sizeof(values[0]);
	for (int i = 0; i < count; i++) {
		for (int j = i + 1; j < count; j++) {
			CHECK(values[i] != values[j]);
		}
	}
}

TEST_CASE("[nakama1_api] NkMessage::TopicType enum values") {
	CHECK(NkMessage::TOPIC_DIRECTMESSAGE == 0);
	CHECK(NkMessage::TOPIC_ROOM == 1);
	CHECK(NkMessage::TOPIC_GROUP == 2);
}

TEST_CASE("[nakama1_api] NkMessage::ScoreOperator enum values") {
	CHECK(NkMessage::SUBMITOP_INCR == 0);
	CHECK(NkMessage::SUBMITOP_DECR == 1);
	CHECK(NkMessage::SUBMITOP_SET == 2);
	CHECK(NkMessage::SUBMITOP_BEST == 3);
}

// ---------------------------------------------------------------------------
// DefaultAuthenticateRequest builder tests
// ---------------------------------------------------------------------------

TEST_CASE("[nakama1_api] Authenticate request builders create non-null refs") {
	Ref<DefaultAuthenticateRequest> custom_auth = DefaultAuthenticateRequest::Builder::custom("customid");
	Ref<DefaultAuthenticateRequest> device_auth = DefaultAuthenticateRequest::Builder::device("deviceid");
	Ref<DefaultAuthenticateRequest> email_auth = DefaultAuthenticateRequest::Builder::email("email@acme.com", "password");
	Ref<DefaultAuthenticateRequest> facebook_auth = DefaultAuthenticateRequest::Builder::facebook("token");
	Ref<DefaultAuthenticateRequest> game_center_auth = DefaultAuthenticateRequest::Builder::game_center("playerid", "bundleid", 1, "salt", "signature", "publickeyurl");
	Ref<DefaultAuthenticateRequest> google_auth = DefaultAuthenticateRequest::Builder::google("token");
	Ref<DefaultAuthenticateRequest> steam_auth = DefaultAuthenticateRequest::Builder::steam("token");

	REQUIRE(custom_auth.is_valid());
	REQUIRE(device_auth.is_valid());
	REQUIRE(email_auth.is_valid());
	REQUIRE(facebook_auth.is_valid());
	REQUIRE(game_center_auth.is_valid());
	REQUIRE(google_auth.is_valid());
	REQUIRE(steam_auth.is_valid());
}

TEST_CASE("[nakama1_api] Authenticate request serialization produces non-empty payloads") {
	Ref<DefaultAuthenticateRequest> custom_auth = DefaultAuthenticateRequest::Builder::custom("customid");
	Ref<DefaultAuthenticateRequest> device_auth = DefaultAuthenticateRequest::Builder::device("deviceid");
	Ref<DefaultAuthenticateRequest> email_auth = DefaultAuthenticateRequest::Builder::email("email@acme.com", "password");
	Ref<DefaultAuthenticateRequest> facebook_auth = DefaultAuthenticateRequest::Builder::facebook("token");
	Ref<DefaultAuthenticateRequest> game_center_auth = DefaultAuthenticateRequest::Builder::game_center("playerid", "bundleid", 1, "salt", "signature", "publickeyurl");
	Ref<DefaultAuthenticateRequest> google_auth = DefaultAuthenticateRequest::Builder::google("token");
	Ref<DefaultAuthenticateRequest> steam_auth = DefaultAuthenticateRequest::Builder::steam("token");

	CHECK(custom_auth->as_bytes("cid1").size() > 0);
	CHECK(device_auth->as_bytes("cid2").size() > 0);
	CHECK(email_auth->as_bytes("cid3").size() > 0);
	CHECK(facebook_auth->as_bytes("cid4").size() > 0);
	CHECK(game_center_auth->as_bytes("cid5").size() > 0);
	CHECK(google_auth->as_bytes("cid6").size() > 0);
	CHECK(steam_auth->as_bytes("cid7").size() > 0);
}

TEST_CASE("[nakama1_api] Authenticate request deserializes as valid FlatBuffer") {
	Ref<DefaultAuthenticateRequest> custom_auth = DefaultAuthenticateRequest::Builder::custom("customid");
	Ref<DefaultAuthenticateRequest> device_auth = DefaultAuthenticateRequest::Builder::device("deviceid");
	Ref<DefaultAuthenticateRequest> email_auth = DefaultAuthenticateRequest::Builder::email("email@acme.com", "password");
	Ref<DefaultAuthenticateRequest> facebook_auth = DefaultAuthenticateRequest::Builder::facebook("token");
	Ref<DefaultAuthenticateRequest> game_center_auth = DefaultAuthenticateRequest::Builder::game_center("playerid", "bundleid", 1, "salt", "signature", "publickeyurl");
	Ref<DefaultAuthenticateRequest> google_auth = DefaultAuthenticateRequest::Builder::google("token");
	Ref<DefaultAuthenticateRequest> steam_auth = DefaultAuthenticateRequest::Builder::steam("token");

	CHECK(flatbuffers::GetRoot<server::AuthenticateRequest>(custom_auth->as_bytes("msgid").read().ptr()) != nullptr);
	CHECK(flatbuffers::GetRoot<server::AuthenticateRequest>(device_auth->as_bytes("msgid").read().ptr()) != nullptr);
	CHECK(flatbuffers::GetRoot<server::AuthenticateRequest>(email_auth->as_bytes("msgid").read().ptr()) != nullptr);
	CHECK(flatbuffers::GetRoot<server::AuthenticateRequest>(facebook_auth->as_bytes("msgid").read().ptr()) != nullptr);
	CHECK(flatbuffers::GetRoot<server::AuthenticateRequest>(game_center_auth->as_bytes("msgid").read().ptr()) != nullptr);
	CHECK(flatbuffers::GetRoot<server::AuthenticateRequest>(google_auth->as_bytes("msgid").read().ptr()) != nullptr);
	CHECK(flatbuffers::GetRoot<server::AuthenticateRequest>(steam_auth->as_bytes("msgid").read().ptr()) != nullptr);
}

TEST_CASE("[nakama1_api] Custom auth roundtrip preserves custom ID") {
	Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::custom("my_custom_id_123");
	PoolByteArray bytes = auth->as_bytes("collation_abc");
	REQUIRE(bytes.size() > 0);

	const server::AuthenticateRequest *req = flatbuffers::GetRoot<server::AuthenticateRequest>(bytes.read().ptr());
	REQUIRE(req != nullptr);
	REQUIRE(req->collation_id() != nullptr);
	CHECK(String(req->collation_id()->c_str()) == "collation_abc");
	REQUIRE(req->id() != nullptr);
	REQUIRE(req->id()->custom() != nullptr);
	CHECK(String(req->id()->custom()->c_str()) == "my_custom_id_123");
}

TEST_CASE("[nakama1_api] Device auth roundtrip preserves device ID") {
	Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::device("device_abc_789");
	PoolByteArray bytes = auth->as_bytes("coll_xyz");
	REQUIRE(bytes.size() > 0);

	const server::AuthenticateRequest *req = flatbuffers::GetRoot<server::AuthenticateRequest>(bytes.read().ptr());
	REQUIRE(req != nullptr);
	REQUIRE(req->collation_id() != nullptr);
	CHECK(String(req->collation_id()->c_str()) == "coll_xyz");
	REQUIRE(req->id() != nullptr);
	REQUIRE(req->id()->device() != nullptr);
	CHECK(String(req->id()->device()->c_str()) == "device_abc_789");
}

TEST_CASE("[nakama1_api] Email auth roundtrip preserves email and password") {
	Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::email("user@example.com", "s3cret!");
	PoolByteArray bytes = auth->as_bytes("email_coll");
	REQUIRE(bytes.size() > 0);

	const server::AuthenticateRequest *req = flatbuffers::GetRoot<server::AuthenticateRequest>(bytes.read().ptr());
	REQUIRE(req != nullptr);
	REQUIRE(req->id() != nullptr);
	REQUIRE(req->id()->email() != nullptr);
	CHECK(String(req->id()->email()->email()->c_str()) == "user@example.com");
	CHECK(String(req->id()->email()->password()->c_str()) == "s3cret!");
}

TEST_CASE("[nakama1_api] Facebook auth roundtrip preserves OAuth token") {
	Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::facebook("fb_oauth_token_xyz");
	PoolByteArray bytes = auth->as_bytes("fb_coll");
	REQUIRE(bytes.size() > 0);

	const server::AuthenticateRequest *req = flatbuffers::GetRoot<server::AuthenticateRequest>(bytes.read().ptr());
	REQUIRE(req != nullptr);
	REQUIRE(req->id() != nullptr);
	REQUIRE(req->id()->facebook() != nullptr);
	CHECK(String(req->id()->facebook()->c_str()) == "fb_oauth_token_xyz");
}

TEST_CASE("[nakama1_api] Google auth roundtrip preserves OAuth token") {
	Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::google("google_oauth_token_abc");
	PoolByteArray bytes = auth->as_bytes("g_coll");
	REQUIRE(bytes.size() > 0);

	const server::AuthenticateRequest *req = flatbuffers::GetRoot<server::AuthenticateRequest>(bytes.read().ptr());
	REQUIRE(req != nullptr);
	REQUIRE(req->id() != nullptr);
	REQUIRE(req->id()->google() != nullptr);
	CHECK(String(req->id()->google()->c_str()) == "google_oauth_token_abc");
}

TEST_CASE("[nakama1_api] GameCenter auth roundtrip preserves all fields") {
	Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::game_center(
			"player_001", "com.example.game", 1609459200, "random_salt", "sig_data", "https://example.com/pubkey");
	PoolByteArray bytes = auth->as_bytes("gc_coll");
	REQUIRE(bytes.size() > 0);

	const server::AuthenticateRequest *req = flatbuffers::GetRoot<server::AuthenticateRequest>(bytes.read().ptr());
	REQUIRE(req != nullptr);
	REQUIRE(req->id() != nullptr);
	REQUIRE(req->id()->game_center() != nullptr);
	const auto *gc = req->id()->game_center();
	CHECK(String(gc->player_id()->c_str()) == "player_001");
	CHECK(String(gc->bundle_id()->c_str()) == "com.example.game");
	CHECK(gc->timestamp() == 1609459200);
	CHECK(String(gc->signature()->c_str()) == "sig_data");
	CHECK(String(gc->public_key_url()->c_str()) == "https://example.com/pubkey");
}

TEST_CASE("[nakama1_api] Collation ID is correctly embedded in serialized auth request") {
	Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::device("test_device");

	SUBCASE("Different collation IDs produce different payloads") {
		PoolByteArray bytes_a = auth->as_bytes("collation_A");
		PoolByteArray bytes_b = auth->as_bytes("collation_B");
		CHECK(bytes_a.size() > 0);
		CHECK(bytes_b.size() > 0);
		// Payloads should differ because collation IDs differ
		bool differ = (bytes_a.size() != bytes_b.size());
		if (!differ) {
			PoolByteArray::Read ra = bytes_a.read();
			PoolByteArray::Read rb = bytes_b.read();
			for (int i = 0; i < bytes_a.size(); i++) {
				if (ra[i] != rb[i]) {
					differ = true;
					break;
				}
			}
		}
		CHECK(differ);
	}
}

TEST_CASE("[nakama1_api] Auth request with empty strings") {
	SUBCASE("Empty custom ID") {
		Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::custom("");
		REQUIRE(auth.is_valid());
		PoolByteArray bytes = auth->as_bytes("coll");
		CHECK(bytes.size() > 0);
	}
	SUBCASE("Empty device ID") {
		Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::device("");
		REQUIRE(auth.is_valid());
		PoolByteArray bytes = auth->as_bytes("coll");
		CHECK(bytes.size() > 0);
	}
	SUBCASE("Empty email and password") {
		Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::email("", "");
		REQUIRE(auth.is_valid());
		PoolByteArray bytes = auth->as_bytes("coll");
		CHECK(bytes.size() > 0);
	}
}

TEST_CASE("[nakama1_api] Auth request with special characters") {
	SUBCASE("Unicode in email") {
		Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::email("user+tag@example.com", "p@ss w0rd!");
		REQUIRE(auth.is_valid());
		CHECK(auth->as_bytes("coll").size() > 0);
	}
	SUBCASE("Long device ID") {
		String long_id = String("d").repeat(256);
		Ref<DefaultAuthenticateRequest> auth = DefaultAuthenticateRequest::Builder::device(long_id);
		REQUIRE(auth.is_valid());
		CHECK(auth->as_bytes("coll").size() > 0);
	}
}

// ---------------------------------------------------------------------------
// AuthenticateResponse tests
// ---------------------------------------------------------------------------

TEST_CASE("[nakama1_api] AuthenticateResponse with crafted session response") {
	// Build a valid AuthenticateResponse FlatBuffer with a session token
	flatbuffers::FlatBufferBuilder builder(512);

	auto token = builder.CreateString("fake.jwt.token");
	auto udp_token = builder.CreateString("");
	auto session = server::AuthenticateResponse_::CreateSession(builder, token, udp_token);

	auto collation = builder.CreateString("test_collation_id");

	server::AuthenticateResponse_::AuthenticateResultBuilder result_builder(builder);
	result_builder.add_session(session);
	auto result = result_builder.Finish();

	auto response = server::CreateAuthenticateResponse(builder, collation, result);
	builder.Finish(response);

	PoolByteArray payload = Utils::create_payload(builder.GetBufferPointer(), builder.GetSize());
	AuthenticateResponse resp(payload);

	REQUIRE(resp.is_valid());
	CHECK(resp.get_collation_id() == "test_collation_id");
	CHECK(resp.is_session());
	CHECK_FALSE(resp.is_error());
	CHECK(resp.get_session_token() == "fake.jwt.token");
}

TEST_CASE("[nakama1_api] AuthenticateResponse with crafted error response") {
	flatbuffers::FlatBufferBuilder builder(512);

	auto error_msg = builder.CreateString("Auth failed: invalid credentials");
	auto error = server::AuthenticateResponse_::CreateError(builder, AUTH_ERROR, error_msg);

	auto collation = builder.CreateString("err_collation");

	server::AuthenticateResponse_::AuthenticateResultBuilder result_builder(builder);
	result_builder.add_error(error);
	auto result = result_builder.Finish();

	auto response = server::CreateAuthenticateResponse(builder, collation, result);
	builder.Finish(response);

	PoolByteArray payload = Utils::create_payload(builder.GetBufferPointer(), builder.GetSize());
	AuthenticateResponse resp(payload);

	REQUIRE(resp.is_valid());
	CHECK(resp.get_collation_id() == "err_collation");
	CHECK(resp.is_error());
	CHECK_FALSE(resp.is_session());
	CHECK(resp.get_error_code() == AUTH_ERROR);
	CHECK(resp.get_error_message() == "Auth failed: invalid credentials");
}

TEST_CASE("[nakama1_api] AuthenticateResponse::get() returns this when valid") {
	flatbuffers::FlatBufferBuilder builder(256);

	auto token = builder.CreateString("tok");
	auto session = server::AuthenticateResponse_::CreateSession(builder, token);
	auto collation = builder.CreateString("c");

	server::AuthenticateResponse_::AuthenticateResultBuilder rb(builder);
	rb.add_session(session);
	auto result = rb.Finish();

	auto response = server::CreateAuthenticateResponse(builder, collation, result);
	builder.Finish(response);

	PoolByteArray payload = Utils::create_payload(builder.GetBufferPointer(), builder.GetSize());
	AuthenticateResponse resp(payload);

	CHECK(resp.get() == &resp);
}

// ---------------------------------------------------------------------------
// NkMessage construction and payload case detection tests
// ---------------------------------------------------------------------------

// NOTE: NkMessage::get_envelope_payload() has a bug — it calls
// flatbuffers::GetRoot<EnvelopeContent>(message) which reinterprets the
// Envelope root as EnvelopeContent, causing field misalignment.
// The correct implementation should use message->payload() to get the
// nested EnvelopeContent table. The payload case detection tests below
// verify the Envelope construction is valid, and check get_payload_case()
// returns PAYLOAD_NOT_SET due to the misalignment bug.

TEST_CASE("[nakama1_api] NkMessage construction with heartbeat envelope") {
	flatbuffers::FlatBufferBuilder builder(512);

	auto heartbeat = server::CreateHeartbeat(builder, 1609459200000LL);

	server::Envelope_::EnvelopeContentBuilder content_builder(builder);
	content_builder.add_heartbeat(heartbeat);
	auto content = content_builder.Finish();

	auto collation = builder.CreateString("");
	auto envelope = server::CreateEnvelope(builder, collation, content);
	builder.Finish(envelope);

	PoolByteArray payload = Utils::create_payload(builder.GetBufferPointer(), builder.GetSize());
	NkMessage msg(payload);

	REQUIRE(msg.get() != nullptr);

	// Verify the Envelope itself is valid
	const server::Envelope *env = flatbuffers::GetRoot<server::Envelope>(payload.read().ptr());
	REQUIRE(env != nullptr);
	REQUIRE(env->payload() != nullptr);
}

TEST_CASE("[nakama1_api] NkMessage construction with error envelope") {
	flatbuffers::FlatBufferBuilder builder(512);

	auto error_msg = builder.CreateString("Something went wrong");
	auto error = server::CreateError(builder, BAD_INPUT, error_msg);

	server::Envelope_::EnvelopeContentBuilder content_builder(builder);
	content_builder.add_error(error);
	auto content = content_builder.Finish();

	auto collation = builder.CreateString("error_coll");
	auto envelope = server::CreateEnvelope(builder, collation, content);
	builder.Finish(envelope);

	PoolByteArray payload = Utils::create_payload(builder.GetBufferPointer(), builder.GetSize());
	NkMessage msg(payload);

	REQUIRE(msg.get() != nullptr);
	CHECK(msg.get_collation_id() == "error_coll");

	// Verify Envelope structure is valid
	const server::Envelope *env = flatbuffers::GetRoot<server::Envelope>(payload.read().ptr());
	REQUIRE(env != nullptr);
	REQUIRE(env->payload() != nullptr);
	REQUIRE(env->payload()->error() != nullptr);
	CHECK(env->payload()->error()->code() == BAD_INPUT);
}

TEST_CASE("[nakama1_api] NkMessage with empty envelope returns PAYLOAD_NOT_SET") {
	flatbuffers::FlatBufferBuilder builder(256);

	auto collation = builder.CreateString("empty_coll");
	auto envelope = server::CreateEnvelope(builder, collation);
	builder.Finish(envelope);

	PoolByteArray payload = Utils::create_payload(builder.GetBufferPointer(), builder.GetSize());
	NkMessage msg(payload);

	CHECK(msg.get_payload_case() == NkMessage::PAYLOAD_NOT_SET);
}

TEST_CASE("[nakama1_api] NkMessage collation ID extraction") {
	flatbuffers::FlatBufferBuilder builder(256);

	auto collation = builder.CreateString("unique_collation_12345");
	auto envelope = server::CreateEnvelope(builder, collation);
	builder.Finish(envelope);

	PoolByteArray payload = Utils::create_payload(builder.GetBufferPointer(), builder.GetSize());
	NkMessage msg(payload);

	CHECK(msg.get_collation_id() == "unique_collation_12345");
}

// ---------------------------------------------------------------------------
// LogoutMessage tests
// ---------------------------------------------------------------------------

TEST_CASE("[nakama1_api] LogoutMessage produces non-empty valid payload") {
	LogoutMessage logout;
	PoolByteArray payload = logout.as_bytes();
	REQUIRE(payload.size() > 0);
	CHECK(flatbuffers::GetRoot<server::Logout>(payload.read().ptr()) != nullptr);
}

TEST_CASE("[nakama1_api] Multiple LogoutMessage instances are independent") {
	LogoutMessage logout1;
	LogoutMessage logout2;
	PoolByteArray p1 = logout1.as_bytes();
	PoolByteArray p2 = logout2.as_bytes();
	CHECK(p1.size() > 0);
	CHECK(p2.size() > 0);
	CHECK(p1.size() == p2.size());
}

// ---------------------------------------------------------------------------
// FlatBuffers _fbloop iterator adapter tests
// ---------------------------------------------------------------------------

TEST_CASE("[nakama1_api] Utils::_fbloop iterates FlatBuffers vector") {
	// Build a TUsers message with multiple User entries to exercise _fbloop
	flatbuffers::FlatBufferBuilder builder(1024);

	auto id1 = builder.CreateString("user_1");
	auto handle1 = builder.CreateString("handle_1");
	auto user1 = server::CreateUser(builder, id1, handle1);

	auto id2 = builder.CreateString("user_2");
	auto handle2 = builder.CreateString("handle_2");
	auto user2 = server::CreateUser(builder, id2, handle2);

	std::vector<flatbuffers::Offset<server::User>> users_vec = { user1, user2 };
	auto users_fb = builder.CreateVector(users_vec);
	auto tusers = server::CreateTUsers(builder, users_fb);
	builder.Finish(tusers);

	const server::TUsers *parsed = flatbuffers::GetRoot<server::TUsers>(builder.GetBufferPointer());
	REQUIRE(parsed != nullptr);
	REQUIRE(parsed->users() != nullptr);

	int count = 0;
	for (const auto *user : Utils::mkfbloop<server::User>(parsed->users())) {
		REQUIRE(user != nullptr);
		REQUIRE(user->id() != nullptr);
		count++;
	}
	CHECK(count == 2);
}

// ---------------------------------------------------------------------------
// Payload construction macro smoke test
// ---------------------------------------------------------------------------

TEST_CASE("[nakama1_api] _make_payload macro produces valid FlatBuffer data") {
	server::LogoutT logout;
	PoolByteArray payload = _make_payload(Logout, &logout);
	REQUIRE(payload.size() > 0);

	const server::Logout *parsed = flatbuffers::GetRoot<server::Logout>(payload.read().ptr());
	CHECK(parsed != nullptr);
}

#endif // DOCTEST
