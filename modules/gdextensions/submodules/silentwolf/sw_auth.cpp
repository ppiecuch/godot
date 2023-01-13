/*************************************************************************/
/*  sw_auth.cpp                                                          */
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

SW_Auth &SW_Auth::set_player_logged_in(const String &player_name) {
	logged_in_player = player_name;
	sw_info("SilentWolf - player logged in as ", player_name);
	if (SilentWolf::auth_config.has("session_duration_seconds")) {
		login_timeout = SilentWolf::auth_config["session_duration_seconds"];
	} else {
		login_timeout = 0;
	}
	sw_info("SilentWolf login timeout: ", login_timeout);
	if (login_timeout != 0) {
		setup_login_timer();
	}
	return *this;
}

String SW_Auth::get_anon_user_id() {
	String anon_user_id = OS::get_singleton()->get_unique_id();
	if (anon_user_id.empty()) {
		anon_user_id = generate_uuid_v4();
	}
	sw_info("anon_user_id: ", anon_user_id);
	return anon_user_id;
}

SW_Auth &SW_Auth::logout_player() {
	logged_in_player = "";
	SilentWolf::get_instance()->clear_player_data(); // remove any player data if present
	remove_stored_session(); // remove stored session if any
	emit_signal("sw_logout_succeeded");
	return *this;
}

SW_Auth &SW_Auth::register_player_anon(const String &player_name) {
	String user_local_id = get_anon_user_id();
	RegisterPlayer = newref(HTTPRequestBasic);
	RegisterPlayer->connect("request_completed", this, "_on_RegisterPlayer_request_completed");
	sw_info("Calling SilentWolf to register an anonymous player");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "anon", true, "player_name", player_name, "user_local_id", user_local_id);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			String(application_json),
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	//sw_debug("register_player headers: ", headers);
	RegisterPlayer->request("https://api.silentwolf.com/create_new_player", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

SW_Auth &SW_Auth::register_player(const String &player_name, const String &email, const String &password, bool confirm_password) {
	tmp_username = player_name;
	RegisterPlayer = newref(HTTPRequestBasic);
	RegisterPlayer->connect("request_completed", this, "_on_RegisterPlayer_request_completed");
	sw_info("Calling SilentWolf to register a player");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "anon", false, "player_name", player_name, "email", email, "password", password, "confirm_password", confirm_password);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	//sw_debug("register_player headers: ", headers);
	RegisterPlayer->request("https://api.silentwolf.com/create_new_player", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

SW_Auth &SW_Auth::register_player_user_password(const String &player_name, const String &password, bool confirm_password) {
	tmp_username = player_name;
	RegisterPlayer = newref(HTTPRequestBasic);
	RegisterPlayer->connect("request_completed", this, "_on_RegisterPlayerUserPassword_request_completed");
	sw_info("Calling SilentWolf to register a player");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "player_name", player_name, "password", password, "confirm_password", confirm_password);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	//sw_debug("register_player headers: ", headers);
	RegisterPlayer->request("https://api.silentwolf.com/create_new_player", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

SW_Auth &SW_Auth::verify_email(const String &player_name, const String &code) {
	tmp_username = player_name;
	VerifyEmail = newref(HTTPRequestBasic);
	VerifyEmail->connect("request_completed", this, "_on_VerifyEmail_request_completed");
	sw_info("Calling SilentWolf to verify email address for: ", player_name);
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "username", player_name, "code", code);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	//sw_debug("register_player headers: ", headers);
	VerifyEmail->request("https://api.silentwolf.com/confirm_verif_code", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

SW_Auth &SW_Auth::resend_conf_code(const String &player_name) {
	ResendConfCode = newref(HTTPRequestBasic);
	ResendConfCode->connect("request_completed", this, "_on_ResendConfCode_request_completed");
	sw_info("Calling SilentWolf to resend confirmation code for: ", player_name);
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "username", player_name);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	//sw_debug("register_player headers: ", headers);
	ResendConfCode->request("https://api.silentwolf.com/resend_conf_code", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

SW_Auth &SW_Auth::login_player(const String &username, const String &password, bool remember_me) {
	tmp_username = username;
	LoginPlayer = newref(HTTPRequestBasic);
	sw_info("OS name: ", OS::get_singleton()->get_name());
	LoginPlayer->connect("request_completed", this, "_on_LoginPlayer_request_completed");
	sw_info("Calling SilentWolf to log in a player");
	String game_id = SilentWolf::config["game_id"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "username", username, "password", password, "remember_me", remember_me);
	if (SilentWolf::auth_config.has("saved_session_expiration_days")) {
		payload["remember_me_expires_in"] = SilentWolf::auth_config["saved_session_expiration_days"];
	}
	sw_debug("SilentWolf login player payload: ", payload);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	//sw_info("login_player headers: ", headers);
	LoginPlayer->request("https://api.silentwolf.com/login_player", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

SW_Auth &SW_Auth::request_player_password_reset(const String &player_name) {
	RequestPasswordReset = newref(HTTPRequestBasic);
	sw_info("OS name: ", OS::get_singleton()->get_name());
	RequestPasswordReset->connect("request_completed", this, "_on_RequestPasswordReset_request_completed");
	sw_info("Calling SilentWolf to request a password reset for: ", player_name);
	String game_id = SilentWolf::config["game_id"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "player_name", player_name);
	sw_debug("SilentWolf request player password reset payload: ", payload);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	RequestPasswordReset->request("https://api.silentwolf.com/request_player_password_reset", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

SW_Auth &SW_Auth::reset_player_password(const String &player_name, const String &conf_code, const String &new_password, bool confirm_password) {
	ResetPassword = newref(HTTPRequestBasic);
	ResetPassword->connect("request_completed", this, "_on_ResetPassword_request_completed");
	sw_info("Calling SilentWolf to reset password for: ", player_name);
	String game_id = SilentWolf::config["game_id"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "player_name", player_name, "conf_code", conf_code, "password", new_password, "confirm_password", confirm_password);
	sw_debug("SilentWolf request player password reset payload: ", payload);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	ResetPassword->request("https://api.silentwolf.com/reset_player_password", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

SW_Auth &SW_Auth::get_player_details(const String &player_name) {
	GetPlayerDetails = newref(HTTPRequestBasic);
	RegisterPlayer->connect("request_completed", this, "_on_GetPlayerDetails_request_completed");
	sw_info("Calling SilentWolf to get player details");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "player_name", player_name);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + api_key,
			"x-sw-plugin-version: " + SilentWolf::version,
			"x-sw-game-id: " + game_id,
			"x-sw-godot-version: " + SilentWolf::godot_version);
	//sw_info("register_player headers: ", headers);
	RegisterPlayer->request("https://api.silentwolf.com/get_player_details", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

void SW_Auth::setup_login_timer() {
	login_timer = memnew(Timer);
	login_timer->set_one_shot(true);
	login_timer->set_wait_time(login_timeout);
	login_timer->connect("timeout", this, "on_login_timeout_complete");
}

void SW_Auth::on_login_timeout_complete() {
	logout_player();
}

// store lookup (not logged in player name) and validator in local file
void SW_Auth::save_session(const Dictionary &lookup, const Dictionary &validator) {
	String path = "user://swsession.save";
	Dictionary session_data = helper::dict(
			"lookup", lookup,
			"validator", validator);
	sw_save_data("user://swsession.save", session_data, "Saving SilentWolf session: ");
}

void SW_Auth::remove_stored_session() {
	String path = "user://swsession.save";
	sw_remove_data(path, "Removing SilentWolf session if any: ");
}

// reload lookup and validator and send them back to the server to auto-login user
Dictionary SW_Auth::load_session() {
	String path = "user://swsession.save";
	Dictionary sw_session_data = sw_get_data(path);
	if (sw_session_data.empty()) {
		sw_debug("No local SilentWolf session stored, or session data stored in incorrect format");
	}
	sw_info("Found session data: ", sw_session_data);
	return sw_session_data;
}

SW_Auth &SW_Auth::auto_login_player() {
	Dictionary sw_session_data = load_session();
	if (!sw_session_data.empty()) {
		sw_debug("Found saved SilentWolf session data, attempting autologin...");
		Dictionary lookup = sw_session_data["lookup"];
		Dictionary validator = sw_session_data["validator"];
		// whether successful or not, in the end the "sw_session_check_complete" signal will be emitted
		validate_player_session(lookup, validator);
	} else {
		sw_debug("No saved SilentWolf session data, so no autologin will be performed");
		// the following is needed to delay the emission of the signal just a little bit, otherwise the signal is never received!
		setup_complete_session_check_wait_timer();
		complete_session_check_wait_timer->start();
	}
	return *this;
}

SW_Auth &SW_Auth::validate_player_session(const Dictionary &lookup, const Dictionary &validator) {
	ValidateSession = newref(HTTPRequestBasic);
	ValidateSession->connect("request_completed", this, "_on_ValidateSession_request_completed");
	sw_info("Calling SilentWolf to validate an existing player session");
	String game_id = SilentWolf::config["game_id"];
	String api_key = SilentWolf::config["api_key"];
	Dictionary payload = helper::dict("game_id", game_id, "lookup", lookup, "validator", validator);
	sw_debug("Validate session payload: ", payload);
	String query = JSON::print(payload);
	Vector<String> headers = helper::vector(application_json, "x-api-key: " + api_key, "x-sw-plugin-version: " + SilentWolf::version);
	ValidateSession->request("https://api.silentwolf.com/validate_remember_me", headers, true, HTTPClient::METHOD_POST, query);
	return *this;
}

// Signal can't be emitted directly from auto_login_player() function
// otherwise it won't connect back to calling script
void SW_Auth::complete_session_check(const Variant &return_value) {
	sw_debug("emitting signal....");
	emit_signal("sw_session_check_complete", return_value);
}

void SW_Auth::_on_LoginPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("LoginPlayer request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	//sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			if (response.has("lookup")) {
				sw_debug("remember me lookup: ", response["lookup"]);
				save_session(response["lookup"], response["validator"]);
			}
			if (response.has("validator")) {
				sw_debug("remember me validator: ", response["validator"]);
			}
			sw_info("SilentWolf login player success? : ", response["success"]);
			// TODO: get JWT token and store it
			// send a different signal depending on login success or failure
			if (response["success"]) {
				token = response["swtoken"];
				//id_token = response["swidtoken"];
				sw_debug("token: ", token);
				set_player_logged_in(tmp_username);
				emit_signal("sw_login_succeeded");
			} else {
				emit_signal("sw_login_failed", response["error"]);
			}
		}
	}
}

void SW_Auth::_on_RegisterPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("RegisterPlayer request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("reponse: ", response);
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf create new player success? : ", response["success"]);
			// also get a JWT token here
			// send a different signal depending on registration success or failure
			if (response["success"]) {
				bool anon = response["anon"];
				if (anon) {
					sw_info("Anonymous Player registration succeeded");
					logged_in_anon = true;
					if (response.has("player_name")) {
						logged_in_player = response["player_name"];
					} else if (response["player_local_id"]) {
						logged_in_player = vconcat("anon##", response["player_local_id"]);
					} else {
						logged_in_player = "anon##unknown";
					}
					sw_debug("Anon registration, logged in player: ", logged_in_player);
				} else {
					// if email confirmation is enabled for the game, we can't log in the player just yet
					bool email_conf_enabled = response["email_conf_enabled"];
					if (email_conf_enabled) {
						sw_info("Player registration succeeded, but player still needs to verify email address");
					} else {
						sw_info("Player registration succeeded, email verification is disabled");
						logged_in_player = tmp_username;
					}
					emit_signal("sw_registration_succeeded");
				}
			} else {
				emit_signal("sw_registration_failed", response["error"]);
			}
		}
	}
}

void SW_Auth::_on_RegisterPlayerUserPassword_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("RegisterPlayerUserPassword request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("reponse: ", response);
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf create new player success? : ", response["success"]);
			// also get a JWT token here
			// send a different signal depending on registration success or failure
			if (response["success"]) {
				// if email confirmation is enabled for the game, we can't log in the player just yet
				bool email_conf_enabled = response["email_conf_enabled"];
				if (email_conf_enabled) {
					sw_info("Player registration with username/password succeeded, player account autoconfirmed");
					logged_in_player = tmp_username;
				}
				emit_signal("sw_registration_user_pwd_succeeded", email_conf_enabled);
			} else {
				emit_signal("sw_registration_failed", response["error"]);
			}
		}
	}
}

void SW_Auth::_on_VerifyEmail_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("VerifyEmail request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("reponse: ", response);
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf verify email success? : ", response["success"]);
			// also get a JWT token here
			// send a different signal depending on registration success or failure
			if (response["success"]) {
				logged_in_player = tmp_username;
				emit_signal("sw_email_verif_succeeded");
			} else {
				emit_signal("sw_email_verif_failed", response["error"]);
			}
		}
	}
}

void SW_Auth::_on_ResendConfCode_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("ResendConfCode request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("reponse: ", response);
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf resend conf code success? : ", response["success"]);
			// also get a JWT token here
			// send a different signal depending on registration success or failure
			if (response["success"]) {
				emit_signal("sw_resend_conf_code_succeeded");
			} else {
				emit_signal("sw_resend_conf_code_failed", response["error"]);
			}
		}
	}
}

void SW_Auth::_on_RequestPasswordReset_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("RequestPasswordReset request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("reponse: ", response);
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf request player password reset success? : ", response["success"]);
			if (response["success"]) {
				emit_signal("sw_request_password_reset_succeeded");
			} else {
				emit_signal("sw_request_password_reset_failed", response["error"]);
			}
		}
	}
}

void SW_Auth::_on_ResetPassword_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("ResetPassword request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("reponse: ", response);
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf reset player password success? : ", response["success"]);
			if (response["success"]) {
				emit_signal("sw_reset_password_succeeded");
			} else {
				emit_signal("sw_reset_password_failed", response["error"]);
			}
		}
	}
}

void SW_Auth::_on_ValidateSession_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("SilentWolf - ValidateSession request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("reponse: ", response);
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf validate session success? : ", response["success"]);
			if (response["success"]) {
				set_player_logged_in(response["player_name"]);
				complete_session_check(logged_in_player);
			} else {
				complete_session_check(response["error"]);
			}
		}
	}
}

void SW_Auth::_on_GetPlayerDetails_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("SilentWolf - GetPlayerDetails request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (status_check) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("reponse: ", response);
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf get player details success? : ", response["success"]);
			if (response["success"]) {
				emit_signal("sw_get_player_details_succeeded", response["player_details"]);
			} else {
				emit_signal("sw_get_player_details_failed");
			}
		}
	}
}

void SW_Auth::setup_complete_session_check_wait_timer() {
	complete_session_check_wait_timer = memnew(Timer);
	complete_session_check_wait_timer->set_one_shot(true);
	complete_session_check_wait_timer->set_wait_time(0.01);
	complete_session_check_wait_timer->connect("timeout", this, "complete_session_check");
}

void SW_Auth::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_LoginPlayer_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_LoginPlayer_request_completed);
	ClassDB::bind_method(D_METHOD("_on_RegisterPlayer_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_RegisterPlayer_request_completed);
	ClassDB::bind_method(D_METHOD("_on_RegisterPlayerUserPassword_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_RegisterPlayerUserPassword_request_completed);
	ClassDB::bind_method(D_METHOD("_on_VerifyEmail_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_VerifyEmail_request_completed);
	ClassDB::bind_method(D_METHOD("_on_ResendConfCode_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_ResendConfCode_request_completed);
	ClassDB::bind_method(D_METHOD("_on_RequestPasswordReset_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_RequestPasswordReset_request_completed);
	ClassDB::bind_method(D_METHOD("_on_ResetPassword_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_ResetPassword_request_completed);
	ClassDB::bind_method(D_METHOD("_on_ValidateSession_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_ValidateSession_request_completed);
	ClassDB::bind_method(D_METHOD("_on_GetPlayerDetails_request_completed", "result", "response_code", "headers", "body"), &SW_Auth::_on_GetPlayerDetails_request_completed);

	ADD_SIGNAL(MethodInfo("sw_login_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_login_failed"));
	ADD_SIGNAL(MethodInfo("sw_logout_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_registration_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_registration_user_pwd_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_registration_failed"));
	ADD_SIGNAL(MethodInfo("sw_email_verif_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_email_verif_failed"));
	ADD_SIGNAL(MethodInfo("sw_resend_conf_code_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_resend_conf_code_failed"));
	ADD_SIGNAL(MethodInfo("sw_session_check_complete"));
	ADD_SIGNAL(MethodInfo("sw_request_password_reset_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_request_password_reset_failed"));
	ADD_SIGNAL(MethodInfo("sw_reset_password_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_reset_password_failed"));
	ADD_SIGNAL(MethodInfo("sw_get_player_details_succeeded"));
	ADD_SIGNAL(MethodInfo("sw_get_player_details_failed"));
}

SW_Auth::SW_Auth() {
	logged_in_anon = false;
	login_timeout = 0;
	login_timer = nullptr;
	complete_session_check_wait_timer = nullptr;
}
