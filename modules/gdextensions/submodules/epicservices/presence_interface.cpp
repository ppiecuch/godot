/**************************************************************************/
/*  presence_interface.cpp                                                */
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
#include "epic_presence_modification.h"
#include "epic_utils.h"

#include "eos_presence.h"

static void EOS_CALL _on_presence_query(const EOS_Presence_QueryPresenceCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_eaid_to_string(data->TargetUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::presence_interface_query_presence(const Dictionary &p_options) {
	if (!presence_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("presence_interface_query_presence_callback", payload);
		return;
	}
	EOS_Presence_QueryPresenceOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_QUERYPRESENCE_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("presence_interface_query_presence_callback")));
	EOS_Presence_QueryPresence(presence_handle, &opts, cb, &_on_presence_query);
}

bool EpicServices::presence_interface_has_presence(const Dictionary &p_options) {
	if (!presence_handle) {
		return false;
	}
	EOS_Presence_HasPresenceOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_HASPRESENCE_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	return EOS_Presence_HasPresence(presence_handle, &opts) == EOS_TRUE;
}

Dictionary EpicServices::presence_interface_copy_presence(const Dictionary &p_options) {
	Dictionary out;
	if (!presence_handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotConfigured);
		return out;
	}
	EOS_Presence_CopyPresenceOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_COPYPRESENCE_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	EOS_Presence_Info *info = nullptr;
	EOS_EResult r = EOS_Presence_CopyPresence(presence_handle, &opts, &info);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && info) {
		out["status"] = int(info->Status);
		out["user_id"] = eos_eaid_to_string(info->UserId);
		out["product_id"] = String::utf8(info->ProductId ? info->ProductId : "");
		out["product_version"] = String::utf8(info->ProductVersion ? info->ProductVersion : "");
		out["platform"] = String::utf8(info->Platform ? info->Platform : "");
		out["rich_text"] = String::utf8(info->RichText ? info->RichText : "");
		Array records;
		for (int32_t i = 0; i < info->RecordsCount; ++i) {
			Dictionary rec;
			rec["key"] = String::utf8(info->Records[i].Key ? info->Records[i].Key : "");
			rec["value"] = String::utf8(info->Records[i].Value ? info->Records[i].Value : "");
			records.push_back(rec);
		}
		out["records"] = records;
		out["integrated_platform"] = String::utf8(info->IntegratedPlatform ? info->IntegratedPlatform : "");
		EOS_Presence_Info_Release(info);
	}
	return out;
}

Ref<EpicPresenceModification> EpicServices::presence_interface_create_presence_modification(const String &p_local_user_id) {
	Ref<EpicPresenceModification> result;
	if (!presence_handle) {
		return result;
	}
	EOS_Presence_CreatePresenceModificationOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_CREATEPRESENCEMODIFICATION_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(p_local_user_id);
	EOS_HPresenceModification mod = nullptr;
	if (EOS_Presence_CreatePresenceModification(presence_handle, &opts, &mod) == EOS_EResult::EOS_Success && mod) {
		result.instance();
		result->set_handle(mod);
	}
	return result;
}

static void EOS_CALL _on_presence_set(const EOS_Presence_SetPresenceCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::presence_interface_set_presence(const Dictionary &p_options) {
	if (!presence_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("presence_interface_set_presence_callback", payload);
		return;
	}
	Ref<EpicPresenceModification> mod = p_options.has("presence_modification") ? Ref<EpicPresenceModification>(p_options["presence_modification"]) : Ref<EpicPresenceModification>();
	if (mod.is_null() || !mod->is_valid()) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_InvalidParameters);
		_emit_deferred("presence_interface_set_presence_callback", payload);
		return;
	}
	EOS_Presence_SetPresenceOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_SETPRESENCE_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.PresenceModificationHandle = mod->get_handle();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("presence_interface_set_presence_callback")));
	EOS_Presence_SetPresence(presence_handle, &opts, cb, &_on_presence_set);
}

String EpicServices::presence_interface_get_join_info(const Dictionary &p_options) {
	if (!presence_handle) {
		return String();
	}
	EOS_Presence_GetJoinInfoOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_GETJOININFO_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	int32_t buf_len = EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH + 1;
	CharString buf;
	buf.resize(buf_len);
	if (EOS_Presence_GetJoinInfo(presence_handle, &opts, buf.ptrw(), &buf_len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf.get_data());
}

static void EOS_CALL _on_presence_changed(const EOS_Presence_PresenceChangedCallbackInfo *data) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es) {
		return;
	}
	Dictionary payload;
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["presence_user_id"] = eos_eaid_to_string(data->PresenceUserId);
	epic_emit_deferred(es->get_instance_id(), StringName("presence_interface_on_presence_changed"), payload);
}

uint64_t EpicServices::presence_interface_add_notify_on_presence_changed() {
	if (!presence_handle) {
		return 0;
	}
	EOS_Presence_AddNotifyOnPresenceChangedOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_ADDNOTIFYONPRESENCECHANGED_API_LATEST;
	return uint64_t(EOS_Presence_AddNotifyOnPresenceChanged(presence_handle, &opts, nullptr, &_on_presence_changed));
}

void EpicServices::presence_interface_remove_notify_on_presence_changed(uint64_t p_id) {
	if (!presence_handle) {
		return;
	}
	EOS_Presence_RemoveNotifyOnPresenceChanged(presence_handle, EOS_NotificationId(p_id));
}

static void EOS_CALL _on_join_game_accepted(const EOS_Presence_JoinGameAcceptedCallbackInfo *data) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es) {
		return;
	}
	Dictionary payload;
	payload["join_info"] = String::utf8(data->JoinInfo ? data->JoinInfo : "");
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_eaid_to_string(data->TargetUserId);
	payload["ui_event_id"] = uint64_t(data->UiEventId);
	epic_emit_deferred(es->get_instance_id(), StringName("presence_interface_join_game_accepted"), payload);
}

uint64_t EpicServices::presence_interface_add_notify_join_game_accepted() {
	if (!presence_handle) {
		return 0;
	}
	EOS_Presence_AddNotifyJoinGameAcceptedOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_ADDNOTIFYJOINGAMEACCEPTED_API_LATEST;
	return uint64_t(EOS_Presence_AddNotifyJoinGameAccepted(presence_handle, &opts, nullptr, &_on_join_game_accepted));
}

void EpicServices::presence_interface_remove_notify_join_game_accepted(uint64_t p_id) {
	if (!presence_handle) {
		return;
	}
	EOS_Presence_RemoveNotifyJoinGameAccepted(presence_handle, EOS_NotificationId(p_id));
}
