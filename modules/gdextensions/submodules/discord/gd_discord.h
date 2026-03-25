/**************************************************************************/
/*  gd_discord.h                                                          */
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

#ifndef GD_DISCORD_H
#define GD_DISCORD_H

#include "core/io/json.h"
#include "scene/main/http_request.h"
#include "scene/main/node.h"

#ifdef MODULE_WEBSOCKET_ENABLED
#include "modules/websocket/websocket_client.h"
#endif

#define GODOTDISCORD_MAJOR 0
#define GODOTDISCORD_MINOR 1
#define GODOTDISCORD_PATCH 0

#define _GD_STR(x) #x
#define _GD_XSTR(x) _GD_STR(x)
#define GODOTDISCORD_VERSION _GD_XSTR(GODOTDISCORD_MAJOR) "." _GD_XSTR(GODOTDISCORD_MINOR) "." _GD_XSTR(GODOTDISCORD_PATCH)
#define GODOTDISCORD_USER_AGENT "godot-discord (" GODOTDISCORD_VERSION ")"

class GdDiscordClient : public Node {
	GDCLASS(GdDiscordClient, Node);

public:
	enum ConnectionState {
		STATE_DISCONNECTED,
		STATE_CONNECTING,
		STATE_CONNECTED,
		STATE_RECONNECTING,
	};

	enum Intent {
		INTENT_GUILDS = 1 << 0,
		INTENT_GUILD_MEMBERS = 1 << 1,
		INTENT_GUILD_BANS = 1 << 2,
		INTENT_GUILD_EMOJIS = 1 << 3,
		INTENT_GUILD_INTEGRATIONS = 1 << 4,
		INTENT_GUILD_WEBHOOKS = 1 << 5,
		INTENT_GUILD_INVITES = 1 << 6,
		INTENT_GUILD_VOICE_STATES = 1 << 7,
		INTENT_GUILD_PRESENCES = 1 << 8,
		INTENT_GUILD_MESSAGES = 1 << 9,
		INTENT_GUILD_MESSAGE_REACTIONS = 1 << 10,
		INTENT_GUILD_MESSAGE_TYPING = 1 << 11,
		INTENT_DIRECT_MESSAGES = 1 << 12,
		INTENT_DIRECT_MESSAGE_REACTIONS = 1 << 13,
		INTENT_DIRECT_MESSAGE_TYPING = 1 << 14,
		INTENT_MESSAGE_CONTENT = 1 << 15,
	};

private:
	static GdDiscordClient *singleton;

	// --- Gateway (WebSocket) ---
#ifdef MODULE_WEBSOCKET_ENABLED
	Ref<WebSocketClient> ws_client;
#endif
	String bot_token;
	String gateway_url;
	String session_id;
	String resume_gateway_url;
	int sequence;
	int heartbeat_interval_ms;
	int heartbeat_timer_ms;
	bool heartbeat_ack;
	bool resuming;
	int intents;
	ConnectionState state;
	Dictionary self_user;

	// Gateway handlers
	void _ws_connected(String p_protocol);
	void _ws_data_received();
	void _ws_closed(bool p_clean);
	void _ws_error();
	void _send_gateway_message(int p_opcode, const Variant &p_data);
	void _send_identify();
	void _send_heartbeat();
	void _send_resume();
	void _parse_gateway_message(const String &p_json);
	void _handle_dispatch(const String &p_event, const Dictionary &p_data);
	void _connect_to_gateway();

	// --- REST API ---
	static const char *REST_BASE_URL;

	struct PendingRequest {
		String endpoint;
		HTTPClient::Method method;
		String body;
	};
	List<PendingRequest> request_queue;
	HTTPRequest *rest_http;
	bool rest_busy;

	void _rest_request(const String &p_endpoint, HTTPClient::Method p_method, const String &p_body = "");
	void _on_rest_completed(int p_result, int p_code, const PoolStringArray &p_headers, const PoolByteArray &p_body);
	void _process_request_queue();
	Vector<String> _make_headers() const;

	// Gateway URL fetch
	HTTPRequest *gateway_http;
	void _on_gateway_url_received(int p_result, int p_code, const PoolStringArray &p_headers, const PoolByteArray &p_body);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void connect_bot(const String &p_token, int p_intents = INTENT_GUILDS | INTENT_GUILD_MESSAGES | INTENT_MESSAGE_CONTENT);
	void disconnect_bot();
	bool is_discord_connected() const;
	ConnectionState get_state() const;

	// REST actions
	void send_message(const String &p_channel_id, const String &p_content);
	void send_message_embed(const String &p_channel_id, const Dictionary &p_embed);
	void edit_message(const String &p_channel_id, const String &p_message_id, const String &p_content);
	void delete_message(const String &p_channel_id, const String &p_message_id);
	void add_reaction(const String &p_channel_id, const String &p_message_id, const String &p_emoji);
	void typing_indicator(const String &p_channel_id);

	// Getters
	Dictionary get_self_user() const;
	String get_session_id() const;

	static GdDiscordClient *get_singleton();

	GdDiscordClient();
	~GdDiscordClient();
};

VARIANT_ENUM_CAST(GdDiscordClient::ConnectionState);
VARIANT_ENUM_CAST(GdDiscordClient::Intent);

#endif // GD_DISCORD_H
