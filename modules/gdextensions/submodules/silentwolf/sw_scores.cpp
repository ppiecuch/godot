/*************************************************************************/
/*  sw_scores.cpp                                                        */
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

#include "core/os/os.h"

void SW_Scores::sw_notification(int what) {
	if (what == SW_NOTIFICATION_PROCESS) {
		if (requesting) {
			int check = 0;
			for (auto &http_req : helper::vector(ScorePosition, ScoresAround, HighScores, ScoresByPlayer, WipeLeaderboard, PostScore, DeleteScore)) {
				if (http_req) {
					check += !http_req->poll();
				}
			}
			requesting = check > 0;
		}
	}
}

Array SW_Scores::get_scores(int what) {
	switch (what) {
		case SW_SCORES:
			return scores;
		case SW_LOCAL_SCORES:
			return local_scores;
		case SW_PLAYER_SCORES:
			return player_scores;
		case SW_SCORES_ABOVE:
			return scores_above;
		case SW_SCORES_BELOW:
			return scores_below;
	};
	return Array();
}

SW_Scores *SW_Scores::get_score_position(const String &score, const String &ldboard_name) {
	String score_id, score_value;
	sw_debug("score: ", score);
	if (is_uuid(score)) {
		score_id = score;
	} else {
		score_value = score;
	}
	ScorePosition = newref(BasicHTTPRequest);
	ScorePosition->connect("request_completed", this, "_on_GetScorePosition_request_completed");
	sw_info("Calling SilentWolf to get score position");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	Dictionary payload = helper::dict("game_id", game_id, "game_version", game_version, "ldboard_name", ldboard_name);
	if (!score_id.empty()) {
		payload["score_id"] = score_id;
	}
	if (!score_value.empty()) {
		payload["score_value"] = score_value;
	}
	String request_url = "https://api.silentwolf.com/get_score_position";
	send_post_request(ScorePosition, request_url, payload);
	return this;
}

SW_Scores *SW_Scores::get_scores_around(const String &score, int scores_to_fetch, const String &ldboard_name) {
	String score_id, score_value;
	sw_debug("score: ", score);
	if (is_uuid(score)) {
		score_id = score;
	} else {
		score_value = score;
	}
	if (!ScoresAround) {
		ScoresAround = newref(BasicHTTPRequest);
		ScoresAround->connect("request_completed", this, "_on_ScoresAround_request_completed");
	}
	ERR_FAIL_COND_V(ScoresAround->is_active_request(), this);
	sw_info("Calling SilentWolf backend to scores above and below a certain score...");
	// resetting the latest_number value in case the first requests times out, we need to request the same amount of top scores in the retry
	//latest_max = maximum
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String request_url = "https://api.silentwolf.com/get_scores_around/" + game_id + "?version=" + game_version + "&scores_to_fetch=" + itos(scores_to_fetch) + "&ldboard_name=" + ldboard_name + "&score_id=" + score_id + "&score_value=" + score_value;
	send_get_request(ScoresAround, request_url);
	return this;
}

SW_Scores *SW_Scores::get_high_scores(int maximum, const String &ldboard_name, int period_offset) {
	if (!HighScores) {
		HighScores = newref(BasicHTTPRequest);
		HighScores->connect("request_completed", this, "_on_GetHighScores_request_completed");
	}
	ERR_FAIL_COND_V(HighScores->is_active_request(), this);
	sw_info("Calling SilentWolf backend to get scores...");
	// resetting the latest_number value in case the first requests times out, we need to request the same amount of top scores in the retry
	latest_max = maximum;
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String request_url = "https://api.silentwolf.com/get_top_scores/" + game_id + "?version=" + game_version + "&max=" + itos(maximum) + "&ldboard_name=" + ldboard_name + "&period_offset=" + itos(period_offset);
	send_get_request(HighScores, request_url);
	return this;
}

SW_Scores *SW_Scores::get_scores_by_player(const String &player_name, int maximum, const String &ldboard_name, int period_offset) {
	if (player_name.empty()) {
		sw_error("Error in get_scores_by_player: provided player_name is empty");
	} else {
		sw_info("get_scores_by_player, player_name = ", player_name);
		ScoresByPlayer = newref(BasicHTTPRequest);
		ScoresByPlayer->connect("request_completed", this, "_on_GetScoresByPlayer_request_completed");
		sw_info("Calling SilentWolf backend to get scores for player: ", player_name);
		// resetting the latest_number value in case the first requests times out, we need to request the same amount of top scores in the retry
		latest_max = maximum;
		String game_id = SilentWolf::config["game_id"];
		String game_version = SilentWolf::config["game_version"];
		String request_url = "https://api.silentwolf.com/get_scores_by_player/" + game_id + "?version=" + game_version + "&max=" + itos(maximum) + "&ldboard_name=" + ldboard_name.percent_encode() + "&player_name=" + player_name.percent_encode() + "&period_offset=" + itos(period_offset);
		send_get_request(ScoresByPlayer, request_url);
	}
	return this;
}

SW_Scores *SW_Scores::get_top_score_by_player(const String &player_name, int maximum, const String &ldboard_name, int period_offset) {
	sw_info("get_top_score_by_player, player_name = ", player_name);
	if (player_name.empty()) {
		sw_error("Error in get_top_score_by_player: provided player_name is empty");
	} else {
		TopScoreByPlayer = newref(BasicHTTPRequest);
		TopScoreByPlayer->connect("request_completed", this, "_on_GetTopScoreByPlayer_request_completed");
		sw_info("Calling SilentWolf backend to get scores for player: ", player_name);
		// resetting the latest_number value in case the first requests times out, we need to request the same amount of top scores in the retry
		latest_max = maximum;
		const String &game_id = SilentWolf::config["game_id"];
		const String &game_version = SilentWolf::config["game_version"];
		const String &request_url = "https://api.silentwolf.com/get_top_score_by_player/" + game_id + "?version=" + game_version + "&max=" + itos(maximum) + "&ldboard_name=" + ldboard_name.percent_encode() + "&player_name=" + player_name.percent_encode() + "&period_offset=" + itos(period_offset);
		send_get_request(TopScoreByPlayer, request_url);
	}
	return this;
}

void SW_Scores::add_to_local_scores(const Dictionary &game_result, const String &ld_name) {
	Dictionary local_score = helper::dict(
			"score_id", game_result["score_id"],
			"game_id_version", vconcat(game_result["game_id"], ";", game_result["game_version"]),
			"player_name", game_result["player_name"],
			"score", game_result["score"]);
	local_scores.push_back(local_score);
	// if ld_name == "main":
	// 	TODO: even here, since the main leader board can be customized, we can't just blindly write to the local_scores variable and pull up the scores later
	// 	we need to know what type of leader board it is, or local caching is useless
	// 	local_scores.append(local_score)
	// else:
	// 	if ld_name in custom_local_scores:
	// 		TODO: problem: can't just append here - what if it's a highest/latest/accumulator/time-based leaderboard?
	// 		maybe don't use local scores for these special cases? performance?
	// 		custom_local_scores[ld_name].append(local_score)
	// 	else:
	// 		custom_local_scores[ld_name] = [local_score]
	sw_debug("local scores: ", local_scores);
}

// metadata, if included should be a dictionary
SW_Scores *SW_Scores::persist_score(const String &player_name, const String &score, const String &ldboard_name, const Dictionary &metadata) {
	// player_name must be present
	if (player_name.empty()) {
		sw_error("ERROR in SilentWolf.Scores.persist_score - please enter a valid player name");
	} else {
		if (!PostScore) {
			PostScore = newref(BasicHTTPRequest);
			PostScore->connect("request_completed", this, "_on_PostNewScore_request_completed");
		}
		ERR_FAIL_COND_V(PostScore->is_active_request(), this);
		sw_info("Calling SilentWolf backend to post new score...");
		String game_id = SilentWolf::config["game_id"];
		String game_version = SilentWolf::config["game_version"];

		String score_uuid = generate_uuid_v4();
		score_id = score_uuid;
		Dictionary payload = helper::dict(
				"score_id", score_id,
				"player_name", player_name,
				"game_id", game_id,
				"game_version", game_version,
				"score", score,
				"ldboard_name", ldboard_name);
		sw_debug("!metadata.empty(): ", !metadata.empty());
		if (!metadata.empty()) {
			sw_debug("metadata: ", metadata);
			payload["metadata"] = metadata;
		}
		sw_debug("payload: ", payload);
		// also add to local scores
		add_to_local_scores(payload);
		String request_url = "https://api.silentwolf.com/post_new_score";
		send_post_request(PostScore, request_url, payload);
	}
	return this;
}

// Deletes all your scores for your game and version
// Scores are permanently deleted, no going back!
SW_Scores *SW_Scores::wipe_leaderboard(const String &ldboard_name) {
	if (!WipeLeaderboard) {
		WipeLeaderboard = newref(BasicHTTPRequest);
		WipeLeaderboard->connect("request_completed", this, "_on_WipeLeaderboard_request_completed");
	}
	ERR_FAIL_COND_V(WipeLeaderboard->is_active_request(), this);
	sw_info("Calling SilentWolf backend to wipe leaderboard...");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	Dictionary payload = helper::dict("game_id", game_id, "game_version", game_version, "ldboard_name", ldboard_name);
	String request_url = "https://api.silentwolf.com/wipe_leaderboard";
	send_post_request(WipeLeaderboard, request_url, payload);
	return this;
}

SW_Scores *SW_Scores::delete_score(const String &score_id) {
	if (!DeleteScore) {
		DeleteScore = newref(BasicHTTPRequest);
		DeleteScore->connect("request_completed", this, "_on_DeleteScore_request_completed");
	}
	ERR_FAIL_COND_V(HighScores->is_active_request(), this);
	sw_info("Calling SilentWolf to delete a score");
	String game_id = SilentWolf::config["game_id"];
	String game_version = SilentWolf::config["game_version"];
	String request_url = "https://api.silentwolf.com/delete_score?game_id=" + game_id + "&game_version=" + game_version + "&score_id=" + score_id;
	send_get_request(DeleteScore, request_url);
	return this;
}

void SW_Scores::_on_GetScoresByPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("GetScoresByPlayer request completed");
	//sw_debug("client status: ", HighScores->get_http_client_status());
	sw_debug("response code: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (sw_check_status_code(response_code)) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("json: ", response);
		if (response.empty()) {
			sw_error("No data returned in GetScoresByPlayer response. Leaderboard may be empty");
			emit_signal("sw_player_scores_received", "No Leaderboard found", scores);
		} else if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf get scores by player success");
			if (response.has("top_scores")) {
				player_scores = response["top_scores"];
				sw_debug("scores: ", scores);
				const String &ld_name = response["ld_name"];
				//sw_debug("ld_name: ", ld_name);
				const Dictionary &ld_config = response["ld_config"];
				const String &player_name = response["player_name"];
				//sw_debug("latest_scores: ", leaderboards);
				emit_signal("sw_player_scores_received", player_scores);
			}
		}
	}
}

void SW_Scores::_on_GetTopScoreByPlayer_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("GetTopScoreByPlayer request completed");
	sw_debug("response code: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (sw_check_status_code(response_code)) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		sw_debug("json: ", response);
		if (response.empty()) {
			sw_error("No data returned in GetTopScoreByPlayer response. There was a problem getting the response from the backend.");
			emit_signal("sw_player_scores_received", "No Leaderboard found", scores);
		} else if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf get top score by player success");
			if (response.has("top_score")) {
				Dictionary top_score = response["top_scores"];
				sw_debug("top score from response: ", top_score);
				if (top_score.empty()) {
					player_top_score = Dictionary();
				} else {
					player_top_score = top_score;
				}
				sw_debug("top score: ", player_top_score);
				const String &ld_name = response["ld_name"];
				//sw_debug("ld_name: ", ld_name);
				Dictionary ld_config = response["ld_config"];
				const String &player_name = response["player_name"];
				//sw_debug("latest_scores: ", leaderboards);
				emit_signal("sw_top_player_score_received", player_top_score);
			}
		}
	}
}

void SW_Scores::_on_GetHighScores_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("GetHighScores request completed");
	//sw_debug("client status: ", HighScores->get_http_client_status());
	sw_debug("response code: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (sw_check_status_code(response_code)) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.empty()) {
			sw_error("No data returned in GetHighScores response. Leaderboard may be empty");
			emit_signal("sw_scores_received", "No Leaderboard found", scores);
			emit_signal("scores_received", scores);
		} else if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf get high score success");
			if (response.has("top_scores")) {
				scores = response["top_scores"];
				sw_debug("scores: ", scores);
				const String &ld_name = response["ld_name"];
				//sw_debug("ld_name: ", ld_name);
				Dictionary ld_config = response["ld_config"];
				//sw_debug("ld_config: ", ld_config);
				if (response["period_offset"]) {
					String period_offset = response["period_offset"];
					leaderboards_past_periods[ld_name + ";" + period_offset] = scores;
				} else {
					leaderboards[ld_name] = scores;
				}
				ldboard_config[ld_name] = ld_config;
				//sw_debug("latest_scores: ", leaderboards);
				emit_signal("sw_scores_received", array(ld_name, scores));
				emit_signal("scores_received", scores);
			}
		}
		//int retries = 0;
		//request_timer->stop();
	}
}

void SW_Scores::_on_DeleteScore_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("DeleteScore request completed");
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (sw_check_status_code(response_code)) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf delete score success");
			emit_signal("sw_score_deleted");
		}
	}
}

void SW_Scores::_on_PostNewScore_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("PostNewScore request completed");
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (sw_check_status_code(response_code)) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf post score success: ", response_code);
			if (response.has("score_id")) {
				emit_signal("sw_score_posted", response["score_id"]);
			} else {
				emit_signal("sw_score_posted");
			}
		}
	}
}

void SW_Scores::_on_GetScorePosition_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("GetScorePosition request completed");
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (sw_check_status_code(response_code)) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf find score position success.");
			position = response["position"];
			emit_signal("sw_position_received", position);
			emit_signal("position_received", position);
		}
	}
}

void SW_Scores::_on_ScoresAround_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("ScoresAround request completed");
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (sw_check_status_code(response_code)) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf get scores around success");
			if (response.has("scores_above")) {
				scores_above = response["scores_above"];
				scores_below = response["scores_below"];
				String ld_name = response["ld_name"];
				//sw_debug("ld_name: ", ld_name);
				Dictionary ld_config = response["ld_config"];
				//sw_debug("ld_config: ", ld_config);
				ldboard_config[ld_name] = ld_config;
				if (response.has("score_position")) {
					position = response["score_position"];
				}
				emit_signal("sw_scores_around_received", array(scores_above, scores_below, position));
			}
		}
	}
}

void SW_Scores::_on_WipeLeaderboard_request_completed(int result, int response_code, const PoolStringArray &headers, const PoolByteArray &body) {
	sw_info("WipeLeaderboard request completed");
	sw_debug("response headers: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", get_string_from_utf8(body));

	if (sw_check_status_code(response_code)) {
		Dictionary response = parse_json_from_string(get_string_from_utf8(body));
		if (response.has("message") && response["message"] == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf wipe leaderboard success.");
			emit_signal("sw_leaderboard_wiped");
		}
	}
}

void SW_Scores::send_get_request(Ref<BasicHTTPRequest> http_req, const String &request_url) {
	Vector<String> headers = helper::vector(
			"x-api-key: " + SilentWolf::cfg_str("api_key"),
			"x-sw-plugin-version: " + SilentWolf::version);
	sw_debug("Method: GET");
	sw_debug("request_url: ", request_url);
	sw_debug("headers: ", headers);
	if (OK == http_req->request(request_url, headers)) {
		requesting = true;
		emit_signal("sw_data_requested");
	} else {
		WARN_PRINT("Failed to start GET request.");
	}
}

void SW_Scores::send_post_request(Ref<BasicHTTPRequest> http_req, const String &request_url, const Dictionary &payload) {
	Vector<String> headers = helper::vector(
			application_json,
			"x-api-key: " + SilentWolf::cfg_str("api_key"),
			"x-sw-plugin-version: " + SilentWolf::version);
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
	if (OK == http_req->request(request_url, headers, SilentWolf::config["use_ssl"], HTTPClient::METHOD_POST, query)) {
		requesting = true;
		emit_signal("sw_data_requested");
	} else {
		WARN_PRINT("Failed to start POST request.");
	}
}

void SW_Scores::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_GetScoresByPlayer_request_completed"), &SW_Scores::_on_GetScoresByPlayer_request_completed);
	ClassDB::bind_method(D_METHOD("_on_GetHighScores_request_completed"), &SW_Scores::_on_GetHighScores_request_completed);
	ClassDB::bind_method(D_METHOD("_on_DeleteScore_request_completed"), &SW_Scores::_on_DeleteScore_request_completed);
	ClassDB::bind_method(D_METHOD("_on_PostNewScore_request_completed"), &SW_Scores::_on_PostNewScore_request_completed);
	ClassDB::bind_method(D_METHOD("_on_GetScorePosition_request_completed"), &SW_Scores::_on_GetScorePosition_request_completed);
	ClassDB::bind_method(D_METHOD("_on_ScoresAround_request_completed"), &SW_Scores::_on_ScoresAround_request_completed);
	ClassDB::bind_method(D_METHOD("_on_WipeLeaderboard_request_completed"), &SW_Scores::_on_WipeLeaderboard_request_completed);

	ClassDB::bind_method(D_METHOD("get_score_position", "score", "ldboard_name"), &SW_Scores::get_score_position, DEFVAL("main"));
	ClassDB::bind_method(D_METHOD("get_scores_around", "score", "scores_to_fetch", "ldboard_name"), &SW_Scores::get_scores_around, DEFVAL(3), DEFVAL("main"));
	ClassDB::bind_method(D_METHOD("get_high_scores", "maximum", "ldboard_name", "period_offset"), &SW_Scores::get_high_scores, DEFVAL(10), DEFVAL("main"), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_scores_by_player", "player_name", "maximum", "ldboard_name", "period_offset"), &SW_Scores::get_scores_by_player, DEFVAL(10), DEFVAL("main"), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("wipe_leaderboard", "ldboard_name"), &SW_Scores::wipe_leaderboard, DEFVAL("main"));
	ClassDB::bind_method(D_METHOD("persist_score", "player_name", "score", "ldboard_name", "metadata"), &SW_Scores::persist_score, DEFVAL("main"), DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("delete_score", "score_id"), &SW_Scores::delete_score);
	ClassDB::bind_method(D_METHOD("add_to_local_scores", "game_result", "ld_name"), &SW_Scores::add_to_local_scores, DEFVAL("main"));

	ClassDB::bind_method(D_METHOD("get_leaderboards"), &SW_Scores::get_leaderboards);
	ClassDB::bind_method(D_METHOD("get_ldboard_config"), &SW_Scores::get_ldboard_config);
	ClassDB::bind_method(D_METHOD("get_scores", "score_array"), &SW_Scores::get_scores);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "leaderboards"), "", "get_leaderboards");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "ldboard_config"), "", "get_ldboard_config");
	ADD_PROPERTYI(PropertyInfo(Variant::ARRAY, "scores"), "", "get_scores", SW_SCORES);
	ADD_PROPERTYI(PropertyInfo(Variant::ARRAY, "local_scores"), "", "get_scores", SW_LOCAL_SCORES);
	ADD_PROPERTYI(PropertyInfo(Variant::ARRAY, "player_scores"), "", "get_scores", SW_PLAYER_SCORES);
	ADD_PROPERTYI(PropertyInfo(Variant::ARRAY, "scores_above"), "", "get_scores", SW_SCORES_ABOVE);
	ADD_PROPERTYI(PropertyInfo(Variant::ARRAY, "scores_below"), "", "get_scores", SW_SCORES_BELOW);

	BIND_ENUM_CONSTANT(SW_SCORES);
	BIND_ENUM_CONSTANT(SW_LOCAL_SCORES);
	BIND_ENUM_CONSTANT(SW_PLAYER_SCORES);
	BIND_ENUM_CONSTANT(SW_SCORES_ABOVE);
	BIND_ENUM_CONSTANT(SW_SCORES_BELOW);

	ADD_SIGNAL(MethodInfo("sw_data_requested"));
	ADD_SIGNAL(MethodInfo("sw_scores_received"));
	ADD_SIGNAL(MethodInfo("sw_player_scores_received"));
	ADD_SIGNAL(MethodInfo("sw_top_player_score_received"));
	ADD_SIGNAL(MethodInfo("sw_position_received"));
	ADD_SIGNAL(MethodInfo("sw_scores_around_received", PropertyInfo(Variant::ARRAY, "scores_above"), PropertyInfo(Variant::ARRAY, "scores_below"), PropertyInfo(Variant::INT, "position")));
	ADD_SIGNAL(MethodInfo("sw_score_posted"));
	ADD_SIGNAL(MethodInfo("sw_leaderboard_wiped"));
	ADD_SIGNAL(MethodInfo("sw_score_deleted"));
}

SW_Scores::SW_Scores() {
	requesting = false;
	position = 0;
	latest_max = 10;
}
