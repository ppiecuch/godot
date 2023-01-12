/*************************************************************************/
/*  silent_wolf.h                                                        */
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

#ifndef SILENT_WOLF_H
#define SILENT_WOLF_H

#include "common/gd_core.h"
#include "common/http_request_basic.h"
#include "core/list.h"
#include "core/reference.h"
#include "core/variant.h"
#include "core/os/file_access.h"
#include "modules/websocket/websocket_client.h"

#define _print_debug(...) DEBUG_PRINT(string_format(array("[SilentWolf] ", __VA_ARGS__)))
#define _print_fmt_debug(fmt, ...) DEBUG_PRINT(String("[SilentWolf] " fmt).sprintf(array(__VA_ARGS__)))

int sw_get_log_level();
#define SW_LOG_ERROR 0
#define SW_LOG_WARNING 1
#define SW_LOG_INFO 2
#define SW_LOG_DEBUG 3
void sw_print(int log_level, const String &text);
#define sw_error(...) sw_print(SW_LOG_ERROR, string_format(array("[SilentWolf] ", __VA_ARGS__)))
#define sw_info(...) sw_print(SW_LOG_INFO, string_format(array("[SilentWolf] ", __VA_ARGS__)))
#define sw_warn(...) sw_print(SW_LOG_WARNING, string_format(array("[SilentWolf] ", __VA_ARGS__)))
#define sw_debug(...) sw_print(SW_LOG_DEBUG, string_format(array("[SilentWolf] ", __VA_ARGS__)))

int get_random_int(int max_value);
PoolByteArray random_bytes(int n);
PoolByteArray uuid_bin();
String generate_uuid_v4();
bool is_uuid(const String &test_string);

String sw_hash_values(const Array &values);

bool sw_check_status_code(int status_code);
void sw_no_connection_error();

void sw_save_data(const String &path, const Dictionary &data, const String &debug_message = "Saving data to file in local storage: ");
void sw_remove_data(const String &path, const String &debug_message = "Removing data from file in local storage: ");
bool sw_does_file_exist(const String &path);
Dictionary sw_get_data(const String &path);

class SW_WSClient : public Reference {
	GDCLASS(SW_WSClient, Reference)

	// The URL we will connect to
	String websocket_url = "wss://ws.silentwolfmp.com/server";
	String ws_room_init_url = "wss://ws.silentwolfmp.com/init";

	WebSocketClient *_client;

	void _ready();
	void _process(float p_delta);
	void _closed(bool p_was_clean);
	void _connected(const String &p_proto = "");
	void _on_data();

protected:
	static void _bind_methods();

public:
	void send_to_server(const String &p_message_type, const Dictionary &p_data); // send arbitrary data to backend
	void init_mp_session(const String &p_player_name);

	SW_WSClient();
};

class SW_Scores : public Reference {
	GDCLASS(SW_Scores, Reference)

	Dictionary leaderboards; // leaderboard scores by leaderboard name
	Dictionary leaderboards_past_periods; // leaderboard scores from past periods by leaderboard name and period_offset (negative integers)
	Dictionary ldboard_config; // leaderboard configurations by leaderboard name

	// contains only the scores from one leaderboard at a time
	Vector<Dictionary> scores;
	Vector<Dictionary> player_scores;
	Vector<Dictionary> local_scores;
	// Vector<Dictionary> custom_local_scores;
	String score_id;
	int position;
	Vector<Dictionary> scores_above;
	Vector<Dictionary> scores_below;

	// int request_timeout = 3;
	// Timer *request_timer = nullptr;

	// latest number of scores to be fetched from the backend
	int latest_max;

	Ref<HTTPRequestBasic> ScorePosition;
	Ref<HTTPRequestBasic> ScoresAround;
	Ref<HTTPRequestBasic> HighScores;
	Ref<HTTPRequestBasic> ScoresByPlayer;
	Ref<HTTPRequestBasic> PostScore;
	Ref<HTTPRequestBasic> WipeLeaderboard;
	Ref<HTTPRequestBasic> DeleteScore;

	void _on_GetScoresByPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_GetHighScores_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_DeleteScore_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_PostNewScore_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_GetScorePosition_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_ScoresAround_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_WipeLeaderboard_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);

	void send_get_request(Ref<HTTPRequestBasic> http_node, const String &request_url);
	void send_post_request(Ref<HTTPRequestBasic> http_node, const String &request_url, const Dictionary &payload);

public:
	SW_Scores &get_score_position(const String &score, const String &ldboard_name = "main");
	SW_Scores &get_scores_around(const String &score, int scores_to_fetch = 3, const String &ldboard_name = "main");
	SW_Scores &get_high_scores(int maximum = 10, const String &ldboard_name = "main", int period_offset = 0);
	SW_Scores &get_scores_by_player(const String &player_name, int maximum = 10, const String &ldboard_name = "main", int period_offset = 0);
	SW_Scores &wipe_leaderboard(const String &ldboard_name = "main");
	void add_to_local_scores(const Dictionary &game_result, const String &ld_name = "main");
	void persist_score(const String &player_name, const String &score, const String &ldboard_name = "main", const Dictionary &metadata = Dictionary());
	void delete_score(const String &score_id);

	SW_Scores();
};

class SW_Player : public Reference {
	GDCLASS(SW_Player, Reference)

	Ref<HTTPRequestBasic> GetPlayerData;
	Ref<HTTPRequestBasic> PushPlayerData;
	Ref<HTTPRequestBasic> RemovePlayerData;

	String player_name;
	Dictionary player_data;

	void _on_GetPlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_PushPlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_RemovePlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);

protected:
	static void _bind_methods();

public:
	void set_player_data(const Dictionary &new_player_data);
	void clear_player_data();
	Dictionary get_stats();
	Dictionary get_inventory();
	SW_Player &get_player_data(const String &player_name);
	SW_Player &post_player_data(const String &player_name, const Dictionary &player_data, bool overwrite = true);
	void delete_player_weapons(const Dictionary &player_name);
	void remove_player_money(const Dictionary &player_name);
	void delete_player_items(const Dictionary &player_name, const Dictionary &item_name);
	void delete_all_player_data(const Dictionary &player_name);
	void delete_player_data(const Dictionary &player_name, const Dictionary &player_data);

	SW_Player();
};

class SW_Multiplayer : public Reference {
	GDCLASS(SW_Multiplayer, Reference)

	Ref<SW_WSClient> WSClient;

	bool mp_ws_ready;
	bool mp_session_started;

	String mp_player_name;

	void _ready();
	void _send_init_message();
	void init_mp_session(const String &player_name);
	void send(const Dictionary &data);

protected:
	static void _bind_methods();

public:
	SW_Multiplayer();
};

class SW_Auth : public Reference {
	GDCLASS(SW_Auth, Reference)

	String tmp_username;
	String logged_in_player;
	String logged_in_player_email;
	bool logged_in_anon;
	String token;
	String id_token;

	Ref<HTTPRequestBasic> RegisterPlayer;
	Ref<HTTPRequestBasic> VerifyEmail;
	Ref<HTTPRequestBasic> ResendConfCode;
	Ref<HTTPRequestBasic> LoginPlayer;
	Ref<HTTPRequestBasic> ValidateSession;
	Ref<HTTPRequestBasic> RequestPasswordReset;
	Ref<HTTPRequestBasic> ResetPassword;
	Ref<HTTPRequestBasic> GetPlayerDetails;

	int login_timeout;
	Timer *login_timer;

	Timer *complete_session_check_wait_timer;

	SW_Auth &set_player_logged_in(const String &player_name);
	String get_anon_user_id();
	SW_Auth &logout_player();
	SW_Auth &register_player_anon(const String &player_name);
	SW_Auth &register_player(const String &player_name, const String &email, const String &password, bool confirm_password);
	SW_Auth &register_player_user_password(const String &player_name, const String &password, bool confirm_password);
	SW_Auth &verify_email(const String &player_name, const String &code);
	SW_Auth &resend_conf_code(const String &player_name);
	SW_Auth &login_player(const String &username, const String &password, bool remember_me);
	void request_player_password_reset(const String &player_name);
	void reset_player_password(const String &player_name, const String &conf_code, const String &new_password, bool confirm_password);
	SW_Auth &get_player_details(const String &player_name);
	void setup_login_timer();
	void on_login_timeout_complete();
	void save_session(const Dictionary &lookup, const Dictionary &validator);
	void remove_stored_session();
	Dictionary load_session();
	void auto_login_player();
	void validate_player_session(const Dictionary &lookup, const Dictionary &validator);
	void complete_session_check(const String &return_value);
	void setup_complete_session_check_wait_timer();

	void _on_LoginPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_RegisterPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_RegisterPlayerUserPassword_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_VerifyEmail_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_ResendConfCode_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_RequestPasswordReset_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_ResetPassword_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_ValidateSession_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_GetPlayerDetails_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	SW_Auth();
};

class SilentWolf : public Object {
	GDCLASS(SilentWolf, Object)

	SW_Auth *Auth;
	SW_Scores *Scores;
	SW_Player *Players;
	SW_Multiplayer *Multiplayer;

	String godot_version;

	int session_duration_seconds;
	int saved_session_expiration_days;

	bool use_ssl;

	struct SendRequest {
		Ref<HTTPRequestBasic> http;
		String request_url;
		Vector<String> headers;
		Dictionary payload;
		bool use_ssl;
		HTTPClient::Method method;
	};

	List<SendRequest> send_queue;

protected:
	static void _bind_methods();

public:
	static SilentWolf *get_instance();

	static const Dictionary config;
	static const String version;

	bool check_auth_ready();
	bool check_scores_ready();
	bool check_players_ready();
	bool check_multiplayer_ready();
	bool check_sw_ready();

	void send_get_request(Ref<HTTPRequestBasic> http_node, const String &request_url);
	void send_post_request(Ref<HTTPRequestBasic> http_node, const String &request_url, const Dictionary &payload);

	void queue_request(Ref<HTTPRequestBasic> http_node, const String &request_url, const Vector<String> &headers, bool use_ssl = true, HTTPClient::Method method = HTTPClient::METHOD_GET, const Dictionary &payload = Dictionary());
	void queue_send();

	SilentWolf();
};

class SilentWolfInstance : public Node {
	GDCLASS(SilentWolfInstance, Node)

	SilentWolf *instance;

public:
	SilentWolfInstance();
};

#endif // SILENT_WOLF_H
