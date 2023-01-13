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

const String SilentWolf::version = "0.6.19";

const Dictionary SilentWolf::config = helper::dict(
		"api_key", "ySX34qsKaT7RH1j6795tQ8lPqKlRmQlx55yxkwGy",
		"game_id", "sdktest",
		"game_version", "0.6.19",
#ifdef DEBUG_ENABLED
		"log_level", 0
#else
		"log_level", 2
#endif
);

const Dictionary SilentWolf::auth_config = helper::dict(
		"session_duration_seconds", 0,
		"saved_session_expiration_days", 30);

SilentWolf *instance = nullptr;

// SILENTWOLF CONFIG: THE CONFIG VARIABLES BELOW WILL BE OVERRIDED THE
// NEXT TIME YOU UPDATE YOUR PLUGIN!
//
// As a best practice, use SilentWolf.configure from your game's
// code instead to set the SilentWolf configuration.
//
// See https://silentwolf.com for more details

void SilentWolf::_init() {
	sw_info("SW Init timestamp: ", OS::get_singleton()->get_iso_date_time());
}

void SilentWolf::_ready() {
	// The following line would keep SilentWolf working even if the game tree is paused.
	//pause_mode = Node.PAUSE_MODE_PROCESS
	sw_info("SW ready start timestamp: ", OS::get_singleton()->get_iso_date_time());
	sw_info("SW ready end timestamp: ", OS::get_singleton()->get_iso_date_time());
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

void SilentWolf::send_get_request(Ref<HTTPRequestBasic> http_node, const String &request_url) {
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

void SilentWolf::send_post_request(Ref<HTTPRequestBasic> http_node, const String &request_url, const Dictionary &payload) {
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
	http_node->request(request_url, headers, use_ssl, HTTPClient::METHOD_POST, query);
}

void SilentWolf::clear_player_data() {
	ERR_FAIL_COND(Players.is_null());
	SilentWolf::get_instance()->Players->clear_player_data();
}

void SilentWolf::queue_request(Ref<HTTPRequestBasic> http_node, const String &request_url, const Vector<String> &headers, bool use_ssl, HTTPClient::Method method, const Dictionary &payload) {
	send_queue.push_back({ http_node, request_url, headers, use_ssl, method, payload });
}

void SilentWolf::queue_send() {
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

void SilentWolf::_bind_methods() {
	ClassDB::bind_method(D_METHOD("queue_send"), &SilentWolf::queue_send);
}

SilentWolf::SilentWolf() {
	Auth = newref(SW_Auth);
	Scores = newref(SW_Scores);
	Players = newref(SW_Player);
	Multiplayer = newref(SW_Multiplayer);

	if (godot_version.empty()) {
		const_cast<String *>(&godot_version)->operator=(vconcat(Engine::get_singleton()->get_version_info()));
	}
	use_ssl = true;
}
