/**************************************************************************/
/*  auth_interface.cpp                                                    */
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

#include "eos_auth.h"
#include "eos_auth_types.h"

// Build a result Dictionary common to every Auth callback.
static Dictionary _auth_pin_grant_to_dict(const EOS_Auth_PinGrantInfo *p) {
	Dictionary d;
	if (!p) {
		return d;
	}
	d["user_code"] = String::utf8(p->UserCode ? p->UserCode : "");
	d["verification_uri"] = String::utf8(p->VerificationURI ? p->VerificationURI : "");
	d["verification_uri_complete"] = String::utf8(p->VerificationURIComplete ? p->VerificationURIComplete : "");
	d["expires_in"] = int(p->ExpiresIn);
	return d;
}

// ------ Login --------------------------------------------------------------

static void EOS_CALL _on_auth_login(const EOS_Auth_LoginCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["selected_account_id"] = eos_eaid_to_string(data->SelectedAccountId);
	payload["continuance_token"] = eos_continuance_token_to_string(data->ContinuanceToken);
	payload["pin_grant_info"] = _auth_pin_grant_to_dict(data->PinGrantInfo);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::auth_interface_login(const Dictionary &p_options) {
	if (!auth_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("auth_interface_login_callback", payload);
		return;
	}

	const Dictionary creds_dict = p_options.has("credentials") ? Dictionary(p_options["credentials"]) : Dictionary();
	const CharString id_cs = dict_get_string(creds_dict, "id").utf8();
	const CharString token_cs = dict_get_string(creds_dict, "token").utf8();

	EOS_Auth_Credentials credentials = {};
	credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	credentials.Id = id_cs.length() ? id_cs.get_data() : nullptr;
	credentials.Token = token_cs.length() ? token_cs.get_data() : nullptr;
	credentials.Type = EOS_ELoginCredentialType(dict_get_int(creds_dict, "type", int(EOS_ELoginCredentialType::EOS_LCT_AccountPortal)));
	credentials.SystemAuthCredentialsOptions = nullptr;
	credentials.ExternalType = EOS_EExternalCredentialType(dict_get_int(creds_dict, "external_type", int(EOS_EExternalCredentialType::EOS_ECT_EPIC)));

	EOS_Auth_LoginOptions options = {};
	options.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	options.Credentials = &credentials;
	options.ScopeFlags = EOS_EAuthScopeFlags(dict_get_int64(p_options, "scope_flags",
			int64_t(EOS_EAuthScopeFlags::EOS_AS_BasicProfile | EOS_EAuthScopeFlags::EOS_AS_FriendsList | EOS_EAuthScopeFlags::EOS_AS_Presence)));
	options.LoginFlags = uint64_t(dict_get_int64(p_options, "login_flags", 0));

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("auth_interface_login_callback")));
	EOS_Auth_Login(auth_handle, &options, cb, &_on_auth_login);
}

// ------ Logout -------------------------------------------------------------

static void EOS_CALL _on_auth_logout(const EOS_Auth_LogoutCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::auth_interface_logout(const Dictionary &p_options) {
	if (!auth_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("auth_interface_logout_callback", payload);
		return;
	}
	EOS_Auth_LogoutOptions options = {};
	options.ApiVersion = EOS_AUTH_LOGOUT_API_LATEST;
	options.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("auth_interface_logout_callback")));
	EOS_Auth_Logout(auth_handle, &options, cb, &_on_auth_logout);
}

// ------ Link account -------------------------------------------------------

static void EOS_CALL _on_auth_link_account(const EOS_Auth_LinkAccountCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["selected_account_id"] = eos_eaid_to_string(data->SelectedAccountId);
	payload["pin_grant_info"] = _auth_pin_grant_to_dict(data->PinGrantInfo);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::auth_interface_link_account(const Dictionary &p_options) {
	if (!auth_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("auth_interface_link_account_callback", payload);
		return;
	}
	const String token_str = dict_get_string(p_options, "continuance_token");
	const CharString token_cs = token_str.utf8();

	EOS_Auth_LinkAccountOptions options = {};
	options.ApiVersion = EOS_AUTH_LINKACCOUNT_API_LATEST;
	options.LinkAccountFlags = EOS_ELinkAccountFlags(dict_get_int(p_options, "link_account_flags", int(EOS_ELinkAccountFlags::EOS_LA_NoFlags)));
	options.ContinuanceToken = nullptr; // Resolution requires the original token from the login callback; if string-only token round-trip is needed, see Connect login flow.
	options.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	(void)token_cs; // continuance tokens cannot be re-created from string; caller must pass the live opaque pointer via a future helper

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("auth_interface_link_account_callback")));
	EOS_Auth_LinkAccount(auth_handle, &options, cb, &_on_auth_link_account);
}

// ------ Delete persistent auth ---------------------------------------------

static void EOS_CALL _on_auth_delete_persistent(const EOS_Auth_DeletePersistentAuthCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::auth_interface_delete_persistent_auth(const Dictionary &p_options) {
	if (!auth_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("auth_interface_delete_persistent_auth_callback", payload);
		return;
	}
	const CharString refresh_cs = dict_get_string(p_options, "refresh_token").utf8();
	EOS_Auth_DeletePersistentAuthOptions options = {};
	options.ApiVersion = EOS_AUTH_DELETEPERSISTENTAUTH_API_LATEST;
	options.RefreshToken = refresh_cs.length() ? refresh_cs.get_data() : nullptr;

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("auth_interface_delete_persistent_auth_callback")));
	EOS_Auth_DeletePersistentAuth(auth_handle, &options, cb, &_on_auth_delete_persistent);
}

// ------ Verify user auth ---------------------------------------------------

static void EOS_CALL _on_auth_verify_user(const EOS_Auth_VerifyUserAuthCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::auth_interface_verify_user_auth(const Dictionary &p_options) {
	if (!auth_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("auth_interface_verify_user_auth_callback", payload);
		return;
	}
	(void)p_options; // Verify takes an EOS_Auth_Token* which the caller obtains via copy_user_auth_token; binding for that round-trip will land alongside Token wrapper in Phase 4
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("auth_interface_verify_user_auth_callback")));
	EOS_Auth_VerifyUserAuthOptions options = {};
	options.ApiVersion = EOS_AUTH_VERIFYUSERAUTH_API_LATEST;
	options.AuthToken = nullptr;
	EOS_Auth_VerifyUserAuth(auth_handle, &options, cb, &_on_auth_verify_user);
}

// ------ Query / verify ID token --------------------------------------------

static void EOS_CALL _on_auth_query_id_token(const EOS_Auth_QueryIdTokenCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["target_account_id"] = eos_eaid_to_string(data->TargetAccountId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::auth_interface_query_id_token(const Dictionary &p_options) {
	if (!auth_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("auth_interface_query_id_token_callback", payload);
		return;
	}
	EOS_Auth_QueryIdTokenOptions options = {};
	options.ApiVersion = EOS_AUTH_QUERYIDTOKEN_API_LATEST;
	options.LocalUserId = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	options.TargetAccountId = eos_eaid_from_string(dict_get_string(p_options, "target_account_id"));

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("auth_interface_query_id_token_callback")));
	EOS_Auth_QueryIdToken(auth_handle, &options, cb, &_on_auth_query_id_token);
}

static void EOS_CALL _on_auth_verify_id_token(const EOS_Auth_VerifyIdTokenCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["application_id"] = String::utf8(data->ApplicationId ? data->ApplicationId : "");
	payload["client_id"] = String::utf8(data->ClientId ? data->ClientId : "");
	payload["product_id"] = String::utf8(data->ProductId ? data->ProductId : "");
	payload["sandbox_id"] = String::utf8(data->SandboxId ? data->SandboxId : "");
	payload["deployment_id"] = String::utf8(data->DeploymentId ? data->DeploymentId : "");
	payload["display_name"] = String::utf8(data->DisplayName ? data->DisplayName : "");
	payload["is_external_account_info_present"] = bool(data->bIsExternalAccountInfoPresent == EOS_TRUE);
	payload["external_account_id_type"] = int(data->ExternalAccountIdType);
	payload["external_account_id"] = String::utf8(data->ExternalAccountId ? data->ExternalAccountId : "");
	payload["external_account_display_name"] = String::utf8(data->ExternalAccountDisplayName ? data->ExternalAccountDisplayName : "");
	payload["platform"] = String::utf8(data->Platform ? data->Platform : "");
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::auth_interface_verify_id_token(const Dictionary &p_options) {
	if (!auth_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("auth_interface_verify_id_token_callback", payload);
		return;
	}
	const CharString token_cs = dict_get_string(p_options, "id_token").utf8();
	EOS_Auth_IdToken token = {};
	token.ApiVersion = EOS_AUTH_IDTOKEN_API_LATEST;
	token.AccountId = eos_eaid_from_string(dict_get_string(p_options, "account_id"));
	token.JsonWebToken = token_cs.length() ? token_cs.get_data() : nullptr;

	EOS_Auth_VerifyIdTokenOptions options = {};
	options.ApiVersion = EOS_AUTH_VERIFYIDTOKEN_API_LATEST;
	options.IdToken = &token;

	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("auth_interface_verify_id_token_callback")));
	EOS_Auth_VerifyIdToken(auth_handle, &options, cb, &_on_auth_verify_id_token);
}

// ------ Copy auth/id token (sync) ------------------------------------------

Dictionary EpicServices::auth_interface_copy_user_auth_token(const Dictionary &p_options) {
	Dictionary out;
	if (!auth_handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotConfigured);
		return out;
	}
	EOS_Auth_CopyUserAuthTokenOptions options = {};
	options.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

	EOS_EpicAccountId user = eos_eaid_from_string(dict_get_string(p_options, "local_user_id"));
	EOS_Auth_Token *token = nullptr;
	EOS_EResult r = EOS_Auth_CopyUserAuthToken(auth_handle, &options, user, &token);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && token) {
		out["app"] = String::utf8(token->App ? token->App : "");
		out["client_id"] = String::utf8(token->ClientId ? token->ClientId : "");
		out["account_id"] = eos_eaid_to_string(token->AccountId);
		out["access_token"] = String::utf8(token->AccessToken ? token->AccessToken : "");
		out["expires_in"] = double(token->ExpiresIn);
		out["expires_at"] = String::utf8(token->ExpiresAt ? token->ExpiresAt : "");
		out["auth_type"] = int(token->AuthType);
		out["refresh_token"] = String::utf8(token->RefreshToken ? token->RefreshToken : "");
		out["refresh_expires_in"] = double(token->RefreshExpiresIn);
		out["refresh_expires_at"] = String::utf8(token->RefreshExpiresAt ? token->RefreshExpiresAt : "");
		EOS_Auth_Token_Release(token);
	}
	return out;
}

Dictionary EpicServices::auth_interface_copy_id_token(const Dictionary &p_options) {
	Dictionary out;
	if (!auth_handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotConfigured);
		return out;
	}
	EOS_Auth_CopyIdTokenOptions options = {};
	options.ApiVersion = EOS_AUTH_COPYIDTOKEN_API_LATEST;
	options.AccountId = eos_eaid_from_string(dict_get_string(p_options, "account_id"));

	EOS_Auth_IdToken *token = nullptr;
	EOS_EResult r = EOS_Auth_CopyIdToken(auth_handle, &options, &token);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && token) {
		out["account_id"] = eos_eaid_to_string(token->AccountId);
		out["json_web_token"] = String::utf8(token->JsonWebToken ? token->JsonWebToken : "");
		EOS_Auth_IdToken_Release(token);
	}
	return out;
}

// ------ Sync getters --------------------------------------------------------

int EpicServices::auth_interface_get_logged_in_accounts_count() {
	if (!auth_handle) {
		return 0;
	}
	return EOS_Auth_GetLoggedInAccountsCount(auth_handle);
}

String EpicServices::auth_interface_get_logged_in_account_by_index(int p_index) {
	if (!auth_handle) {
		return String();
	}
	return eos_eaid_to_string(EOS_Auth_GetLoggedInAccountByIndex(auth_handle, p_index));
}

int EpicServices::auth_interface_get_login_status(const String &p_local_user_id) {
	if (!auth_handle) {
		return int(EOS_ELoginStatus::EOS_LS_NotLoggedIn);
	}
	return int(EOS_Auth_GetLoginStatus(auth_handle, eos_eaid_from_string(p_local_user_id)));
}

String EpicServices::auth_interface_get_selected_account_id(const String &p_local_user_id) {
	if (!auth_handle) {
		return String();
	}
	EOS_EpicAccountId out_id = nullptr;
	EOS_EResult r = EOS_Auth_GetSelectedAccountId(auth_handle, eos_eaid_from_string(p_local_user_id), &out_id);
	if (r != EOS_EResult::EOS_Success) {
		return String();
	}
	return eos_eaid_to_string(out_id);
}

int EpicServices::auth_interface_get_merged_accounts_count(const String &p_local_user_id) {
	if (!auth_handle) {
		return 0;
	}
	return int(EOS_Auth_GetMergedAccountsCount(auth_handle, eos_eaid_from_string(p_local_user_id)));
}

String EpicServices::auth_interface_get_merged_account_by_index(const String &p_local_user_id, int p_index) {
	if (!auth_handle) {
		return String();
	}
	return eos_eaid_to_string(EOS_Auth_GetMergedAccountByIndex(auth_handle, eos_eaid_from_string(p_local_user_id), uint32_t(p_index)));
}

// ------ Notify subscriptions -----------------------------------------------

static void EOS_CALL _on_auth_login_status_changed(const EOS_Auth_LoginStatusChangedCallbackInfo *data) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es) {
		return;
	}
	Dictionary payload;
	payload["local_user_id"] = eos_eaid_to_string(data->LocalUserId);
	payload["prev_status"] = int(data->PrevStatus);
	payload["current_status"] = int(data->CurrentStatus);
	epic_emit_deferred(es->get_instance_id(), StringName("auth_interface_login_status_changed"), payload);
}

uint64_t EpicServices::auth_interface_add_notify_login_status_changed() {
	if (!auth_handle) {
		return 0;
	}
	EOS_Auth_AddNotifyLoginStatusChangedOptions options = {};
	options.ApiVersion = EOS_AUTH_ADDNOTIFYLOGINSTATUSCHANGED_API_LATEST;
	return uint64_t(EOS_Auth_AddNotifyLoginStatusChanged(auth_handle, &options, nullptr, &_on_auth_login_status_changed));
}

void EpicServices::auth_interface_remove_notify_login_status_changed(uint64_t p_id) {
	if (!auth_handle) {
		return;
	}
	EOS_Auth_RemoveNotifyLoginStatusChanged(auth_handle, EOS_NotificationId(p_id));
}
