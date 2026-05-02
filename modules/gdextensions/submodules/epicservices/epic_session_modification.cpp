/**************************************************************************/
/*  epic_session_modification.cpp                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#include "epic_session_modification.h"

#include "epic_utils.h"

#include "eos_sessions.h"

void EpicSessionModification::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicSessionModification::is_valid);
	ClassDB::bind_method(D_METHOD("set_bucket_id", "bucket_id"), &EpicSessionModification::set_bucket_id);
	ClassDB::bind_method(D_METHOD("set_max_players", "count"), &EpicSessionModification::set_max_players);
	ClassDB::bind_method(D_METHOD("set_host_address", "host_address"), &EpicSessionModification::set_host_address);
	ClassDB::bind_method(D_METHOD("set_permission_level", "level"), &EpicSessionModification::set_permission_level);
	ClassDB::bind_method(D_METHOD("set_join_in_progress_allowed", "allowed"), &EpicSessionModification::set_join_in_progress_allowed);
	ClassDB::bind_method(D_METHOD("set_invites_allowed", "allowed"), &EpicSessionModification::set_invites_allowed);
	ClassDB::bind_method(D_METHOD("add_attribute", "attribute"), &EpicSessionModification::add_attribute);
	ClassDB::bind_method(D_METHOD("remove_attribute", "key"), &EpicSessionModification::remove_attribute);
}

void EpicSessionModification::set_handle(EOS_HSessionModification p_handle) {
	if (handle && handle != p_handle) {
		EOS_SessionModification_Release(handle);
	}
	handle = p_handle;
}

EpicSessionModification::~EpicSessionModification() {
	if (handle) {
		EOS_SessionModification_Release(handle);
		handle = nullptr;
	}
}

int EpicSessionModification::set_bucket_id(const String &p_bucket_id) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_bucket_id.utf8();
	EOS_SessionModification_SetBucketIdOptions opts = {};
	opts.ApiVersion = EOS_SESSIONMODIFICATION_SETBUCKETID_API_LATEST;
	opts.BucketId = cs.length() ? cs.get_data() : nullptr;
	return int(EOS_SessionModification_SetBucketId(handle, &opts));
}

int EpicSessionModification::set_max_players(int p_count) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_SessionModification_SetMaxPlayersOptions opts = {};
	opts.ApiVersion = EOS_SESSIONMODIFICATION_SETMAXPLAYERS_API_LATEST;
	opts.MaxPlayers = uint32_t(p_count);
	return int(EOS_SessionModification_SetMaxPlayers(handle, &opts));
}

int EpicSessionModification::set_host_address(const String &p_host) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_host.utf8();
	EOS_SessionModification_SetHostAddressOptions opts = {};
	opts.ApiVersion = EOS_SESSIONMODIFICATION_SETHOSTADDRESS_API_LATEST;
	opts.HostAddress = cs.length() ? cs.get_data() : nullptr;
	return int(EOS_SessionModification_SetHostAddress(handle, &opts));
}

int EpicSessionModification::set_permission_level(int p_level) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_SessionModification_SetPermissionLevelOptions opts = {};
	opts.ApiVersion = EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
	opts.PermissionLevel = EOS_EOnlineSessionPermissionLevel(p_level);
	return int(EOS_SessionModification_SetPermissionLevel(handle, &opts));
}

int EpicSessionModification::set_join_in_progress_allowed(bool p_allowed) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_SessionModification_SetJoinInProgressAllowedOptions opts = {};
	opts.ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST;
	opts.bAllowJoinInProgress = p_allowed ? EOS_TRUE : EOS_FALSE;
	return int(EOS_SessionModification_SetJoinInProgressAllowed(handle, &opts));
}

int EpicSessionModification::set_invites_allowed(bool p_allowed) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_SessionModification_SetInvitesAllowedOptions opts = {};
	opts.ApiVersion = EOS_SESSIONMODIFICATION_SETINVITESALLOWED_API_LATEST;
	opts.bInvitesAllowed = p_allowed ? EOS_TRUE : EOS_FALSE;
	return int(EOS_SessionModification_SetInvitesAllowed(handle, &opts));
}

int EpicSessionModification::add_attribute(const Dictionary &p_attr) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString key_cs = String(dict_get_string(p_attr, "key")).utf8();
	CharString str_cs;
	EOS_Sessions_AttributeData data = {};
	data.ApiVersion = EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST;
	data.Key = key_cs.length() ? key_cs.get_data() : nullptr;
	data.ValueType = EOS_ESessionAttributeType(dict_get_int(p_attr, "value_type", int(EOS_ESessionAttributeType::EOS_AT_STRING)));
	if (p_attr.has("value")) {
		const Variant &v = p_attr["value"];
		switch (data.ValueType) {
			case EOS_ESessionAttributeType::EOS_AT_BOOLEAN:
				data.Value.AsBool = bool(v) ? EOS_TRUE : EOS_FALSE;
				break;
			case EOS_ESessionAttributeType::EOS_AT_INT64:
				data.Value.AsInt64 = int64_t(v);
				break;
			case EOS_ESessionAttributeType::EOS_AT_DOUBLE:
				data.Value.AsDouble = double(v);
				break;
			case EOS_ESessionAttributeType::EOS_AT_STRING:
			default:
				str_cs = String(v).utf8();
				data.Value.AsUtf8 = str_cs.get_data();
				break;
		}
	}
	EOS_SessionModification_AddAttributeOptions opts = {};
	opts.ApiVersion = EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST;
	opts.SessionAttribute = &data;
	opts.AdvertisementType = EOS_ESessionAttributeAdvertisementType(dict_get_int(p_attr, "advertisement_type", int(EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise)));
	return int(EOS_SessionModification_AddAttribute(handle, &opts));
}

int EpicSessionModification::remove_attribute(const String &p_key) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_key.utf8();
	EOS_SessionModification_RemoveAttributeOptions opts = {};
	opts.ApiVersion = EOS_SESSIONMODIFICATION_REMOVEATTRIBUTE_API_LATEST;
	opts.Key = cs.get_data();
	return int(EOS_SessionModification_RemoveAttribute(handle, &opts));
}
