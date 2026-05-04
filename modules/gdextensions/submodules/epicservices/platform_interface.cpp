/**************************************************************************/
/*  platform_interface.cpp                                                */
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

#include "epic_utils.h"

#include "core/print_string.h"

#include "eos_init.h"
#include "eos_sdk.h"
#include "eos_types.h"

#ifdef EPIC_SECRETS
#include EPIC_SECRETS
#endif

int EpicServices::platform_interface_initialize(const Dictionary &p_options) {
	if (sdk_initialized) {
		return int(EOS_EResult::EOS_AlreadyConfigured);
	}

	const CharString product_name = String(dict_get_string(p_options, "product_name", "GodotEpicServices")).utf8();
	const CharString product_version = String(dict_get_string(p_options, "product_version", "1.0.0")).utf8();

	EOS_InitializeOptions options = {};
	options.ApiVersion = EOS_INITIALIZE_API_LATEST;
	options.AllocateMemoryFunction = nullptr;
	options.ReallocateMemoryFunction = nullptr;
	options.ReleaseMemoryFunction = nullptr;
	options.ProductName = product_name.get_data();
	options.ProductVersion = product_version.get_data();
	options.Reserved = nullptr;
	options.SystemInitializeOptions = nullptr;
	options.OverrideThreadAffinity = nullptr;

	EOS_EResult result = EOS_Initialize(&options);
	if (result == EOS_EResult::EOS_Success || result == EOS_EResult::EOS_AlreadyConfigured) {
		sdk_initialized = true;
	}
	return int(result);
}

Dictionary EpicServices::platform_interface_create(const Dictionary &p_options) {
	Dictionary out;
	out["result_code"] = int(EOS_EResult::EOS_Success);

	if (!sdk_initialized) {
		// Auto-init from the same options dict — saves callers a step.
		int init_result = platform_interface_initialize(p_options);
		if (init_result != int(EOS_EResult::EOS_Success) && init_result != int(EOS_EResult::EOS_AlreadyConfigured)) {
			out["result_code"] = init_result;
			out["error"] = "EOS_Initialize failed";
			return out;
		}
	}

	if (platform_handle) {
		out["result_code"] = int(EOS_EResult::EOS_AlreadyConfigured);
		out["error"] = "Platform already created";
		return out;
	}

	// Caller-supplied dict values win; fall back to compile-time EPIC_SECRETS constants.
	String product_id_s = dict_get_string(p_options, "product_id");
	String sandbox_id_s = dict_get_string(p_options, "sandbox_id");
	String deployment_id_s = dict_get_string(p_options, "deployment_id");
	String client_id_s = dict_get_string(p_options, "client_id");
	String client_secret_s = dict_get_string(p_options, "client_secret");
	String encryption_key_s = dict_get_string(p_options, "encryption_key");
#ifdef EPIC_SECRETS
	if (product_id_s.empty()) {
		product_id_s = EpicServicesConstants::ProductId;
	}
	if (sandbox_id_s.empty()) {
		sandbox_id_s = EpicServicesConstants::SandboxId;
	}
	if (deployment_id_s.empty()) {
		deployment_id_s = EpicServicesConstants::DeploymentId;
	}
	if (client_id_s.empty()) {
		client_id_s = EpicServicesConstants::ClientCredentialsId;
	}
	if (client_secret_s.empty()) {
		client_secret_s = EpicServicesConstants::ClientCredentialsSecret;
	}
	if (encryption_key_s.empty()) {
		encryption_key_s = EpicServicesConstants::EncryptionKey;
	}
#endif
	const CharString product_id = product_id_s.utf8();
	const CharString sandbox_id = sandbox_id_s.utf8();
	const CharString deployment_id = deployment_id_s.utf8();
	const CharString client_id = client_id_s.utf8();
	const CharString client_secret = client_secret_s.utf8();
	const CharString encryption_key = encryption_key_s.utf8();
	const CharString cache_directory = dict_get_string(p_options, "cache_directory").utf8();
	const CharString override_country = dict_get_string(p_options, "override_country_code").utf8();
	const CharString override_locale = dict_get_string(p_options, "override_locale_code").utf8();

	EOS_Platform_ClientCredentials credentials = {};
	credentials.ClientId = client_id.length() ? client_id.get_data() : nullptr;
	credentials.ClientSecret = client_secret.length() ? client_secret.get_data() : nullptr;

	EOS_Platform_Options options = {};
	options.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	options.Reserved = nullptr;
	options.ProductId = product_id.length() ? product_id.get_data() : nullptr;
	options.SandboxId = sandbox_id.length() ? sandbox_id.get_data() : nullptr;
	options.DeploymentId = deployment_id.length() ? deployment_id.get_data() : nullptr;
	options.ClientCredentials = credentials;
	options.bIsServer = dict_get_bool(p_options, "is_server", false) ? EOS_TRUE : EOS_FALSE;
	options.EncryptionKey = encryption_key.length() ? encryption_key.get_data() : nullptr;
	options.OverrideCountryCode = override_country.length() ? override_country.get_data() : nullptr;
	options.OverrideLocaleCode = override_locale.length() ? override_locale.get_data() : nullptr;
	options.Flags = uint64_t(dict_get_int64(p_options, "flags", 0));
	options.CacheDirectory = cache_directory.length() ? cache_directory.get_data() : nullptr;
	options.TickBudgetInMilliseconds = uint32_t(dict_get_int(p_options, "tick_budget_in_milliseconds", 0));
	options.RTCOptions = nullptr;
	options.IntegratedPlatformOptionsContainerHandle = nullptr;
	options.SystemSpecificOptions = nullptr;
	options.TaskNetworkTimeoutSeconds = nullptr;

	platform_handle = EOS_Platform_Create(&options);
	if (!platform_handle) {
		out["result_code"] = int(EOS_EResult::EOS_UnexpectedError);
		out["error"] = "EOS_Platform_Create returned null";
		return out;
	}

	auth_handle = EOS_Platform_GetAuthInterface(platform_handle);
	connect_handle = EOS_Platform_GetConnectInterface(platform_handle);
	lobby_handle = EOS_Platform_GetLobbyInterface(platform_handle);
	sessions_handle = EOS_Platform_GetSessionsInterface(platform_handle);
	p2p_handle = EOS_Platform_GetP2PInterface(platform_handle);
	custom_invites_handle = EOS_Platform_GetCustomInvitesInterface(platform_handle);
	stats_handle = EOS_Platform_GetStatsInterface(platform_handle);
	leaderboards_handle = EOS_Platform_GetLeaderboardsInterface(platform_handle);
	achievements_handle = EOS_Platform_GetAchievementsInterface(platform_handle);
	friends_handle = EOS_Platform_GetFriendsInterface(platform_handle);
	presence_handle = EOS_Platform_GetPresenceInterface(platform_handle);
	user_info_handle = EOS_Platform_GetUserInfoInterface(platform_handle);
	player_data_storage_handle = EOS_Platform_GetPlayerDataStorageInterface(platform_handle);
	title_storage_handle = EOS_Platform_GetTitleStorageInterface(platform_handle);

	_ensure_processing();
	return out;
}

void EpicServices::platform_interface_release() {
	_stop_processing();
	if (platform_handle) {
		EOS_Platform_Release(platform_handle);
		platform_handle = nullptr;
		auth_handle = nullptr;
		connect_handle = nullptr;
	}
	if (sdk_initialized) {
		EOS_Shutdown();
		sdk_initialized = false;
	}
}

void EpicServices::platform_interface_tick() {
	if (platform_handle) {
		EOS_Platform_Tick(platform_handle);
	}
}

int EpicServices::platform_interface_check_for_launcher_and_restart() {
	if (!platform_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	return int(EOS_Platform_CheckForLauncherAndRestart(platform_handle));
}

String EpicServices::platform_interface_get_active_locale_code(const String &p_local_user_id) {
	if (!platform_handle) {
		return String();
	}
	char buf[EOS_LOCALECODE_MAX_BUFFER_LEN] = { 0 };
	int32_t len = sizeof(buf);
	EOS_EpicAccountId user = eos_eaid_from_string(p_local_user_id);
	if (EOS_Platform_GetActiveLocaleCode(platform_handle, user, buf, &len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf);
}

String EpicServices::platform_interface_get_active_country_code(const String &p_local_user_id) {
	if (!platform_handle) {
		return String();
	}
	char buf[EOS_COUNTRYCODE_MAX_BUFFER_LEN] = { 0 };
	int32_t len = sizeof(buf);
	EOS_EpicAccountId user = eos_eaid_from_string(p_local_user_id);
	if (EOS_Platform_GetActiveCountryCode(platform_handle, user, buf, &len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf);
}

String EpicServices::platform_interface_get_override_locale_code() {
	if (!platform_handle) {
		return String();
	}
	char buf[EOS_LOCALECODE_MAX_BUFFER_LEN] = { 0 };
	int32_t len = sizeof(buf);
	if (EOS_Platform_GetOverrideLocaleCode(platform_handle, buf, &len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf);
}

String EpicServices::platform_interface_get_override_country_code() {
	if (!platform_handle) {
		return String();
	}
	char buf[EOS_COUNTRYCODE_MAX_BUFFER_LEN] = { 0 };
	int32_t len = sizeof(buf);
	if (EOS_Platform_GetOverrideCountryCode(platform_handle, buf, &len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf);
}

int EpicServices::platform_interface_set_override_locale_code(const String &p_locale) {
	if (!platform_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_locale.utf8();
	return int(EOS_Platform_SetOverrideLocaleCode(platform_handle, cs.get_data()));
}

int EpicServices::platform_interface_set_override_country_code(const String &p_country) {
	if (!platform_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_country.utf8();
	return int(EOS_Platform_SetOverrideCountryCode(platform_handle, cs.get_data()));
}

int EpicServices::platform_interface_set_application_status(int p_status) {
	if (!platform_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	return int(EOS_Platform_SetApplicationStatus(platform_handle, EOS_EApplicationStatus(p_status)));
}

int EpicServices::platform_interface_get_application_status() {
	if (!platform_handle) {
		return int(EOS_EApplicationStatus::EOS_AS_Foreground);
	}
	return int(EOS_Platform_GetApplicationStatus(platform_handle));
}

int EpicServices::platform_interface_set_network_status(int p_status) {
	if (!platform_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	return int(EOS_Platform_SetNetworkStatus(platform_handle, EOS_ENetworkStatus(p_status)));
}

int EpicServices::platform_interface_get_network_status() {
	if (!platform_handle) {
		return int(EOS_ENetworkStatus::EOS_NS_Disabled);
	}
	return int(EOS_Platform_GetNetworkStatus(platform_handle));
}

int EpicServices::platform_interface_get_desktop_crossplay_status() {
	if (!platform_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_Platform_GetDesktopCrossplayStatusOptions options = {};
	options.ApiVersion = EOS_PLATFORM_GETDESKTOPCROSSPLAYSTATUS_API_LATEST;
	EOS_Platform_DesktopCrossplayStatusInfo info = {};
	EOS_EResult r = EOS_Platform_GetDesktopCrossplayStatus(platform_handle, &options, &info);
	if (r != EOS_EResult::EOS_Success) {
		return -1;
	}
	return int(info.Status);
}

#ifdef TOOLS_ENABLED
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "core/project_settings.h"

// Embed the .gdap content directly — avoids dependency on source file location at runtime.
static const char *_eos_gdap_content =
		"[config]\n\n"
		"name=\"EpicServices\"\n"
		"binary_type=\"local\"\n"
		"binary=\"eossdk-StaticSTDC-release.aar\"\n\n"
		"[dependencies]\n\n"
		"local=[]\n"
		"remote=[]\n"
		"custom_maven_repos=[]\n";

Dictionary EpicServices::android_install_plugin(const String &p_eos_sdk_android_root) {
	Dictionary out;

	const String aar_src = p_eos_sdk_android_root.plus_file("Bin/Android/static-stdc++/aar/eossdk-StaticSTDC-release.aar");
	const String dst_dir = ProjectSettings::get_singleton()->globalize_path("res://android/plugins");
	const String dst_gdap = dst_dir.plus_file("EpicServices.gdap");
	const String dst_aar = dst_dir.plus_file("eossdk-StaticSTDC-release.aar");

	// Create destination directory.
	DirAccess *res_da = DirAccess::open("res://");
	if (!res_da) {
		out["error"] = "Cannot open res://";
		return out;
	}
	res_da->make_dir_recursive("android/plugins");
	memdelete(res_da);

	// Write .gdap from embedded content.
	FileAccess *f = FileAccess::open(dst_gdap, FileAccess::WRITE);
	if (!f) {
		out["error"] = "Cannot write: " + dst_gdap;
		return out;
	}
	f->store_string(String(_eos_gdap_content));
	memdelete(f);

	// Copy AAR from EOS SDK android root.
	DirAccess *fs = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (!fs) {
		out["error"] = "Cannot create filesystem DirAccess";
		return out;
	}
	if (!fs->file_exists(aar_src)) {
		out["error"] = "EOS AAR not found at: " + aar_src;
		memdelete(fs);
		return out;
	}
	Error err = fs->copy(aar_src, dst_aar);
	memdelete(fs);
	if (err != OK) {
		out["error"] = "Failed to copy AAR (error " + itos(err) + ")";
		return out;
	}

	out["ok"] = true;
	out["gdap"] = dst_gdap;
	out["aar"] = dst_aar;
	return out;
}
#endif
