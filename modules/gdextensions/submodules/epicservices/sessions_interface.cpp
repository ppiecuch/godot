/**************************************************************************/
/*  sessions_interface.cpp                                                */
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

#include "epic_active_session.h"
#include "epic_callback.h"
#include "epic_session_details.h"
#include "epic_session_modification.h"
#include "epic_session_search.h"
#include "epic_utils.h"

#include "eos_sessions.h"

#define SESSIONS_GUARD_VOID(SIGNAL)                                             \
	if (!sessions_handle) {                                                     \
		Dictionary payload;                                                     \
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured); \
		_emit_deferred(SIGNAL, payload);                                        \
		return;                                                                 \
	}

Ref<EpicSessionModification> EpicServices::sessions_interface_create_session_modification(const Dictionary &p_options) {
	Ref<EpicSessionModification> result;
	if (!sessions_handle) {
		return result;
	}
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	const CharString bucket_cs = dict_get_string(p_options, "bucket_id").utf8();
	const CharString session_id_cs = dict_get_string(p_options, "session_id").utf8();
	EOS_Sessions_CreateSessionModificationOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	opts.BucketId = bucket_cs.length() ? bucket_cs.get_data() : nullptr;
	opts.MaxPlayers = uint32_t(dict_get_int(p_options, "max_players", 4));
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.bPresenceEnabled = dict_get_bool(p_options, "presence_enabled", false) ? EOS_TRUE : EOS_FALSE;
	opts.SessionId = session_id_cs.length() ? session_id_cs.get_data() : nullptr;
	opts.bSanctionsEnabled = dict_get_bool(p_options, "sanctions_enabled", true) ? EOS_TRUE : EOS_FALSE;
	opts.AllowedPlatformIds = nullptr;
	opts.AllowedPlatformIdsCount = 0;
	EOS_HSessionModification mod = nullptr;
	if (EOS_Sessions_CreateSessionModification(sessions_handle, &opts, &mod) == EOS_EResult::EOS_Success && mod) {
		result.instance();
		result->set_handle(mod);
	}
	return result;
}

Ref<EpicSessionModification> EpicServices::sessions_interface_update_session_modification(const Dictionary &p_options) {
	Ref<EpicSessionModification> result;
	if (!sessions_handle) {
		return result;
	}
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	EOS_Sessions_UpdateSessionModificationOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_UPDATESESSIONMODIFICATION_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	EOS_HSessionModification mod = nullptr;
	if (EOS_Sessions_UpdateSessionModification(sessions_handle, &opts, &mod) == EOS_EResult::EOS_Success && mod) {
		result.instance();
		result->set_handle(mod);
	}
	return result;
}

static void EOS_CALL _on_sessions_update(const EOS_Sessions_UpdateSessionCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["session_id"] = String::utf8(data->SessionId ? data->SessionId : "");
	payload["session_name"] = String::utf8(data->SessionName ? data->SessionName : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::sessions_interface_update_session(const Dictionary &p_options) {
	SESSIONS_GUARD_VOID("sessions_interface_update_session_callback");
	Ref<EpicSessionModification> mod = p_options.has("session_modification") ? Ref<EpicSessionModification>(p_options["session_modification"]) : Ref<EpicSessionModification>();
	if (mod.is_null() || !mod->is_valid()) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_InvalidParameters);
		_emit_deferred("sessions_interface_update_session_callback", payload);
		return;
	}
	EOS_Sessions_UpdateSessionOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
	opts.SessionModificationHandle = mod->get_handle();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("sessions_interface_update_session_callback")));
	EOS_Sessions_UpdateSession(sessions_handle, &opts, cb, &_on_sessions_update);
}

static void EOS_CALL _on_sessions_destroy(const EOS_Sessions_DestroySessionCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::sessions_interface_destroy_session(const Dictionary &p_options) {
	SESSIONS_GUARD_VOID("sessions_interface_destroy_session_callback");
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	EOS_Sessions_DestroySessionOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_DESTROYSESSION_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("sessions_interface_destroy_session_callback")));
	EOS_Sessions_DestroySession(sessions_handle, &opts, cb, &_on_sessions_destroy);
}

static void EOS_CALL _on_sessions_join(const EOS_Sessions_JoinSessionCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::sessions_interface_join_session(const Dictionary &p_options) {
	SESSIONS_GUARD_VOID("sessions_interface_join_session_callback");
	Ref<EpicSessionDetails> details = p_options.has("session_details") ? Ref<EpicSessionDetails>(p_options["session_details"]) : Ref<EpicSessionDetails>();
	if (details.is_null() || !details->is_valid()) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_InvalidParameters);
		_emit_deferred("sessions_interface_join_session_callback", payload);
		return;
	}
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	EOS_Sessions_JoinSessionOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	opts.SessionHandle = details->get_handle();
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.bPresenceEnabled = dict_get_bool(p_options, "presence_enabled", false) ? EOS_TRUE : EOS_FALSE;
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("sessions_interface_join_session_callback")));
	EOS_Sessions_JoinSession(sessions_handle, &opts, cb, &_on_sessions_join);
}

static void EOS_CALL _on_sessions_start(const EOS_Sessions_StartSessionCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::sessions_interface_start_session(const Dictionary &p_options) {
	SESSIONS_GUARD_VOID("sessions_interface_start_session_callback");
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	EOS_Sessions_StartSessionOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_STARTSESSION_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("sessions_interface_start_session_callback")));
	EOS_Sessions_StartSession(sessions_handle, &opts, cb, &_on_sessions_start);
}

static void EOS_CALL _on_sessions_end(const EOS_Sessions_EndSessionCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::sessions_interface_end_session(const Dictionary &p_options) {
	SESSIONS_GUARD_VOID("sessions_interface_end_session_callback");
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	EOS_Sessions_EndSessionOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_ENDSESSION_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("sessions_interface_end_session_callback")));
	EOS_Sessions_EndSession(sessions_handle, &opts, cb, &_on_sessions_end);
}

static void EOS_CALL _on_sessions_register(const EOS_Sessions_RegisterPlayersCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::sessions_interface_register_players(const Dictionary &p_options) {
	SESSIONS_GUARD_VOID("sessions_interface_register_players_callback");
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	const Array player_ids_arr = p_options.has("players_to_register") ? Array(p_options["players_to_register"]) : Array();
	Vector<EOS_ProductUserId> ids;
	ids.resize(player_ids_arr.size());
	for (int i = 0; i < player_ids_arr.size(); ++i) {
		ids.write[i] = eos_pui_from_string(String(player_ids_arr[i]));
	}
	EOS_Sessions_RegisterPlayersOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_REGISTERPLAYERS_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	opts.PlayersToRegister = ids.size() ? ids.ptrw() : nullptr;
	opts.PlayersToRegisterCount = uint32_t(ids.size());
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("sessions_interface_register_players_callback")));
	EOS_Sessions_RegisterPlayers(sessions_handle, &opts, cb, &_on_sessions_register);
}

static void EOS_CALL _on_sessions_unregister(const EOS_Sessions_UnregisterPlayersCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::sessions_interface_unregister_players(const Dictionary &p_options) {
	SESSIONS_GUARD_VOID("sessions_interface_unregister_players_callback");
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	const Array player_ids_arr = p_options.has("players_to_unregister") ? Array(p_options["players_to_unregister"]) : Array();
	Vector<EOS_ProductUserId> ids;
	ids.resize(player_ids_arr.size());
	for (int i = 0; i < player_ids_arr.size(); ++i) {
		ids.write[i] = eos_pui_from_string(String(player_ids_arr[i]));
	}
	EOS_Sessions_UnregisterPlayersOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_UNREGISTERPLAYERS_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	opts.PlayersToUnregister = ids.size() ? ids.ptrw() : nullptr;
	opts.PlayersToUnregisterCount = uint32_t(ids.size());
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("sessions_interface_unregister_players_callback")));
	EOS_Sessions_UnregisterPlayers(sessions_handle, &opts, cb, &_on_sessions_unregister);
}

static void EOS_CALL _on_sessions_send_invite(const EOS_Sessions_SendInviteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::sessions_interface_send_invite(const Dictionary &p_options) {
	SESSIONS_GUARD_VOID("sessions_interface_send_invite_callback");
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	EOS_Sessions_SendInviteOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_SENDINVITE_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("sessions_interface_send_invite_callback")));
	EOS_Sessions_SendInvite(sessions_handle, &opts, cb, &_on_sessions_send_invite);
}

int EpicServices::sessions_interface_reject_invite(const Dictionary &p_options) {
	if (!sessions_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString invite_cs = dict_get_string(p_options, "invite_id").utf8();
	EOS_Sessions_RejectInviteOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_REJECTINVITE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.InviteId = invite_cs.get_data();
	struct Wrap {
		static void EOS_CALL cb(const EOS_Sessions_RejectInviteCallbackInfo *) {}
	};
	EOS_Sessions_RejectInvite(sessions_handle, &opts, nullptr, &Wrap::cb);
	return int(EOS_EResult::EOS_Success);
}

int EpicServices::sessions_interface_get_invite_count(const Dictionary &p_options) {
	if (!sessions_handle) {
		return 0;
	}
	EOS_Sessions_GetInviteCountOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_GETINVITECOUNT_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	return int(EOS_Sessions_GetInviteCount(sessions_handle, &opts));
}

String EpicServices::sessions_interface_get_invite_id_by_index(const Dictionary &p_options) {
	if (!sessions_handle) {
		return String();
	}
	EOS_Sessions_GetInviteIdByIndexOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_GETINVITEIDBYINDEX_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Index = uint32_t(dict_get_int(p_options, "index", 0));
	int32_t buf_len = 256;
	CharString buf;
	buf.resize(buf_len);
	if (EOS_Sessions_GetInviteIdByIndex(sessions_handle, &opts, buf.ptrw(), &buf_len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf.get_data());
}

Ref<EpicSessionSearch> EpicServices::sessions_interface_create_session_search(int p_max_results) {
	Ref<EpicSessionSearch> result;
	if (!sessions_handle) {
		return result;
	}
	EOS_Sessions_CreateSessionSearchOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
	opts.MaxSearchResults = uint32_t(p_max_results);
	EOS_HSessionSearch search = nullptr;
	if (EOS_Sessions_CreateSessionSearch(sessions_handle, &opts, &search) == EOS_EResult::EOS_Success && search) {
		result.instance();
		result->set_handle(search);
	}
	return result;
}

Ref<EpicActiveSession> EpicServices::sessions_interface_copy_active_session_handle(const Dictionary &p_options) {
	Ref<EpicActiveSession> result;
	if (!sessions_handle) {
		return result;
	}
	const CharString session_name_cs = dict_get_string(p_options, "session_name").utf8();
	EOS_Sessions_CopyActiveSessionHandleOptions opts = {};
	opts.ApiVersion = EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST;
	opts.SessionName = session_name_cs.get_data();
	EOS_HActiveSession active = nullptr;
	if (EOS_Sessions_CopyActiveSessionHandle(sessions_handle, &opts, &active) == EOS_EResult::EOS_Success && active) {
		result.instance();
		result->set_handle(active);
	}
	return result;
}

#define SESSIONS_NOTIFY_HELPER(NAME, OPT_TY, OPT_API, ADD_FN, REMOVE_FN, INFO_TY, SIGNAL, FILL_BLOCK) \
	static void EOS_CALL _on_sessions_##NAME(const INFO_TY *data) {                                   \
		EpicServices *es = EpicServices::get_singleton();                                             \
		if (!es) {                                                                                    \
			return;                                                                                   \
		}                                                                                             \
		Dictionary payload;                                                                           \
		FILL_BLOCK                                                                                    \
		epic_emit_deferred(es->get_instance_id(), StringName(SIGNAL), payload);                       \
	}                                                                                                 \
	uint64_t EpicServices::sessions_interface_add_notify_##NAME() {                                   \
		if (!sessions_handle) {                                                                       \
			return 0;                                                                                 \
		}                                                                                             \
		OPT_TY opts = {};                                                                             \
		opts.ApiVersion = OPT_API;                                                                    \
		return uint64_t(ADD_FN(sessions_handle, &opts, nullptr, &_on_sessions_##NAME));               \
	}                                                                                                 \
	void EpicServices::sessions_interface_remove_notify_##NAME(uint64_t p_id) {                       \
		if (!sessions_handle) {                                                                       \
			return;                                                                                   \
		}                                                                                             \
		REMOVE_FN(sessions_handle, EOS_NotificationId(p_id));                                         \
	}

SESSIONS_NOTIFY_HELPER(
		session_invite_received,
		EOS_Sessions_AddNotifySessionInviteReceivedOptions,
		EOS_SESSIONS_ADDNOTIFYSESSIONINVITERECEIVED_API_LATEST,
		EOS_Sessions_AddNotifySessionInviteReceived,
		EOS_Sessions_RemoveNotifySessionInviteReceived,
		EOS_Sessions_SessionInviteReceivedCallbackInfo,
		"sessions_interface_session_invite_received",
		{
			payload["invite_id"] = String::utf8(data->InviteId ? data->InviteId : "");
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
		})

SESSIONS_NOTIFY_HELPER(
		session_invite_accepted,
		EOS_Sessions_AddNotifySessionInviteAcceptedOptions,
		EOS_SESSIONS_ADDNOTIFYSESSIONINVITEACCEPTED_API_LATEST,
		EOS_Sessions_AddNotifySessionInviteAccepted,
		EOS_Sessions_RemoveNotifySessionInviteAccepted,
		EOS_Sessions_SessionInviteAcceptedCallbackInfo,
		"sessions_interface_session_invite_accepted",
		{
			payload["session_id"] = String::utf8(data->SessionId ? data->SessionId : "");
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
			payload["invite_id"] = String::utf8(data->InviteId ? data->InviteId : "");
		})

SESSIONS_NOTIFY_HELPER(
		join_session_accepted,
		EOS_Sessions_AddNotifyJoinSessionAcceptedOptions,
		EOS_SESSIONS_ADDNOTIFYJOINSESSIONACCEPTED_API_LATEST,
		EOS_Sessions_AddNotifyJoinSessionAccepted,
		EOS_Sessions_RemoveNotifyJoinSessionAccepted,
		EOS_Sessions_JoinSessionAcceptedCallbackInfo,
		"sessions_interface_join_session_accepted",
		{
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["ui_event_id"] = uint64_t(data->UiEventId);
		})
