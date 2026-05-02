/**************************************************************************/
/*  connect_interface.cpp                                                 */
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

#include "eos_connect.h"
#include "eos_connect_types.h"

// ------ Login --------------------------------------------------------------

static void EOS_CALL _on_connect_login(const EOS_Connect_LoginCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["continuance_token"] = eos_continuance_token_to_string(data->ContinuanceToken);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_login(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_login_callback", payload);
		return;
	}
	const Dictionary creds_dict = p_options.has("credentials") ? Dictionary(p_options["credentials"]) : Dictionary();
	const CharString token_cs = dict_get_string(creds_dict, "token").utf8();

	EOS_Connect_Credentials credentials = {};
	credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	credentials.Token = token_cs.length() ? token_cs.get_data() : nullptr;
	credentials.Type = EOS_EExternalCredentialType(dict_get_int(creds_dict, "type", int(EOS_EExternalCredentialType::EOS_ECT_EPIC)));

	EOS_Connect_LoginOptions options = {};
	options.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	options.Credentials = &credentials;

	EOS_Connect_UserLoginInfo user_info = {};
	CharString display_name_cs;
	CharString nsa_cs;
	if (p_options.has("user_login_info")) {
		const Dictionary uli = Dictionary(p_options["user_login_info"]);
		display_name_cs = dict_get_string(uli, "display_name").utf8();
		nsa_cs = dict_get_string(uli, "nsa_id_token").utf8();
		user_info.ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
		user_info.DisplayName = display_name_cs.length() ? display_name_cs.get_data() : nullptr;
		user_info.NsaIdToken = nsa_cs.length() ? nsa_cs.get_data() : nullptr;
		options.UserLoginInfo = &user_info;
	}

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_login_callback")));
	EOS_Connect_Login(connect_handle, &options, cb, &_on_connect_login);
}

// ------ Logout -------------------------------------------------------------

static void EOS_CALL _on_connect_logout(const EOS_Connect_LogoutCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_logout(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_logout_callback", payload);
		return;
	}
	EOS_Connect_LogoutOptions options = {};
	options.ApiVersion = EOS_CONNECT_LOGOUT_API_LATEST;
	options.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_logout_callback")));
	EOS_Connect_Logout(connect_handle, &options, cb, &_on_connect_logout);
}

// ------ Create user --------------------------------------------------------

static void EOS_CALL _on_connect_create_user(const EOS_Connect_CreateUserCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_create_user(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_create_user_callback", payload);
		return;
	}
	(void)p_options; // Continuance token cannot be reconstructed from a String yet — handled by future ContinuanceToken Reference wrapper. For now CreateUser must be called from inside the login_callback when InvalidUser is returned, using the live token re-passed through a sentinel.
	EOS_Connect_CreateUserOptions options = {};
	options.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
	options.ContinuanceToken = nullptr;

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_create_user_callback")));
	EOS_Connect_CreateUser(connect_handle, &options, cb, &_on_connect_create_user);
}

// ------ Link account -------------------------------------------------------

static void EOS_CALL _on_connect_link_account(const EOS_Connect_LinkAccountCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_link_account(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_link_account_callback", payload);
		return;
	}
	EOS_Connect_LinkAccountOptions options = {};
	options.ApiVersion = EOS_CONNECT_LINKACCOUNT_API_LATEST;
	options.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	options.ContinuanceToken = nullptr; // Same constraint as CreateUser — see above.

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_link_account_callback")));
	EOS_Connect_LinkAccount(connect_handle, &options, cb, &_on_connect_link_account);
}

// ------ Unlink account -----------------------------------------------------

static void EOS_CALL _on_connect_unlink_account(const EOS_Connect_UnlinkAccountCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_unlink_account(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_unlink_account_callback", payload);
		return;
	}
	EOS_Connect_UnlinkAccountOptions options = {};
	options.ApiVersion = EOS_CONNECT_UNLINKACCOUNT_API_LATEST;
	options.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_unlink_account_callback")));
	EOS_Connect_UnlinkAccount(connect_handle, &options, cb, &_on_connect_unlink_account);
}

// ------ Device id ----------------------------------------------------------

static void EOS_CALL _on_connect_create_device_id(const EOS_Connect_CreateDeviceIdCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_create_device_id(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_create_device_id_callback", payload);
		return;
	}
	const CharString device_model_cs = dict_get_string(p_options, "device_model").utf8();
	EOS_Connect_CreateDeviceIdOptions options = {};
	options.ApiVersion = EOS_CONNECT_CREATEDEVICEID_API_LATEST;
	options.DeviceModel = device_model_cs.length() ? device_model_cs.get_data() : nullptr;

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_create_device_id_callback")));
	EOS_Connect_CreateDeviceId(connect_handle, &options, cb, &_on_connect_create_device_id);
}

static void EOS_CALL _on_connect_delete_device_id(const EOS_Connect_DeleteDeviceIdCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_delete_device_id(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_delete_device_id_callback", payload);
		return;
	}
	(void)p_options;
	EOS_Connect_DeleteDeviceIdOptions options = {};
	options.ApiVersion = EOS_CONNECT_DELETEDEVICEID_API_LATEST;

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_delete_device_id_callback")));
	EOS_Connect_DeleteDeviceId(connect_handle, &options, cb, &_on_connect_delete_device_id);
}

// ------ Account mappings ---------------------------------------------------

static void EOS_CALL _on_connect_query_external(const EOS_Connect_QueryExternalAccountMappingsCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_query_external_account_mappings(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_query_external_account_mappings_callback", payload);
		return;
	}
	const Array external_ids_arr = p_options.has("external_account_ids") ? Array(p_options["external_account_ids"]) : Array();
	Vector<CharString> backing;
	backing.resize(external_ids_arr.size());
	Vector<const char *> ptrs;
	ptrs.resize(external_ids_arr.size());
	for (int i = 0; i < external_ids_arr.size(); ++i) {
		backing.write[i] = String(external_ids_arr[i]).utf8();
		ptrs.write[i] = backing[i].length() ? backing[i].get_data() : "";
	}

	EOS_Connect_QueryExternalAccountMappingsOptions options = {};
	options.ApiVersion = EOS_CONNECT_QUERYEXTERNALACCOUNTMAPPINGS_API_LATEST;
	options.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	options.AccountIdType = EOS_EExternalAccountType(dict_get_int(p_options, "account_id_type", int(EOS_EExternalAccountType::EOS_EAT_EPIC)));
	options.ExternalAccountIds = ptrs.size() ? ptrs.ptrw() : nullptr;
	options.ExternalAccountIdCount = uint32_t(ptrs.size());

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_query_external_account_mappings_callback")));
	EOS_Connect_QueryExternalAccountMappings(connect_handle, &options, cb, &_on_connect_query_external);
}

static void EOS_CALL _on_connect_query_pui(const EOS_Connect_QueryProductUserIdMappingsCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_query_product_user_id_mappings(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_query_product_user_id_mappings_callback", payload);
		return;
	}
	const Array pui_arr = p_options.has("product_user_ids") ? Array(p_options["product_user_ids"]) : Array();
	Vector<EOS_ProductUserId> ids;
	ids.resize(pui_arr.size());
	for (int i = 0; i < pui_arr.size(); ++i) {
		ids.write[i] = eos_pui_from_string(String(pui_arr[i]));
	}

	EOS_Connect_QueryProductUserIdMappingsOptions options = {};
	options.ApiVersion = EOS_CONNECT_QUERYPRODUCTUSERIDMAPPINGS_API_LATEST;
	options.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	options.ProductUserIds = ids.size() ? ids.ptrw() : nullptr;
	options.ProductUserIdCount = uint32_t(ids.size());

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_query_product_user_id_mappings_callback")));
	EOS_Connect_QueryProductUserIdMappings(connect_handle, &options, cb, &_on_connect_query_pui);
}

String EpicServices::connect_interface_get_external_account_mapping(const Dictionary &p_options) {
	if (!connect_handle) {
		return String();
	}
	const CharString target_user_cs = dict_get_string(p_options, "target_external_user_id").utf8();
	EOS_Connect_GetExternalAccountMappingsOptions options = {};
	options.ApiVersion = EOS_CONNECT_GETEXTERNALACCOUNTMAPPINGS_API_LATEST;
	options.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	options.AccountIdType = EOS_EExternalAccountType(dict_get_int(p_options, "account_id_type", int(EOS_EExternalAccountType::EOS_EAT_EPIC)));
	options.TargetExternalUserId = target_user_cs.get_data();
	return eos_pui_to_string(EOS_Connect_GetExternalAccountMapping(connect_handle, &options));
}

Dictionary EpicServices::connect_interface_get_product_user_id_mapping(const Dictionary &p_options) {
	Dictionary out;
	if (!connect_handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotConfigured);
		return out;
	}
	EOS_Connect_GetProductUserIdMappingOptions options = {};
	options.ApiVersion = EOS_CONNECT_GETPRODUCTUSERIDMAPPING_API_LATEST;
	options.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	options.AccountIdType = EOS_EExternalAccountType(dict_get_int(p_options, "account_id_type", int(EOS_EExternalAccountType::EOS_EAT_EPIC)));
	options.TargetProductUserId = eos_pui_from_string(dict_get_string(p_options, "target_product_user_id"));

	int32_t buf_len = EOS_CONNECT_EXTERNAL_ACCOUNT_ID_MAX_LENGTH + 1;
	CharString buffer;
	buffer.resize(buf_len);
	EOS_EResult r = EOS_Connect_GetProductUserIdMapping(connect_handle, &options, buffer.ptrw(), &buf_len);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success) {
		out["external_id"] = String::utf8(buffer.get_data());
	}
	return out;
}

int EpicServices::connect_interface_get_logged_in_users_count() {
	if (!connect_handle) {
		return 0;
	}
	return EOS_Connect_GetLoggedInUsersCount(connect_handle);
}

String EpicServices::connect_interface_get_logged_in_user_by_index(int p_index) {
	if (!connect_handle) {
		return String();
	}
	return eos_pui_to_string(EOS_Connect_GetLoggedInUserByIndex(connect_handle, p_index));
}

int EpicServices::connect_interface_get_login_status(const String &p_product_user_id) {
	if (!connect_handle) {
		return int(EOS_ELoginStatus::EOS_LS_NotLoggedIn);
	}
	return int(EOS_Connect_GetLoginStatus(connect_handle, eos_pui_from_string(p_product_user_id)));
}

Dictionary EpicServices::connect_interface_copy_id_token(const Dictionary &p_options) {
	Dictionary out;
	if (!connect_handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotConfigured);
		return out;
	}
	EOS_Connect_CopyIdTokenOptions options = {};
	options.ApiVersion = EOS_CONNECT_COPYIDTOKEN_API_LATEST;
	options.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));

	EOS_Connect_IdToken *token = nullptr;
	EOS_EResult r = EOS_Connect_CopyIdToken(connect_handle, &options, &token);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && token) {
		out["product_user_id"] = eos_pui_to_string(token->ProductUserId);
		out["json_web_token"] = String::utf8(token->JsonWebToken ? token->JsonWebToken : "");
		EOS_Connect_IdToken_Release(token);
	}
	return out;
}

static void EOS_CALL _on_connect_verify_id(const EOS_Connect_VerifyIdTokenCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["product_user_id"] = eos_pui_to_string(data->ProductUserId);
	payload["is_account_info_present"] = bool(data->bIsAccountInfoPresent == EOS_TRUE);
	payload["account_id_type"] = int(data->AccountIdType);
	payload["account_id"] = String::utf8(data->AccountId ? data->AccountId : "");
	payload["platform"] = String::utf8(data->Platform ? data->Platform : "");
	payload["device_type"] = String::utf8(data->DeviceType ? data->DeviceType : "");
	payload["client_id"] = String::utf8(data->ClientId ? data->ClientId : "");
	payload["product_id"] = String::utf8(data->ProductId ? data->ProductId : "");
	payload["sandbox_id"] = String::utf8(data->SandboxId ? data->SandboxId : "");
	payload["deployment_id"] = String::utf8(data->DeploymentId ? data->DeploymentId : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::connect_interface_verify_id_token(const Dictionary &p_options) {
	if (!connect_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("connect_interface_verify_id_token_callback", payload);
		return;
	}
	const CharString token_cs = dict_get_string(p_options, "json_web_token").utf8();
	EOS_Connect_IdToken token = {};
	token.ApiVersion = EOS_CONNECT_IDTOKEN_API_LATEST;
	token.ProductUserId = eos_pui_from_string(dict_get_string(p_options, "product_user_id"));
	token.JsonWebToken = token_cs.length() ? token_cs.get_data() : nullptr;

	EOS_Connect_VerifyIdTokenOptions options = {};
	options.ApiVersion = EOS_CONNECT_VERIFYIDTOKEN_API_LATEST;
	options.IdToken = &token;

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("connect_interface_verify_id_token_callback")));
	EOS_Connect_VerifyIdToken(connect_handle, &options, cb, &_on_connect_verify_id);
}

// ------ Notifications ------------------------------------------------------

static void EOS_CALL _on_connect_login_status_changed(const EOS_Connect_LoginStatusChangedCallbackInfo *data) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es) {
		return;
	}
	Dictionary payload;
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["previous_status"] = int(data->PreviousStatus);
	payload["current_status"] = int(data->CurrentStatus);
	epic_emit_deferred(es->get_instance_id(), StringName("connect_interface_login_status_changed"), payload);
}

uint64_t EpicServices::connect_interface_add_notify_login_status_changed() {
	if (!connect_handle) {
		return 0;
	}
	EOS_Connect_AddNotifyLoginStatusChangedOptions options = {};
	options.ApiVersion = EOS_CONNECT_ADDNOTIFYLOGINSTATUSCHANGED_API_LATEST;
	return uint64_t(EOS_Connect_AddNotifyLoginStatusChanged(connect_handle, &options, nullptr, &_on_connect_login_status_changed));
}

void EpicServices::connect_interface_remove_notify_login_status_changed(uint64_t p_id) {
	if (!connect_handle) {
		return;
	}
	EOS_Connect_RemoveNotifyLoginStatusChanged(connect_handle, EOS_NotificationId(p_id));
}

static void EOS_CALL _on_connect_auth_expiration(const EOS_Connect_AuthExpirationCallbackInfo *data) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es) {
		return;
	}
	Dictionary payload;
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(es->get_instance_id(), StringName("connect_interface_auth_expiration"), payload);
}

uint64_t EpicServices::connect_interface_add_notify_auth_expiration() {
	if (!connect_handle) {
		return 0;
	}
	EOS_Connect_AddNotifyAuthExpirationOptions options = {};
	options.ApiVersion = EOS_CONNECT_ADDNOTIFYAUTHEXPIRATION_API_LATEST;
	return uint64_t(EOS_Connect_AddNotifyAuthExpiration(connect_handle, &options, nullptr, &_on_connect_auth_expiration));
}

void EpicServices::connect_interface_remove_notify_auth_expiration(uint64_t p_id) {
	if (!connect_handle) {
		return;
	}
	EOS_Connect_RemoveNotifyAuthExpiration(connect_handle, EOS_NotificationId(p_id));
}
