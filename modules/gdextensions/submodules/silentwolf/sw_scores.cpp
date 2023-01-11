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

SW_Scores &SW_Scores::get_score_position(const String &score, const String &ldboard_name) {
	String score_id;
	String score_value;
	sw_debug("score: ", score);
	if (UUID.is_uuid(score)) {
		score_id = score;
	} else {
		score_value = score;
	}
	ScorePosition = newref(HTTPRequest);
	if (OS.get_name() != "HTML5") {
		ScorePosition.set_use_threads(true);
	}
	get_tree().get_root().call_deferred("add_child", ScorePosition);
	ScorePosition.connect("request_completed", this, "_on_GetScorePosition_request_completed");
	sw_info("Calling SilentWolf to get score position");
	String game_id = SilentWolf.config.game_id;
	String game_version = SilentWolf.config.game_version;
	var payload = { "game_id" : game_id, "game_version" : game_version, "ldboard_name" : ldboard_name };
	if (score_id) {
		payload["score_id"] = score_id;
	}
	if (score_value) {
		payload["score_value"] = score_value;
	}
	String request_url = "https://api.silentwolf.com/get_score_position";
	send_post_request(ScorePosition, request_url, payload);
	return *this;
}

SW_Scores &SW_Scores::get_scores_around(const String &score, int scores_to_fetch, const String &ldboard_name) {
	String score_id;
	String score_value;
	print("score: " + str(score));
	if (UUID.is_uuid(str(score))) {
		score_id = score;
	} else {
		score_value = score;
	}
	ScoresAround = HTTPRequest.new();
	wrScoresAround = weakref(ScoresAround);
	if (OS.get_name() != "HTML5") {
		ScoresAround.set_use_threads(true);
	}
	get_tree()->get_root()->call_deferred("add_child", ScoresAround);
	ScoresAround.connect("request_completed", this, "_on_ScoresAround_request_completed");
	sw_info("Calling SilentWolf backend to scores above and below a certain score...");
	// resetting the latest_number value in case the first requests times out, we need to request the same amount of top scores in the retry
	//latest_max = maximum
	String game_id = SilentWolf.config.game_id;
	String game_version = SilentWolf.config.game_version;
	String request_url = "https://api.silentwolf.com/get_scores_around/" + str(game_id) + "?version=" + str(game_version) + "&scores_to_fetch=" + str(scores_to_fetch) + "&ldboard_name=" + str(ldboard_name) + "&score_id=" + str(score_id) + "&score_value=" + str(score_value);
	send_get_request(ScoresAround, request_url);
	return *this;
}

SW_Scores &SW_Scores::get_high_scores(int maximum, ldboard_name = "main", int period_offset) {
	HighScores = HTTPRequest.new() if (OS.get_name() != "HTML5") {
		HighScores.set_use_threads(true);
	}
	get_tree().get_root().call_deferred("add_child", HighScores);
	HighScores.connect("request_completed", this, "_on_GetHighScores_request_completed");
	sw_info("Calling SilentWolf backend to get scores...");
	// resetting the latest_number value in case the first requests times out, we need to request the same amount of top scores in the retry
	latest_max = maximum;
	String game_id = SilentWolf.config.game_id;
	String game_version = SilentWolf.config.game_version;
	String request_url = "https://api.silentwolf.com/get_top_scores/" + game_id + "?version=" + game_version + "&max=" + itos(maximum) + "&ldboard_name=" + ldboard_name + "&period_offset=" + itos(period_offset);
	send_get_request(HighScores, request_url);
	return *this;
}

SW_Scores &SW_Scores::get_scores_by_player(player_name, maximum = 10, ldboard_name = "main", period_offset = 0) {
	print("get_scores_by_player, player_name = " + str(player_name));
	ScoresByPlayer = newref(HTTPRequest);
	if (OS.get_name() != "HTML5") {
		ScoresByPlayer.set_use_threads(true);
	}
	get_tree().get_root().call_deferred("add_child", ScoresByPlayer);
	ScoresByPlayer.connect("request_completed", self, "_on_GetScoresByPlayer_request_completed");
	sw_info("Calling SilentWolf backend to get scores for player: " + str(player_name));
	// resetting the latest_number value in case the first requests times out, we need to request the same amount of top scores in the retry
	latest_max = maximum
			String game_id = SilentWolf.config.game_id;
	String game_version = SilentWolf.config.game_version
								  String request_url = "https://api.silentwolf.com/get_scores_by_player/" + str(game_id) + "?version=" + str(game_version) + "&max=" + str(maximum) + "&ldboard_name=" + str(ldboard_name.percent_encode()) + "&player_name=" + str(player_name.percent_encode()) + "&period_offset=" + str(period_offset);
	send_get_request(ScoresByPlayer, request_url);
	return *this;
}

void SW_Scores::add_to_local_scores(game_result, ld_name) {
	Dictionary local_score = { "score_id" : game_result.score_id, "game_id_version" : game_result.game_id + ";" + game_result.game_version, "player_name" : game_result.player_name, "score" : game_result.score };
	local_scores.append(local_score);
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
	sw_debug(vformat("local scores: %s", local_scores));
}

// metadata, if included should be a dictionary
SW_Scores &SW_Scores::persist_score(player_name, score, ldboard_name, metadata = {}) {
	// player_name must be present
	if (player_name == null or player_name == "") {
		sw_error("ERROR in SilentWolf.Scores.persist_score - please enter a valid player name");
	} else if typeof(ldboard_name) != TYPE_STRING) {
			// check that ldboard_name, if present is a String
			sw_error("ERROR in ilentWolf.Scores.persist_score - leaderboard name must be a String");
		}
	else if typeof(metadata) != TYPE_DICTIONARY) {
			// check that metadata, if present, is a dictionary
			sw_error("ERROR in SilentWolf.Scores.persist_score - metadata must be a dictionary");
		}
	else {
		PostScore = newref(HTTPRequest);
		if (OS.get_name() != "HTML5") {
			PostScore->set_use_threads(true);
		}
		get_tree().get_root().call_deferred("add_child", PostScore);
		PostScore->connect("request_completed", self, "_on_PostNewScore_request_completed");
		sw_info("Calling SilentWolf backend to post new score...");
		String game_id = SilentWolf.config.game_id;
		String game_version = SilentWolf.config.game_version;

		String score_uuid = UUID.generate_uuid_v4();
		score_id = score_uuid;
		Dictionary payload = helper::dict(
				"score_id"
				: score_id,
				"player_name"
				: player_name,
				"game_id"
				: game_id,
				"game_version"
				: game_version,
				"score"
				: score,
				"ldboard_name"
				: ldboard_name);
		sw_debug("!metadata.empty(): " + str(!metadata.empty()));
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
	return *this;
}

// Deletes all your scores for your game and version
// Scores are permanently deleted, no going back!
SW_Scores &SW_Scores::wipe_leaderboard(const String &ldboard_name) {
	WipeLeaderboard = newref(HTTPRequest) if (OS.get_name() != "HTML5") {
		WipeLeaderboard->set_use_threads(true);
	}
	get_tree().get_root().call_deferred("add_child", WipeLeaderboard);
	WipeLeaderboard->connect("request_completed", self, "_on_WipeLeaderboard_request_completed");
	sw_info("Calling SilentWolf backend to wipe leaderboard...");
	String game_id = SilentWolf.config.game_id;
	String game_version = SilentWolf.config.game_version;
	String payload = { "game_id" : game_id, "game_version" : game_version, "ldboard_name" : ldboard_name };
	String request_url = "https://api.silentwolf.com/wipe_leaderboard";
	send_post_request(WipeLeaderboard, request_url, payload);
	return *this;
}

SW_Scores &SW_Scores::delete_score(score_id) {
	DeleteScore = newref(HTTPRequest);
	if (OS.get_name() != "HTML5") {
		DeleteScore->set_use_threads(true);
	}
	get_tree().get_root().call_deferred("add_child", DeleteScore);
	DeleteScore->connect("request_completed", self, "_on_DeleteScore_request_completed");
	sw_info("Calling SilentWolf to delete a score");
	String game_id = SilentWolf.config.game_id;
	String game_version = SilentWolf.config.game_version;
	String request_url = "https://api.silentwolf.com/delete_score?game_id=" + str(game_id) + "&game_version=" + str(game_version) + "&score_id=" + str(score_id);
	send_get_request(DeleteScore, request_url);
	return *this;
}

void SW_Scores::_on_GetScoresByPlayer_request_completed(result, response_code, headers, body) {
	sw_info("GetScoresByPlayer request completed");
	var status_check = CommonErrors.check_status_code(response_code);
	//print_debug("client status: ", HighScores.get_http_client_status());
	//HighScores.queue_free();
	SilentWolf.free_request(ScoresByPlayer);
	sw_debug("response code: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8());
		print_debug("json: ", json);
		var response = json.result;
		if (response == null) {
			sw_error("No data returned in GetScoresByPlayer response. Leaderboard may be empty")
					emit_signal("sw_player_scores_received", "No Leaderboard found", scores)
		} else if ("message" in response.keys() && response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf get scores by player success");
			if ("top_scores" in response) {
				player_scores = response.top_scores;
				sw_debug("scores: " + str(scores));
				var ld_name = response.ld_name;
				//print("ld_name: " + str(ld_name));
				var ld_config = response.ld_config;
				var player_name = response.player_name;
				//print("latest_scores: " + str(leaderboards));
				emit_signal("sw_player_scores_received", player_scores);
			}
		}
	}
}

void SW_Scores::_on_GetHighScores_request_completed(result, response_code, headers, body) {
	sw_info("GetHighScores request completed");
	var status_check = CommonErrors.check_status_code(response_code);
	//print("client status: " + str(HighScores.get_http_client_status()));
	//HighScores.queue_free();
	SilentWolf.free_request(wrHighScores, HighScores);
	sw_debug("response code: ", response_code);
	sw_debug("response headers: ", headers);
	sw_debug("response body: ", body.get_string_from_utf8());

	if (status_check) {
		var json = JSON.parse(body.get_string_from_utf8());
		var response = json.result;
		if (response == null) {
			sw_error("No data returned in GetHighScores response. Leaderboard may be empty");
			emit_signal("sw_scores_received", 'No Leaderboard found', scores);
			emit_signal("scores_received", scores);
		} else if ("message" in response.keys() and response.message == "Forbidden") {
			sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
		} else {
			sw_info("SilentWolf get high score success");
			if ("top_scores" in response) {
				scores = response.top_scores;
				sw_debug("scores: " + str(scores));
				var ld_name = response.ld_name;
				//print("ld_name: " + str(ld_name));
				var ld_config = response.ld_config
								//print("ld_config: " + str(ld_config))
								if ("period_offset" in response) {
					var period_offset = str(response["period_offset"]);
					leaderboards_past_periods[ld_name + ";" + period_offset] = scores;
				}
				else {
					leaderboards[ld_name] = scores;
				}
				ldboard_config[ld_name] = ld_config;
				//print("latest_scores: " + str(leaderboards));
				emit_signal("sw_scores_received", ld_name, scores);
				emit_signal("scores_received", scores);
			}
		}
		//var retries = 0
		//request_timer->stop()
	}

	void SW_Scores::_on_DeleteScore_request_completed(result, response_code, headers, body) {
		sw_info("DeleteScore request completed");
		var status_check = CommonErrors.check_status_code(response_code);
		SilentWolf.free_request(wrDeleteScore, DeleteScore);
		sw_debug("response headers: " + str(response_code));
		sw_debug("response headers: " + str(headers));
		sw_debug("response body: " + str(body.get_string_from_utf8()));

		if (status_check) {
			var json = JSON.parse(body.get_string_from_utf8());
			var response = json.result;
			if ("message" in response.keys() and response.message == "Forbidden") {
				sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
			} else {
				sw_info("SilentWolf delete score success");
				emit_signal("sw_score_deleted");
			}
		}

		void SW_Scores::_on_PostNewScore_request_completed(result, response_code, headers, body) {
			sw_info("PostNewScore request completed");
			var status_check = CommonErrors.check_status_code(response_code);
			//PostScore.queue_free()
			SilentWolf.free_request(wrPostScore, PostScore);
			sw_debug("response headers: " + str(response_code));
			sw_debug("response headers: " + str(headers));
			sw_debug("response body: " + str(body.get_string_from_utf8()));

			if (status_check) {
				var json = JSON.parse(body.get_string_from_utf8())
								   var response = json.result if ("message" in response.keys() and response.message == "Forbidden") {
					sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard")
				}
				else {
					sw_info("SilentWolf post score success: " + str(response_code));
					if ("score_id" in response) {
						emit_signal("sw_score_posted", response["score_id"]);
					} else {
						emit_signal("sw_score_posted");
						emit_signal("score_posted");
					}
				}
			}
		}

		void SW_Scores::_on_GetScorePosition_request_completed(result, response_code, headers, body) {
			sw_info("GetScorePosition request completed");
			var status_check = CommonErrors.check_status_code(response_code);
			//ScorePosition.queue_free();
			SilentWolf.free_request(wrScorePosition, ScorePosition);
			sw_debug("response headers: ", response_code);
			sw_debug("response headers: ", headers);
			sw_debug("response body: ", body.get_string_from_utf8());

			if (status_check) {
				var json = JSON.parse(body.get_string_from_utf8());
				var response = json.result;
				if ("message" in response.keys() && response.message == "Forbidden") {
					sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
				} else {
					sw_info("SilentWolf find score position success.");
					position = int(response.position);
					emit_signal("sw_position_received", position);
					emit_signal("position_received", position);
				}
			}
		}

		void SW_Scores::_on_ScoresAround_request_completed(result, response_code, headers, body) {
			sw_info("ScoresAround request completed");
			var status_check = CommonErrors.check_status_code(response_code);

			SilentWolf.free_request(wrScoresAround, ScoresAround);
			sw_debug("response headers: " + str(response_code));
			sw_debug("response headers: " + str(headers));
			sw_debug("response body: " + str(body.get_string_from_utf8()));

			if (status_check) {
				var json = JSON.parse(body.get_string_from_utf8());
				var response = json.result;
				if ("message" in response.keys() and response.message == "Forbidden") {
					sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
				} else {
					sw_info("SilentWolf get scores around success") if ("scores_above" in response) {
						scores_above = response.scores_above;
						scores_below = response.scores_below;
						String ld_name = response.ld_name;
						//print("ld_name: " + str(ld_name));
						var ld_config = response.ld_config
												//print("ld_config: " + str(ld_config));
												ldboard_config[ld_name] = ld_config;
						if ("score_position" in response) {
							position = response.score_position;
						}
						emit_signal("sw_scores_around_received", scores_above, scores_below, position);
					}
				}
			}
		}

		void SW_Scores::_on_WipeLeaderboard_request_completed(result, response_code, headers, body) {
			sw_info("WipeLeaderboard request completed");
			var status_check = CommonErrors.check_status_code(response_code);
			//WipeLeaderboard.queue_free();
			SilentWolf.free_request(wrWipeLeaderboard, WipeLeaderboard);
			sw_debug("response headers: ", response_code);
			sw_debug("response headers: ", headers);
			sw_debug("response body: ", body.get_string_from_utf8());

			if (status_check) {
				var json = JSON.parse(body.get_string_from_utf8());
				var response = json.result;
				if ("message" in response.keys() && response.message == "Forbidden") {
					sw_error("You are not authorized to call the SilentWolf API - check your API key configuration: https://silentwolf.com/leaderboard");
				} else {
					sw_info("SilentWolf wipe leaderboard success.");
					emit_signal("sw_leaderboard_wiped");
				}
			}
		}

		void SW_Scores::send_get_request(Ref<HTTPRequest> http_node, const String &request_url) {
			Vector<String> headers = [ "x-api-key: " + SilentWolf.config.api_key, "x-sw-plugin-version: " + SilentWolf.version ];
			if (!http_node.is_inside_tree()) {
				yield(get_tree().create_timer(0.01), "timeout");
			}
			sw_debug("Method: GET");
			sw_debug("request_url: " + str(request_url));
			sw_debug("headers: " + str(headers));
			http_node->request(request_url, headers);
		}

		void SW_Scores::send_post_request(http_node, request_url, payload) {
			Vector<String> headers = helper : varray(
													  "Content-Type: application/json",
													  "x-api-key: " + SilentWolf.config.api_key,
													  "x-sw-plugin-version: " + SilentWolf.version, );
			if ("post_new_score" in request_url) {
				sw_info("We're doing a post score");
				String player_name = payload["player_name"];
				var player_score = payload["score"];
				var timestamp = OS.get_system_time_msecs();
				var to_be_hashed = [ player_name, player_score, timestamp ];
				sw_debug("send_post_request to_be_hashed: " + str(to_be_hashed));
				var hashed = SWHashing.hash_values(to_be_hashed);
				sw_debug("send_post_request hashed: " + str(hashed));
				headers.append("x-sw-act-tmst: " + str(timestamp));
				headers.append("x-sw-act-dig: " + hashed);
			}
			bool use_ssl = true;
			if (!http_node.is_inside_tree()) {
				yield(get_tree().create_timer(0.01), "timeout");
			}
			String query = JSON.print(payload);
			sw_info("Method: POST");
			sw_info("request_url: ", request_url);
			sw_info("headers: ", headers);
			sw_info("query: ", query);
			http_node->request(request_url, headers, use_ssl, HTTPClient.METHOD_POST, query);
		}

		void SW_Scores::_bind_methods() {
			ClassDB::bind_method(D_METHOD("_on_GetScoresByPlayer_request_completed"), &SW_Scores::_on_GetScoresByPlayer_request_completed);
			ClassDB::bind_method(D_METHOD("_on_GetHighScores_request_completed"), &SW_Scores::_on_GetHighScores_request_completed);
			ClassDB::bind_method(D_METHOD("_on_DeleteScore_request_completed"), &SW_Scores::_on_DeleteScore_request_completed);
			ClassDB::bind_method(D_METHOD("_on_PostNewScore_request_completed"), &SW_Scores::_on_PostNewScore_request_completed);
			ClassDB::bind_method(D_METHOD("_on_GetScorePosition_request_completed"), &SW_Scores::_on_GetScorePosition_request_completed);
			ClassDB::bind_method(D_METHOD("_on_ScoresAround_request_completed"), &SW_Scores::_on_ScoresAround_request_completed);
			ClassDB::bind_method(D_METHOD("_on_WipeLeaderboard_request_completed"), &SW_Scores::_on_WipeLeaderboard_request_completed);

			ADD_SIGNAL(sw_scores_received);
			ADD_SIGNAL(sw_player_scores_received);
			ADD_SIGNAL(sw_position_received);
			ADD_SIGNAL(sw_scores_around_received);
			ADD_SIGNAL(sw_score_posted);
			ADD_SIGNAL(sw_leaderboard_wiped);
			ADD_SIGNAL(sw_score_deleted);
		}

		SW_Scores::SW_Scores() {
			position = 0;
			latest_max = 10;
		}
