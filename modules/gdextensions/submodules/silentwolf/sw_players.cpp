/**************************************************************************/
/*  sw_players.cpp                                                        */
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

#include "silent_wolf.h"

#include "core/io/json.h"

#define NOTIFY_REQUEST()                  \
	{                                     \
		requesting = true;                \
		emit_signal("sw_data_requested"); \
		return this;                      \
	}

void SW_Player::sw_notification(int what) {
	if (what == SW_NOTIFICATION_PROCESS) {
		if (requesting) {
			int check = 0; // any active request
			for (auto &http_req : helper::vector(GetPlayerData, PushPlayerData, RemovePlayerData)) {
				if (http_req) {
					check += !http_req->poll();
				}
			}
			requesting = check > 0;
		}
	}
}

void SW_Player::set_player_data(const Dictionary &new_player_data) {
	player_data = new_player_data;
}

void SW_Player::clear_player_data() {
	player_name = String();
	player_data = Dictionary();
}

Dictionary SW_Player::get_stats() {
	Dictionary stats;
	if (!player_data.empty()) {
		stats = helper::dict(
				"strength", player_data["strength"],
				"speed", player_data["speed"],
				"reflexes", player_data["reflexes"],
				"max_health", player_data["max_health"],
				"career", player_data["career"]);
	}
	return stats;
}

Dictionary SW_Player::get_inventory() {
	Dictionary inventory;
	if (!player_data.empty()) {
		inventory = helper::dict(
				"weapons", player_data["weapons"],
				"gold", player_data["gold"]);
	}
	return inventory;
}

SW_Player *SW_Player::get_player_data(const String &player_name) {
	if (!GetPlayerData) {
		GetPlayerData = newref(BasicHTTPRequest);
		GetPlayerData->connect("request_completed", this, "_on_GetPlayerData_request_completed");
	}
	ERR_FAIL_COND_V(GetPlayerData->is_active_request(), this);
	sw_info("Calling SilentWolf to get player data");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String api_key = SilentWolf::config["api_key"];
	Vector<String> headers = helper::vector(
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	GetPlayerData->request("https://api.silentwolf.com/get_player_data/" + game_id + "/" + player_name, headers, true, HTTPClient::METHOD_GET);
	NOTIFY_REQUEST();
}

SW_Player *SW_Player::post_player_data(const String &player_name, const Dictionary &player_data, bool overwrite) {
	if (!PushPlayerData) {
		PushPlayerData = newref(BasicHTTPRequest);
		PushPlayerData->connect("request_completed", this, "_on_PushPlayerData_request_completed");
	}
	ERR_FAIL_COND_V(PushPlayerData->is_active_request(), this);
	sw_info("Calling SilentWolf to post player data");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String api_key = SilentWolf::config["api_key"];
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	Dictionary payload = helper::dict("game_id", game_id, "game_version", game_version, "player_name", player_name, "player_data", player_data, "overwrite", overwrite);
	String query = JSON::print(payload);
	PushPlayerData->request("https://api.silentwolf.com/push_player_data", headers, true, HTTPClient::METHOD_POST, query);
	NOTIFY_REQUEST();
}

SW_Player *SW_Player::delete_player_items(const String &player_name, const String &item_name) {
	Dictionary item = helper::dict(item_name, "");
	return delete_player_data(player_name, item);
}

SW_Player *SW_Player::delete_all_player_data(const String &player_name) {
	return delete_player_data(player_name, Dictionary());
}

SW_Player *SW_Player::delete_player_data(const String &player_name, const Dictionary &player_data) {
	if (!RemovePlayerData) {
		RemovePlayerData = newref(BasicHTTPRequest);
		RemovePlayerData->connect("request_completed", this, "_on_RemovePlayerData_request_completed");
	}
	ERR_FAIL_COND_V(RemovePlayerData->is_active_request(), this);
	sw_info("Calling SilentWolf to remove player data");
	String game_id = SilentWolf::config["game_id"];
	String api_key = SilentWolf::config["api_key"];
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	Dictionary payload = helper::dict("game_id", game_id, "player_name", player_name, "player_data", player_data);
	String query = JSON::print(payload);
	RemovePlayerData->request("https://api.silentwolf.com/remove_player_data", headers, true, HTTPClient::METHOD_POST, query);
	NOTIFY_REQUEST();
}

void SW_Player::_on_GetPlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("GetPlayerData request completed");
	bool status_check = sw_check_status_code(response_code);
	//sw_debug("client status: ", GetPlayerData->get_http_client_status());
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerdata");
		} else {
			sw_info("SilentWolf get player data success");
			player_name = response["player_name"];
			player_data = response["player_data"];
			sw_debug("Request completed: Player data: ", player_data);
			emit_signal("sw_player_data_received", array(player_name, player_data));
		}
	}
}

void SW_Player::_on_PushPlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("PushPlayerData request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerdata");
		} else {
			sw_info("SilentWolf post player data score success: ", response_code);
			String player_name = response["player_name"];
			emit_signal("sw_player_data_posted", player_name);
		}
	}
}

void SW_Player::_on_RemovePlayerData_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("RemovePlayerData request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerdata");
		} else {
			sw_info("SilentWolf post player data score success: ", response_code);
			String player_name = response["player_name"];
			// return player_data after (maybe partial) removal
			Dictionary player_data = response["player_data"];
			emit_signal("sw_player_data_removed", array(player_name, player_data));
		}
	}
}

void SW_Player::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_GetPlayerData_request_completed"), &SW_Player::_on_GetPlayerData_request_completed);
	ClassDB::bind_method(D_METHOD("_on_PushPlayerData_request_completed"), &SW_Player::_on_PushPlayerData_request_completed);
	ClassDB::bind_method(D_METHOD("_on_RemovePlayerData_request_completed"), &SW_Player::_on_RemovePlayerData_request_completed);

	ClassDB::bind_method(D_METHOD("get_stats"), &SW_Player::get_stats);
	ClassDB::bind_method(D_METHOD("get_inventory"), &SW_Player::get_inventory);
	ClassDB::bind_method(D_METHOD("clear_player_data"), &SW_Player::clear_player_data);
	ClassDB::bind_method(D_METHOD("set_player_data", "new_player_data"), &SW_Player::set_player_data);
	ClassDB::bind_method(D_METHOD("get_player_data", "player_name"), &SW_Player::get_player_data);
	ClassDB::bind_method(D_METHOD("post_player_data", "player_name", "player_data", "overwrite"), &SW_Player::post_player_data, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("delete_player_items", "player_name", "item_name"), &SW_Player::delete_player_items);
	ClassDB::bind_method(D_METHOD("delete_player_data", "player_name", "player_data"), &SW_Player::delete_player_data);
	ClassDB::bind_method(D_METHOD("delete_all_player_data", "player_name"), &SW_Player::delete_all_player_data);

	ADD_SIGNAL(MethodInfo("sw_data_requested"));
	ADD_SIGNAL(MethodInfo("sw_player_data_received", PropertyInfo(Variant::STRING, "player_name"), PropertyInfo(Variant::DICTIONARY, "player_data")));
	ADD_SIGNAL(MethodInfo("sw_player_data_posted", PropertyInfo(Variant::STRING, "player_name")));
	ADD_SIGNAL(MethodInfo("sw_player_data_removed", PropertyInfo(Variant::STRING, "player_name"), PropertyInfo(Variant::DICTIONARY, "player_data")));
}

SW_Player::SW_Player() {
	requesting = false;
}
