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

#include "common/basic_http_request.h"
#include "common/gd_core.h"
#include "core/io/json.h"
#include "core/list.h"
#include "core/os/file_access.h"
#include "core/os/thread.h"
#include "core/reference.h"
#include "core/variant.h"
#include "modules/websocket/websocket_client.h"
#include "scene/2d/node_2d.h"

#define _print_debug(...) DEBUG_PRINT(vconcat("[SilentWolf] ", __VA_ARGS__))
#define _print_fmt_debug(fmt, ...) DEBUG_PRINT(String("[SilentWolf] " fmt).sprintf(array(__VA_ARGS__)))

int sw_get_log_level();
enum SWLogLevel {
	SW_LOG_ERROR,
	SW_LOG_WARNING,
	SW_LOG_INFO,
	SW_LOG_DEBUG,
};
enum SWScoresArray {
	SW_SCORES,
	SW_LOCAL_SCORES,
	SW_PLAYER_SCORES,
	SW_SCORES_ABOVE,
	SW_SCORES_BELOW,
};
enum SWStatus {
	SW_STATUS_REQUESTING,
	SW_STATUS_THREADING,
};
void sw_print(int log_level, const String &text);
#define sw_error(...) sw_print(SW_LOG_ERROR, vconcat("[SilentWolf] ", __VA_ARGS__))
#define sw_info(...) sw_print(SW_LOG_INFO, vconcat("[SilentWolf] ", __VA_ARGS__))
#define sw_warn(...) sw_print(SW_LOG_WARNING, vconcat("[SilentWolf] ", __VA_ARGS__))
#define sw_debug(...) sw_print(SW_LOG_DEBUG, vconcat("[SilentWolf] ", __VA_ARGS__))

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

const static String application_json = "Content-Type: application/json";

enum {
	SW_NOTIFICATION_READY,
	SW_NOTIFICATION_PROCESS,
	SW_NOTIFICATION_EXIT,
};

static _FORCE_INLINE_ String get_string_from_utf8(const PoolByteArray &data) {
	String s;
	if (data.size() > 0) {
		PoolByteArray::Read r = data.read();
		if (s.parse_utf8((const char *)r.ptr(), data.size())) {
			ERR_PRINT("Failed to parse utf8 data");
		}
	}
	return s;
}

static _FORCE_INLINE_ Dictionary parse_json_from_string(const String &json_data) {
	Variant data;
	String error_string;
	int error_line = 0;
	int err = JSON::parse(json_data, data, error_string, error_line);
	ERR_FAIL_COND_V_MSG(err != OK, Dictionary(), "Can not parse JSON: " + error_string + " on line " + rtos(error_line));
	return data;
}

struct SWSendQueue {
	struct SendRequest {
		Ref<BasicHTTPRequest> http;
		String request_url;
		Vector<String> headers;
		bool use_ssl;
		HTTPClient::Method method;
		Dictionary payload;
	};

	List<SendRequest> send_queue;

	void queue_request(Ref<BasicHTTPRequest> http_node, const String &request_url, const Vector<String> &headers, bool use_ssl = true, HTTPClient::Method method = HTTPClient::METHOD_GET, const Dictionary &payload = Dictionary());
	void queue_send();
};

class SW_WSClient : public Reference {
	GDCLASS(SW_WSClient, Reference)

	// The URL we will connect to
	String websocket_url = "wss://ws.silentwolfmp.com/server";
	String ws_room_init_url = "wss://ws.silentwolfmp.com/init";

	WebSocketClient *_client;

	uint64_t _last_poll;

protected:
	static void _bind_methods();

	void _on_closed(bool p_was_clean);
	void _on_connected(const String &p_proto = "");
	void _on_data();

public:
	void _ready();
	void _terminate();
	void _process();

	void send_to_server(const String &p_category, const Dictionary &p_data); // send arbitrary data to backend
	void init_mp_session(const String &p_player_name);

	SW_WSClient();
};

class SW_Scores : public Reference {
	GDCLASS(SW_Scores, Reference)

	Dictionary leaderboards; // leaderboard scores by leaderboard name
	Dictionary leaderboards_past_periods; // leaderboard scores from past periods by leaderboard name and period_offset (negative integers)
	Dictionary ldboard_config; // leaderboard configurations by leaderboard name

	// contains only the scores from one leaderboard at a time
	Array scores;
	Array player_scores;
	Dictionary player_top_score;
	Array local_scores;
	String score_id;
	int position;
	Array scores_above;
	Array scores_below;

	// int request_timeout = 3;
	// Timer *request_timer = nullptr;

	int latest_max; // latest number of scores to be fetched from the backend
	bool requesting; // any active request

	Ref<BasicHTTPRequest> ScorePosition;
	Ref<BasicHTTPRequest> ScoresAround;
	Ref<BasicHTTPRequest> HighScores;
	Ref<BasicHTTPRequest> ScoresByPlayer;
	Ref<BasicHTTPRequest> TopScoreByPlayer;
	Ref<BasicHTTPRequest> PostScore;
	Ref<BasicHTTPRequest> WipeLeaderboard;
	Ref<BasicHTTPRequest> DeleteScore;

	void _on_GetScoresByPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_GetHighScores_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_GetTopScoreByPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_DeleteScore_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_PostNewScore_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_GetScorePosition_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_ScoresAround_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_WipeLeaderboard_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);

	void send_get_request(Ref<BasicHTTPRequest> http_req, const String &request_url);
	void send_post_request(Ref<BasicHTTPRequest> http_req, const String &request_url, const Dictionary &payload);

protected:
	static void _bind_methods();

public:
	void sw_notification(int what);
	_FORCE_INLINE_ bool sw_requesting() const { return requesting; }

	Array get_scores(int what);
	Dictionary get_leaderboards() { return leaderboards; }
	Dictionary get_ldboard_config() { return ldboard_config; }

	SW_Scores *get_score_position(const String &score, const String &ldboard_name = "main");
	SW_Scores *get_scores_around(const String &score, int scores_to_fetch = 3, const String &ldboard_name = "main");
	SW_Scores *get_high_scores(int maximum = 10, const String &ldboard_name = "main", int period_offset = 0);
	SW_Scores *get_scores_by_player(const String &player_name, int maximum = 10, const String &ldboard_name = "main", int period_offset = 0);
	SW_Scores *get_top_score_by_player(const String &player_name, int maximum = 10, const String &ldboard_name = "main", int period_offset = 0);

	SW_Scores *wipe_leaderboard(const String &ldboard_name = "main");
	SW_Scores *persist_score(const String &player_name, const String &score, const String &ldboard_name = "main", const Dictionary &metadata = Dictionary());
	SW_Scores *delete_score(const String &score_id);

	void add_to_local_scores(const Dictionary &game_result, const String &ld_name = "main");

	SW_Scores();
};

class SW_Player : public Reference {
	GDCLASS(SW_Player, Reference)

	Ref<BasicHTTPRequest> GetPlayerData;
	Ref<BasicHTTPRequest> PushPlayerData;
	Ref<BasicHTTPRequest> RemovePlayerData;

	String player_name;
	Dictionary player_data;

	bool requesting;

	void _on_GetPlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_PushPlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);
	void _on_RemovePlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body);

protected:
	static void _bind_methods();

public:
	void sw_notification(int what);
	_FORCE_INLINE_ bool sw_requesting() const { return requesting; }

	Dictionary get_stats();
	Dictionary get_inventory();

	void clear_player_data();
	void set_player_data(const Dictionary &new_player_data);

	SW_Player *get_player_data(const String &player_name);
	SW_Player *post_player_data(const String &player_name, const Dictionary &player_data, bool overwrite = true);
	SW_Player *delete_player_items(const String &player_name, const String &item_name);
	SW_Player *delete_player_data(const String &player_name, const Dictionary &player_data);
	SW_Player *delete_all_player_data(const String &player_name);

	SW_Player();
};

class SW_Multiplayer : public Reference {
	GDCLASS(SW_Multiplayer, Reference)

	Ref<SW_WSClient> WSClient;

	bool mp_ws_ready;
	bool mp_session_started;
	String mp_player_name;

	Timer *poll_timer;
	uint64_t _last_send;

	void _send_init_message();
	void _on_ws_data(const String &data);

protected:
	static void _bind_methods();

public:
	void sw_notification(int what);
	bool sw_requesting() const;

	void init_mp_session(const String &player_name);
	void send(const Dictionary &data);

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

	Ref<BasicHTTPRequest> RegisterPlayer;
	Ref<BasicHTTPRequest> VerifyEmail;
	Ref<BasicHTTPRequest> ResendConfCode;
	Ref<BasicHTTPRequest> LoginPlayer;
	Ref<BasicHTTPRequest> ValidateSession;
	Ref<BasicHTTPRequest> RequestPasswordReset;
	Ref<BasicHTTPRequest> ResetPassword;
	Ref<BasicHTTPRequest> GetPlayerDetails;

	bool requesting;
	int login_timeout;
	Timer *login_timer;

	Timer *complete_session_check_wait_timer;

	String get_anon_user_id();
	void set_player_logged_in(const String &player_name);
	void setup_login_timer();
	void on_login_timeout_complete();
	void setup_complete_session_check_wait_timer();
	void complete_session_check(const Variant &return_value);

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
	static void _bind_methods();

public:
	void sw_notification(int what);
	_FORCE_INLINE_ bool sw_requesting() const { return requesting; }

	SW_Auth *logout_player();
	SW_Auth *register_player_anon(const String &player_name);
	SW_Auth *register_player(const String &player_name, const String &email, const String &password, bool confirm_password);
	SW_Auth *register_player_user_password(const String &player_name, const String &password, bool confirm_password);
	SW_Auth *verify_email(const String &player_name, const String &code);
	SW_Auth *resend_conf_code(const String &player_name);
	SW_Auth *login_player(const String &username, const String &password, bool remember_me);
	SW_Auth *request_player_password_reset(const String &player_name);
	SW_Auth *reset_player_password(const String &player_name, const String &conf_code, const String &new_password, bool confirm_password);
	SW_Auth *get_player_details(const String &player_name);
	SW_Auth *validate_player_session(const Dictionary &lookup, const Dictionary &validator);

	void auto_login_player();
	Dictionary load_session();
	void save_session(const Dictionary &lookup, const Dictionary &validator);
	void remove_stored_session();

	SW_Auth();
};

class SilentWolf : public Object {
	GDCLASS(SilentWolf, Object)

	Ref<SW_Auth> Auth;
	Ref<SW_Scores> Scores;
	Ref<SW_Player> Players;
	Ref<SW_Multiplayer> Multiplayer;

	int session_duration_seconds;
	int saved_session_expiration_days;

	void config_set(const Dictionary &dict, const Variant &key, const Variant &value);

	void _init();
	void _on_data_requested();
	void _setup_thread();

	// thread support
	bool use_threads;
	SafeFlag thread_done;
	SafeFlag thread_request_quit;

	Thread thread;

	static void _thread_func(void *userdata);

protected:
	static void _bind_methods();

public:
	static SilentWolf *get_instance();

	void set_use_threads(bool use);
	bool is_using_threads() const;

	Ref<SW_Auth> _get_auth() { return Auth; }
	Ref<SW_Scores> _get_scores() { return Scores; }
	Ref<SW_Player> _get_players() { return Players; }
	Ref<SW_Multiplayer> _get_multiplayer() { return Multiplayer; }

	static const Dictionary config;
	static const Dictionary auth_config;
	static const String version;
	static const String godot_version;

	static String cfg_str(const String &key);
	static int cfg_int(const String &key);

	void sw_ready();
	void sw_process();

	void sw_print_line(int log_level, const String &msg);
	void sw_debug_line(const String &msg);
	void sw_info_line(const String &msg);
	void sw_warn_line(const String &msg);
	void sw_error_line(const String &msg);

	void configure(const Dictionary &config);
	void configure_api_key(const String &api_key);
	void configure_game_id(const String &game_id);
	void configure_game_version(const String &game_version);
	void configure_log_level(int log_level);
	void configure_auth_session_duration(int duration);
	void configure_session_expiration_days(int expiration);

	bool check_auth_ready();
	bool check_scores_ready();
	bool check_players_ready();
	bool check_multiplayer_ready();
	bool check_sw_ready();
	bool check_sw_requesting();

	void clear_player_data();

	void send_get_request(Ref<BasicHTTPRequest> http_node, const String &request_url);
	void send_post_request(Ref<BasicHTTPRequest> http_node, const String &request_url, const Dictionary &payload);

	SilentWolf();
	~SilentWolf();
};

VARIANT_ENUM_CAST(SWStatus);
VARIANT_ENUM_CAST(SWScoresArray);
VARIANT_ENUM_CAST(SWLogLevel);

class SilentWolfInstance : public Node2D {
	GDCLASS(SilentWolfInstance, Node2D)

	SilentWolf *instance;

	bool server_active;

	void _on_sw_status_changed(int p_status);

protected:
	void _notification(int what);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	virtual Rect2 _edit_get_rect() const { return Rect2(Point2(), Size2(24, 24)); }
#endif

	void set_server_active(bool p_active);
	bool get_server_active() const;
	void set_silentwolf_game_id(String p_game_id);
	String get_silentwolf_game_id() const;
	void set_silentwolf_api_key(String p_game_id);
	String get_silentwolf_api_key() const;

	void sw_debug_line(const String &msg);
	void sw_info_line(const String &msg);
	void sw_warn_line(const String &msg);
	void sw_error_line(const String &msg);

	SilentWolfInstance();
};

#endif // SILENT_WOLF_H
