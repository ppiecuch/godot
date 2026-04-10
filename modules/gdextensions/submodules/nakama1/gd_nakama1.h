/**************************************************************************/
/*  gd_nakama1.h                                                          */
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

#ifndef GD_NAKAMA1_H
#define GD_NAKAMA1_H

#include "scene/main/node.h"

#include "client/nakamaclient.h"

// Internal C++ singleton — NOT exposed to GDScript.
// Persists authentication state (session token) independently of
// GdNakama1Node lifetime so reconnection does not require re-authentication.
class GdNakama1 {
	static GdNakama1 *singleton;

	DefaultSession *nk_session = nullptr;

public:
	static GdNakama1 *get_singleton();

	// Takes ownership of p_session (will memdelete on replacement or destruction).
	void set_session(DefaultSession *p_session);
	DefaultSession *get_session() const { return nk_session; }
	bool is_authenticated() const { return nk_session != nullptr; }
	bool is_session_expired() const;

	GdNakama1();
	~GdNakama1();
};

// GDScript-facing Node. Add to the scene tree to activate Nakama networking.
// Drives HTTP/WebSocket processing via _process() and exposes the full API.
// The underlying session persists in GdNakama1 singleton across node lifecycle.
class GdNakama1Node : public Node {
	GDCLASS(GdNakama1Node, Node);

	DefaultClient *nk_client = nullptr;

	void _authenticated(const String &p_session_token);
	void _client_request_error(String error_message);
	void _client_network_error(int status, int code);
	void _client_session_error(NkErrorCode error_code, String error_message, String collation_id);
	void _client_session_accepted(String session_token, String collation_id);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void create_client(String p_server_key, String p_server_host, int p_port = 7349, bool p_ssl = false, int p_timeout = 60);
	void authenticate_device(String p_device, bool create = true);
	void authenticate_email(String p_email, String p_password, bool create = true);
	bool is_session_expired();
	void logout();

	void set_lang(String p_lang);
	String get_lang() const;
	void set_trace(bool p_trace);
	bool get_trace() const;

	void join_chat_room(String p_room_name);
	void write_chat_message(String p_channel_id, String p_content);
	void submit_score(String p_leaderboard_id, NkMessage::ScoreOperator p_op, int64_t p_score);

	GdNakama1Node();
	~GdNakama1Node();
};

#endif // GD_NAKAMA1_H
