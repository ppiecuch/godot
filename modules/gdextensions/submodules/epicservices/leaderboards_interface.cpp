/**************************************************************************/
/*  leaderboards_interface.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#include "gd_epic_services.h"

#include "epic_callback.h"
#include "epic_utils.h"

#include "eos_leaderboards.h"

#define LB_GUARD_VOID(SIGNAL)                                                   \
	if (!leaderboards_handle) {                                                 \
		Dictionary payload;                                                     \
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured); \
		_emit_deferred(SIGNAL, payload);                                        \
		return;                                                                 \
	}

static void EOS_CALL _on_lb_query_defs(const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::leaderboards_interface_query_leaderboard_definitions(const Dictionary &p_options) {
	LB_GUARD_VOID("leaderboards_interface_query_leaderboard_definitions_callback");
	EOS_Leaderboards_QueryLeaderboardDefinitionsOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST;
	opts.StartTime = int64_t(dict_get_int64(p_options, "start_time", EOS_LEADERBOARDS_TIME_UNDEFINED));
	opts.EndTime = int64_t(dict_get_int64(p_options, "end_time", EOS_LEADERBOARDS_TIME_UNDEFINED));
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("leaderboards_interface_query_leaderboard_definitions_callback")));
	EOS_Leaderboards_QueryLeaderboardDefinitions(leaderboards_handle, &opts, cb, &_on_lb_query_defs);
}

int EpicServices::leaderboards_interface_get_leaderboard_definition_count() {
	if (!leaderboards_handle) {
		return 0;
	}
	EOS_Leaderboards_GetLeaderboardDefinitionCountOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDDEFINITIONCOUNT_API_LATEST;
	return int(EOS_Leaderboards_GetLeaderboardDefinitionCount(leaderboards_handle, &opts));
}

static Dictionary _lb_def_to_dict(const EOS_Leaderboards_Definition *d) {
	Dictionary out;
	if (!d) {
		return out;
	}
	out["leaderboard_id"] = String::utf8(d->LeaderboardId ? d->LeaderboardId : "");
	out["stat_name"] = String::utf8(d->StatName ? d->StatName : "");
	out["aggregation"] = int(d->Aggregation);
	out["start_time"] = int64_t(d->StartTime);
	out["end_time"] = int64_t(d->EndTime);
	return out;
}

Dictionary EpicServices::leaderboards_interface_copy_leaderboard_definition_by_index(int p_index) {
	Dictionary out;
	if (!leaderboards_handle) {
		return out;
	}
	EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYINDEX_API_LATEST;
	opts.LeaderboardIndex = uint32_t(p_index);
	EOS_Leaderboards_Definition *def = nullptr;
	if (EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(leaderboards_handle, &opts, &def) == EOS_EResult::EOS_Success && def) {
		out = _lb_def_to_dict(def);
		EOS_Leaderboards_Definition_Release(def);
	}
	return out;
}

Dictionary EpicServices::leaderboards_interface_copy_leaderboard_definition_by_id(const String &p_id) {
	Dictionary out;
	if (!leaderboards_handle) {
		return out;
	}
	const CharString cs = p_id.utf8();
	EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYLEADERBOARDID_API_LATEST;
	opts.LeaderboardId = cs.get_data();
	EOS_Leaderboards_Definition *def = nullptr;
	if (EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(leaderboards_handle, &opts, &def) == EOS_EResult::EOS_Success && def) {
		out = _lb_def_to_dict(def);
		EOS_Leaderboards_Definition_Release(def);
	}
	return out;
}

static void EOS_CALL _on_lb_query_ranks(const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::leaderboards_interface_query_leaderboard_ranks(const Dictionary &p_options) {
	LB_GUARD_VOID("leaderboards_interface_query_leaderboard_ranks_callback");
	const CharString lb_id_cs = dict_get_string(p_options, "leaderboard_id").utf8();
	EOS_Leaderboards_QueryLeaderboardRanksOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDRANKS_API_LATEST;
	opts.LeaderboardId = lb_id_cs.get_data();
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("leaderboards_interface_query_leaderboard_ranks_callback")));
	EOS_Leaderboards_QueryLeaderboardRanks(leaderboards_handle, &opts, cb, &_on_lb_query_ranks);
}

int EpicServices::leaderboards_interface_get_leaderboard_record_count() {
	if (!leaderboards_handle) {
		return 0;
	}
	EOS_Leaderboards_GetLeaderboardRecordCountOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDRECORDCOUNT_API_LATEST;
	return int(EOS_Leaderboards_GetLeaderboardRecordCount(leaderboards_handle, &opts));
}

static Dictionary _lb_record_to_dict(const EOS_Leaderboards_LeaderboardRecord *r) {
	Dictionary out;
	if (!r) {
		return out;
	}
	out["user_id"] = eos_pui_to_string(r->UserId);
	out["rank"] = int(r->Rank);
	out["score"] = int(r->Score);
	out["user_display_name"] = String::utf8(r->UserDisplayName ? r->UserDisplayName : "");
	return out;
}

Dictionary EpicServices::leaderboards_interface_copy_leaderboard_record_by_index(int p_index) {
	Dictionary out;
	if (!leaderboards_handle) {
		return out;
	}
	EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYINDEX_API_LATEST;
	opts.LeaderboardRecordIndex = uint32_t(p_index);
	EOS_Leaderboards_LeaderboardRecord *rec = nullptr;
	if (EOS_Leaderboards_CopyLeaderboardRecordByIndex(leaderboards_handle, &opts, &rec) == EOS_EResult::EOS_Success && rec) {
		out = _lb_record_to_dict(rec);
		EOS_Leaderboards_LeaderboardRecord_Release(rec);
	}
	return out;
}

Dictionary EpicServices::leaderboards_interface_copy_leaderboard_record_by_user_id(const String &p_user_id) {
	Dictionary out;
	if (!leaderboards_handle) {
		return out;
	}
	EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYUSERID_API_LATEST;
	opts.UserId = eos_pui_from_string(p_user_id);
	EOS_Leaderboards_LeaderboardRecord *rec = nullptr;
	if (EOS_Leaderboards_CopyLeaderboardRecordByUserId(leaderboards_handle, &opts, &rec) == EOS_EResult::EOS_Success && rec) {
		out = _lb_record_to_dict(rec);
		EOS_Leaderboards_LeaderboardRecord_Release(rec);
	}
	return out;
}

static void EOS_CALL _on_lb_query_user_scores(const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::leaderboards_interface_query_leaderboard_user_scores(const Dictionary &p_options) {
	LB_GUARD_VOID("leaderboards_interface_query_leaderboard_user_scores_callback");
	const Array user_ids = p_options.has("user_ids") ? Array(p_options["user_ids"]) : Array();
	Vector<EOS_ProductUserId> ids;
	ids.resize(user_ids.size());
	for (int i = 0; i < user_ids.size(); ++i) {
		ids.write[i] = eos_pui_from_string(String(user_ids[i]));
	}
	const Array stat_infos = p_options.has("stat_info") ? Array(p_options["stat_info"]) : Array();
	Vector<CharString> name_storage;
	name_storage.resize(stat_infos.size());
	Vector<EOS_Leaderboards_UserScoresQueryStatInfo> infos;
	infos.resize(stat_infos.size());
	for (int i = 0; i < stat_infos.size(); ++i) {
		const Dictionary si = Dictionary(stat_infos[i]);
		name_storage.write[i] = String(dict_get_string(si, "stat_name")).utf8();
		infos.write[i].ApiVersion = EOS_LEADERBOARDS_USERSCORESQUERYSTATINFO_API_LATEST;
		infos.write[i].StatName = name_storage[i].get_data();
		infos.write[i].Aggregation = EOS_ELeaderboardAggregation(dict_get_int(si, "aggregation", int(EOS_ELeaderboardAggregation::EOS_LA_Min)));
	}
	EOS_Leaderboards_QueryLeaderboardUserScoresOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDUSERSCORES_API_LATEST;
	opts.UserIds = ids.size() ? ids.ptrw() : nullptr;
	opts.UserIdsCount = uint32_t(ids.size());
	opts.StatInfo = infos.size() ? infos.ptrw() : nullptr;
	opts.StatInfoCount = uint32_t(infos.size());
	opts.StartTime = int64_t(dict_get_int64(p_options, "start_time", EOS_LEADERBOARDS_TIME_UNDEFINED));
	opts.EndTime = int64_t(dict_get_int64(p_options, "end_time", EOS_LEADERBOARDS_TIME_UNDEFINED));
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("leaderboards_interface_query_leaderboard_user_scores_callback")));
	EOS_Leaderboards_QueryLeaderboardUserScores(leaderboards_handle, &opts, cb, &_on_lb_query_user_scores);
}

int EpicServices::leaderboards_interface_get_leaderboard_user_score_count(const String &p_stat_name) {
	if (!leaderboards_handle) {
		return 0;
	}
	const CharString cs = p_stat_name.utf8();
	EOS_Leaderboards_GetLeaderboardUserScoreCountOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDUSERSCORECOUNT_API_LATEST;
	opts.StatName = cs.get_data();
	return int(EOS_Leaderboards_GetLeaderboardUserScoreCount(leaderboards_handle, &opts));
}

static Dictionary _lb_user_score_to_dict(const EOS_Leaderboards_LeaderboardUserScore *s) {
	Dictionary out;
	if (!s) {
		return out;
	}
	out["user_id"] = eos_pui_to_string(s->UserId);
	out["score"] = int(s->Score);
	return out;
}

Dictionary EpicServices::leaderboards_interface_copy_leaderboard_user_score_by_index(const Dictionary &p_options) {
	Dictionary out;
	if (!leaderboards_handle) {
		return out;
	}
	const CharString stat_cs = dict_get_string(p_options, "stat_name").utf8();
	EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYINDEX_API_LATEST;
	opts.LeaderboardUserScoreIndex = uint32_t(dict_get_int(p_options, "index", 0));
	opts.StatName = stat_cs.get_data();
	EOS_Leaderboards_LeaderboardUserScore *score = nullptr;
	if (EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(leaderboards_handle, &opts, &score) == EOS_EResult::EOS_Success && score) {
		out = _lb_user_score_to_dict(score);
		EOS_Leaderboards_LeaderboardUserScore_Release(score);
	}
	return out;
}

Dictionary EpicServices::leaderboards_interface_copy_leaderboard_user_score_by_user_id(const Dictionary &p_options) {
	Dictionary out;
	if (!leaderboards_handle) {
		return out;
	}
	const CharString stat_cs = dict_get_string(p_options, "stat_name").utf8();
	EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions opts = {};
	opts.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYUSERID_API_LATEST;
	opts.UserId = eos_pui_from_string(dict_get_string(p_options, "user_id"));
	opts.StatName = stat_cs.get_data();
	EOS_Leaderboards_LeaderboardUserScore *score = nullptr;
	if (EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(leaderboards_handle, &opts, &score) == EOS_EResult::EOS_Success && score) {
		out = _lb_user_score_to_dict(score);
		EOS_Leaderboards_LeaderboardUserScore_Release(score);
	}
	return out;
}
