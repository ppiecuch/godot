/**************************************************************************/
/*  lobby_interface.cpp                                                   */
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
#include "epic_lobby_details.h"
#include "epic_lobby_modification.h"
#include "epic_lobby_search.h"
#include "epic_utils.h"

#include "eos_lobby.h"

// Helper macro: emit a "not configured" payload if the lobby handle isn't ready.
#define LOBBY_GUARD_VOID(SIGNAL)                                                \
	if (!lobby_handle) {                                                        \
		Dictionary payload;                                                     \
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured); \
		_emit_deferred(SIGNAL, payload);                                        \
		return;                                                                 \
	}

// ---- Create / Destroy / Join / Leave -------------------------------------

static void EOS_CALL _on_lobby_create(const EOS_Lobby_CreateLobbyCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_create_lobby(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_create_lobby_callback");
	const CharString bucket_cs = dict_get_string(p_options, "bucket_id").utf8();
	const CharString lobby_id_override_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_CreateLobbyOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.MaxLobbyMembers = uint32_t(dict_get_int(p_options, "max_lobby_members", 4));
	opts.PermissionLevel = EOS_ELobbyPermissionLevel(dict_get_int(p_options, "permission_level", int(EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED)));
	opts.bPresenceEnabled = dict_get_bool(p_options, "presence_enabled", false) ? EOS_TRUE : EOS_FALSE;
	opts.bAllowInvites = dict_get_bool(p_options, "allow_invites", true) ? EOS_TRUE : EOS_FALSE;
	opts.BucketId = bucket_cs.length() ? bucket_cs.get_data() : nullptr;
	opts.bDisableHostMigration = dict_get_bool(p_options, "disable_host_migration", false) ? EOS_TRUE : EOS_FALSE;
	opts.bEnableRTCRoom = dict_get_bool(p_options, "enable_rtc_room", false) ? EOS_TRUE : EOS_FALSE;
	opts.LocalRTCOptions = nullptr;
	opts.LobbyId = lobby_id_override_cs.length() ? lobby_id_override_cs.get_data() : nullptr;
	opts.bEnableJoinById = dict_get_bool(p_options, "enable_join_by_id", false) ? EOS_TRUE : EOS_FALSE;
	opts.bRejoinAfterKickRequiresInvite = dict_get_bool(p_options, "rejoin_after_kick_requires_invite", false) ? EOS_TRUE : EOS_FALSE;
	opts.AllowedPlatformIds = nullptr;
	opts.AllowedPlatformIdsCount = 0;
	opts.bCrossplayOptOut = dict_get_bool(p_options, "crossplay_opt_out", false) ? EOS_TRUE : EOS_FALSE;
	opts.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType(dict_get_int(p_options, "rtc_room_join_action_type", int(EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin)));

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_create_lobby_callback")));
	EOS_Lobby_CreateLobby(lobby_handle, &opts, cb, &_on_lobby_create);
}

static void EOS_CALL _on_lobby_destroy(const EOS_Lobby_DestroyLobbyCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_destroy_lobby(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_destroy_lobby_callback");
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_DestroyLobbyOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_DESTROYLOBBY_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.LobbyId = lobby_id_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_destroy_lobby_callback")));
	EOS_Lobby_DestroyLobby(lobby_handle, &opts, cb, &_on_lobby_destroy);
}

static void EOS_CALL _on_lobby_join(const EOS_Lobby_JoinLobbyCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_join_lobby(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_join_lobby_callback");
	Ref<EpicLobbyDetails> details = p_options.has("lobby_details") ? Ref<EpicLobbyDetails>(p_options["lobby_details"]) : Ref<EpicLobbyDetails>();
	if (details.is_null() || !details->is_valid()) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_InvalidParameters);
		_emit_deferred("lobby_interface_join_lobby_callback", payload);
		return;
	}
	EOS_Lobby_JoinLobbyOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
	opts.LobbyDetailsHandle = details->get_handle();
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.bPresenceEnabled = dict_get_bool(p_options, "presence_enabled", false) ? EOS_TRUE : EOS_FALSE;
	opts.LocalRTCOptions = nullptr;
	opts.bCrossplayOptOut = dict_get_bool(p_options, "crossplay_opt_out", false) ? EOS_TRUE : EOS_FALSE;
	opts.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType(dict_get_int(p_options, "rtc_room_join_action_type", int(EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin)));

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_join_lobby_callback")));
	EOS_Lobby_JoinLobby(lobby_handle, &opts, cb, &_on_lobby_join);
}

static void EOS_CALL _on_lobby_join_by_id(const EOS_Lobby_JoinLobbyByIdCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_join_lobby_by_id(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_join_lobby_by_id_callback");
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_JoinLobbyByIdOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_JOINLOBBYBYID_API_LATEST;
	opts.LobbyId = lobby_id_cs.get_data();
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.bPresenceEnabled = dict_get_bool(p_options, "presence_enabled", false) ? EOS_TRUE : EOS_FALSE;
	opts.LocalRTCOptions = nullptr;
	opts.bCrossplayOptOut = dict_get_bool(p_options, "crossplay_opt_out", false) ? EOS_TRUE : EOS_FALSE;
	opts.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType(dict_get_int(p_options, "rtc_room_join_action_type", int(EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin)));

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_join_lobby_by_id_callback")));
	EOS_Lobby_JoinLobbyById(lobby_handle, &opts, cb, &_on_lobby_join_by_id);
}

static void EOS_CALL _on_lobby_leave(const EOS_Lobby_LeaveLobbyCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_leave_lobby(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_leave_lobby_callback");
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_LeaveLobbyOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.LobbyId = lobby_id_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_leave_lobby_callback")));
	EOS_Lobby_LeaveLobby(lobby_handle, &opts, cb, &_on_lobby_leave);
}

// ---- Update modification + Update lobby -----------------------------------

Ref<EpicLobbyModification> EpicServices::lobby_interface_update_lobby_modification(const Dictionary &p_options) {
	Ref<EpicLobbyModification> result;
	if (!lobby_handle) {
		return result;
	}
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_UpdateLobbyModificationOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.LobbyId = lobby_id_cs.get_data();
	EOS_HLobbyModification mod = nullptr;
	if (EOS_Lobby_UpdateLobbyModification(lobby_handle, &opts, &mod) == EOS_EResult::EOS_Success && mod) {
		result.instance();
		result->set_handle(mod);
	}
	return result;
}

static void EOS_CALL _on_lobby_update(const EOS_Lobby_UpdateLobbyCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_update_lobby(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_update_lobby_callback");
	Ref<EpicLobbyModification> mod = p_options.has("lobby_modification") ? Ref<EpicLobbyModification>(p_options["lobby_modification"]) : Ref<EpicLobbyModification>();
	if (mod.is_null() || !mod->is_valid()) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_InvalidParameters);
		_emit_deferred("lobby_interface_update_lobby_callback", payload);
		return;
	}
	EOS_Lobby_UpdateLobbyOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
	opts.LobbyModificationHandle = mod->get_handle();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_update_lobby_callback")));
	EOS_Lobby_UpdateLobby(lobby_handle, &opts, cb, &_on_lobby_update);
}

// ---- Copy details by id / invite id; create search -----------------------

Ref<EpicLobbyDetails> EpicServices::lobby_interface_copy_lobby_details_handle(const Dictionary &p_options) {
	Ref<EpicLobbyDetails> result;
	if (!lobby_handle) {
		return result;
	}
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_CopyLobbyDetailsHandleOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLE_API_LATEST;
	opts.LobbyId = lobby_id_cs.get_data();
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	EOS_HLobbyDetails details = nullptr;
	if (EOS_Lobby_CopyLobbyDetailsHandle(lobby_handle, &opts, &details) == EOS_EResult::EOS_Success && details) {
		result.instance();
		result->set_handle(details);
	}
	return result;
}

Ref<EpicLobbyDetails> EpicServices::lobby_interface_copy_lobby_details_handle_by_invite_id(const Dictionary &p_options) {
	Ref<EpicLobbyDetails> result;
	if (!lobby_handle) {
		return result;
	}
	const CharString invite_cs = dict_get_string(p_options, "invite_id").utf8();
	EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLEBYINVITEID_API_LATEST;
	opts.InviteId = invite_cs.get_data();
	EOS_HLobbyDetails details = nullptr;
	if (EOS_Lobby_CopyLobbyDetailsHandleByInviteId(lobby_handle, &opts, &details) == EOS_EResult::EOS_Success && details) {
		result.instance();
		result->set_handle(details);
	}
	return result;
}

Ref<EpicLobbySearch> EpicServices::lobby_interface_create_lobby_search(int p_max_results) {
	Ref<EpicLobbySearch> result;
	if (!lobby_handle) {
		return result;
	}
	EOS_Lobby_CreateLobbySearchOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
	opts.MaxResults = uint32_t(p_max_results);
	EOS_HLobbySearch search = nullptr;
	if (EOS_Lobby_CreateLobbySearch(lobby_handle, &opts, &search) == EOS_EResult::EOS_Success && search) {
		result.instance();
		result->set_handle(search);
	}
	return result;
}

// ---- Invites --------------------------------------------------------------

static void EOS_CALL _on_lobby_send_invite(const EOS_Lobby_SendInviteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_send_invite(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_send_invite_callback");
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_SendInviteOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_SENDINVITE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.LobbyId = lobby_id_cs.get_data();
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_send_invite_callback")));
	EOS_Lobby_SendInvite(lobby_handle, &opts, cb, &_on_lobby_send_invite);
}

int EpicServices::lobby_interface_reject_invite(const Dictionary &p_options) {
	if (!lobby_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString invite_cs = dict_get_string(p_options, "invite_id").utf8();
	EOS_Lobby_RejectInviteOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_REJECTINVITE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.InviteId = invite_cs.get_data();
	// EOS_Lobby_RejectInvite is async with a callback; expose sync return for the immediate dispatch result.
	struct Wrap {
		static void EOS_CALL cb(const EOS_Lobby_RejectInviteCallbackInfo *) {}
	};
	EOS_Lobby_RejectInvite(lobby_handle, &opts, nullptr, &Wrap::cb);
	return int(EOS_EResult::EOS_Success);
}

int EpicServices::lobby_interface_get_invite_count(const Dictionary &p_options) {
	if (!lobby_handle) {
		return 0;
	}
	EOS_Lobby_GetInviteCountOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_GETINVITECOUNT_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	return int(EOS_Lobby_GetInviteCount(lobby_handle, &opts));
}

String EpicServices::lobby_interface_get_invite_id_by_index(const Dictionary &p_options) {
	if (!lobby_handle) {
		return String();
	}
	EOS_Lobby_GetInviteIdByIndexOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_GETINVITEIDBYINDEX_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Index = uint32_t(dict_get_int(p_options, "index", 0));
	int32_t buf_len = 256;
	CharString buf;
	buf.resize(buf_len);
	if (EOS_Lobby_GetInviteIdByIndex(lobby_handle, &opts, buf.ptrw(), &buf_len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf.get_data());
}

String EpicServices::lobby_interface_get_rtc_room_name(const Dictionary &p_options) {
	if (!lobby_handle) {
		return String();
	}
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_GetRTCRoomNameOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_GETRTCROOMNAME_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.LobbyId = lobby_id_cs.get_data();
	uint32_t buf_len = 256;
	CharString buf;
	buf.resize(buf_len);
	if (EOS_Lobby_GetRTCRoomName(lobby_handle, &opts, buf.ptrw(), &buf_len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf.get_data());
}

// ---- Member ops -----------------------------------------------------------

static void EOS_CALL _on_lobby_kick(const EOS_Lobby_KickMemberCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_kick_member(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_kick_member_callback");
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_KickMemberOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_KICKMEMBER_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.LobbyId = lobby_id_cs.get_data();
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_kick_member_callback")));
	EOS_Lobby_KickMember(lobby_handle, &opts, cb, &_on_lobby_kick);
}

static void EOS_CALL _on_lobby_promote(const EOS_Lobby_PromoteMemberCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_promote_member(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_promote_member_callback");
	const CharString lobby_id_cs = dict_get_string(p_options, "lobby_id").utf8();
	EOS_Lobby_PromoteMemberOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_PROMOTEMEMBER_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.LobbyId = lobby_id_cs.get_data();
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_promote_member_callback")));
	EOS_Lobby_PromoteMember(lobby_handle, &opts, cb, &_on_lobby_promote);
}

static void EOS_CALL _on_lobby_query_invites(const EOS_Lobby_QueryInvitesCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::lobby_interface_query_invites(const Dictionary &p_options) {
	LOBBY_GUARD_VOID("lobby_interface_query_invites_callback");
	EOS_Lobby_QueryInvitesOptions opts = {};
	opts.ApiVersion = EOS_LOBBY_QUERYINVITES_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("lobby_interface_query_invites_callback")));
	EOS_Lobby_QueryInvites(lobby_handle, &opts, cb, &_on_lobby_query_invites);
}

// ---- Notifications --------------------------------------------------------

#define LOBBY_NOTIFY_HELPER(NAME, OPT_TY, OPT_API, ADD_FN, REMOVE_FN, INFO_TY, SIGNAL, FILL_BLOCK) \
	static void EOS_CALL _on_##NAME(const INFO_TY *data) {                                         \
		EpicServices *es = EpicServices::get_singleton();                                          \
		if (!es) {                                                                                 \
			return;                                                                                \
		}                                                                                          \
		Dictionary payload;                                                                        \
		FILL_BLOCK                                                                                 \
		epic_emit_deferred(es->get_instance_id(), StringName(SIGNAL), payload);                    \
	}                                                                                              \
	uint64_t EpicServices::lobby_interface_add_notify_##NAME() {                                   \
		if (!lobby_handle) {                                                                       \
			return 0;                                                                              \
		}                                                                                          \
		OPT_TY opts = {};                                                                          \
		opts.ApiVersion = OPT_API;                                                                 \
		return uint64_t(ADD_FN(lobby_handle, &opts, nullptr, &_on_##NAME));                        \
	}                                                                                              \
	void EpicServices::lobby_interface_remove_notify_##NAME(uint64_t p_id) {                       \
		if (!lobby_handle) {                                                                       \
			return;                                                                                \
		}                                                                                          \
		REMOVE_FN(lobby_handle, EOS_NotificationId(p_id));                                         \
	}

LOBBY_NOTIFY_HELPER(
		lobby_update_received,
		EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions,
		EOS_LOBBY_ADDNOTIFYLOBBYUPDATERECEIVED_API_LATEST,
		EOS_Lobby_AddNotifyLobbyUpdateReceived,
		EOS_Lobby_RemoveNotifyLobbyUpdateReceived,
		EOS_Lobby_LobbyUpdateReceivedCallbackInfo,
		"lobby_interface_lobby_update_received",
		{
			payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
		})

LOBBY_NOTIFY_HELPER(
		lobby_member_update_received,
		EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions,
		EOS_LOBBY_ADDNOTIFYLOBBYMEMBERUPDATERECEIVED_API_LATEST,
		EOS_Lobby_AddNotifyLobbyMemberUpdateReceived,
		EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived,
		EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo,
		"lobby_interface_lobby_member_update_received",
		{
			payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
		})

LOBBY_NOTIFY_HELPER(
		lobby_member_status_received,
		EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions,
		EOS_LOBBY_ADDNOTIFYLOBBYMEMBERSTATUSRECEIVED_API_LATEST,
		EOS_Lobby_AddNotifyLobbyMemberStatusReceived,
		EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived,
		EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo,
		"lobby_interface_lobby_member_status_received",
		{
			payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
			payload["current_status"] = int(data->CurrentStatus);
		})

LOBBY_NOTIFY_HELPER(
		lobby_invite_received,
		EOS_Lobby_AddNotifyLobbyInviteReceivedOptions,
		EOS_LOBBY_ADDNOTIFYLOBBYINVITERECEIVED_API_LATEST,
		EOS_Lobby_AddNotifyLobbyInviteReceived,
		EOS_Lobby_RemoveNotifyLobbyInviteReceived,
		EOS_Lobby_LobbyInviteReceivedCallbackInfo,
		"lobby_interface_lobby_invite_received",
		{
			payload["invite_id"] = String::utf8(data->InviteId ? data->InviteId : "");
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
		})

LOBBY_NOTIFY_HELPER(
		lobby_invite_accepted,
		EOS_Lobby_AddNotifyLobbyInviteAcceptedOptions,
		EOS_LOBBY_ADDNOTIFYLOBBYINVITEACCEPTED_API_LATEST,
		EOS_Lobby_AddNotifyLobbyInviteAccepted,
		EOS_Lobby_RemoveNotifyLobbyInviteAccepted,
		EOS_Lobby_LobbyInviteAcceptedCallbackInfo,
		"lobby_interface_lobby_invite_accepted",
		{
			payload["invite_id"] = String::utf8(data->InviteId ? data->InviteId : "");
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
			payload["lobby_id"] = String::utf8(data->LobbyId ? data->LobbyId : "");
		})
