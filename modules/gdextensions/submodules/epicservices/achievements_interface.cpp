/**************************************************************************/
/*  achievements_interface.cpp                                            */
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

#include "gd_epic_services.h"

#include "epic_callback.h"
#include "epic_utils.h"

#include "eos_achievements.h"

#define ACH_GUARD_VOID(SIGNAL)                                                  \
	if (!achievements_handle) {                                                 \
		Dictionary payload;                                                     \
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured); \
		_emit_deferred(SIGNAL, payload);                                        \
		return;                                                                 \
	}

static void EOS_CALL _on_ach_query_defs(const EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::achievements_interface_query_definitions(const Dictionary &p_options) {
	ACH_GUARD_VOID("achievements_interface_query_definitions_callback");
	EOS_Achievements_QueryDefinitionsOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_QUERYDEFINITIONS_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("achievements_interface_query_definitions_callback")));
	EOS_Achievements_QueryDefinitions(achievements_handle, &opts, cb, &_on_ach_query_defs);
}

int EpicServices::achievements_interface_get_achievement_definition_count() {
	if (!achievements_handle) {
		return 0;
	}
	EOS_Achievements_GetAchievementDefinitionCountOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_GETACHIEVEMENTDEFINITIONCOUNT_API_LATEST;
	return int(EOS_Achievements_GetAchievementDefinitionCount(achievements_handle, &opts));
}

static Dictionary _ach_def_to_dict(const EOS_Achievements_DefinitionV2 *d) {
	Dictionary out;
	if (!d) {
		return out;
	}
	out["achievement_id"] = String::utf8(d->AchievementId ? d->AchievementId : "");
	out["unlocked_display_name"] = String::utf8(d->UnlockedDisplayName ? d->UnlockedDisplayName : "");
	out["unlocked_description"] = String::utf8(d->UnlockedDescription ? d->UnlockedDescription : "");
	out["locked_display_name"] = String::utf8(d->LockedDisplayName ? d->LockedDisplayName : "");
	out["locked_description"] = String::utf8(d->LockedDescription ? d->LockedDescription : "");
	out["flavor_text"] = String::utf8(d->FlavorText ? d->FlavorText : "");
	out["unlocked_icon_url"] = String::utf8(d->UnlockedIconURL ? d->UnlockedIconURL : "");
	out["locked_icon_url"] = String::utf8(d->LockedIconURL ? d->LockedIconURL : "");
	out["is_hidden"] = bool(d->bIsHidden == EOS_TRUE);
	Array thresholds;
	for (uint32_t i = 0; i < d->StatThresholdsCount; ++i) {
		Dictionary th;
		th["name"] = String::utf8(d->StatThresholds[i].Name ? d->StatThresholds[i].Name : "");
		th["threshold"] = int(d->StatThresholds[i].Threshold);
		thresholds.push_back(th);
	}
	out["stat_thresholds"] = thresholds;
	return out;
}

Dictionary EpicServices::achievements_interface_copy_achievement_definition_by_index(int p_index) {
	Dictionary out;
	if (!achievements_handle) {
		return out;
	}
	EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_COPYDEFINITIONV2BYINDEX_API_LATEST;
	opts.AchievementIndex = uint32_t(p_index);
	EOS_Achievements_DefinitionV2 *def = nullptr;
	if (EOS_Achievements_CopyAchievementDefinitionV2ByIndex(achievements_handle, &opts, &def) == EOS_EResult::EOS_Success && def) {
		out = _ach_def_to_dict(def);
		EOS_Achievements_DefinitionV2_Release(def);
	}
	return out;
}

Dictionary EpicServices::achievements_interface_copy_achievement_definition_by_id(const String &p_id) {
	Dictionary out;
	if (!achievements_handle) {
		return out;
	}
	const CharString cs = p_id.utf8();
	EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_COPYDEFINITIONV2BYACHIEVEMENTID_API_LATEST;
	opts.AchievementId = cs.get_data();
	EOS_Achievements_DefinitionV2 *def = nullptr;
	if (EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId(achievements_handle, &opts, &def) == EOS_EResult::EOS_Success && def) {
		out = _ach_def_to_dict(def);
		EOS_Achievements_DefinitionV2_Release(def);
	}
	return out;
}

static void EOS_CALL _on_ach_query_player(const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::achievements_interface_query_player_achievements(const Dictionary &p_options) {
	ACH_GUARD_VOID("achievements_interface_query_player_achievements_callback");
	EOS_Achievements_QueryPlayerAchievementsOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_QUERYPLAYERACHIEVEMENTS_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("achievements_interface_query_player_achievements_callback")));
	EOS_Achievements_QueryPlayerAchievements(achievements_handle, &opts, cb, &_on_ach_query_player);
}

int EpicServices::achievements_interface_get_player_achievement_count(const String &p_target_user_id) {
	if (!achievements_handle) {
		return 0;
	}
	EOS_Achievements_GetPlayerAchievementCountOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_GETPLAYERACHIEVEMENTCOUNT_API_LATEST;
	opts.UserId = eos_pui_from_string(p_target_user_id);
	return int(EOS_Achievements_GetPlayerAchievementCount(achievements_handle, &opts));
}

static Dictionary _player_ach_to_dict(const EOS_Achievements_PlayerAchievement *a) {
	Dictionary out;
	if (!a) {
		return out;
	}
	out["achievement_id"] = String::utf8(a->AchievementId ? a->AchievementId : "");
	out["progress"] = double(a->Progress);
	out["unlock_time"] = int64_t(a->UnlockTime);
	out["display_name"] = String::utf8(a->DisplayName ? a->DisplayName : "");
	out["description"] = String::utf8(a->Description ? a->Description : "");
	out["icon_url"] = String::utf8(a->IconURL ? a->IconURL : "");
	out["flavor_text"] = String::utf8(a->FlavorText ? a->FlavorText : "");
	Array stats;
	for (int32_t i = 0; i < a->StatInfoCount; ++i) {
		Dictionary s;
		s["name"] = String::utf8(a->StatInfo[i].Name ? a->StatInfo[i].Name : "");
		s["current_value"] = int(a->StatInfo[i].CurrentValue);
		s["threshold_value"] = int(a->StatInfo[i].ThresholdValue);
		stats.push_back(s);
	}
	out["stat_info"] = stats;
	return out;
}

Dictionary EpicServices::achievements_interface_copy_player_achievement_by_index(const Dictionary &p_options) {
	Dictionary out;
	if (!achievements_handle) {
		return out;
	}
	EOS_Achievements_CopyPlayerAchievementByIndexOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYINDEX_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	opts.AchievementIndex = uint32_t(dict_get_int(p_options, "achievement_index", 0));
	EOS_Achievements_PlayerAchievement *ach = nullptr;
	if (EOS_Achievements_CopyPlayerAchievementByIndex(achievements_handle, &opts, &ach) == EOS_EResult::EOS_Success && ach) {
		out = _player_ach_to_dict(ach);
		EOS_Achievements_PlayerAchievement_Release(ach);
	}
	return out;
}

Dictionary EpicServices::achievements_interface_copy_player_achievement_by_id(const Dictionary &p_options) {
	Dictionary out;
	if (!achievements_handle) {
		return out;
	}
	const CharString id_cs = dict_get_string(p_options, "achievement_id").utf8();
	EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYACHIEVEMENTID_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	opts.AchievementId = id_cs.get_data();
	EOS_Achievements_PlayerAchievement *ach = nullptr;
	if (EOS_Achievements_CopyPlayerAchievementByAchievementId(achievements_handle, &opts, &ach) == EOS_EResult::EOS_Success && ach) {
		out = _player_ach_to_dict(ach);
		EOS_Achievements_PlayerAchievement_Release(ach);
	}
	return out;
}

static void EOS_CALL _on_ach_unlock(const EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["user_id"] = eos_pui_to_string(data->UserId);
	payload["achievements_count"] = int(data->AchievementsCount);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::achievements_interface_unlock_achievements(const Dictionary &p_options) {
	ACH_GUARD_VOID("achievements_interface_unlock_achievements_callback");
	const Array ids = p_options.has("achievement_ids") ? Array(p_options["achievement_ids"]) : Array();
	Vector<CharString> id_storage;
	id_storage.resize(ids.size());
	Vector<const char *> id_ptrs;
	id_ptrs.resize(ids.size());
	for (int i = 0; i < ids.size(); ++i) {
		id_storage.write[i] = String(ids[i]).utf8();
		id_ptrs.write[i] = id_storage[i].length() ? id_storage[i].get_data() : "";
	}
	EOS_Achievements_UnlockAchievementsOptions opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
	opts.UserId = eos_pui_from_string(dict_get_string(p_options, "user_id"));
	opts.AchievementIds = id_ptrs.size() ? id_ptrs.ptrw() : nullptr;
	opts.AchievementsCount = uint32_t(id_ptrs.size());
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("achievements_interface_unlock_achievements_callback")));
	EOS_Achievements_UnlockAchievements(achievements_handle, &opts, cb, &_on_ach_unlock);
}

static void EOS_CALL _on_ach_unlocked(const EOS_Achievements_OnAchievementsUnlockedCallbackV2Info *data) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es) {
		return;
	}
	Dictionary payload;
	payload["user_id"] = eos_pui_to_string(data->UserId);
	payload["achievement_id"] = String::utf8(data->AchievementId ? data->AchievementId : "");
	payload["unlock_time"] = int64_t(data->UnlockTime);
	epic_emit_deferred(es->get_instance_id(), StringName("achievements_interface_achievement_unlocked"), payload);
}

uint64_t EpicServices::achievements_interface_add_notify_achievements_unlocked() {
	if (!achievements_handle) {
		return 0;
	}
	EOS_Achievements_AddNotifyAchievementsUnlockedV2Options opts = {};
	opts.ApiVersion = EOS_ACHIEVEMENTS_ADDNOTIFYACHIEVEMENTSUNLOCKEDV2_API_LATEST;
	return uint64_t(EOS_Achievements_AddNotifyAchievementsUnlockedV2(achievements_handle, &opts, nullptr, &_on_ach_unlocked));
}

void EpicServices::achievements_interface_remove_notify_achievements_unlocked(uint64_t p_id) {
	if (!achievements_handle) {
		return;
	}
	EOS_Achievements_RemoveNotifyAchievementsUnlocked(achievements_handle, EOS_NotificationId(p_id));
}
