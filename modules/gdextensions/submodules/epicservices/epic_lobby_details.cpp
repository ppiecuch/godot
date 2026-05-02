/**************************************************************************/
/*  epic_lobby_details.cpp                                                */
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

#include "epic_lobby_details.h"

#include "epic_utils.h"

#include "eos_lobby.h"

void EpicLobbyDetails::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicLobbyDetails::is_valid);
	ClassDB::bind_method(D_METHOD("get_lobby_id"), &EpicLobbyDetails::get_lobby_id);
	ClassDB::bind_method(D_METHOD("get_lobby_owner"), &EpicLobbyDetails::get_lobby_owner);
	ClassDB::bind_method(D_METHOD("copy_info"), &EpicLobbyDetails::copy_info);
	ClassDB::bind_method(D_METHOD("get_member_count"), &EpicLobbyDetails::get_member_count);
	ClassDB::bind_method(D_METHOD("get_member_by_index", "index"), &EpicLobbyDetails::get_member_by_index);
	ClassDB::bind_method(D_METHOD("get_member_attribute_count", "target_user_id"), &EpicLobbyDetails::get_member_attribute_count);
	ClassDB::bind_method(D_METHOD("copy_member_attribute_by_index", "target_user_id", "index"), &EpicLobbyDetails::copy_member_attribute_by_index);
	ClassDB::bind_method(D_METHOD("copy_member_attribute_by_key", "target_user_id", "key"), &EpicLobbyDetails::copy_member_attribute_by_key);
	ClassDB::bind_method(D_METHOD("get_attribute_count"), &EpicLobbyDetails::get_attribute_count);
	ClassDB::bind_method(D_METHOD("copy_attribute_by_index", "index"), &EpicLobbyDetails::copy_attribute_by_index);
	ClassDB::bind_method(D_METHOD("copy_attribute_by_key", "key"), &EpicLobbyDetails::copy_attribute_by_key);
}

void EpicLobbyDetails::set_handle(EOS_HLobbyDetails p_handle) {
	if (handle && handle != p_handle) {
		EOS_LobbyDetails_Release(handle);
	}
	handle = p_handle;
}

EpicLobbyDetails::~EpicLobbyDetails() {
	if (handle) {
		EOS_LobbyDetails_Release(handle);
		handle = nullptr;
	}
}

String EpicLobbyDetails::get_lobby_id() const {
	if (!handle) {
		return String();
	}
	EOS_LobbyDetails_GetLobbyOwnerOptions opts = {}; // misuse — pull lobby id from copy_info
	(void)opts;
	Dictionary info = copy_info();
	if (info.has("lobby_id")) {
		return String(info["lobby_id"]);
	}
	return String();
}

String EpicLobbyDetails::get_lobby_owner() const {
	if (!handle) {
		return String();
	}
	EOS_LobbyDetails_GetLobbyOwnerOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_GETLOBBYOWNER_API_LATEST;
	return eos_pui_to_string(EOS_LobbyDetails_GetLobbyOwner(handle, &opts));
}

Dictionary EpicLobbyDetails::copy_info() const {
	Dictionary out;
	if (!handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotFound);
		return out;
	}
	EOS_LobbyDetails_CopyInfoOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
	EOS_LobbyDetails_Info *info = nullptr;
	EOS_EResult r = EOS_LobbyDetails_CopyInfo(handle, &opts, &info);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && info) {
		out["lobby_id"] = String::utf8(info->LobbyId ? info->LobbyId : "");
		out["lobby_owner_user_id"] = eos_pui_to_string(info->LobbyOwnerUserId);
		out["permission_level"] = int(info->PermissionLevel);
		out["available_slots"] = int(info->AvailableSlots);
		out["max_members"] = int(info->MaxMembers);
		out["allow_invites"] = bool(info->bAllowInvites == EOS_TRUE);
		out["bucket_id"] = String::utf8(info->BucketId ? info->BucketId : "");
		out["allow_host_migration"] = bool(info->bAllowHostMigration == EOS_TRUE);
		out["rtc_room_enabled"] = bool(info->bRTCRoomEnabled == EOS_TRUE);
		out["allow_join_by_id"] = bool(info->bAllowJoinById == EOS_TRUE);
		out["rejoin_after_kick_requires_invite"] = bool(info->bRejoinAfterKickRequiresInvite == EOS_TRUE);
		EOS_LobbyDetails_Info_Release(info);
	}
	return out;
}

int EpicLobbyDetails::get_member_count() const {
	if (!handle) {
		return 0;
	}
	EOS_LobbyDetails_GetMemberCountOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;
	return int(EOS_LobbyDetails_GetMemberCount(handle, &opts));
}

String EpicLobbyDetails::get_member_by_index(int p_index) const {
	if (!handle) {
		return String();
	}
	EOS_LobbyDetails_GetMemberByIndexOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
	opts.MemberIndex = uint32_t(p_index);
	return eos_pui_to_string(EOS_LobbyDetails_GetMemberByIndex(handle, &opts));
}

int EpicLobbyDetails::get_member_attribute_count(const String &p_target_user_id) const {
	if (!handle) {
		return 0;
	}
	EOS_LobbyDetails_GetMemberAttributeCountOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERATTRIBUTECOUNT_API_LATEST;
	opts.TargetUserId = eos_pui_from_string(p_target_user_id);
	return int(EOS_LobbyDetails_GetMemberAttributeCount(handle, &opts));
}

static Dictionary _attr_to_dict(const EOS_Lobby_Attribute *attr) {
	Dictionary d;
	if (!attr || !attr->Data) {
		return d;
	}
	d["visibility"] = int(attr->Visibility);
	d["key"] = String::utf8(attr->Data->Key ? attr->Data->Key : "");
	d["value_type"] = int(attr->Data->ValueType);
	switch (attr->Data->ValueType) {
		case EOS_EAttributeType::EOS_AT_BOOLEAN:
			d["value"] = bool(attr->Data->Value.AsBool == EOS_TRUE);
			break;
		case EOS_EAttributeType::EOS_AT_INT64:
			d["value"] = int64_t(attr->Data->Value.AsInt64);
			break;
		case EOS_EAttributeType::EOS_AT_DOUBLE:
			d["value"] = double(attr->Data->Value.AsDouble);
			break;
		case EOS_EAttributeType::EOS_AT_STRING:
			d["value"] = String::utf8(attr->Data->Value.AsUtf8 ? attr->Data->Value.AsUtf8 : "");
			break;
		default:
			break;
	}
	return d;
}

Dictionary EpicLobbyDetails::copy_member_attribute_by_index(const String &p_target_user_id, int p_index) const {
	Dictionary out;
	if (!handle) {
		return out;
	}
	EOS_LobbyDetails_CopyMemberAttributeByIndexOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYINDEX_API_LATEST;
	opts.TargetUserId = eos_pui_from_string(p_target_user_id);
	opts.AttrIndex = uint32_t(p_index);
	EOS_Lobby_Attribute *attr = nullptr;
	if (EOS_LobbyDetails_CopyMemberAttributeByIndex(handle, &opts, &attr) == EOS_EResult::EOS_Success && attr) {
		out = _attr_to_dict(attr);
		EOS_Lobby_Attribute_Release(attr);
	}
	return out;
}

Dictionary EpicLobbyDetails::copy_member_attribute_by_key(const String &p_target_user_id, const String &p_key) const {
	Dictionary out;
	if (!handle) {
		return out;
	}
	const CharString key_cs = p_key.utf8();
	EOS_LobbyDetails_CopyMemberAttributeByKeyOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYKEY_API_LATEST;
	opts.TargetUserId = eos_pui_from_string(p_target_user_id);
	opts.AttrKey = key_cs.get_data();
	EOS_Lobby_Attribute *attr = nullptr;
	if (EOS_LobbyDetails_CopyMemberAttributeByKey(handle, &opts, &attr) == EOS_EResult::EOS_Success && attr) {
		out = _attr_to_dict(attr);
		EOS_Lobby_Attribute_Release(attr);
	}
	return out;
}

int EpicLobbyDetails::get_attribute_count() const {
	if (!handle) {
		return 0;
	}
	EOS_LobbyDetails_GetAttributeCountOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_GETATTRIBUTECOUNT_API_LATEST;
	return int(EOS_LobbyDetails_GetAttributeCount(handle, &opts));
}

Dictionary EpicLobbyDetails::copy_attribute_by_index(int p_index) const {
	Dictionary out;
	if (!handle) {
		return out;
	}
	EOS_LobbyDetails_CopyAttributeByIndexOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_COPYATTRIBUTEBYINDEX_API_LATEST;
	opts.AttrIndex = uint32_t(p_index);
	EOS_Lobby_Attribute *attr = nullptr;
	if (EOS_LobbyDetails_CopyAttributeByIndex(handle, &opts, &attr) == EOS_EResult::EOS_Success && attr) {
		out = _attr_to_dict(attr);
		EOS_Lobby_Attribute_Release(attr);
	}
	return out;
}

Dictionary EpicLobbyDetails::copy_attribute_by_key(const String &p_key) const {
	Dictionary out;
	if (!handle) {
		return out;
	}
	const CharString key_cs = p_key.utf8();
	EOS_LobbyDetails_CopyAttributeByKeyOptions opts = {};
	opts.ApiVersion = EOS_LOBBYDETAILS_COPYATTRIBUTEBYKEY_API_LATEST;
	opts.AttrKey = key_cs.get_data();
	EOS_Lobby_Attribute *attr = nullptr;
	if (EOS_LobbyDetails_CopyAttributeByKey(handle, &opts, &attr) == EOS_EResult::EOS_Success && attr) {
		out = _attr_to_dict(attr);
		EOS_Lobby_Attribute_Release(attr);
	}
	return out;
}

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_CASE("[EpicLobbyDetails] null handle is invalid and returns empty values") {
	Ref<EpicLobbyDetails> d;
	d.instance();
	CHECK_FALSE(d->is_valid());
	CHECK(d->get_lobby_id().empty());
	CHECK(d->get_lobby_owner().empty());
	CHECK(d->get_member_count() == 0);
	CHECK(d->get_member_by_index(0).empty());
	CHECK(d->get_attribute_count() == 0);
}
#endif
