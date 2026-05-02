/**************************************************************************/
/*  epic_lobby_modification.cpp                                           */
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

#include "epic_lobby_modification.h"

#include "epic_utils.h"

#include "eos_lobby.h"

void EpicLobbyModification::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicLobbyModification::is_valid);
	ClassDB::bind_method(D_METHOD("set_permission_level", "level"), &EpicLobbyModification::set_permission_level);
	ClassDB::bind_method(D_METHOD("set_max_members", "count"), &EpicLobbyModification::set_max_members);
	ClassDB::bind_method(D_METHOD("set_bucket_id", "bucket_id"), &EpicLobbyModification::set_bucket_id);
	ClassDB::bind_method(D_METHOD("set_invites_allowed", "allowed"), &EpicLobbyModification::set_invites_allowed);
	ClassDB::bind_method(D_METHOD("set_allowed_platform_ids", "platform_ids"), &EpicLobbyModification::set_allowed_platform_ids);
	ClassDB::bind_method(D_METHOD("add_attribute", "attribute"), &EpicLobbyModification::add_attribute);
	ClassDB::bind_method(D_METHOD("remove_attribute", "key"), &EpicLobbyModification::remove_attribute);
	ClassDB::bind_method(D_METHOD("add_member_attribute", "attribute"), &EpicLobbyModification::add_member_attribute);
	ClassDB::bind_method(D_METHOD("remove_member_attribute", "key"), &EpicLobbyModification::remove_member_attribute);
}

void EpicLobbyModification::set_handle(EOS_HLobbyModification p_handle) {
	if (handle && handle != p_handle) {
		EOS_LobbyModification_Release(handle);
	}
	handle = p_handle;
}

EpicLobbyModification::~EpicLobbyModification() {
	if (handle) {
		EOS_LobbyModification_Release(handle);
		handle = nullptr;
	}
}

int EpicLobbyModification::set_permission_level(int p_level) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_LobbyModification_SetPermissionLevelOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
	opts.PermissionLevel = EOS_ELobbyPermissionLevel(p_level);
	return int(EOS_LobbyModification_SetPermissionLevel(handle, &opts));
}

int EpicLobbyModification::set_max_members(int p_count) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_LobbyModification_SetMaxMembersOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_SETMAXMEMBERS_API_LATEST;
	opts.MaxMembers = uint32_t(p_count);
	return int(EOS_LobbyModification_SetMaxMembers(handle, &opts));
}

int EpicLobbyModification::set_bucket_id(const String &p_bucket_id) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_bucket_id.utf8();
	EOS_LobbyModification_SetBucketIdOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_SETBUCKETID_API_LATEST;
	opts.BucketId = cs.length() ? cs.get_data() : nullptr;
	return int(EOS_LobbyModification_SetBucketId(handle, &opts));
}

int EpicLobbyModification::set_invites_allowed(bool p_allowed) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_LobbyModification_SetInvitesAllowedOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_SETINVITESALLOWED_API_LATEST;
	opts.bInvitesAllowed = p_allowed ? EOS_TRUE : EOS_FALSE;
	return int(EOS_LobbyModification_SetInvitesAllowed(handle, &opts));
}

int EpicLobbyModification::set_allowed_platform_ids(const Array &p_platform_ids) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	Vector<uint32_t> ids;
	ids.resize(p_platform_ids.size());
	for (int i = 0; i < p_platform_ids.size(); ++i) {
		ids.write[i] = uint32_t(int(p_platform_ids[i]));
	}
	EOS_LobbyModification_SetAllowedPlatformIdsOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_SETALLOWEDPLATFORMIDS_API_LATEST;
	opts.AllowedPlatformIds = ids.size() ? ids.ptrw() : nullptr;
	opts.AllowedPlatformIdsCount = uint32_t(ids.size());
	return int(EOS_LobbyModification_SetAllowedPlatformIds(handle, &opts));
}

// Build an EOS_Lobby_AttributeData from a Godot Dictionary {key, value, value_type, visibility}.
// Returned CharStrings keep the underlying C strings alive for the duration of the EOS call.
static void _dict_to_attr_data(const Dictionary &p_attr, EOS_Lobby_AttributeData &out_data, CharString &out_key_cs, CharString &out_str_cs) {
	out_data.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
	out_key_cs = String(dict_get_string(p_attr, "key")).utf8();
	out_data.Key = out_key_cs.length() ? out_key_cs.get_data() : nullptr;
	out_data.ValueType = EOS_EAttributeType(dict_get_int(p_attr, "value_type", int(EOS_EAttributeType::EOS_AT_STRING)));
	if (!p_attr.has("value")) {
		return;
	}
	const Variant &v = p_attr["value"];
	switch (out_data.ValueType) {
		case EOS_EAttributeType::EOS_AT_BOOLEAN:
			out_data.Value.AsBool = bool(v) ? EOS_TRUE : EOS_FALSE;
			break;
		case EOS_EAttributeType::EOS_AT_INT64:
			out_data.Value.AsInt64 = int64_t(v);
			break;
		case EOS_EAttributeType::EOS_AT_DOUBLE:
			out_data.Value.AsDouble = double(v);
			break;
		case EOS_EAttributeType::EOS_AT_STRING:
		default:
			out_str_cs = String(v).utf8();
			out_data.Value.AsUtf8 = out_str_cs.get_data();
			break;
	}
}

int EpicLobbyModification::add_attribute(const Dictionary &p_attr) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_Lobby_AttributeData data = {};
	CharString key_cs, str_cs;
	_dict_to_attr_data(p_attr, data, key_cs, str_cs);
	EOS_LobbyModification_AddAttributeOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
	opts.Attribute = &data;
	opts.Visibility = EOS_ELobbyAttributeVisibility(dict_get_int(p_attr, "visibility", int(EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC)));
	return int(EOS_LobbyModification_AddAttribute(handle, &opts));
}

int EpicLobbyModification::remove_attribute(const String &p_key) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString key_cs = p_key.utf8();
	EOS_LobbyModification_RemoveAttributeOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_REMOVEATTRIBUTE_API_LATEST;
	opts.Key = key_cs.get_data();
	return int(EOS_LobbyModification_RemoveAttribute(handle, &opts));
}

int EpicLobbyModification::add_member_attribute(const Dictionary &p_attr) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_Lobby_AttributeData data = {};
	CharString key_cs, str_cs;
	_dict_to_attr_data(p_attr, data, key_cs, str_cs);
	EOS_LobbyModification_AddMemberAttributeOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_ADDMEMBERATTRIBUTE_API_LATEST;
	opts.Attribute = &data;
	opts.Visibility = EOS_ELobbyAttributeVisibility(dict_get_int(p_attr, "visibility", int(EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC)));
	return int(EOS_LobbyModification_AddMemberAttribute(handle, &opts));
}

int EpicLobbyModification::remove_member_attribute(const String &p_key) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString key_cs = p_key.utf8();
	EOS_LobbyModification_RemoveMemberAttributeOptions opts = {};
	opts.ApiVersion = EOS_LOBBYMODIFICATION_REMOVEMEMBERATTRIBUTE_API_LATEST;
	opts.Key = key_cs.get_data();
	return int(EOS_LobbyModification_RemoveMemberAttribute(handle, &opts));
}
