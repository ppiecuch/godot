/**************************************************************************/
/*  friends_interface.cpp                                                 */
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

#include "eos_friends.h"

static void EOS_CALL _on_friends_query(const EOS_Friends_QueryFriendsCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::friends_interface_query_friends(const Dictionary &p_options) {
	if (!friends_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("friends_interface_query_friends_callback", payload);
		return;
	}
	EOS_Friends_QueryFriendsOptions opts = {};
	opts.ApiVersion = EOS_FRIENDS_QUERYFRIENDS_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("friends_interface_query_friends_callback")));
	EOS_Friends_QueryFriends(friends_handle, &opts, cb, &_on_friends_query);
}

int EpicServices::friends_interface_get_friends_count(const String &p_local_user_id) {
	if (!friends_handle) {
		return 0;
	}
	EOS_Friends_GetFriendsCountOptions opts = {};
	opts.ApiVersion = EOS_FRIENDS_GETFRIENDSCOUNT_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(p_local_user_id);
	return EOS_Friends_GetFriendsCount(friends_handle, &opts);
}

String EpicServices::friends_interface_get_friend_at_index(const String &p_local_user_id, int p_index) {
	if (!friends_handle) {
		return String();
	}
	EOS_Friends_GetFriendAtIndexOptions opts = {};
	opts.ApiVersion = EOS_FRIENDS_GETFRIENDATINDEX_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(p_local_user_id);
	opts.Index = int32_t(p_index);
	return eos_eaid_to_string(EOS_Friends_GetFriendAtIndex(friends_handle, &opts));
}

int EpicServices::friends_interface_get_status(const String &p_local_user_id, const String &p_target_user_id) {
	if (!friends_handle) {
		return int(EOS_EFriendsStatus::EOS_FS_NotFriends);
	}
	EOS_Friends_GetStatusOptions opts = {};
	opts.ApiVersion = EOS_FRIENDS_GETSTATUS_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(p_local_user_id);
	opts.TargetUserId = eos_eaid_from_string(p_target_user_id);
	return int(EOS_Friends_GetStatus(friends_handle, &opts));
}

int EpicServices::friends_interface_get_blocked_users_count(const String &p_local_user_id) {
	if (!friends_handle) {
		return 0;
	}
	EOS_Friends_GetBlockedUsersCountOptions opts = {};
	opts.ApiVersion = EOS_FRIENDS_GETBLOCKEDUSERSCOUNT_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(p_local_user_id);
	return EOS_Friends_GetBlockedUsersCount(friends_handle, &opts);
}

String EpicServices::friends_interface_get_blocked_user_at_index(const String &p_local_user_id, int p_index) {
	if (!friends_handle) {
		return String();
	}
	EOS_Friends_GetBlockedUserAtIndexOptions opts = {};
	opts.ApiVersion = EOS_FRIENDS_GETBLOCKEDUSERATINDEX_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(p_local_user_id);
	opts.Index = int32_t(p_index);
	return eos_eaid_to_string(EOS_Friends_GetBlockedUserAtIndex(friends_handle, &opts));
}

static void EOS_CALL _on_friends_update(const EOS_Friends_OnFriendsUpdateInfo *data) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es) {
		return;
	}
	Dictionary payload;
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_eaid_to_string(data->TargetUserId);
	payload["previous_status"] = int(data->PreviousStatus);
	payload["current_status"] = int(data->CurrentStatus);
	epic_emit_deferred(es->get_instance_id(), StringName("friends_interface_friends_update"), payload);
}

uint64_t EpicServices::friends_interface_add_notify_friends_update() {
	if (!friends_handle) {
		return 0;
	}
	EOS_Friends_AddNotifyFriendsUpdateOptions opts = {};
	opts.ApiVersion = EOS_FRIENDS_ADDNOTIFYFRIENDSUPDATE_API_LATEST;
	return uint64_t(EOS_Friends_AddNotifyFriendsUpdate(friends_handle, &opts, nullptr, &_on_friends_update));
}

void EpicServices::friends_interface_remove_notify_friends_update(uint64_t p_id) {
	if (!friends_handle) {
		return;
	}
	EOS_Friends_RemoveNotifyFriendsUpdate(friends_handle, EOS_NotificationId(p_id));
}

static void EOS_CALL _on_blocked_update(const EOS_Friends_OnBlockedUsersUpdateInfo *data) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es) {
		return;
	}
	Dictionary payload;
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_eaid_to_string(data->TargetUserId);
	payload["blocked"] = bool(data->bBlocked == EOS_TRUE);
	epic_emit_deferred(es->get_instance_id(), StringName("friends_interface_blocked_users_update"), payload);
}

uint64_t EpicServices::friends_interface_add_notify_blocked_users_update() {
	if (!friends_handle) {
		return 0;
	}
	EOS_Friends_AddNotifyBlockedUsersUpdateOptions opts = {};
	opts.ApiVersion = EOS_FRIENDS_ADDNOTIFYBLOCKEDUSERSUPDATE_API_LATEST;
	return uint64_t(EOS_Friends_AddNotifyBlockedUsersUpdate(friends_handle, &opts, nullptr, &_on_blocked_update));
}

void EpicServices::friends_interface_remove_notify_blocked_users_update(uint64_t p_id) {
	if (!friends_handle) {
		return;
	}
	EOS_Friends_RemoveNotifyBlockedUsersUpdate(friends_handle, EOS_NotificationId(p_id));
}
