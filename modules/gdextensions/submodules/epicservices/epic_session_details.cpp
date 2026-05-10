/**************************************************************************/
/*  epic_session_details.cpp                                              */
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

#include "epic_session_details.h"

#include "epic_utils.h"

#include "eos_sessions.h"

void EpicSessionDetails::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicSessionDetails::is_valid);
	ClassDB::bind_method(D_METHOD("copy_info"), &EpicSessionDetails::copy_info);
	ClassDB::bind_method(D_METHOD("get_session_attribute_count"), &EpicSessionDetails::get_session_attribute_count);
	ClassDB::bind_method(D_METHOD("copy_session_attribute_by_index", "index"), &EpicSessionDetails::copy_session_attribute_by_index);
	ClassDB::bind_method(D_METHOD("copy_session_attribute_by_key", "key"), &EpicSessionDetails::copy_session_attribute_by_key);
}

void EpicSessionDetails::set_handle(EOS_HSessionDetails p_handle) {
	if (handle && handle != p_handle) {
		EOS_SessionDetails_Release(handle);
	}
	handle = p_handle;
}

EpicSessionDetails::~EpicSessionDetails() {
	if (handle) {
		EOS_SessionDetails_Release(handle);
		handle = nullptr;
	}
}

Dictionary EpicSessionDetails::copy_info() const {
	Dictionary out;
	if (!handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotFound);
		return out;
	}
	EOS_SessionDetails_CopyInfoOptions opts = {};
	opts.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;
	EOS_SessionDetails_Info *info = nullptr;
	EOS_EResult r = EOS_SessionDetails_CopyInfo(handle, &opts, &info);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && info) {
		out["session_id"] = String::utf8(info->SessionId ? info->SessionId : "");
		out["host_address"] = String::utf8(info->HostAddress ? info->HostAddress : "");
		out["num_open_public_connections"] = int(info->NumOpenPublicConnections);
		if (info->Settings) {
			Dictionary settings;
			settings["bucket_id"] = String::utf8(info->Settings->BucketId ? info->Settings->BucketId : "");
			settings["num_public_connections"] = int(info->Settings->NumPublicConnections);
			settings["allow_join_in_progress"] = bool(info->Settings->bAllowJoinInProgress == EOS_TRUE);
			settings["permission_level"] = int(info->Settings->PermissionLevel);
			settings["invites_allowed"] = bool(info->Settings->bInvitesAllowed == EOS_TRUE);
			settings["sanctions_enabled"] = bool(info->Settings->bSanctionsEnabled == EOS_TRUE);
			out["settings"] = settings;
		}
		EOS_SessionDetails_Info_Release(info);
	}
	return out;
}

int EpicSessionDetails::get_session_attribute_count() const {
	if (!handle) {
		return 0;
	}
	EOS_SessionDetails_GetSessionAttributeCountOptions opts = {};
	opts.ApiVersion = EOS_SESSIONDETAILS_GETSESSIONATTRIBUTECOUNT_API_LATEST;
	return int(EOS_SessionDetails_GetSessionAttributeCount(handle, &opts));
}

static Dictionary _session_attr_to_dict(const EOS_SessionDetails_Attribute *attr) {
	Dictionary d;
	if (!attr || !attr->Data) {
		return d;
	}
	d["advertisement_type"] = int(attr->AdvertisementType);
	d["key"] = String::utf8(attr->Data->Key ? attr->Data->Key : "");
	d["value_type"] = int(attr->Data->ValueType);
	switch (attr->Data->ValueType) {
		case EOS_ESessionAttributeType::EOS_AT_BOOLEAN:
			d["value"] = bool(attr->Data->Value.AsBool == EOS_TRUE);
			break;
		case EOS_ESessionAttributeType::EOS_AT_INT64:
			d["value"] = int64_t(attr->Data->Value.AsInt64);
			break;
		case EOS_ESessionAttributeType::EOS_AT_DOUBLE:
			d["value"] = double(attr->Data->Value.AsDouble);
			break;
		case EOS_ESessionAttributeType::EOS_AT_STRING:
			d["value"] = String::utf8(attr->Data->Value.AsUtf8 ? attr->Data->Value.AsUtf8 : "");
			break;
		default:
			break;
	}
	return d;
}

Dictionary EpicSessionDetails::copy_session_attribute_by_index(int p_index) const {
	Dictionary out;
	if (!handle) {
		return out;
	}
	EOS_SessionDetails_CopySessionAttributeByIndexOptions opts = {};
	opts.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST;
	opts.AttrIndex = uint32_t(p_index);
	EOS_SessionDetails_Attribute *attr = nullptr;
	if (EOS_SessionDetails_CopySessionAttributeByIndex(handle, &opts, &attr) == EOS_EResult::EOS_Success && attr) {
		out = _session_attr_to_dict(attr);
		EOS_SessionDetails_Attribute_Release(attr);
	}
	return out;
}

Dictionary EpicSessionDetails::copy_session_attribute_by_key(const String &p_key) const {
	Dictionary out;
	if (!handle) {
		return out;
	}
	const CharString cs = p_key.utf8();
	EOS_SessionDetails_CopySessionAttributeByKeyOptions opts = {};
	opts.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYKEY_API_LATEST;
	opts.AttrKey = cs.get_data();
	EOS_SessionDetails_Attribute *attr = nullptr;
	if (EOS_SessionDetails_CopySessionAttributeByKey(handle, &opts, &attr) == EOS_EResult::EOS_Success && attr) {
		out = _session_attr_to_dict(attr);
		EOS_SessionDetails_Attribute_Release(attr);
	}
	return out;
}
