/*************************************************************************/
/*  sw_players.cpp                                                       */
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

void SW_Player::set_player_data(const Dictionary &new_player_data) {
	player_data = new_player_data;
}

void SW_Player::clear_player_data() {
	player_name = String();
	player_data = Dictionary();
}

Dictionary SW_Player::get_stats() {
	Dictionary stats;
	if (player_data) {
		stats = helper::dict(
				"strength", player_data.strength,
				"speed", player_data.speed,
				"reflexes", player_data.reflexes,
				"max_health", player_data.max_health,
				"career", player_data.career, );
	}
	return stats;
}

Dictionary SW_Player::get_inventory() {
	Dictionary inventory;
	if (player_data) {
		inventory = helper::dict(
				"weapons", player_data.weapons,
				"gold", player_data.gold, );
	}
	return inventory;
}

Player SW_Player::get_player_data(const String &player_name) {
	GetPlayerData = HTTPRequest.new();
	wrGetPlayerData = weakref(GetPlayerData);
	if (OS.get_name() != "HTML5") {
		GetPlayerData.set_use_threads(true);
	}
	get_tree().get_root().add_child(GetPlayerData);
	GetPlayerData.connect("request_completed", self, "_on_GetPlayerData_request_completed");
	sw_info("Calling SilentWolf to get player data");
	var game_id = SilentWolf.config.game_id;
	var game_version = SilentWolf.config.game_version;
	var api_key = SilentWolf.config.api_key;
	var headers = [
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	] GetPlayerData.request("https://api.silentwolf.com/get_player_data/" + game_id + "/" + player_name, headers, true, HTTPClient.METHOD_GET) return self
}

Player SW_Player::post_player_data(const String &player_name, player_data, overwrite = true) {
	if
		typeof(player_data) != TYPE_DICTIONARY : sw_error("Player data should be of type Dictionary, instead it is of type: " + str(typeof(player_data))) PushPlayerData = HTTPRequest.new()
																																												   wrPushPlayerData = weakref(PushPlayerData) if OS.get_name() != "HTML5" : PushPlayerData.set_use_threads(true) get_tree().get_root().add_child(PushPlayerData);
	PushPlayerData.connect("request_completed", self, "_on_PushPlayerData_request_completed");
	sw_info("Calling SilentWolf to post player data");
	var game_id = SilentWolf.config.game_id;
	var game_version = SilentWolf.config.game_version;
	var api_key = SilentWolf.config.api_key;
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	var payload = { "game_id" : game_id, "game_version" : game_version, "player_name" : player_name, "player_data" : player_data, "overwrite" : overwrite };
	var query = JSON.print(payload);
	PushPlayerData.request("https://api.silentwolf.com/push_player_data", headers, true, HTTPClient.METHOD_POST, query);
	return self;
}

void SW_Player::delete_player_weapons(const String &player_name) {
	Dictionary weapons = helper::dict("Weapons"
									  : Array());
	delete_player_data(player_name, weapons);
}

void SW_Player::remove_player_money(const String &player_name) {
	Dictionary money = helper::dict("Money" : 0);
	delete_player_data(player_name, money);
}

void SW_Player::delete_player_items(const String &player_name, const String &item_name) {
	Dictionary item = helper::dict(item_name
								   : "");
	delete_player_data(player_name, item);
}

void SW_Player::delete_all_player_data(player_name) {
	delete_player_data(player_name, "");
}

Player SW_Player::delete_player_data(player_name, player_data) {
	RemovePlayerData = HTTPRequest.new()
							   wrRemovePlayerData = weakref(RemovePlayerData) if OS.get_name() != "HTML5" : RemovePlayerData.set_use_threads(true) get_tree().get_root().add_child(RemovePlayerData) RemovePlayerData.connect("request_completed", self, "_on_RemovePlayerData_request_completed") sw_info("Calling SilentWolf to remove player data") var game_id = SilentWolf.config.game_id
																																																																																											 var api_key = SilentWolf.config.api_key
																																																																																																   var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	] var payload = { "game_id" : game_id, "player_name" : player_name, "player_data" : player_data } var query = JSON.print(payload) RemovePlayerData.request("https://api.silentwolf.com/remove_player_data", headers, true, HTTPClient.METHOD_POST, query) return self
}

void SW_Player::_on_GetPlayerData_request_completed(result, response_code, headers, body) {
	sw_info("GetPlayerData request completed")
			var status_check = CommonErrors.check_status_code(response_code)
							   //print("client status: " + str(GetPlayerData.get_http_client_status()))
							   if is_instance_valid (GetPlayerData) :
			SilentWolf.free_request(wrGetPlayerData, GetPlayerData)
			//GetPlayerData.queue_free()
			sw_debug("response headers: " + str(response_code))
					sw_debug("response headers: " + str(headers))
							sw_debug("response body: " + str(body.get_string_from_utf8()))

									if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8())
						   var response = json.result if "message" in response.keys() and response.message == "Forbidden" : sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerdata") else : sw_info("SilentWolf get player data success") player_name = response.player_name
																																																																																		   player_data = response.player_data
																																																																																								 sw_debug("Request completed: Player data: " + str(player_data))
																																																																																										 emit_signal("sw_player_data_received", player_name, player_data)
	}
}

void SW_Player::_on_PushPlayerData_request_completed(result, response_code, headers, body) {
	sw_info("PushPlayerData request completed")
			var status_check = CommonErrors.check_status_code(response_code) if is_instance_valid (PushPlayerData) :
			//PushPlayerData.queue_free()
			SilentWolf.free_request(wrPushPlayerData, PushPlayerData)
					sw_debug("response headers: " + str(response_code))
							sw_debug("response headers: " + str(headers))
									sw_debug("response body: " + str(body.get_string_from_utf8()))

											if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8())
						   var response = json.result if "message" in response.keys() and response.message == "Forbidden" : sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerdata") else : sw_info("SilentWolf post player data score success: " + str(response_code)) var player_name = response.player_name
																																																																																											 emit_signal("sw_player_data_posted", player_name)
	}
}

void SW_Player::_on_RemovePlayerData_request_completed(result, response_code, headers, body) {
	sw_info("RemovePlayerData request completed")
			var status_check = CommonErrors.check_status_code(response_code) if is_instance_valid (RemovePlayerData) :
			RemovePlayerData.queue_free()
					sw_debug("response headers: " + str(response_code))
							sw_debug("response headers: " + str(headers))
									sw_debug("response body: " + str(body.get_string_from_utf8()))

											if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8())
						   var response = json.result if "message" in response.keys() and response.message == "Forbidden" : sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerdata") else : sw_info("SilentWolf post player data score success: " + str(response_code)) var player_name = response.player_name
																																																																																											 // return player_data after (maybe partial) removal
																																																																																											 var player_data = response.player_data
																																																																																																	   emit_signal("sw_player_data_removed", player_name, player_data)
	}
}

void SW_Player::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_GetPlayerData_request_completed"), &SW_Player::_on_GetPlayerData_request_completed);
	ClassDB::bind_method(D_METHOD("_on_PushPlayerData_request_completed"), &SW_Player::_on_PushPlayerData_request_completed);
	ClassDB::bind_method(D_METHOD("_on_RemovePlayerData_request_completed"), &SW_Player::_on_RemovePlayerData_request_completed);

	ADD_SIGNAL(sw_player_data_received);
	ADD_SIGNAL(sw_player_data_posted);
	ADD_SIGNAL(sw_player_data_removed);
}

void SW_Player::SW_Player() {
}
