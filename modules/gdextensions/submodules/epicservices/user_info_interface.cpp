/**************************************************************************/
/*  user_info_interface.cpp                                               */
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

#include "eos_userinfo.h"

#define UI_GUARD_VOID(SIGNAL)                                                   \
	if (!user_info_handle) {                                                    \
		Dictionary payload;                                                     \
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured); \
		_emit_deferred(SIGNAL, payload);                                        \
		return;                                                                 \
	}

static void EOS_CALL _on_ui_query(const EOS_UserInfo_QueryUserInfoCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_eaid_to_string(data->TargetUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::user_info_interface_query_user_info(const Dictionary &p_options) {
	UI_GUARD_VOID("user_info_interface_query_user_info_callback");
	EOS_UserInfo_QueryUserInfoOptions opts = {};
	opts.ApiVersion = EOS_USERINFO_QUERYUSERINFO_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("user_info_interface_query_user_info_callback")));
	EOS_UserInfo_QueryUserInfo(user_info_handle, &opts, cb, &_on_ui_query);
}

static void EOS_CALL _on_ui_query_by_name(const EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_eaid_to_string(data->TargetUserId);
	payload["display_name"] = String::utf8(data->DisplayName ? data->DisplayName : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::user_info_interface_query_user_info_by_display_name(const Dictionary &p_options) {
	UI_GUARD_VOID("user_info_interface_query_user_info_by_display_name_callback");
	const CharString name_cs = dict_get_string(p_options, "display_name").utf8();
	EOS_UserInfo_QueryUserInfoByDisplayNameOptions opts = {};
	opts.ApiVersion = EOS_USERINFO_QUERYUSERINFOBYDISPLAYNAME_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.DisplayName = name_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("user_info_interface_query_user_info_by_display_name_callback")));
	EOS_UserInfo_QueryUserInfoByDisplayName(user_info_handle, &opts, cb, &_on_ui_query_by_name);
}

static void EOS_CALL _on_ui_query_by_external(const EOS_UserInfo_QueryUserInfoByExternalAccountCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["external_account_id"] = String::utf8(data->ExternalAccountId ? data->ExternalAccountId : "");
	payload["account_type"] = int(data->AccountType);
	payload["target_user_id"] = eos_eaid_to_string(data->TargetUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::user_info_interface_query_user_info_by_external_account(const Dictionary &p_options) {
	UI_GUARD_VOID("user_info_interface_query_user_info_by_external_account_callback");
	const CharString id_cs = dict_get_string(p_options, "external_account_id").utf8();
	EOS_UserInfo_QueryUserInfoByExternalAccountOptions opts = {};
	opts.ApiVersion = EOS_USERINFO_QUERYUSERINFOBYEXTERNALACCOUNT_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.ExternalAccountId = id_cs.get_data();
	opts.AccountType = EOS_EExternalAccountType(dict_get_int(p_options, "account_type", int(EOS_EExternalAccountType::EOS_EAT_EPIC)));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("user_info_interface_query_user_info_by_external_account_callback")));
	EOS_UserInfo_QueryUserInfoByExternalAccount(user_info_handle, &opts, cb, &_on_ui_query_by_external);
}

Dictionary EpicServices::user_info_interface_copy_user_info(const Dictionary &p_options) {
	Dictionary out;
	if (!user_info_handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotConfigured);
		return out;
	}
	EOS_UserInfo_CopyUserInfoOptions opts = {};
	opts.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	EOS_UserInfo *info = nullptr;
	EOS_EResult r = EOS_UserInfo_CopyUserInfo(user_info_handle, &opts, &info);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && info) {
		out["user_id"] = eos_eaid_to_string(info->UserId);
		out["country"] = String::utf8(info->Country ? info->Country : "");
		out["display_name"] = String::utf8(info->DisplayName ? info->DisplayName : "");
		out["preferred_language"] = String::utf8(info->PreferredLanguage ? info->PreferredLanguage : "");
		out["nickname"] = String::utf8(info->Nickname ? info->Nickname : "");
		out["display_name_sanitized"] = String::utf8(info->DisplayNameSanitized ? info->DisplayNameSanitized : "");
		EOS_UserInfo_Release(info);
	}
	return out;
}

int EpicServices::user_info_interface_get_external_user_info_count(const Dictionary &p_options) {
	if (!user_info_handle) {
		return 0;
	}
	EOS_UserInfo_GetExternalUserInfoCountOptions opts = {};
	opts.ApiVersion = EOS_USERINFO_GETEXTERNALUSERINFOCOUNT_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	return int(EOS_UserInfo_GetExternalUserInfoCount(user_info_handle, &opts));
}

Dictionary EpicServices::user_info_interface_copy_external_user_info_by_index(const Dictionary &p_options) {
	Dictionary out;
	if (!user_info_handle) {
		return out;
	}
	EOS_UserInfo_CopyExternalUserInfoByIndexOptions opts = {};
	opts.ApiVersion = EOS_USERINFO_COPYEXTERNALUSERINFOBYINDEX_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	opts.Index = uint32_t(dict_get_int(p_options, "index", 0));
	EOS_UserInfo_ExternalUserInfo *info = nullptr;
	if (EOS_UserInfo_CopyExternalUserInfoByIndex(user_info_handle, &opts, &info) == EOS_EResult::EOS_Success && info) {
		out["account_type"] = int(info->AccountType);
		out["account_id"] = String::utf8(info->AccountId ? info->AccountId : "");
		out["display_name"] = String::utf8(info->DisplayName ? info->DisplayName : "");
		out["display_name_sanitized"] = String::utf8(info->DisplayNameSanitized ? info->DisplayNameSanitized : "");
		EOS_UserInfo_ExternalUserInfo_Release(info);
	}
	return out;
}

Dictionary EpicServices::user_info_interface_copy_best_display_name(const Dictionary &p_options) {
	Dictionary out;
	if (!user_info_handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotConfigured);
		return out;
	}
	EOS_UserInfo_CopyBestDisplayNameOptions opts = {};
	opts.ApiVersion = EOS_USERINFO_COPYBESTDISPLAYNAME_API_LATEST;
	opts.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_eaid_from_string(dict_get_string(p_options, "target_user_id"));
	EOS_UserInfo_BestDisplayName *bdn = nullptr;
	EOS_EResult r = EOS_UserInfo_CopyBestDisplayName(user_info_handle, &opts, &bdn);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && bdn) {
		out["user_id"] = eos_eaid_to_string(bdn->UserId);
		out["display_name"] = String::utf8(bdn->DisplayName ? bdn->DisplayName : "");
		out["display_name_sanitized"] = String::utf8(bdn->DisplayNameSanitized ? bdn->DisplayNameSanitized : "");
		out["nickname"] = String::utf8(bdn->Nickname ? bdn->Nickname : "");
		out["platform_type"] = int(bdn->PlatformType);
		EOS_UserInfo_BestDisplayName_Release(bdn);
	}
	return out;
}

int EpicServices::user_info_interface_get_local_platform_type() {
	if (!user_info_handle) {
		return 0;
	}
	EOS_UserInfo_GetLocalPlatformTypeOptions opts = {};
	opts.ApiVersion = EOS_USERINFO_GETLOCALPLATFORMTYPE_API_LATEST;
	return int(EOS_UserInfo_GetLocalPlatformType(user_info_handle, &opts));
}
