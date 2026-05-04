/**************************************************************************/
/*  gd_epic_services.cpp                                                  */
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
#include "epic_continuance_token.h"
#include "epic_file_transfer_request.h"
#include "epic_lobby_details.h"
#include "epic_lobby_modification.h"
#include "epic_lobby_search.h"
#include "epic_presence_modification.h"
#include "epic_session_details.h"
#include "epic_session_modification.h"
#include "epic_session_search.h"
#include "epic_utils.h"

#include "core/engine.h"
#include "core/os/main_loop.h"
#include "core/os/os.h"

#include "eos_init.h"
#include "eos_logging.h"
#include "eos_version.h"

EpicServices *EpicServices::singleton = nullptr;
EpicServices *EpicServices::get_singleton() { return singleton; }

void EpicServices::_emit_deferred(const StringName &p_signal, const Dictionary &p_payload) {
	epic_emit_deferred(get_instance_id(), p_signal, p_payload);
}

// Object has no per-frame notification — Node does. To keep EpicServices a
// plain singleton, ticking is caller-driven: GDScript invokes
// EpicServices.platform_interface_tick() each frame from a Node._process()
// (a one-line autoload is enough). The reference upstream binding
// (3ddelano) follows the same pattern.
void EpicServices::_ensure_processing() { processing = true; }
void EpicServices::_stop_processing() { processing = false; }

void EpicServices::_notification(int p_what) {
	if (p_what == NOTIFICATION_PREDELETE) {
		platform_interface_release();
	}
}

Dictionary EpicServices::get_sdk_version() const {
	Dictionary out;
	const char *v = EOS_GetVersion();
	out["string"] = String::utf8(v ? v : "");
	out["major"] = EOS_MAJOR_VERSION;
	out["minor"] = EOS_MINOR_VERSION;
	out["patch"] = EOS_PATCH_VERSION;
	return out;
}

String EpicServices::result_to_string(int p_result_code) const {
	const char *s = EOS_EResult_ToString(static_cast<EOS_EResult>(p_result_code));
	return String::utf8(s ? s : "");
}

bool EpicServices::is_operation_complete(int p_result_code) const {
	return EOS_EResult_IsOperationComplete(static_cast<EOS_EResult>(p_result_code)) == EOS_TRUE;
}

void EpicServices::_bind_methods() {
	// Core ---------------------------------------------------------------
	ClassDB::bind_method(D_METHOD("get_sdk_version"), &EpicServices::get_sdk_version);
	ClassDB::bind_method(D_METHOD("is_initialized"), &EpicServices::is_initialized);
	ClassDB::bind_method(D_METHOD("is_platform_created"), &EpicServices::is_platform_created);
	ClassDB::bind_method(D_METHOD("result_to_string", "result_code"), &EpicServices::result_to_string);
	ClassDB::bind_method(D_METHOD("is_operation_complete", "result_code"), &EpicServices::is_operation_complete);

	// Platform -----------------------------------------------------------
	ClassDB::bind_method(D_METHOD("platform_interface_initialize", "options"), &EpicServices::platform_interface_initialize);
	ClassDB::bind_method(D_METHOD("platform_interface_create", "options"), &EpicServices::platform_interface_create);
	ClassDB::bind_method(D_METHOD("platform_interface_release"), &EpicServices::platform_interface_release);
	ClassDB::bind_method(D_METHOD("platform_interface_tick"), &EpicServices::platform_interface_tick);
	ClassDB::bind_method(D_METHOD("platform_interface_check_for_launcher_and_restart"), &EpicServices::platform_interface_check_for_launcher_and_restart);
	ClassDB::bind_method(D_METHOD("platform_interface_get_active_locale_code", "local_user_id"), &EpicServices::platform_interface_get_active_locale_code);
	ClassDB::bind_method(D_METHOD("platform_interface_get_active_country_code", "local_user_id"), &EpicServices::platform_interface_get_active_country_code);
	ClassDB::bind_method(D_METHOD("platform_interface_get_override_locale_code"), &EpicServices::platform_interface_get_override_locale_code);
	ClassDB::bind_method(D_METHOD("platform_interface_get_override_country_code"), &EpicServices::platform_interface_get_override_country_code);
	ClassDB::bind_method(D_METHOD("platform_interface_set_override_locale_code", "locale"), &EpicServices::platform_interface_set_override_locale_code);
	ClassDB::bind_method(D_METHOD("platform_interface_set_override_country_code", "country"), &EpicServices::platform_interface_set_override_country_code);
	ClassDB::bind_method(D_METHOD("platform_interface_set_application_status", "status"), &EpicServices::platform_interface_set_application_status);
	ClassDB::bind_method(D_METHOD("platform_interface_get_application_status"), &EpicServices::platform_interface_get_application_status);
	ClassDB::bind_method(D_METHOD("platform_interface_set_network_status", "status"), &EpicServices::platform_interface_set_network_status);
	ClassDB::bind_method(D_METHOD("platform_interface_get_network_status"), &EpicServices::platform_interface_get_network_status);
	ClassDB::bind_method(D_METHOD("platform_interface_get_desktop_crossplay_status"), &EpicServices::platform_interface_get_desktop_crossplay_status);
#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("android_install_plugin", "eos_sdk_android_root"), &EpicServices::android_install_plugin);
#endif

	// Logging ------------------------------------------------------------
	ClassDB::bind_method(D_METHOD("logging_interface_set_callback"), &EpicServices::logging_interface_set_callback);
	ClassDB::bind_method(D_METHOD("logging_interface_set_log_level", "category", "level"), &EpicServices::logging_interface_set_log_level);
	ADD_SIGNAL(MethodInfo("log_message_received",
			PropertyInfo(Variant::INT, "category"),
			PropertyInfo(Variant::INT, "level"),
			PropertyInfo(Variant::STRING, "category_name"),
			PropertyInfo(Variant::STRING, "message")));

	// Auth ---------------------------------------------------------------
	ClassDB::bind_method(D_METHOD("auth_interface_login", "options"), &EpicServices::auth_interface_login);
	ClassDB::bind_method(D_METHOD("auth_interface_logout", "options"), &EpicServices::auth_interface_logout);
	ClassDB::bind_method(D_METHOD("auth_interface_link_account", "options"), &EpicServices::auth_interface_link_account);
	ClassDB::bind_method(D_METHOD("auth_interface_delete_persistent_auth", "options"), &EpicServices::auth_interface_delete_persistent_auth);
	ClassDB::bind_method(D_METHOD("auth_interface_verify_user_auth", "options"), &EpicServices::auth_interface_verify_user_auth);
	ClassDB::bind_method(D_METHOD("auth_interface_query_id_token", "options"), &EpicServices::auth_interface_query_id_token);
	ClassDB::bind_method(D_METHOD("auth_interface_verify_id_token", "options"), &EpicServices::auth_interface_verify_id_token);
	ClassDB::bind_method(D_METHOD("auth_interface_copy_user_auth_token", "options"), &EpicServices::auth_interface_copy_user_auth_token);
	ClassDB::bind_method(D_METHOD("auth_interface_copy_id_token", "options"), &EpicServices::auth_interface_copy_id_token);
	ClassDB::bind_method(D_METHOD("auth_interface_get_logged_in_accounts_count"), &EpicServices::auth_interface_get_logged_in_accounts_count);
	ClassDB::bind_method(D_METHOD("auth_interface_get_logged_in_account_by_index", "index"), &EpicServices::auth_interface_get_logged_in_account_by_index);
	ClassDB::bind_method(D_METHOD("auth_interface_get_login_status", "local_user_id"), &EpicServices::auth_interface_get_login_status);
	ClassDB::bind_method(D_METHOD("auth_interface_get_selected_account_id", "local_user_id"), &EpicServices::auth_interface_get_selected_account_id);
	ClassDB::bind_method(D_METHOD("auth_interface_get_merged_accounts_count", "local_user_id"), &EpicServices::auth_interface_get_merged_accounts_count);
	ClassDB::bind_method(D_METHOD("auth_interface_get_merged_account_by_index", "local_user_id", "index"), &EpicServices::auth_interface_get_merged_account_by_index);
	ClassDB::bind_method(D_METHOD("auth_interface_add_notify_login_status_changed"), &EpicServices::auth_interface_add_notify_login_status_changed);
	ClassDB::bind_method(D_METHOD("auth_interface_remove_notify_login_status_changed", "id"), &EpicServices::auth_interface_remove_notify_login_status_changed);
	ADD_SIGNAL(MethodInfo("auth_interface_login_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("auth_interface_logout_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("auth_interface_link_account_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("auth_interface_delete_persistent_auth_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("auth_interface_verify_user_auth_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("auth_interface_query_id_token_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("auth_interface_verify_id_token_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("auth_interface_login_status_changed", PropertyInfo(Variant::DICTIONARY, "data")));

	// Connect ------------------------------------------------------------
	ClassDB::bind_method(D_METHOD("connect_interface_login", "options"), &EpicServices::connect_interface_login);
	ClassDB::bind_method(D_METHOD("connect_interface_logout", "options"), &EpicServices::connect_interface_logout);
	ClassDB::bind_method(D_METHOD("connect_interface_create_user", "options"), &EpicServices::connect_interface_create_user);
	ClassDB::bind_method(D_METHOD("connect_interface_link_account", "options"), &EpicServices::connect_interface_link_account);
	ClassDB::bind_method(D_METHOD("connect_interface_unlink_account", "options"), &EpicServices::connect_interface_unlink_account);
	ClassDB::bind_method(D_METHOD("connect_interface_create_device_id", "options"), &EpicServices::connect_interface_create_device_id);
	ClassDB::bind_method(D_METHOD("connect_interface_delete_device_id", "options"), &EpicServices::connect_interface_delete_device_id);
	ClassDB::bind_method(D_METHOD("connect_interface_query_external_account_mappings", "options"), &EpicServices::connect_interface_query_external_account_mappings);
	ClassDB::bind_method(D_METHOD("connect_interface_query_product_user_id_mappings", "options"), &EpicServices::connect_interface_query_product_user_id_mappings);
	ClassDB::bind_method(D_METHOD("connect_interface_get_external_account_mapping", "options"), &EpicServices::connect_interface_get_external_account_mapping);
	ClassDB::bind_method(D_METHOD("connect_interface_get_product_user_id_mapping", "options"), &EpicServices::connect_interface_get_product_user_id_mapping);
	ClassDB::bind_method(D_METHOD("connect_interface_get_logged_in_users_count"), &EpicServices::connect_interface_get_logged_in_users_count);
	ClassDB::bind_method(D_METHOD("connect_interface_get_logged_in_user_by_index", "index"), &EpicServices::connect_interface_get_logged_in_user_by_index);
	ClassDB::bind_method(D_METHOD("connect_interface_get_login_status", "product_user_id"), &EpicServices::connect_interface_get_login_status);
	ClassDB::bind_method(D_METHOD("connect_interface_copy_id_token", "options"), &EpicServices::connect_interface_copy_id_token);
	ClassDB::bind_method(D_METHOD("connect_interface_verify_id_token", "options"), &EpicServices::connect_interface_verify_id_token);
	ClassDB::bind_method(D_METHOD("connect_interface_add_notify_login_status_changed"), &EpicServices::connect_interface_add_notify_login_status_changed);
	ClassDB::bind_method(D_METHOD("connect_interface_remove_notify_login_status_changed", "id"), &EpicServices::connect_interface_remove_notify_login_status_changed);
	ClassDB::bind_method(D_METHOD("connect_interface_add_notify_auth_expiration"), &EpicServices::connect_interface_add_notify_auth_expiration);
	ClassDB::bind_method(D_METHOD("connect_interface_remove_notify_auth_expiration", "id"), &EpicServices::connect_interface_remove_notify_auth_expiration);
	ADD_SIGNAL(MethodInfo("connect_interface_login_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_logout_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_create_user_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_link_account_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_unlink_account_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_create_device_id_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_delete_device_id_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_query_external_account_mappings_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_query_product_user_id_mappings_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_verify_id_token_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_login_status_changed", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("connect_interface_auth_expiration", PropertyInfo(Variant::DICTIONARY, "data")));

	// Lobby --------------------------------------------------------------
	ClassDB::bind_method(D_METHOD("lobby_interface_create_lobby", "options"), &EpicServices::lobby_interface_create_lobby);
	ClassDB::bind_method(D_METHOD("lobby_interface_destroy_lobby", "options"), &EpicServices::lobby_interface_destroy_lobby);
	ClassDB::bind_method(D_METHOD("lobby_interface_join_lobby", "options"), &EpicServices::lobby_interface_join_lobby);
	ClassDB::bind_method(D_METHOD("lobby_interface_join_lobby_by_id", "options"), &EpicServices::lobby_interface_join_lobby_by_id);
	ClassDB::bind_method(D_METHOD("lobby_interface_leave_lobby", "options"), &EpicServices::lobby_interface_leave_lobby);
	ClassDB::bind_method(D_METHOD("lobby_interface_update_lobby", "options"), &EpicServices::lobby_interface_update_lobby);
	ClassDB::bind_method(D_METHOD("lobby_interface_update_lobby_modification", "options"), &EpicServices::lobby_interface_update_lobby_modification);
	ClassDB::bind_method(D_METHOD("lobby_interface_copy_lobby_details_handle", "options"), &EpicServices::lobby_interface_copy_lobby_details_handle);
	ClassDB::bind_method(D_METHOD("lobby_interface_copy_lobby_details_handle_by_invite_id", "options"), &EpicServices::lobby_interface_copy_lobby_details_handle_by_invite_id);
	ClassDB::bind_method(D_METHOD("lobby_interface_create_lobby_search", "max_results"), &EpicServices::lobby_interface_create_lobby_search);
	ClassDB::bind_method(D_METHOD("lobby_interface_send_invite", "options"), &EpicServices::lobby_interface_send_invite);
	ClassDB::bind_method(D_METHOD("lobby_interface_reject_invite", "options"), &EpicServices::lobby_interface_reject_invite);
	ClassDB::bind_method(D_METHOD("lobby_interface_get_invite_count", "options"), &EpicServices::lobby_interface_get_invite_count);
	ClassDB::bind_method(D_METHOD("lobby_interface_get_invite_id_by_index", "options"), &EpicServices::lobby_interface_get_invite_id_by_index);
	ClassDB::bind_method(D_METHOD("lobby_interface_get_rtc_room_name", "options"), &EpicServices::lobby_interface_get_rtc_room_name);
	ClassDB::bind_method(D_METHOD("lobby_interface_kick_member", "options"), &EpicServices::lobby_interface_kick_member);
	ClassDB::bind_method(D_METHOD("lobby_interface_promote_member", "options"), &EpicServices::lobby_interface_promote_member);
	ClassDB::bind_method(D_METHOD("lobby_interface_query_invites", "options"), &EpicServices::lobby_interface_query_invites);
	ClassDB::bind_method(D_METHOD("lobby_interface_add_notify_lobby_update_received"), &EpicServices::lobby_interface_add_notify_lobby_update_received);
	ClassDB::bind_method(D_METHOD("lobby_interface_remove_notify_lobby_update_received", "id"), &EpicServices::lobby_interface_remove_notify_lobby_update_received);
	ClassDB::bind_method(D_METHOD("lobby_interface_add_notify_lobby_member_update_received"), &EpicServices::lobby_interface_add_notify_lobby_member_update_received);
	ClassDB::bind_method(D_METHOD("lobby_interface_remove_notify_lobby_member_update_received", "id"), &EpicServices::lobby_interface_remove_notify_lobby_member_update_received);
	ClassDB::bind_method(D_METHOD("lobby_interface_add_notify_lobby_member_status_received"), &EpicServices::lobby_interface_add_notify_lobby_member_status_received);
	ClassDB::bind_method(D_METHOD("lobby_interface_remove_notify_lobby_member_status_received", "id"), &EpicServices::lobby_interface_remove_notify_lobby_member_status_received);
	ClassDB::bind_method(D_METHOD("lobby_interface_add_notify_lobby_invite_received"), &EpicServices::lobby_interface_add_notify_lobby_invite_received);
	ClassDB::bind_method(D_METHOD("lobby_interface_remove_notify_lobby_invite_received", "id"), &EpicServices::lobby_interface_remove_notify_lobby_invite_received);
	ClassDB::bind_method(D_METHOD("lobby_interface_add_notify_lobby_invite_accepted"), &EpicServices::lobby_interface_add_notify_lobby_invite_accepted);
	ClassDB::bind_method(D_METHOD("lobby_interface_remove_notify_lobby_invite_accepted", "id"), &EpicServices::lobby_interface_remove_notify_lobby_invite_accepted);
	ADD_SIGNAL(MethodInfo("lobby_interface_create_lobby_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_destroy_lobby_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_join_lobby_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_join_lobby_by_id_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_leave_lobby_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_update_lobby_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_send_invite_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_kick_member_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_promote_member_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_query_invites_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_lobby_update_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_lobby_member_update_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_lobby_member_status_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_lobby_invite_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_interface_lobby_invite_accepted", PropertyInfo(Variant::DICTIONARY, "data")));

	// Sessions -----------------------------------------------------------
	ClassDB::bind_method(D_METHOD("sessions_interface_create_session_modification", "options"), &EpicServices::sessions_interface_create_session_modification);
	ClassDB::bind_method(D_METHOD("sessions_interface_update_session_modification", "options"), &EpicServices::sessions_interface_update_session_modification);
	ClassDB::bind_method(D_METHOD("sessions_interface_update_session", "options"), &EpicServices::sessions_interface_update_session);
	ClassDB::bind_method(D_METHOD("sessions_interface_destroy_session", "options"), &EpicServices::sessions_interface_destroy_session);
	ClassDB::bind_method(D_METHOD("sessions_interface_join_session", "options"), &EpicServices::sessions_interface_join_session);
	ClassDB::bind_method(D_METHOD("sessions_interface_start_session", "options"), &EpicServices::sessions_interface_start_session);
	ClassDB::bind_method(D_METHOD("sessions_interface_end_session", "options"), &EpicServices::sessions_interface_end_session);
	ClassDB::bind_method(D_METHOD("sessions_interface_register_players", "options"), &EpicServices::sessions_interface_register_players);
	ClassDB::bind_method(D_METHOD("sessions_interface_unregister_players", "options"), &EpicServices::sessions_interface_unregister_players);
	ClassDB::bind_method(D_METHOD("sessions_interface_send_invite", "options"), &EpicServices::sessions_interface_send_invite);
	ClassDB::bind_method(D_METHOD("sessions_interface_reject_invite", "options"), &EpicServices::sessions_interface_reject_invite);
	ClassDB::bind_method(D_METHOD("sessions_interface_get_invite_count", "options"), &EpicServices::sessions_interface_get_invite_count);
	ClassDB::bind_method(D_METHOD("sessions_interface_get_invite_id_by_index", "options"), &EpicServices::sessions_interface_get_invite_id_by_index);
	ClassDB::bind_method(D_METHOD("sessions_interface_create_session_search", "max_results"), &EpicServices::sessions_interface_create_session_search);
	ClassDB::bind_method(D_METHOD("sessions_interface_copy_active_session_handle", "options"), &EpicServices::sessions_interface_copy_active_session_handle);
	ClassDB::bind_method(D_METHOD("sessions_interface_add_notify_session_invite_received"), &EpicServices::sessions_interface_add_notify_session_invite_received);
	ClassDB::bind_method(D_METHOD("sessions_interface_remove_notify_session_invite_received", "id"), &EpicServices::sessions_interface_remove_notify_session_invite_received);
	ClassDB::bind_method(D_METHOD("sessions_interface_add_notify_session_invite_accepted"), &EpicServices::sessions_interface_add_notify_session_invite_accepted);
	ClassDB::bind_method(D_METHOD("sessions_interface_remove_notify_session_invite_accepted", "id"), &EpicServices::sessions_interface_remove_notify_session_invite_accepted);
	ClassDB::bind_method(D_METHOD("sessions_interface_add_notify_join_session_accepted"), &EpicServices::sessions_interface_add_notify_join_session_accepted);
	ClassDB::bind_method(D_METHOD("sessions_interface_remove_notify_join_session_accepted", "id"), &EpicServices::sessions_interface_remove_notify_join_session_accepted);
	ADD_SIGNAL(MethodInfo("sessions_interface_update_session_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_destroy_session_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_join_session_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_start_session_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_end_session_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_register_players_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_unregister_players_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_send_invite_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_session_invite_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_session_invite_accepted", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("sessions_interface_join_session_accepted", PropertyInfo(Variant::DICTIONARY, "data")));

	// P2P ----------------------------------------------------------------
	ClassDB::bind_method(D_METHOD("p2p_interface_send_packet", "options"), &EpicServices::p2p_interface_send_packet);
	ClassDB::bind_method(D_METHOD("p2p_interface_get_next_received_packet_size", "options"), &EpicServices::p2p_interface_get_next_received_packet_size);
	ClassDB::bind_method(D_METHOD("p2p_interface_receive_packet", "options"), &EpicServices::p2p_interface_receive_packet);
	ClassDB::bind_method(D_METHOD("p2p_interface_accept_connection", "options"), &EpicServices::p2p_interface_accept_connection);
	ClassDB::bind_method(D_METHOD("p2p_interface_close_connection", "options"), &EpicServices::p2p_interface_close_connection);
	ClassDB::bind_method(D_METHOD("p2p_interface_close_connections", "options"), &EpicServices::p2p_interface_close_connections);
	ClassDB::bind_method(D_METHOD("p2p_interface_query_nat_type"), &EpicServices::p2p_interface_query_nat_type);
	ClassDB::bind_method(D_METHOD("p2p_interface_get_nat_type"), &EpicServices::p2p_interface_get_nat_type);
	ClassDB::bind_method(D_METHOD("p2p_interface_set_relay_control", "relay_control"), &EpicServices::p2p_interface_set_relay_control);
	ClassDB::bind_method(D_METHOD("p2p_interface_get_relay_control"), &EpicServices::p2p_interface_get_relay_control);
	ClassDB::bind_method(D_METHOD("p2p_interface_set_port_range", "port", "max_additional_ports_to_try"), &EpicServices::p2p_interface_set_port_range);
	ClassDB::bind_method(D_METHOD("p2p_interface_get_port_range"), &EpicServices::p2p_interface_get_port_range);
	ClassDB::bind_method(D_METHOD("p2p_interface_set_packet_queue_size", "max_incoming_bytes", "max_outgoing_bytes"), &EpicServices::p2p_interface_set_packet_queue_size);
	ClassDB::bind_method(D_METHOD("p2p_interface_get_packet_queue_info"), &EpicServices::p2p_interface_get_packet_queue_info);
	ClassDB::bind_method(D_METHOD("p2p_interface_clear_packet_queue", "options"), &EpicServices::p2p_interface_clear_packet_queue);
	ClassDB::bind_method(D_METHOD("p2p_interface_add_notify_peer_connection_request", "socket_name"), &EpicServices::p2p_interface_add_notify_peer_connection_request);
	ClassDB::bind_method(D_METHOD("p2p_interface_remove_notify_peer_connection_request", "id"), &EpicServices::p2p_interface_remove_notify_peer_connection_request);
	ClassDB::bind_method(D_METHOD("p2p_interface_add_notify_peer_connection_established", "socket_name"), &EpicServices::p2p_interface_add_notify_peer_connection_established);
	ClassDB::bind_method(D_METHOD("p2p_interface_remove_notify_peer_connection_established", "id"), &EpicServices::p2p_interface_remove_notify_peer_connection_established);
	ClassDB::bind_method(D_METHOD("p2p_interface_add_notify_peer_connection_interrupted", "socket_name"), &EpicServices::p2p_interface_add_notify_peer_connection_interrupted);
	ClassDB::bind_method(D_METHOD("p2p_interface_remove_notify_peer_connection_interrupted", "id"), &EpicServices::p2p_interface_remove_notify_peer_connection_interrupted);
	ClassDB::bind_method(D_METHOD("p2p_interface_add_notify_peer_connection_closed", "socket_name"), &EpicServices::p2p_interface_add_notify_peer_connection_closed);
	ClassDB::bind_method(D_METHOD("p2p_interface_remove_notify_peer_connection_closed", "id"), &EpicServices::p2p_interface_remove_notify_peer_connection_closed);
	ADD_SIGNAL(MethodInfo("p2p_interface_query_nat_type_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("p2p_interface_peer_connection_request", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("p2p_interface_peer_connection_established", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("p2p_interface_peer_connection_interrupted", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("p2p_interface_peer_connection_closed", PropertyInfo(Variant::DICTIONARY, "data")));

	// CustomInvites ------------------------------------------------------
	ClassDB::bind_method(D_METHOD("custom_invites_interface_set_custom_invite", "options"), &EpicServices::custom_invites_interface_set_custom_invite);
	ClassDB::bind_method(D_METHOD("custom_invites_interface_send_custom_invite", "options"), &EpicServices::custom_invites_interface_send_custom_invite);
	ClassDB::bind_method(D_METHOD("custom_invites_interface_finalize_invite", "options"), &EpicServices::custom_invites_interface_finalize_invite);
	ClassDB::bind_method(D_METHOD("custom_invites_interface_add_notify_custom_invite_received"), &EpicServices::custom_invites_interface_add_notify_custom_invite_received);
	ClassDB::bind_method(D_METHOD("custom_invites_interface_remove_notify_custom_invite_received", "id"), &EpicServices::custom_invites_interface_remove_notify_custom_invite_received);
	ClassDB::bind_method(D_METHOD("custom_invites_interface_add_notify_custom_invite_accepted"), &EpicServices::custom_invites_interface_add_notify_custom_invite_accepted);
	ClassDB::bind_method(D_METHOD("custom_invites_interface_remove_notify_custom_invite_accepted", "id"), &EpicServices::custom_invites_interface_remove_notify_custom_invite_accepted);
	ClassDB::bind_method(D_METHOD("custom_invites_interface_add_notify_custom_invite_rejected"), &EpicServices::custom_invites_interface_add_notify_custom_invite_rejected);
	ClassDB::bind_method(D_METHOD("custom_invites_interface_remove_notify_custom_invite_rejected", "id"), &EpicServices::custom_invites_interface_remove_notify_custom_invite_rejected);
	ADD_SIGNAL(MethodInfo("custom_invites_interface_send_custom_invite_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("custom_invites_interface_custom_invite_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("custom_invites_interface_custom_invite_accepted", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("custom_invites_interface_custom_invite_rejected", PropertyInfo(Variant::DICTIONARY, "data")));

	// Stats --------------------------------------------------------------
	ClassDB::bind_method(D_METHOD("stats_interface_ingest_stat", "options"), &EpicServices::stats_interface_ingest_stat);
	ClassDB::bind_method(D_METHOD("stats_interface_query_stats", "options"), &EpicServices::stats_interface_query_stats);
	ClassDB::bind_method(D_METHOD("stats_interface_get_stats_count", "options"), &EpicServices::stats_interface_get_stats_count);
	ClassDB::bind_method(D_METHOD("stats_interface_copy_stat_by_index", "options"), &EpicServices::stats_interface_copy_stat_by_index);
	ClassDB::bind_method(D_METHOD("stats_interface_copy_stat_by_name", "options"), &EpicServices::stats_interface_copy_stat_by_name);
	ADD_SIGNAL(MethodInfo("stats_interface_ingest_stat_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("stats_interface_query_stats_callback", PropertyInfo(Variant::DICTIONARY, "data")));

	// Leaderboards -------------------------------------------------------
	ClassDB::bind_method(D_METHOD("leaderboards_interface_query_leaderboard_definitions", "options"), &EpicServices::leaderboards_interface_query_leaderboard_definitions);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_get_leaderboard_definition_count"), &EpicServices::leaderboards_interface_get_leaderboard_definition_count);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_copy_leaderboard_definition_by_index", "index"), &EpicServices::leaderboards_interface_copy_leaderboard_definition_by_index);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_copy_leaderboard_definition_by_id", "leaderboard_id"), &EpicServices::leaderboards_interface_copy_leaderboard_definition_by_id);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_query_leaderboard_ranks", "options"), &EpicServices::leaderboards_interface_query_leaderboard_ranks);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_get_leaderboard_record_count"), &EpicServices::leaderboards_interface_get_leaderboard_record_count);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_copy_leaderboard_record_by_index", "index"), &EpicServices::leaderboards_interface_copy_leaderboard_record_by_index);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_copy_leaderboard_record_by_user_id", "user_id"), &EpicServices::leaderboards_interface_copy_leaderboard_record_by_user_id);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_query_leaderboard_user_scores", "options"), &EpicServices::leaderboards_interface_query_leaderboard_user_scores);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_get_leaderboard_user_score_count", "stat_name"), &EpicServices::leaderboards_interface_get_leaderboard_user_score_count);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_copy_leaderboard_user_score_by_index", "options"), &EpicServices::leaderboards_interface_copy_leaderboard_user_score_by_index);
	ClassDB::bind_method(D_METHOD("leaderboards_interface_copy_leaderboard_user_score_by_user_id", "options"), &EpicServices::leaderboards_interface_copy_leaderboard_user_score_by_user_id);
	ADD_SIGNAL(MethodInfo("leaderboards_interface_query_leaderboard_definitions_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("leaderboards_interface_query_leaderboard_ranks_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("leaderboards_interface_query_leaderboard_user_scores_callback", PropertyInfo(Variant::DICTIONARY, "data")));

	// Achievements -------------------------------------------------------
	ClassDB::bind_method(D_METHOD("achievements_interface_query_definitions", "options"), &EpicServices::achievements_interface_query_definitions);
	ClassDB::bind_method(D_METHOD("achievements_interface_get_achievement_definition_count"), &EpicServices::achievements_interface_get_achievement_definition_count);
	ClassDB::bind_method(D_METHOD("achievements_interface_copy_achievement_definition_by_index", "index"), &EpicServices::achievements_interface_copy_achievement_definition_by_index);
	ClassDB::bind_method(D_METHOD("achievements_interface_copy_achievement_definition_by_id", "id"), &EpicServices::achievements_interface_copy_achievement_definition_by_id);
	ClassDB::bind_method(D_METHOD("achievements_interface_query_player_achievements", "options"), &EpicServices::achievements_interface_query_player_achievements);
	ClassDB::bind_method(D_METHOD("achievements_interface_get_player_achievement_count", "target_user_id"), &EpicServices::achievements_interface_get_player_achievement_count);
	ClassDB::bind_method(D_METHOD("achievements_interface_copy_player_achievement_by_index", "options"), &EpicServices::achievements_interface_copy_player_achievement_by_index);
	ClassDB::bind_method(D_METHOD("achievements_interface_copy_player_achievement_by_id", "options"), &EpicServices::achievements_interface_copy_player_achievement_by_id);
	ClassDB::bind_method(D_METHOD("achievements_interface_unlock_achievements", "options"), &EpicServices::achievements_interface_unlock_achievements);
	ClassDB::bind_method(D_METHOD("achievements_interface_add_notify_achievements_unlocked"), &EpicServices::achievements_interface_add_notify_achievements_unlocked);
	ClassDB::bind_method(D_METHOD("achievements_interface_remove_notify_achievements_unlocked", "id"), &EpicServices::achievements_interface_remove_notify_achievements_unlocked);
	ADD_SIGNAL(MethodInfo("achievements_interface_query_definitions_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("achievements_interface_query_player_achievements_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("achievements_interface_unlock_achievements_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("achievements_interface_achievement_unlocked", PropertyInfo(Variant::DICTIONARY, "data")));

	// Friends ------------------------------------------------------------
	ClassDB::bind_method(D_METHOD("friends_interface_query_friends", "options"), &EpicServices::friends_interface_query_friends);
	ClassDB::bind_method(D_METHOD("friends_interface_get_friends_count", "local_user_id"), &EpicServices::friends_interface_get_friends_count);
	ClassDB::bind_method(D_METHOD("friends_interface_get_friend_at_index", "local_user_id", "index"), &EpicServices::friends_interface_get_friend_at_index);
	ClassDB::bind_method(D_METHOD("friends_interface_get_status", "local_user_id", "target_user_id"), &EpicServices::friends_interface_get_status);
	ClassDB::bind_method(D_METHOD("friends_interface_get_blocked_users_count", "local_user_id"), &EpicServices::friends_interface_get_blocked_users_count);
	ClassDB::bind_method(D_METHOD("friends_interface_get_blocked_user_at_index", "local_user_id", "index"), &EpicServices::friends_interface_get_blocked_user_at_index);
	ClassDB::bind_method(D_METHOD("friends_interface_add_notify_friends_update"), &EpicServices::friends_interface_add_notify_friends_update);
	ClassDB::bind_method(D_METHOD("friends_interface_remove_notify_friends_update", "id"), &EpicServices::friends_interface_remove_notify_friends_update);
	ClassDB::bind_method(D_METHOD("friends_interface_add_notify_blocked_users_update"), &EpicServices::friends_interface_add_notify_blocked_users_update);
	ClassDB::bind_method(D_METHOD("friends_interface_remove_notify_blocked_users_update", "id"), &EpicServices::friends_interface_remove_notify_blocked_users_update);
	ADD_SIGNAL(MethodInfo("friends_interface_query_friends_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("friends_interface_friends_update", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("friends_interface_blocked_users_update", PropertyInfo(Variant::DICTIONARY, "data")));

	// Presence -----------------------------------------------------------
	ClassDB::bind_method(D_METHOD("presence_interface_query_presence", "options"), &EpicServices::presence_interface_query_presence);
	ClassDB::bind_method(D_METHOD("presence_interface_has_presence", "options"), &EpicServices::presence_interface_has_presence);
	ClassDB::bind_method(D_METHOD("presence_interface_copy_presence", "options"), &EpicServices::presence_interface_copy_presence);
	ClassDB::bind_method(D_METHOD("presence_interface_create_presence_modification", "local_user_id"), &EpicServices::presence_interface_create_presence_modification);
	ClassDB::bind_method(D_METHOD("presence_interface_set_presence", "options"), &EpicServices::presence_interface_set_presence);
	ClassDB::bind_method(D_METHOD("presence_interface_get_join_info", "options"), &EpicServices::presence_interface_get_join_info);
	ClassDB::bind_method(D_METHOD("presence_interface_add_notify_on_presence_changed"), &EpicServices::presence_interface_add_notify_on_presence_changed);
	ClassDB::bind_method(D_METHOD("presence_interface_remove_notify_on_presence_changed", "id"), &EpicServices::presence_interface_remove_notify_on_presence_changed);
	ClassDB::bind_method(D_METHOD("presence_interface_add_notify_join_game_accepted"), &EpicServices::presence_interface_add_notify_join_game_accepted);
	ClassDB::bind_method(D_METHOD("presence_interface_remove_notify_join_game_accepted", "id"), &EpicServices::presence_interface_remove_notify_join_game_accepted);
	ADD_SIGNAL(MethodInfo("presence_interface_query_presence_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("presence_interface_set_presence_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("presence_interface_on_presence_changed", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("presence_interface_join_game_accepted", PropertyInfo(Variant::DICTIONARY, "data")));

	// UserInfo -----------------------------------------------------------
	ClassDB::bind_method(D_METHOD("user_info_interface_query_user_info", "options"), &EpicServices::user_info_interface_query_user_info);
	ClassDB::bind_method(D_METHOD("user_info_interface_query_user_info_by_display_name", "options"), &EpicServices::user_info_interface_query_user_info_by_display_name);
	ClassDB::bind_method(D_METHOD("user_info_interface_query_user_info_by_external_account", "options"), &EpicServices::user_info_interface_query_user_info_by_external_account);
	ClassDB::bind_method(D_METHOD("user_info_interface_copy_user_info", "options"), &EpicServices::user_info_interface_copy_user_info);
	ClassDB::bind_method(D_METHOD("user_info_interface_get_external_user_info_count", "options"), &EpicServices::user_info_interface_get_external_user_info_count);
	ClassDB::bind_method(D_METHOD("user_info_interface_copy_external_user_info_by_index", "options"), &EpicServices::user_info_interface_copy_external_user_info_by_index);
	ClassDB::bind_method(D_METHOD("user_info_interface_copy_best_display_name", "options"), &EpicServices::user_info_interface_copy_best_display_name);
	ClassDB::bind_method(D_METHOD("user_info_interface_get_local_platform_type"), &EpicServices::user_info_interface_get_local_platform_type);
	ADD_SIGNAL(MethodInfo("user_info_interface_query_user_info_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("user_info_interface_query_user_info_by_display_name_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("user_info_interface_query_user_info_by_external_account_callback", PropertyInfo(Variant::DICTIONARY, "data")));

	// PlayerDataStorage --------------------------------------------------
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_query_file", "options"), &EpicServices::playerdatastorage_interface_query_file);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_query_file_list", "options"), &EpicServices::playerdatastorage_interface_query_file_list);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_copy_file_metadata_by_filename", "options"), &EpicServices::playerdatastorage_interface_copy_file_metadata_by_filename);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_get_file_metadata_count", "options"), &EpicServices::playerdatastorage_interface_get_file_metadata_count);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_copy_file_metadata_at_index", "options"), &EpicServices::playerdatastorage_interface_copy_file_metadata_at_index);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_duplicate_file", "options"), &EpicServices::playerdatastorage_interface_duplicate_file);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_delete_file", "options"), &EpicServices::playerdatastorage_interface_delete_file);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_read_file", "options"), &EpicServices::playerdatastorage_interface_read_file);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_write_file", "options"), &EpicServices::playerdatastorage_interface_write_file);
	ClassDB::bind_method(D_METHOD("playerdatastorage_interface_delete_cache", "options"), &EpicServices::playerdatastorage_interface_delete_cache);
	ADD_SIGNAL(MethodInfo("playerdatastorage_interface_query_file_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("playerdatastorage_interface_query_file_list_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("playerdatastorage_interface_duplicate_file_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("playerdatastorage_interface_delete_file_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("playerdatastorage_interface_read_file_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("playerdatastorage_interface_write_file_callback", PropertyInfo(Variant::DICTIONARY, "data")));

	// TitleStorage -------------------------------------------------------
	ClassDB::bind_method(D_METHOD("titlestorage_interface_query_file", "options"), &EpicServices::titlestorage_interface_query_file);
	ClassDB::bind_method(D_METHOD("titlestorage_interface_query_file_list", "options"), &EpicServices::titlestorage_interface_query_file_list);
	ClassDB::bind_method(D_METHOD("titlestorage_interface_copy_file_metadata_by_filename", "options"), &EpicServices::titlestorage_interface_copy_file_metadata_by_filename);
	ClassDB::bind_method(D_METHOD("titlestorage_interface_get_file_metadata_count", "options"), &EpicServices::titlestorage_interface_get_file_metadata_count);
	ClassDB::bind_method(D_METHOD("titlestorage_interface_copy_file_metadata_at_index", "options"), &EpicServices::titlestorage_interface_copy_file_metadata_at_index);
	ClassDB::bind_method(D_METHOD("titlestorage_interface_read_file", "options"), &EpicServices::titlestorage_interface_read_file);
	ClassDB::bind_method(D_METHOD("titlestorage_interface_delete_cache", "options"), &EpicServices::titlestorage_interface_delete_cache);
	ADD_SIGNAL(MethodInfo("titlestorage_interface_query_file_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("titlestorage_interface_query_file_list_callback", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("titlestorage_interface_read_file_callback", PropertyInfo(Variant::DICTIONARY, "data")));
}

EpicServices::EpicServices() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "EpicServices singleton already exists");
	singleton = this;
}

EpicServices::~EpicServices() {
	platform_interface_release(); // idempotent — also handles SDK shutdown.
	if (singleton == this) {
		singleton = nullptr;
	}
}

#ifdef DOCTEST
#include "doctest/doctest.h"

// Tests for the EpicServices submodule. Inline here so the compilation
// unit's symbols (EpicServices ctor, etc.) keep the .o pinned in the
// final link. A standalone tests-only .cpp gets dropped by the linker
// because nothing references its symbols. Tests cover only logic that
// runs without a live EOS platform — Variant↔EOS conversions, Dict
// accessors, handle-wrapper null behaviour, and the singleton's sync
// helpers. Real EOS_* calls require a real platform handle and credentials
// and are exercised by the demo project at runtime.

TEST_CASE("[EpicUtils] eaid_to_string null returns empty") {
	CHECK(eos_eaid_to_string(nullptr).empty());
}

TEST_CASE("[EpicUtils] pui_to_string null returns empty") {
	CHECK(eos_pui_to_string(nullptr).empty());
}

TEST_CASE("[EpicUtils] continuance_token_to_string null returns empty") {
	CHECK(eos_continuance_token_to_string(nullptr).empty());
}

TEST_CASE("[EpicUtils] eaid_from_string returns null for empty input") {
	CHECK(eos_eaid_from_string(String()) == nullptr);
	// Real round-trip needs a live initialised SDK — exercised by the demo
	// project at runtime, not at test time.
}

TEST_CASE("[EpicUtils] pui_from_string returns null for empty input") {
	CHECK(eos_pui_from_string(String()) == nullptr);
}

TEST_CASE("[EpicUtils] dict_get_string default and lookup") {
	Dictionary d;
	CHECK(dict_get_string(d, "missing", "fallback") == "fallback");
	d["k"] = "value";
	CHECK(dict_get_string(d, "k") == "value");
}

TEST_CASE("[EpicUtils] dict_get_int / int64 / bool / double defaults and lookup") {
	Dictionary d;
	CHECK(dict_get_int(d, "missing", 42) == 42);
	d["x"] = 7;
	CHECK(dict_get_int(d, "x") == 7);
	CHECK(dict_get_int64(d, "missing", int64_t(1) << 40) == (int64_t(1) << 40));
	CHECK(dict_get_bool(d, "missing", true) == true);
	CHECK(dict_get_double(d, "missing", 3.14) == doctest::Approx(3.14));
}

TEST_CASE("[EpicContinuanceToken] null handle is invalid") {
	Ref<EpicContinuanceToken> t;
	t.instance();
	CHECK_FALSE(t->is_valid());
	CHECK(t->to_string_value().empty());
}

TEST_CASE("[EpicLobbyModification] null handle returns NotConfigured") {
	Ref<EpicLobbyModification> m;
	m.instance();
	CHECK_FALSE(m->is_valid());
	CHECK(m->set_max_members(8) == int(EOS_EResult::EOS_NotConfigured));
	CHECK(m->set_permission_level(0) == int(EOS_EResult::EOS_NotConfigured));
	CHECK(m->set_invites_allowed(true) == int(EOS_EResult::EOS_NotConfigured));
	CHECK(m->remove_attribute("k") == int(EOS_EResult::EOS_NotConfigured));
}

TEST_CASE("[EpicLobbySearch] null handle returns NotConfigured / empty") {
	Ref<EpicLobbySearch> s;
	s.instance();
	CHECK_FALSE(s->is_valid());
	CHECK(s->set_max_results(10) == int(EOS_EResult::EOS_NotConfigured));
	CHECK(s->get_search_result_count() == 0);
	Ref<EpicLobbyDetails> r = s->copy_search_result_by_index(0);
	CHECK(r.is_null());
}

TEST_CASE("[EpicSessionDetails] null handle copy_info reports NotFound") {
	Ref<EpicSessionDetails> d;
	d.instance();
	CHECK_FALSE(d->is_valid());
	Dictionary info = d->copy_info();
	CHECK(int(info["result_code"]) == int(EOS_EResult::EOS_NotFound));
	CHECK(d->get_session_attribute_count() == 0);
}

TEST_CASE("[EpicServices] singleton + version + helpers") {
	EpicServices *es = EpicServices::get_singleton();
	REQUIRE(es != nullptr);
	Dictionary v = es->get_sdk_version();
	CHECK(int(v["major"]) >= 1);
	CHECK(es->result_to_string(int(EOS_EResult::EOS_Success)) == String("EOS_Success"));
	CHECK(es->is_operation_complete(int(EOS_EResult::EOS_Success)) == true);
}

#endif // DOCTEST
