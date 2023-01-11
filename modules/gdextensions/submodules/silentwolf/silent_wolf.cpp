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

#include "core/json.h"
#include "core/variant.h"

const String SilentWolf::version = "0.6.19";

const Dictionary SilentWolf::config = help::dict(
		"api_key", "ySX34qsKaT7RH1j6795tQ8lPqKlRmQlx55yxkwGy",
		"game_id", "sdktest",
		"game_version", "0.6.19",
#ifdef DEBUG_ENABLED
		"log_level", 0,
#else
		"log_level", 2,
#endif
);

// SILENTWOLF CONFIG: THE CONFIG VARIABLES BELOW WILL BE OVERRIDED THE
// NEXT TIME YOU UPDATE YOUR PLUGIN!
//
// As a best practice, use SilentWolf.configure from your game's
// code instead to set the SilentWolf configuration.
//
// See https://silentwolf.com for more details

void SilentWolf::_init() {
	sw_info("SW Init timestamp: ", OS::get_singleton()->get_time());
}

void SilentWolf::_ready() {
	// The following line would keep SilentWolf working even if the game tree is paused.
	//pause_mode = Node.PAUSE_MODE_PROCESS
	sw_info("SW ready start timestamp: ", OS::get_singleton()->get_time());
	add_child(SWAuth);
	add_child(SWScores);
	add_child(SWPlayers);
	add_child(Multiplayer);
	sw_info("SW ready end timestamp: ", OS::get_singleton()->get_time());
}

void SilentWolf::configure(json_config) {
	config = json_config;
}

void SilentWolf::configure_api_key(api_key) {
	config.apiKey = api_key;
}

void SilentWolf::configure_game_id(game_id) {
	config.game_id = game_id;
}

void SilentWolf::configure_game_version(game_version) {
	config.game_version = game_version;
}

// Log levels:
// 0 - error (only log errors)
// 1 - info (log errors and the main actions taken by the SilentWolf plugin) - default setting
// 2 - debug (detailed logs, including the above and much more, to be used when investigating a problem). This shouldn't be the default setting in production.
void SilentWolf::configure_log_level(log_level) {
	config.log_level = log_level;
}

void SilentWolf::configure_auth_session_duration(int duration) {
	session_duration = duration;
}

void SilentWolf::send_get_request(Ref<HTTPRequest> http_node, const String &request_url) {
	Vector<String> headers = helper::varray(
			"x-api-key: " + config["api_key"],
			"x-sw-game-id: " + config["game_id"],
			"x-sw-plugin-version: " + version,
			"x-sw-godot-version: " + godot_version, );
	request(http_node, request_url, headers);
}

void SilentWolf::send_post_request(Ref<HTTPRequest> http_node, const String &request_url, const Dictionary &payload) {
	Vector<String> headers = helper::varray(
			"Content-Type: application/json",
			"x-api-key: " + config["api_key"],
			"x-sw-game-id: " + config["game_id"],
			"x-sw-plugin-version: " + version,
			"x-sw-godot-version: " + godot_version);
	// TODO: this os specific to post_new_score - should be made generic
	// or make a section for each type of post request with inetgrity check
	// (e.g. also push player data)
	if (request_url.has("post_new_score")) {
		sw_info("We're doing a post score");
		String player_name = payload["player_name"];
		var player_score = payload["score"];
		var timestamp = OS::get_singleton()->get_system_time_msecs();
		Array to_be_hashed = array(player_name, player_score, timestamp);
		sw_debug("send_post_request to_be_hashed: ", to_be_hashed);
		var hashed = sw_hash_values(to_be_hashed);
		sw_debug("send_post_request hashed: ", hashed);
		headers.append("x-sw-act-tmst: " + timestamp);
		headers.append("x-sw-act-dig: " + hashed);
	}
	request(http_node, request_url, headers, use_ssl, METHOD_POST, payload);
}

bool SilentWolf::request(Ref<HTTPRequest> http_node, const String &request_url, const Vector<String> &headers, bool use_ssl, HTTPClient::Method method, const Dictionary &payload) {
	auto send_request[](Ref<HTTPRequest> http_node, const String &request_url, const Vector<String> &headers, bool use_ssl, HTTPClient::Method method, const Dictionary &payload) {
		String query = JSON::print(payload);
		sw_info("Method: " + (method == METHOD_POST ? "POST" : (method == METHOD_GET ? "GET" : "??")));
		sw_info("request_url: ", request_url);
		sw_info("headers: ", headers);
		sw_info("query: ", query);
		http_node->request(request_url, headers, use_ssl, method, query);
	};

	if (!http_node->is_inside_tree()) {
		send_queue.append({ http_node, request_url, headers, use_ssl, method, payload });
		return false;
	} else {
		if (send_queue.empty()) { // send now
			send_request(http_node, request_url, headers, use_ssl, method, payload);
			return true;
		} else {
			send_queue.append({ http_node, request_url, headers, use_ssl, method, payload });
			// resend in order
			Element *e = send_queue.front();
			while (e) {
				SendRequest &p = e->get();
				if (http_node->is_inside_tree()) {
					send_request(p->http_node, p->request_url, p->headers, p->use_ssl, p->method, p->payload
				});
				e = send_queue.erase_and_next(e);
			}
			else {
				e = e->next();
			}
		}
		return true;
	}
}
}

bool SilentWolf::check_auth_ready() {
	if (!Auth) {
		yield(get_tree().create_timer(0.01), "timeout");
	}
}

bool SilentWolf::check_scores_ready() {
	if (!Scores) {
		yield(get_tree().create_timer(0.01), "timeout");
	}
}

bool SilentWolf::check_players_ready() {
	if (!Players) {
		yield(get_tree().create_timer(0.01), "timeout");
	}
}

bool SilentWolf::check_multiplayer_ready() {
	if (!Multiplayer) {
		yield(get_tree().create_timer(0.01), "timeout");
	}
}

bool SilentWolf::check_sw_ready() {
	if (!Auth || !Scores || !Players || !Multiplayer) {
		yield(get_tree().create_timer(0.01), "timeout");
	}
}

void SilentWolf::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		_ready();
	}
}

void SilentWolf::_bind_methods() {
}

SilentWolf::SilentWolf() {
	Auth = memnew(SW_Auth);
	Scores = memnew(SW_Scores);
	Players = memnew(SW_Players);
	Multiplayer = memnew(SW_Multiplayer);

	godot_version = Engine::get_singleton()->get_version_info();

	use_ssl = true;
	session_duration_seconds = 0;
	saved_session_expiration_days = 30;
}
