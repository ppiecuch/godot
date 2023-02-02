/*************************************************************************/
/*  silent_wolf.cpp                                                      */
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

#include "silent_wolf.h"

#include "core/io/json.h"
#include "core/variant.h"
#include "editor/editor_node.h"

const String SilentWolf::version = "0.6.20";
const String SilentWolf::godot_version;

const Dictionary SilentWolf::config = helper::dict(
		"api_key", "ySX34qsKaT7RH1j6795tQ8lPqKlRmQlx55yxkwGy",
		"game_id", "sdktest",
		"game_version", "0.6.19",
		"use_ssl", true,
#ifdef DEBUG_ENABLED
		"log_level", 3
#else
		"log_level", 0
#endif
);

const Dictionary SilentWolf::auth_config = helper::dict(
		"session_duration_seconds", 0,
		"saved_session_expiration_days", 30);

static SilentWolf *instance = nullptr;
SilentWolf *SilentWolf::get_instance() { return instance; }

void SWSendQueue::queue_request(Ref<BasicHTTPRequest> http_node, const String &request_url, const Vector<String> &headers, bool use_ssl, HTTPClient::Method method, const Dictionary &payload) {
	send_queue.push_back({ http_node, request_url, headers, use_ssl, method, payload });
}

void SWSendQueue::queue_send() {
	List<SendRequest>::Element *e = send_queue.front();
	while (e) {
		SendRequest &p = e->get();
		String query = JSON::print(p.payload);
		sw_info("Method: ", (p.method == HTTPClient::METHOD_POST ? "POST" : (p.method == HTTPClient::METHOD_GET ? "GET" : itos(p.method))));
		sw_info("request_url: ", p.request_url);
		sw_info("headers: ", p.headers);
		sw_info("query: ", query);
		p.http->request(p.request_url, p.headers, p.use_ssl, p.method, query);
		e = send_queue.erase_and_next(e);
	}
}

// SILENTWOLF CONFIG: THE CONFIG VARIABLES BELOW WILL BE OVERRIDED THE
// NEXT TIME YOU UPDATE YOUR PLUGIN!
//
// As a best practice, use SilentWolf.configure from your game's
// code instead to set the SilentWolf configuration.
//
// See https://silentwolf.com for more details

void SilentWolf::_init() {
	sw_info("SW/v", version, " Init timestamp: ", OS::get_singleton()->get_iso_date_time());

	Auth->connect("sw_data_requested", this, "_on_data_requested");
	Scores->connect("sw_data_requested", this, "_on_data_requested");
	Players->connect("sw_data_requested", this, "_on_data_requested");
	Multiplayer->connect("sw_data_requested", this, "_on_data_requested");
}

void SilentWolf::_on_data_requested() {
	emit_signal("sw_status_change", SW_STATUS_REQUESTING);
	if (use_threads) {
		_setup_thread();
	}
}

void SilentWolf::_setup_thread() {
	if (thread_done.is_set()) { // restart thread if not running
		thread_done.clear();
		thread_request_quit.clear();
		thread.start(_thread_func, this);
	}
}

#define SEND_NOTIFICATION(N)             \
	{                                    \
		Auth->sw_notification(N);        \
		Scores->sw_notification(N);      \
		Players->sw_notification(N);     \
		Multiplayer->sw_notification(N); \
	}

void SilentWolf::sw_ready() {
	sw_info("SW ready start timestamp: ", OS::get_singleton()->get_iso_date_time());
	sw_info("SW ready end timestamp: ", OS::get_singleton()->get_iso_date_time());

	SEND_NOTIFICATION(SW_NOTIFICATION_READY);
}

void SilentWolf::sw_process() {
	SEND_NOTIFICATION(SW_NOTIFICATION_PROCESS);
}

void SilentWolf::sw_print_line(int log_level, const String &msg) {
	sw_print(log_level, msg);
}

void SilentWolf::sw_debug_line(const String &msg) {
	sw_debug(msg);
}

void SilentWolf::sw_info_line(const String &msg) {
	sw_info(msg);
}

void SilentWolf::sw_warn_line(const String &msg) {
	sw_warn(msg);
}

void SilentWolf::sw_error_line(const String &msg) {
	sw_error(msg);
}

void SilentWolf::config_set(const Dictionary &dict, const Variant &key, const Variant &value) {
	const_cast<Dictionary *>(&dict)->set(key, value);
}

void SilentWolf::configure(const Dictionary &config) {
	for (const auto &entry : config) {
		config_set(config, entry.key, entry.value);
	}
}

void SilentWolf::configure_api_key(const String &api_key) {
	config_set(config, "apiKey", api_key);
}

void SilentWolf::configure_game_id(const String &game_id) {
	config_set(config, "game_id", game_id);
}

void SilentWolf::configure_game_version(const String &game_version) {
	config_set(config, "game_version", game_version);
}

String SilentWolf::cfg_str(const String &key) {
	return config[key];
}

int SilentWolf::cfg_int(const String &key) {
	return config[key];
}

// Log levels:
// -----------
// 0 - error (only log errors)
// 1 - warning (log errors and warnings)
// 2 - info (log above and the main actions taken by the SilentWolf plugin) - default setting
// 3 - debug (detailed logs, including the above and much more, to be used when investigating a problem). This shouldn't be the default setting in production.
void SilentWolf::configure_log_level(int log_level) {
	config_set(config, "log_level", log_level);
}

void SilentWolf::configure_auth_session_duration(int duration) {
	config_set(auth_config, "session_duration_seconds", duration);
}

void SilentWolf::configure_session_expiration_days(int expiration) {
	config_set(auth_config, "saved_session_expiration_days", expiration);
}

void SilentWolf::send_get_request(Ref<BasicHTTPRequest> http_node, const String &request_url) {
	Vector<String> headers = helper::vector(
			"x-api-key: " + cfg_str("api_key"),
			"x-sw-game-id: " + cfg_str("game_id"),
			"x-sw-plugin-version: " + version,
			"x-sw-godot-version: " + godot_version);
	sw_debug("Method: GET");
	sw_debug("request_url: ", request_url);
	sw_debug("headers: ", headers);
	http_node->request(request_url, headers);
}

void SilentWolf::send_post_request(Ref<BasicHTTPRequest> http_node, const String &request_url, const Dictionary &payload) {
	Vector<String> headers = helper::vector(
			String("Content-Type: application/json"),
			"x-api-key: " + (String)config["api_key"],
			"x-sw-game-id: " + (String)config["game_id"],
			"x-sw-plugin-version: " + version,
			"x-sw-godot-version: " + godot_version);
	// TODO: this os specific to post_new_score - should be made generic
	// or make a section for each type of post request with inetgrity check
	// (e.g. also push player data)
	if (request_url.has("post_new_score")) {
		sw_info("We're doing a post score");
		String player_name = payload["player_name"];
		String player_score = payload["score"];
		uint64_t timestamp = OS::get_singleton()->get_system_time_msecs();
		Array to_be_hashed = array(player_name, player_score, timestamp);
		sw_debug("send_post_request to_be_hashed: ", to_be_hashed);
		String hashed = sw_hash_values(to_be_hashed);
		sw_debug("send_post_request hashed: ", hashed);
		headers.push_back("x-sw-act-tmst: " + itos(timestamp));
		headers.push_back("x-sw-act-dig: " + hashed);
	}
	String query = JSON::print(payload);
	sw_info("Method: POST");
	sw_info("request_url: ", request_url);
	sw_info("headers: ", headers);
	sw_info("query: ", query);
	http_node->request(request_url, headers, config["use_ssl"], HTTPClient::METHOD_POST, query);
}

void SilentWolf::clear_player_data() {
	ERR_FAIL_COND(Players.is_null());
	SilentWolf::get_instance()->Players->clear_player_data();
}

bool SilentWolf::check_auth_ready() {
	return (!Auth);
}

bool SilentWolf::check_scores_ready() {
	return (!Scores);
}

bool SilentWolf::check_players_ready() {
	return (!Players);
}

bool SilentWolf::check_multiplayer_ready() {
	return (!Multiplayer);
}

bool SilentWolf::check_sw_ready() {
	return (!Auth || !Scores || !Players || !Multiplayer);
}

bool SilentWolf::check_sw_requesting() {
	return (Scores->sw_requesting() || Players->sw_requesting() || Auth->sw_requesting());
}

void SilentWolf::set_use_threads(bool use) {
	ERR_FAIL_COND(check_sw_requesting());
	if (use_threads != use) {
		use_threads = use;
		// no need to do anything - no connections
		emit_signal("sw_status_change", SW_STATUS_THREADING);
	}
}

bool SilentWolf::is_using_threads() const {
	return use_threads;
}

void SilentWolf::_thread_func(void *userdata) {
	SilentWolf *sw = (SilentWolf *)userdata;

	while (!sw->thread_request_quit.is_set()) {
		sw->sw_process();
		if (!sw->check_sw_requesting()) {
			break;
		}
		OS::get_singleton()->delay_usec(1);
	}

	sw->thread_done.set();
}

void SilentWolf::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_use_threads", "use"), &SilentWolf::set_use_threads);
	ClassDB::bind_method(D_METHOD("is_using_threads"), &SilentWolf::is_using_threads);

	ClassDB::bind_method(D_METHOD("sw_print", "log_level", "text"), &SilentWolf::sw_print_line);
	ClassDB::bind_method(D_METHOD("sw_debug", "text"), &SilentWolf::sw_debug_line);
	ClassDB::bind_method(D_METHOD("sw_info", "text"), &SilentWolf::sw_info_line);
	ClassDB::bind_method(D_METHOD("sw_warn", "text"), &SilentWolf::sw_warn_line);
	ClassDB::bind_method(D_METHOD("sw_error", "text"), &SilentWolf::sw_error_line);

	ClassDB::bind_method(D_METHOD("_get_auth"), &SilentWolf::_get_auth);
	ClassDB::bind_method(D_METHOD("_get_scores"), &SilentWolf::_get_scores);
	ClassDB::bind_method(D_METHOD("_get_players"), &SilentWolf::_get_players);
	ClassDB::bind_method(D_METHOD("_get_multiplayer"), &SilentWolf::_get_multiplayer);

	ClassDB::bind_method(D_METHOD("clear_player_data"), &SilentWolf::clear_player_data);
	ClassDB::bind_method(D_METHOD("_on_data_requested"), &SilentWolf::_on_data_requested);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_thread"), "set_use_threads", "is_using_threads");

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "Auth"), "", "_get_auth");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "Scores"), "", "_get_scores");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "Players"), "", "_get_players");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "Multiplayer"), "", "_get_multiplayer");

	ADD_SIGNAL(MethodInfo("sw_status_change", PropertyInfo(Variant::INT, "status")));

	BIND_ENUM_CONSTANT(SW_LOG_DEBUG);
	BIND_ENUM_CONSTANT(SW_LOG_INFO);
	BIND_ENUM_CONSTANT(SW_LOG_WARNING);
	BIND_ENUM_CONSTANT(SW_LOG_ERROR);
}

SilentWolf::SilentWolf() {
	instance = this;
	use_threads = false;

	Auth = newref(SW_Auth);
	Scores = newref(SW_Scores);
	Players = newref(SW_Player);
	Multiplayer = newref(SW_Multiplayer);

	if (godot_version.empty()) {
		const_cast<String *>(&godot_version)->operator=(vconcat(Engine::get_singleton()->get_version_info()));
	}

	_init();
}

SilentWolf::~SilentWolf() {
	if (!thread_done.is_set()) {
		thread_request_quit.set();
		thread.wait_to_finish();
	}
}

void SilentWolfInstance::set_server_active(bool p_active) {
	if (server_active != p_active) {
		server_active = p_active;
		set_process_internal(p_active);
	}
}

bool SilentWolfInstance::get_server_active() const {
	return server_active;
}

void SilentWolfInstance::set_silentwolf_game_id(String p_game_id) {
	if (instance->config["game_id"] != p_game_id) {
		instance->configure_game_id(p_game_id);
	}
}

String SilentWolfInstance::get_silentwolf_game_id() const {
	return instance->config["game_id"];
}

void SilentWolfInstance::set_silentwolf_api_key(String p_api_key) {
	if (instance->config["api_key"] != p_api_key) {
		instance->configure_api_key(p_api_key);
	}
}

String SilentWolfInstance::get_silentwolf_api_key() const {
	return instance->config["api_key"];
}

void SilentWolfInstance::sw_debug_line(const String &msg) {
	instance->sw_debug_line(msg);
}

void SilentWolfInstance::sw_info_line(const String &msg) {
	instance->sw_info_line(msg);
}

void SilentWolfInstance::sw_warn_line(const String &msg) {
	instance->sw_warn_line(msg);
}

void SilentWolfInstance::sw_error_line(const String &msg) {
	instance->sw_error_line(msg);
}

void SilentWolfInstance::_notification(int what) {
	if (what == NOTIFICATION_READY) {
		instance->sw_ready();
	} else if (what == NOTIFICATION_ENTER_TREE) {
		instance->connect("sw_status_change", this, "_on_sw_status_changed");
		set_process_internal(server_active && instance->check_sw_requesting());
	} else if (what == NOTIFICATION_EXIT_TREE) {
		instance->disconnect("sw_status_change", this, "_on_sw_status_changed");
		set_process_internal(false);
	} else if (what == NOTIFICATION_INTERNAL_PROCESS) {
		if (!instance->is_using_threads()) {
			instance->sw_process();
			if (!instance->check_sw_requesting()) {
				set_process_internal(false);
			}
		}
#ifndef TOOLS_ENABLED
		else {
			set_process_internal(false); // not needed for threads
		}
#else
		update();
	} else if (what == NOTIFICATION_DRAW) {
		static real_t _progress = 0;
		if (EditorNode *editor = EditorNode::get_singleton()) {
			_progress += get_process_delta_time() * instance->check_sw_requesting();
			const String icon = instance->check_sw_requesting() ? "Progress" + itos(1 + int(_progress) % 8) : "Environment";
			const Ref<Texture> ic = editor->get_gui_base()->get_icon(icon, "EditorIcons"); // progress icon
			draw_texture(ic, (_edit_get_rect().size - ic->get_size()) / 2);
		}
#endif
	}
}

void SilentWolfInstance::_on_sw_status_changed(int p_status) {
	if (is_inside_tree()) {
		if (p_status == SW_STATUS_REQUESTING) {
			if (!instance->is_using_threads()) {
				set_process_internal(server_active);
			}
#ifdef TOOLS_ENABLED
			update();
#endif
		}
	}
}

void SilentWolfInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_server_active"), &SilentWolfInstance::set_server_active);
	ClassDB::bind_method(D_METHOD("get_server_active"), &SilentWolfInstance::get_server_active);
	ClassDB::bind_method(D_METHOD("set_silentwolf_game_id"), &SilentWolfInstance::set_silentwolf_game_id);
	ClassDB::bind_method(D_METHOD("get_silentwolf_game_id"), &SilentWolfInstance::get_silentwolf_game_id);
	ClassDB::bind_method(D_METHOD("set_silentwolf_api_key"), &SilentWolfInstance::set_silentwolf_api_key);
	ClassDB::bind_method(D_METHOD("get_silentwolf_api_key"), &SilentWolfInstance::get_silentwolf_api_key);
	ClassDB::bind_method(D_METHOD("sw_debug", "text"), &SilentWolfInstance::sw_debug_line);
	ClassDB::bind_method(D_METHOD("sw_info", "text"), &SilentWolfInstance::sw_info_line);
	ClassDB::bind_method(D_METHOD("sw_warn", "text"), &SilentWolfInstance::sw_warn_line);
	ClassDB::bind_method(D_METHOD("sw_error", "text"), &SilentWolfInstance::sw_error_line);

	ClassDB::bind_method(D_METHOD("_on_sw_status_changed", "status"), &SilentWolfInstance::_on_sw_status_changed);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_server_active", "get_server_active");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "game_id"), "set_silentwolf_game_id", "get_silentwolf_game_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "api_key"), "set_silentwolf_api_key", "get_silentwolf_api_key");
}

SilentWolfInstance::SilentWolfInstance() {
	server_active = false;
	instance = SilentWolf::get_instance();
}
