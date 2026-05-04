/**************************************************************************/
/*  gd_epic_services.h                                                    */
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

#ifndef GD_EPIC_SERVICES_H
#define GD_EPIC_SERVICES_H

#include "core/object.h"
#include "core/reference.h"

#include "eos_achievements_types.h"
#include "eos_auth_types.h"
#include "eos_common.h"
#include "eos_connect_types.h"
#include "eos_custominvites_types.h"
#include "eos_friends_types.h"
#include "eos_leaderboards_types.h"
#include "eos_lobby_types.h"
#include "eos_logging.h"
#include "eos_p2p_types.h"
#include "eos_playerdatastorage_types.h"
#include "eos_presence_types.h"
#include "eos_sdk.h"
#include "eos_sessions_types.h"
#include "eos_stats_types.h"
#include "eos_titlestorage_types.h"
#include "eos_types.h"
#include "eos_userinfo_types.h"

class EpicLobbyDetails;
class EpicLobbyModification;
class EpicLobbySearch;
class EpicSessionDetails;
class EpicSessionModification;
class EpicSessionSearch;
class EpicActiveSession;
class EpicPresenceModification;
class EpicPlayerDataStorageFileTransferRequest;
class EpicTitleStorageFileTransferRequest;

class EpicServices : public Object {
	GDCLASS(EpicServices, Object);

	static EpicServices *singleton;

	// SDK lifecycle state.
	bool sdk_initialized = false; // EOS_Initialize succeeded
	EOS_HPlatform platform_handle = nullptr;

	// Cached interface handles (lazily fetched from platform_handle).
	EOS_HAuth auth_handle = nullptr;
	EOS_HConnect connect_handle = nullptr;
	EOS_HLobby lobby_handle = nullptr;
	EOS_HSessions sessions_handle = nullptr;
	EOS_HP2P p2p_handle = nullptr;
	EOS_HCustomInvites custom_invites_handle = nullptr;
	EOS_HStats stats_handle = nullptr;
	EOS_HLeaderboards leaderboards_handle = nullptr;
	EOS_HAchievements achievements_handle = nullptr;
	EOS_HFriends friends_handle = nullptr;
	EOS_HPresence presence_handle = nullptr;
	EOS_HUserInfo user_info_handle = nullptr;
	EOS_HPlayerDataStorage player_data_storage_handle = nullptr;
	EOS_HTitleStorage title_storage_handle = nullptr;

public:
	EOS_HPlatform _get_platform_handle() const { return platform_handle; }
	EOS_HLobby _get_lobby_handle() const { return lobby_handle; }
	EOS_HSessions _get_sessions_handle() const { return sessions_handle; }
	EOS_HP2P _get_p2p_handle() const { return p2p_handle; }
	EOS_HCustomInvites _get_custom_invites_handle() const { return custom_invites_handle; }

private:
	// Periodic EOS_Platform_Tick driven from NOTIFICATION_INTERNAL_PROCESS.
	bool processing = false;

	void _ensure_processing();
	void _stop_processing();

	// Callback dispatch — used by static C trampolines in *_interface.cpp to
	// emit a signal on this singleton on the main thread regardless of
	// which EOS internal thread fired the callback.
	void _emit_deferred(const StringName &p_signal, const Dictionary &p_payload);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	static EpicServices *get_singleton();

	// ---- core / lifecycle (gd_epic_services.cpp) -----------------------
	Dictionary get_sdk_version() const; // sdk version dictionary {string,major,minor,patch}
	bool is_initialized() const { return sdk_initialized; }
	bool is_platform_created() const { return platform_handle != nullptr; }
	String result_to_string(int p_result_code) const;
	bool is_operation_complete(int p_result_code) const;

	// ---- platform_interface.cpp ----------------------------------------
	int platform_interface_initialize(const Dictionary &p_options);
	Dictionary platform_interface_create(const Dictionary &p_options);
#ifdef TOOLS_ENABLED
	// Copies EpicServices.gdap and the EOS AAR into res://android/plugins/ so the
	// Godot Android build system picks them up. Call once from an editor tool script.
	Dictionary android_install_plugin(const String &p_eos_sdk_android_root);
#endif
	void platform_interface_release();
	void platform_interface_tick();
	int platform_interface_check_for_launcher_and_restart();
	String platform_interface_get_active_locale_code(const String &p_local_user_id);
	String platform_interface_get_active_country_code(const String &p_local_user_id);
	String platform_interface_get_override_locale_code();
	String platform_interface_get_override_country_code();
	int platform_interface_set_override_locale_code(const String &p_locale);
	int platform_interface_set_override_country_code(const String &p_country);
	int platform_interface_set_application_status(int p_status);
	int platform_interface_get_application_status();
	int platform_interface_set_network_status(int p_status);
	int platform_interface_get_network_status();
	int platform_interface_get_desktop_crossplay_status();

	// ---- logging_interface.cpp -----------------------------------------
	int logging_interface_set_callback();
	int logging_interface_set_log_level(int p_category, int p_level);

	// ---- auth_interface.cpp --------------------------------------------
	void auth_interface_login(const Dictionary &p_options);
	void auth_interface_logout(const Dictionary &p_options);
	void auth_interface_link_account(const Dictionary &p_options);
	void auth_interface_delete_persistent_auth(const Dictionary &p_options);
	void auth_interface_verify_user_auth(const Dictionary &p_options);
	void auth_interface_query_id_token(const Dictionary &p_options);
	void auth_interface_verify_id_token(const Dictionary &p_options);
	Dictionary auth_interface_copy_user_auth_token(const Dictionary &p_options);
	Dictionary auth_interface_copy_id_token(const Dictionary &p_options);
	int auth_interface_get_logged_in_accounts_count();
	String auth_interface_get_logged_in_account_by_index(int p_index);
	int auth_interface_get_login_status(const String &p_local_user_id);
	String auth_interface_get_selected_account_id(const String &p_local_user_id);
	int auth_interface_get_merged_accounts_count(const String &p_local_user_id);
	String auth_interface_get_merged_account_by_index(const String &p_local_user_id, int p_index);
	uint64_t auth_interface_add_notify_login_status_changed();
	void auth_interface_remove_notify_login_status_changed(uint64_t p_id);

	// ---- connect_interface.cpp -----------------------------------------
	void connect_interface_login(const Dictionary &p_options);
	void connect_interface_logout(const Dictionary &p_options);
	void connect_interface_create_user(const Dictionary &p_options);
	void connect_interface_link_account(const Dictionary &p_options);
	void connect_interface_unlink_account(const Dictionary &p_options);
	void connect_interface_create_device_id(const Dictionary &p_options);
	void connect_interface_delete_device_id(const Dictionary &p_options);
	void connect_interface_query_external_account_mappings(const Dictionary &p_options);
	void connect_interface_query_product_user_id_mappings(const Dictionary &p_options);
	String connect_interface_get_external_account_mapping(const Dictionary &p_options);
	Dictionary connect_interface_get_product_user_id_mapping(const Dictionary &p_options);
	int connect_interface_get_logged_in_users_count();
	String connect_interface_get_logged_in_user_by_index(int p_index);
	int connect_interface_get_login_status(const String &p_product_user_id);
	Dictionary connect_interface_copy_id_token(const Dictionary &p_options);
	void connect_interface_verify_id_token(const Dictionary &p_options);
	uint64_t connect_interface_add_notify_login_status_changed();
	void connect_interface_remove_notify_login_status_changed(uint64_t p_id);
	uint64_t connect_interface_add_notify_auth_expiration();
	void connect_interface_remove_notify_auth_expiration(uint64_t p_id);

	// ---- lobby_interface.cpp -------------------------------------------
	void lobby_interface_create_lobby(const Dictionary &p_options);
	void lobby_interface_destroy_lobby(const Dictionary &p_options);
	void lobby_interface_join_lobby(const Dictionary &p_options);
	void lobby_interface_join_lobby_by_id(const Dictionary &p_options);
	void lobby_interface_leave_lobby(const Dictionary &p_options);
	void lobby_interface_update_lobby(const Dictionary &p_options);
	Ref<EpicLobbyModification> lobby_interface_update_lobby_modification(const Dictionary &p_options);
	Ref<EpicLobbyDetails> lobby_interface_copy_lobby_details_handle(const Dictionary &p_options);
	Ref<EpicLobbyDetails> lobby_interface_copy_lobby_details_handle_by_invite_id(const Dictionary &p_options);
	Ref<EpicLobbySearch> lobby_interface_create_lobby_search(int p_max_results);
	void lobby_interface_send_invite(const Dictionary &p_options);
	int lobby_interface_reject_invite(const Dictionary &p_options);
	int lobby_interface_get_invite_count(const Dictionary &p_options);
	String lobby_interface_get_invite_id_by_index(const Dictionary &p_options);
	String lobby_interface_get_rtc_room_name(const Dictionary &p_options);
	void lobby_interface_kick_member(const Dictionary &p_options);
	void lobby_interface_promote_member(const Dictionary &p_options);
	void lobby_interface_query_invites(const Dictionary &p_options);
	uint64_t lobby_interface_add_notify_lobby_update_received();
	void lobby_interface_remove_notify_lobby_update_received(uint64_t p_id);
	uint64_t lobby_interface_add_notify_lobby_member_update_received();
	void lobby_interface_remove_notify_lobby_member_update_received(uint64_t p_id);
	uint64_t lobby_interface_add_notify_lobby_member_status_received();
	void lobby_interface_remove_notify_lobby_member_status_received(uint64_t p_id);
	uint64_t lobby_interface_add_notify_lobby_invite_received();
	void lobby_interface_remove_notify_lobby_invite_received(uint64_t p_id);
	uint64_t lobby_interface_add_notify_lobby_invite_accepted();
	void lobby_interface_remove_notify_lobby_invite_accepted(uint64_t p_id);

	// ---- sessions_interface.cpp ----------------------------------------
	Ref<EpicSessionModification> sessions_interface_create_session_modification(const Dictionary &p_options);
	Ref<EpicSessionModification> sessions_interface_update_session_modification(const Dictionary &p_options);
	void sessions_interface_update_session(const Dictionary &p_options);
	void sessions_interface_destroy_session(const Dictionary &p_options);
	void sessions_interface_join_session(const Dictionary &p_options);
	void sessions_interface_start_session(const Dictionary &p_options);
	void sessions_interface_end_session(const Dictionary &p_options);
	void sessions_interface_register_players(const Dictionary &p_options);
	void sessions_interface_unregister_players(const Dictionary &p_options);
	void sessions_interface_send_invite(const Dictionary &p_options);
	int sessions_interface_reject_invite(const Dictionary &p_options);
	int sessions_interface_get_invite_count(const Dictionary &p_options);
	String sessions_interface_get_invite_id_by_index(const Dictionary &p_options);
	Ref<EpicSessionSearch> sessions_interface_create_session_search(int p_max_results);
	Ref<EpicActiveSession> sessions_interface_copy_active_session_handle(const Dictionary &p_options);
	uint64_t sessions_interface_add_notify_session_invite_received();
	void sessions_interface_remove_notify_session_invite_received(uint64_t p_id);
	uint64_t sessions_interface_add_notify_session_invite_accepted();
	void sessions_interface_remove_notify_session_invite_accepted(uint64_t p_id);
	uint64_t sessions_interface_add_notify_join_session_accepted();
	void sessions_interface_remove_notify_join_session_accepted(uint64_t p_id);

	// ---- p2p_interface.cpp ---------------------------------------------
	int p2p_interface_send_packet(const Dictionary &p_options);
	int p2p_interface_get_next_received_packet_size(const Dictionary &p_options);
	Dictionary p2p_interface_receive_packet(const Dictionary &p_options);
	int p2p_interface_accept_connection(const Dictionary &p_options);
	int p2p_interface_close_connection(const Dictionary &p_options);
	int p2p_interface_close_connections(const Dictionary &p_options);
	void p2p_interface_query_nat_type();
	int p2p_interface_get_nat_type();
	int p2p_interface_set_relay_control(int p_relay_control);
	int p2p_interface_get_relay_control();
	int p2p_interface_set_port_range(int p_port, int p_max_additional_ports_to_try);
	Dictionary p2p_interface_get_port_range();
	int p2p_interface_set_packet_queue_size(int64_t p_max_incoming_packet_size, int64_t p_max_outgoing_packet_size);
	Dictionary p2p_interface_get_packet_queue_info();
	int p2p_interface_clear_packet_queue(const Dictionary &p_options);
	uint64_t p2p_interface_add_notify_peer_connection_request(const String &p_socket_name);
	void p2p_interface_remove_notify_peer_connection_request(uint64_t p_id);
	uint64_t p2p_interface_add_notify_peer_connection_established(const String &p_socket_name);
	void p2p_interface_remove_notify_peer_connection_established(uint64_t p_id);
	uint64_t p2p_interface_add_notify_peer_connection_interrupted(const String &p_socket_name);
	void p2p_interface_remove_notify_peer_connection_interrupted(uint64_t p_id);
	uint64_t p2p_interface_add_notify_peer_connection_closed(const String &p_socket_name);
	void p2p_interface_remove_notify_peer_connection_closed(uint64_t p_id);

	// ---- custom_invites_interface.cpp ----------------------------------
	int custom_invites_interface_set_custom_invite(const Dictionary &p_options);
	void custom_invites_interface_send_custom_invite(const Dictionary &p_options);
	int custom_invites_interface_finalize_invite(const Dictionary &p_options);
	uint64_t custom_invites_interface_add_notify_custom_invite_received();
	void custom_invites_interface_remove_notify_custom_invite_received(uint64_t p_id);
	uint64_t custom_invites_interface_add_notify_custom_invite_accepted();
	void custom_invites_interface_remove_notify_custom_invite_accepted(uint64_t p_id);
	uint64_t custom_invites_interface_add_notify_custom_invite_rejected();
	void custom_invites_interface_remove_notify_custom_invite_rejected(uint64_t p_id);

	// ---- stats_interface.cpp -------------------------------------------
	void stats_interface_ingest_stat(const Dictionary &p_options);
	void stats_interface_query_stats(const Dictionary &p_options);
	int stats_interface_get_stats_count(const Dictionary &p_options);
	Dictionary stats_interface_copy_stat_by_index(const Dictionary &p_options);
	Dictionary stats_interface_copy_stat_by_name(const Dictionary &p_options);

	// ---- leaderboards_interface.cpp ------------------------------------
	void leaderboards_interface_query_leaderboard_definitions(const Dictionary &p_options);
	int leaderboards_interface_get_leaderboard_definition_count();
	Dictionary leaderboards_interface_copy_leaderboard_definition_by_index(int p_index);
	Dictionary leaderboards_interface_copy_leaderboard_definition_by_id(const String &p_leaderboard_id);
	void leaderboards_interface_query_leaderboard_ranks(const Dictionary &p_options);
	int leaderboards_interface_get_leaderboard_record_count();
	Dictionary leaderboards_interface_copy_leaderboard_record_by_index(int p_index);
	Dictionary leaderboards_interface_copy_leaderboard_record_by_user_id(const String &p_user_id);
	void leaderboards_interface_query_leaderboard_user_scores(const Dictionary &p_options);
	int leaderboards_interface_get_leaderboard_user_score_count(const String &p_stat_name);
	Dictionary leaderboards_interface_copy_leaderboard_user_score_by_index(const Dictionary &p_options);
	Dictionary leaderboards_interface_copy_leaderboard_user_score_by_user_id(const Dictionary &p_options);

	// ---- achievements_interface.cpp ------------------------------------
	void achievements_interface_query_definitions(const Dictionary &p_options);
	int achievements_interface_get_achievement_definition_count();
	Dictionary achievements_interface_copy_achievement_definition_by_index(int p_index);
	Dictionary achievements_interface_copy_achievement_definition_by_id(const String &p_id);
	void achievements_interface_query_player_achievements(const Dictionary &p_options);
	int achievements_interface_get_player_achievement_count(const String &p_target_user_id);
	Dictionary achievements_interface_copy_player_achievement_by_index(const Dictionary &p_options);
	Dictionary achievements_interface_copy_player_achievement_by_id(const Dictionary &p_options);
	void achievements_interface_unlock_achievements(const Dictionary &p_options);
	uint64_t achievements_interface_add_notify_achievements_unlocked();
	void achievements_interface_remove_notify_achievements_unlocked(uint64_t p_id);

	// ---- friends_interface.cpp -----------------------------------------
	void friends_interface_query_friends(const Dictionary &p_options);
	int friends_interface_get_friends_count(const String &p_local_user_id);
	String friends_interface_get_friend_at_index(const String &p_local_user_id, int p_index);
	int friends_interface_get_status(const String &p_local_user_id, const String &p_target_user_id);
	int friends_interface_get_blocked_users_count(const String &p_local_user_id);
	String friends_interface_get_blocked_user_at_index(const String &p_local_user_id, int p_index);
	uint64_t friends_interface_add_notify_friends_update();
	void friends_interface_remove_notify_friends_update(uint64_t p_id);
	uint64_t friends_interface_add_notify_blocked_users_update();
	void friends_interface_remove_notify_blocked_users_update(uint64_t p_id);

	// ---- presence_interface.cpp ----------------------------------------
	void presence_interface_query_presence(const Dictionary &p_options);
	bool presence_interface_has_presence(const Dictionary &p_options);
	Dictionary presence_interface_copy_presence(const Dictionary &p_options);
	Ref<EpicPresenceModification> presence_interface_create_presence_modification(const String &p_local_user_id);
	void presence_interface_set_presence(const Dictionary &p_options);
	String presence_interface_get_join_info(const Dictionary &p_options);
	uint64_t presence_interface_add_notify_on_presence_changed();
	void presence_interface_remove_notify_on_presence_changed(uint64_t p_id);
	uint64_t presence_interface_add_notify_join_game_accepted();
	void presence_interface_remove_notify_join_game_accepted(uint64_t p_id);

	// ---- user_info_interface.cpp ---------------------------------------
	void user_info_interface_query_user_info(const Dictionary &p_options);
	void user_info_interface_query_user_info_by_display_name(const Dictionary &p_options);
	void user_info_interface_query_user_info_by_external_account(const Dictionary &p_options);
	Dictionary user_info_interface_copy_user_info(const Dictionary &p_options);
	int user_info_interface_get_external_user_info_count(const Dictionary &p_options);
	Dictionary user_info_interface_copy_external_user_info_by_index(const Dictionary &p_options);
	Dictionary user_info_interface_copy_best_display_name(const Dictionary &p_options);
	int user_info_interface_get_local_platform_type();

	// ---- playerdatastorage_interface.cpp -------------------------------
	void playerdatastorage_interface_query_file(const Dictionary &p_options);
	void playerdatastorage_interface_query_file_list(const Dictionary &p_options);
	Dictionary playerdatastorage_interface_copy_file_metadata_by_filename(const Dictionary &p_options);
	int playerdatastorage_interface_get_file_metadata_count(const Dictionary &p_options);
	Dictionary playerdatastorage_interface_copy_file_metadata_at_index(const Dictionary &p_options);
	void playerdatastorage_interface_duplicate_file(const Dictionary &p_options);
	void playerdatastorage_interface_delete_file(const Dictionary &p_options);
	Ref<EpicPlayerDataStorageFileTransferRequest> playerdatastorage_interface_read_file(const Dictionary &p_options);
	Ref<EpicPlayerDataStorageFileTransferRequest> playerdatastorage_interface_write_file(const Dictionary &p_options);
	int playerdatastorage_interface_delete_cache(const Dictionary &p_options);

	// ---- titlestorage_interface.cpp ------------------------------------
	void titlestorage_interface_query_file(const Dictionary &p_options);
	void titlestorage_interface_query_file_list(const Dictionary &p_options);
	Dictionary titlestorage_interface_copy_file_metadata_by_filename(const Dictionary &p_options);
	int titlestorage_interface_get_file_metadata_count(const Dictionary &p_options);
	Dictionary titlestorage_interface_copy_file_metadata_at_index(const Dictionary &p_options);
	Ref<EpicTitleStorageFileTransferRequest> titlestorage_interface_read_file(const Dictionary &p_options);
	int titlestorage_interface_delete_cache(const Dictionary &p_options);

	EpicServices();
	~EpicServices();
};

#endif // GD_EPIC_SERVICES_H
