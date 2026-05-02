/**************************************************************************/
/*  custom_invites_interface.cpp                                          */
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

#include "eos_custominvites.h"

int EpicServices::custom_invites_interface_set_custom_invite(const Dictionary &p_options) {
	if (!custom_invites_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString payload_cs = dict_get_string(p_options, "payload").utf8();
	EOS_CustomInvites_SetCustomInviteOptions opts = {};
	opts.ApiVersion = EOS_CUSTOMINVITES_SETCUSTOMINVITE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Payload = payload_cs.length() ? payload_cs.get_data() : nullptr;
	return int(EOS_CustomInvites_SetCustomInvite(custom_invites_handle, &opts));
}

static void EOS_CALL _on_ci_send(const EOS_CustomInvites_SendCustomInviteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::custom_invites_interface_send_custom_invite(const Dictionary &p_options) {
	if (!custom_invites_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("custom_invites_interface_send_custom_invite_callback", payload);
		return;
	}
	const Array targets = p_options.has("target_user_ids") ? Array(p_options["target_user_ids"]) : Array();
	Vector<EOS_ProductUserId> ids;
	ids.resize(targets.size());
	for (int i = 0; i < targets.size(); ++i) {
		ids.write[i] = eos_pui_from_string(String(targets[i]));
	}
	EOS_CustomInvites_SendCustomInviteOptions opts = {};
	opts.ApiVersion = EOS_CUSTOMINVITES_SENDCUSTOMINVITE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserIds = ids.size() ? ids.ptrw() : nullptr;
	opts.TargetUserIdsCount = uint32_t(ids.size());
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("custom_invites_interface_send_custom_invite_callback")));
	EOS_CustomInvites_SendCustomInvite(custom_invites_handle, &opts, cb, &_on_ci_send);
}

int EpicServices::custom_invites_interface_finalize_invite(const Dictionary &p_options) {
	if (!custom_invites_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString custom_invite_id_cs = dict_get_string(p_options, "custom_invite_id").utf8();
	EOS_CustomInvites_FinalizeInviteOptions opts = {};
	opts.ApiVersion = EOS_CUSTOMINVITES_FINALIZEINVITE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	opts.CustomInviteId = custom_invite_id_cs.get_data();
	opts.ProcessingResult = EOS_EResult(dict_get_int(p_options, "processing_result", int(EOS_EResult::EOS_Success)));
	return int(EOS_CustomInvites_FinalizeInvite(custom_invites_handle, &opts));
}

#define CI_NOTIFY_HELPER(NAME, OPT_TY, OPT_API, ADD_FN, REMOVE_FN, INFO_TY, SIGNAL, FILL_BLOCK) \
	static void EOS_CALL _on_ci_##NAME(const INFO_TY *data) {                                   \
		EpicServices *es = EpicServices::get_singleton();                                       \
		if (!es) {                                                                              \
			return;                                                                             \
		}                                                                                       \
		Dictionary payload;                                                                     \
		FILL_BLOCK                                                                              \
		epic_emit_deferred(es->get_instance_id(), StringName(SIGNAL), payload);                 \
	}                                                                                           \
	uint64_t EpicServices::custom_invites_interface_add_notify_##NAME() {                       \
		if (!custom_invites_handle) {                                                           \
			return 0;                                                                           \
		}                                                                                       \
		OPT_TY opts = {};                                                                       \
		opts.ApiVersion = OPT_API;                                                              \
		return uint64_t(ADD_FN(custom_invites_handle, &opts, nullptr, &_on_ci_##NAME));         \
	}                                                                                           \
	void EpicServices::custom_invites_interface_remove_notify_##NAME(uint64_t p_id) {           \
		if (!custom_invites_handle) {                                                           \
			return;                                                                             \
		}                                                                                       \
		REMOVE_FN(custom_invites_handle, EOS_NotificationId(p_id));                             \
	}

CI_NOTIFY_HELPER(
		custom_invite_received,
		EOS_CustomInvites_AddNotifyCustomInviteReceivedOptions,
		EOS_CUSTOMINVITES_ADDNOTIFYCUSTOMINVITERECEIVED_API_LATEST,
		EOS_CustomInvites_AddNotifyCustomInviteReceived,
		EOS_CustomInvites_RemoveNotifyCustomInviteReceived,
		EOS_CustomInvites_OnCustomInviteReceivedCallbackInfo,
		"custom_invites_interface_custom_invite_received",
		{
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["custom_invite_id"] = String::utf8(data->CustomInviteId ? data->CustomInviteId : "");
			payload["payload"] = String::utf8(data->Payload ? data->Payload : "");
		})

CI_NOTIFY_HELPER(
		custom_invite_accepted,
		EOS_CustomInvites_AddNotifyCustomInviteAcceptedOptions,
		EOS_CUSTOMINVITES_ADDNOTIFYCUSTOMINVITEACCEPTED_API_LATEST,
		EOS_CustomInvites_AddNotifyCustomInviteAccepted,
		EOS_CustomInvites_RemoveNotifyCustomInviteAccepted,
		EOS_CustomInvites_OnCustomInviteAcceptedCallbackInfo,
		"custom_invites_interface_custom_invite_accepted",
		{
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["custom_invite_id"] = String::utf8(data->CustomInviteId ? data->CustomInviteId : "");
			payload["payload"] = String::utf8(data->Payload ? data->Payload : "");
		})

CI_NOTIFY_HELPER(
		custom_invite_rejected,
		EOS_CustomInvites_AddNotifyCustomInviteRejectedOptions,
		EOS_CUSTOMINVITES_ADDNOTIFYCUSTOMINVITEREJECTED_API_LATEST,
		EOS_CustomInvites_AddNotifyCustomInviteRejected,
		EOS_CustomInvites_RemoveNotifyCustomInviteRejected,
		EOS_CustomInvites_CustomInviteRejectedCallbackInfo,
		"custom_invites_interface_custom_invite_rejected",
		{
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["custom_invite_id"] = String::utf8(data->CustomInviteId ? data->CustomInviteId : "");
			payload["payload"] = String::utf8(data->Payload ? data->Payload : "");
		})
