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

SW_Auth *SW_Auth::set_player_logged_in(const String &player_name) {
	logged_in_player  = player_name;
	sw_info("SilentWolf - player logged in as ", player_name);
	if (SilentWolf.auth_config.has("session_duration_seconds") and typeof(SilentWolf.auth_config.session_duration_seconds) == 2) {
		login_timeout = SilentWolf.auth_config.session_duration_seconds;
	} else {
		login_timeout = 0;
	}
	sw_info("SilentWolf login timeout: ", login_timeout);
	if (login_timeout != 0) {
		setup_login_timer();
	}
}

String SW_Auth::get_anon_user_id() {
	String anon_user_id = OS::get_singleton().get_unique_id();
	if (anon_user_id.empty()) {
		anon_user_id = UUID.generate_uuid_v4();
	}
	sw_info("anon_user_id: ", anon_user_id);
	return anon_user_id;
}

SW_Auth *SW_Auth::logout_player() {
	logged_in_player = "";
	SilentWolf.Players.clear_player_data(); // remove any player data if present
	remove_stored_session(); // remove stored session if any
	emit_signal("sw_logout_succeeded");
}

SW_Auth *SW_Auth::register_player_anon(const String &player_name) {
	var user_local_id: String = get_anon_user_id();
	RegisterPlayer = HTTPRequest.new();
	if (OS::get_singleton()->get_name() != "HTML5") {
		RegisterPlayer.set_use_threads(true);
	}
	get_tree().get_root().add_child(RegisterPlayer);
	RegisterPlayer.connect("request_completed", self, "_on_RegisterPlayer_request_completed");
	sw_info("Calling SilentWolf to register an anonymous player");
	String game_id = SilentWolf.config.game_id;
	var game_version = SilentWolf.config.game_version;
	var api_key = SilentWolf.config.api_key;
	var payload = { "game_id": game_id, "anon": true, "player_name": player_name, "user_local_id": user_local_id };
	var query = JSON::print(payload);
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	//sw_debug("register_player headers: ", headers);
	RegisterPlayer.request("https://api.silentwolf.com/create_new_player", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

SW_Auth *SW_Auth::register_player(const String &player_name, const String &email, const String &password, bool confirm_password) {
	tmp_username = player_name;
	RegisterPlayer = HTTPRequest.new();
	if (OS::get_singleton()->get_name() != "HTML5") {
		RegisterPlayer.set_use_threads(true);
	}
	get_tree().get_root().add_child(RegisterPlayer);
	RegisterPlayer.connect("request_completed", self, "_on_RegisterPlayer_request_completed");
	sw_info("Calling SilentWolf to register a player");
	var game_id = SilentWolf.config.game_id;
	var game_version = SilentWolf.config.game_version;
	var api_key = SilentWolf.config.api_key;
	var payload = { "game_id": game_id, "anon": false, "player_name": player_name, "email":  email, "password": password, "confirm_password": confirm_password };
	var query = JSON.print(payload);
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	//sw_debug("register_player headers: ", headers);
	RegisterPlayer.request("https://api.silentwolf.com/create_new_player", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

SW_Auth *SW_Auth::register_player_user_password(const String &player_name, const String &password, bool confirm_password) {
	tmp_username = player_name;
	RegisterPlayer = HTTPRequest.new();
	if (OS::get_singleton()->get_name() != "HTML5") {
		RegisterPlayer.set_use_threads(true);
	}
	get_tree().get_root().add_child(RegisterPlayer);
	RegisterPlayer.connect("request_completed", self, "_on_RegisterPlayerUserPassword_request_completed");
	sw_info("Calling SilentWolf to register a player");
	String game_id = SilentWolf.config.game_id;
	var game_version = SilentWolf.config.game_version;
	var api_key = SilentWolf.config.api_key;
	var payload = { "game_id": game_id, "player_name": player_name, "password": password, "confirm_password": confirm_password };
	var query = JSON.print(payload);
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	//sw_debug("register_player headers: ", headers);
	RegisterPlayer.request("https://api.silentwolf.com/create_new_player", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

SW_Auth *SW_Auth::verify_email(const String &player_name, const String &code) {
	tmp_username = player_name;
	VerifyEmail = HTTPRequest.new();
	wrVerifyEmail = weakref(VerifyEmail);
	if (OS::get_singleton()->get_name() != "HTML5") {
		VerifyEmail.set_use_threads(true);
	}
	get_tree().get_root().add_child(VerifyEmail);
	VerifyEmail.connect("request_completed", self, "_on_VerifyEmail_request_completed");
	sw_info("Calling SilentWolf to verify email address for: " + str(player_name));
	var game_id = SilentWolf.config.game_id;
	var game_version = SilentWolf.config.game_version;
	var api_key = SilentWolf.config.api_key;
	var payload = { "game_id": game_id, "username":  player_name, "code": code };
	var query = JSON.print(payload);
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	//sw_debug("register_player headers: ", headers);
	VerifyEmail.request("https://api.silentwolf.com/confirm_verif_code", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

SW_Auth *SW_Auth::resend_conf_code(const String &player_name) {
	ResendConfCode = HTTPRequest.new();
	if (OS::get_singleton()->get_name() != "HTML5") {
		ResendConfCode.set_use_threads(true);
	}
	get_tree().get_root().add_child(ResendConfCode);
	ResendConfCode.connect("request_completed", this, "_on_ResendConfCode_request_completed");
	sw_info("Calling SilentWolf to resend confirmation code for: ", player_name);
	String game_id = SilentWolf.config.game_id;
	String game_version = SilentWolf.config.game_version;
	String api_key = SilentWolf.config.api_key;
	Dictionary payload = helper::dict( "game_id", game_id, "username", player_name );
	String query = JSON::print(payload);
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	//sw_info("register_player headers: ", headers);
	ResendConfCode->request("https://api.silentwolf.com/resend_conf_code", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

SW_Auth *SW_Auth::login_player(const String &username, const String &password, bool remember_me) {
	tmp_username = username;
	LoginPlayer = HTTPRequest.new();
	sw_info("OS name: ", OS::get_singleton()->get_name());
	if (OS::get_singleton()->get_name() != "HTML5") {
		LoginPlayer.set_use_threads(true);
	}
	sw_info("get_tree().get_root(): ", get_tree().get_root());
	get_tree().get_root().add_child(LoginPlayer);
	LoginPlayer.connect("request_completed", self, "_on_LoginPlayer_request_completed");
	sw_info("Calling SilentWolf to log in a player");
	String game_id = SilentWolf.config.game_id;
	String api_key = SilentWolf.config.api_key;
	Dictionary payload = helper::dict( "game_id": game_id, "username": username, "password": password, "remember_me": str(remember_me) );
	if (SilentWolf.auth_config.has("saved_session_expiration_days") && typeof(SilentWolf.auth_config.saved_session_expiration_days) == 2) {
		payload["remember_me_expires_in"] = str(SilentWolf.auth_config.saved_session_expiration_days);
	}
	sw_debug("SilentWolf login player payload: ", payload);
	String query = JSON::print(payload);
	Vector<String> headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	//sw_info("login_player headers: ", headers);
	LoginPlayer.request("https://api.silentwolf.com/login_player", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

void SW_Auth::request_player_password_reset(const String &player_name) {
	RequestPasswordReset = HTTPRequest.new();
	sw_info("OS name: " + str(OS::get_singleton()->get_name()));
	if (OS::get_singleton()->get_name() != "HTML5") {
		RequestPasswordReset.set_use_threads(true);
	}
	get_tree().get_root().add_child(RequestPasswordReset);
	RequestPasswordReset.connect("request_completed", self, "_on_RequestPasswordReset_request_completed");
	sw_info("Calling SilentWolf to request a password reset for: ", player_name);
	var game_id = SilentWolf.config.game_id;
	var api_key = SilentWolf.config.api_key;
	var payload = { "game_id": game_id, "player_name": player_name };
	sw_debug("SilentWolf request player password reset payload: " + str(payload));
	var query = JSON.print(payload);
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	RequestPasswordReset.request("https://api.silentwolf.com/request_player_password_reset", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

void SW_Auth::reset_player_password(const String &player_name, const String &conf_code, const String &new_password, bool confirm_password) {
	ResetPassword = HTTPRequest.new();
	if (OS::get_singleton()->get_name() != "HTML5") {
		ResetPassword.set_use_threads(true);
	}
	get_tree().get_root().add_child(ResetPassword);
	ResetPassword.connect("request_completed", self, "_on_ResetPassword_request_completed");
	sw_info("Calling SilentWolf to reset password for: ", player_name);
	String game_id = SilentWolf.config.game_id;
	String api_key = SilentWolf.config.api_key;
	Dictionary payload = helper::dict( "game_id", game_id, "player_name", player_name, "conf_code", conf_code, "password", new_password, "confirm_password", confirm_password );
	sw_debug("SilentWolf request player password reset payload: ", payload);
	String query = JSON::print(payload);
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	ResetPassword.request("https://api.silentwolf.com/reset_player_password", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

SW_Auth *SW_Auth::get_player_details(const String &player_name) {
	GetPlayerDetails = HTTPRequest.new();
	if (OS::get_singleton()->get_name() != "HTML5") {
		GetPlayerDetails.set_use_threads(true);
	}
	get_tree().get_root().add_child(GetPlayerDetails);
	RegisterPlayer.connect("request_completed", this, "_on_GetPlayerDetails_request_completed");
	sw_info("Calling SilentWolf to get player details");
	var game_id = SilentWolf.config.game_id;
	var game_version = SilentWolf.config.game_version;
	var api_key = SilentWolf.config.api_key;
	var payload = { "game_id": game_id, "player_name": player_name };
	var query = JSON.print(payload);
	var headers = [
		"Content-Type: application/json",
		"x-api-key: " + api_key,
		"x-sw-plugin-version: " + SilentWolf.version,
		"x-sw-game-id: " + SilentWolf.config.game_id,
		"x-sw-godot-version: " + SilentWolf.godot_version
	];
	//sw_info("register_player headers: ", headers);
	RegisterPlayer.request("https://api.silentwolf.com/get_player_details", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}


void SW_Auth::setup_login_timer() {
	login_timer = Timer.new();
	login_timer.set_one_shot(true);
	login_timer.set_wait_time(login_timeout);
	login_timer.connect("timeout", this, "on_login_timeout_complete");
	add_child(login_timer);
}

void SW_Auth::on_login_timeout_complete() {
	logout_player();
}

// store lookup (not logged in player name) and validator in local file
void SW_Auth::save_session(lookup, validator) {
	String path = "user://swsession.save"
	Dictionary session_data = helper::dict(
		"lookup", lookup,
		"validator", validator,
	);
	sw_save_data("user://swsession.save", session_data, "Saving SilentWolf session: ");
}

void SW_Auth::remove_stored_session() {
	String path = "user://swsession.save";
	sw_remove_data(path, "Removing SilentWolf session if any: " );
}

// reload lookup and validator and send them back to the server to auto-login user
Dictionary SW_Auth::load_session() {
	var sw_session_data = null;
	String path = "user://swsession.save";
	sw_session_data = sw_get_data(path);
	if (sw_session_data == null) {
		sw_debug("No local SilentWolf session stored, or session data stored in incorrect format");
	}
	sw_info("Found session data: ", sw_session_data);
	return sw_session_data;
}

void SW_Auth::auto_login_player() {
	var sw_session_data = load_session();
	if (sw_session_data) {
		sw_debug("Found saved SilentWolf session data, attempting autologin...");
		var lookup = sw_session_data.lookup;
		var validator = sw_session_data.validator;
		// whether successful or not, in the end the "sw_session_check_complete" signal will be emitted
		validate_player_session(lookup, validator);
	} else {
		sw_debug("No saved SilentWolf session data, so no autologin will be performed");
		// the following is needed to delay the emission of the signal just a little bit, otherwise the signal is never received!
		setup_complete_session_check_wait_timer();
		complete_session_check_wait_timer.start();
	}
	return this;
}

void SW_Auth::validate_player_session(lookup, validator, scene = get_tree().get_current_scene()) {
	ValidateSession = HTTPRequest.new();
	if (OS::get_singleton()->get_name() != "HTML5") {
		ValidateSession.set_use_threads(true);
	}
	scene.add_child(ValidateSession);
	ValidateSession.connect("request_completed", self, "_on_ValidateSession_request_completed");
	sw_info("Calling SilentWolf to validate an existing player session");
	var game_id = SilentWolf.config.game_id;
	var api_key = SilentWolf.config.api_key;
	var payload = helper::dict( "game_id", game_id, "lookup", lookup, "validator", validator );
	sw_debug("Validate session payload: ", payload);
	String query = JSON::print(payload);
	var headers = ["Content-Type: application/json", "x-api-key: " + api_key, "x-sw-plugin-version: ", SilentWolf.version];
	ValidateSession.request("https://api.silentwolf.com/validate_remember_me", headers, true, HTTPClient.METHOD_POST, query);
	return this;
}

// Signal can't be emitted directly from auto_login_player() function
// otherwise it won't connect back to calling script
void SW_Auth::complete_session_check(const Variant &return_value) {
	sw_debug("emitting signal....");
	emit_signal("sw_session_check_complete", return_value);
}

void SW_Auth::_on_LoginPlayer_request_completed( result, response_code, headers, body ) {
	sw_info("LoginPlayer request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	//sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON::parse(body.get_string_from_utf8());
		var response = json.result;
		if ("message" in response.keys() and response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			if ("lookup" in response.keys()) {
				sw_debug("remember me lookup: " + str(response.lookup));
				save_session(response.lookup, response.validator);
			}
			if ("validator" in response.keys()) {
				sw_debug("remember me validator: " + str(response.validator));
			}
			sw_info("SilentWolf login player success? : ", response.success);
			// TODO: get JWT token and store it
			// send a different signal depending on login success or failure
			if (response.success) {
				token = response.swtoken;
				//id_token = response.swidtoken
				sw_debug("token: ", token);
				set_player_logged_in(tmp_username);
				emit_signal("sw_login_succeeded");
			} else {
				emit_signal("sw_login_failed", response.error);
			}
		}
	}
}

void SW_Auth::_on_RegisterPlayer_request_completed( result, response_code, headers, body ) {
	sw_info("RegisterPlayer request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON::parse(body.get_string_from_utf8());
		var response = json.result;
		sw_debug("reponse: ", response);
		if ("message" in response.keys() and response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf create new player success? : " + str(response.success))
			// also get a JWT token here
			// send a different signal depending on registration success or failure
			if (response.success) {
				var anon = response.anon;
				if (anon) {
					sw_info("Anonymous Player registration succeeded");
					logged_in_anon = true;
					if ("player_name" in response) {
						logged_in_player = response.player_name;
					} else if ("player_local_id" in response) {
						logged_in_player = str("anon##" + response.player_local_id);
					} else {
						logged_in_player = "anon##unknown";
					}
					sw_debug("Anon registration, logged in player: " + str(logged_in_player));
				} else {
					// if email confirmation is enabled for the game, we can't log in the player just yet
					bool email_conf_enabled = response.email_conf_enabled;
					if (email_conf_enabled) {
						sw_info("Player registration succeeded, but player still needs to verify email address");
					} else {
						sw_info("Player registration succeeded, email verification is disabled");
						logged_in_player = tmp_username;
					}
					emit_signal("sw_registration_succeeded");
			} else {
				emit_signal("sw_registration_failed", response.error);
			}
		}
	}
}

void SW_Auth::_on_RegisterPlayerUserPassword_request_completed( result, response_code, headers, body ) {
	sw_info("RegisterPlayerUserPassword request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8());
		var response = json.result;
		sw_debug("reponse: ", response);
		if ("message" in response.keys() && response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf create new player success? : ", response.success);
			// also get a JWT token here
			// send a different signal depending on registration success or failure
			if (response.success) {
				// if email confirmation is enabled for the game, we can't log in the player just yet
				bool email_conf_enabled = response.email_conf_enabled;
				sw_info("Player registration with username/password succeeded, player account autoconfirmed");
				logged_in_player = tmp_username;
				emit_signal("sw_registration_user_pwd_succeeded");
			} else {
				emit_signal("sw_registration_failed", response.error);
			}
		}
	}
}

void SW_Auth::_on_VerifyEmail_request_completed( result, response_code, headers, body ) {
	sw_info("VerifyEmail request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON::parse(body.get_string_from_utf8());
		var response = json.result;
		sw_debug("reponse: ", response);
		if ("message" in response.keys() && response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf verify email success? : ", response.success);
			// also get a JWT token here
			// send a different signal depending on registration success or failure
			if (response.success) {
				logged_in_player  = tmp_username;
				emit_signal("sw_email_verif_succeeded");
			} else {
				emit_signal("sw_email_verif_failed", response.error);
			}
		}
	}
}

void SW_Auth::_on_ResendConfCode_request_completed( result, response_code, headers, body ) {
	sw_info("ResendConfCode request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8());
		var response = json.result;
		sw_debug("reponse: ", response);
		if ("message" in response.keys() && response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth")
		} else {
			sw_info("SilentWolf resend conf code success? : ", response.success);
			// also get a JWT token here
			// send a different signal depending on registration success or failure
			if (response.success) {
				emit_signal("sw_resend_conf_code_succeeded");
			} else {
				emit_signal("sw_resend_conf_code_failed", response.error);
			}
		}
	}
}

void SW_Auth::_on_RequestPasswordReset_request_completed( result, response_code, headers, body ) {
	sw_info("RequestPasswordReset request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8());
		var response = json.result;
		sw_debug("reponse: ", response);
		if ("message" in response.keys() && response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf request player password reset success? : ", response.success);
			if (response.success) {
				emit_signal("sw_request_password_reset_succeeded");
			} else {
				emit_signal("sw_request_password_reset_failed", response.error);
			}
		}
}

void SW_Auth::_on_ResetPassword_request_completed( result, response_code, headers, body ) {
	sw_info("ResetPassword request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON::parse(body.get_string_from_utf8());
		var response = json.result;
		sw_debug("reponse: ", response);
		if ("message" in response.keys() and response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf reset player password success? : ", response.success);
			if (response.success) {
				emit_signal("sw_reset_password_succeeded");
			} else {
				emit_signal("sw_reset_password_failed", response.error);
			}
		}
	}
}

void SW_Auth::_on_ValidateSession_request_completed( result, response_code, headers, body ) {
	sw_info("SilentWolf - ValidateSession request completed");
	bool status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8());
		var response = json.result;
		sw_debug("reponse: ", response);
		if ("message" in response.keys() and response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf validate session success? : ", response.success);
			if (response.success) {
				set_player_logged_in(response.player_name);
				complete_session_check(logged_in_player);
			} else {
				complete_session_check(response.error);
			}
		}
}

void SW_Auth::_on_GetPlayerDetails_request_completed( result, response_code, headers, body ) {
	sw_info("SilentWolf - GetPlayerDetails request completed");
	var status_check = sw_check_status_code(response_code);
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON::parse(body.get_string_from_utf8());
		var response = json.result;
		sw_debug("reponse: ", response);
		if ("message" in response.keys() and response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/playerauth");
		} else {
			sw_info("SilentWolf get player details success? : ", response.success);
			if (response.success) {
				emit_signal("sw_get_player_details_succeeded", response.player_details);
			} else {
				emit_signal("sw_get_player_details_failed");
			}
	}
}

void SW_Auth::setup_complete_session_check_wait_timer() {
	complete_session_check_wait_timer = Timer.new();
	complete_session_check_wait_timer.set_one_shot(true);
	complete_session_check_wait_timer.set_wait_time(0.01);
	complete_session_check_wait_timer.connect("timeout", this, "complete_session_check");
	add_child(complete_session_check_wait_timer);
}

void SW_Auth::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		_ready();
	}
}

void SW_Auth::_bind_methods() {
	ADD_SIGNAL("sw_login_succeeded");
	ADD_SIGNAL("sw_login_failed");
	ADD_SIGNAL("sw_logout_succeeded");
	ADD_SIGNAL("sw_registration_succeeded");
	ADD_SIGNAL("sw_registration_user_pwd_succeeded");
	ADD_SIGNAL("sw_registration_failed");
	ADD_SIGNAL("sw_email_verif_succeeded");
	ADD_SIGNAL("sw_email_verif_failed");
	ADD_SIGNAL("sw_resend_conf_code_succeeded");
	ADD_SIGNAL("sw_resend_conf_code_failed");
	ADD_SIGNAL("sw_session_check_complete");
	ADD_SIGNAL("sw_request_password_reset_succeeded");
	ADD_SIGNAL("sw_request_password_reset_failed");
	ADD_SIGNAL("sw_reset_password_succeeded");
	ADD_SIGNAL("sw_reset_password_failed");
	ADD_SIGNAL("sw_get_player_details_succeeded");
	ADD_SIGNAL("sw_get_player_details_failed");
}

SW_Auth::SW_Auth() {
	logged_in_anon = false;
	login_timeout = 0;
	login_timer = nullptr;
	complete_session_check_wait_timer = nullptr;
}
