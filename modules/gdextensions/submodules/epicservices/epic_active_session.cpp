/**************************************************************************/
/*  epic_active_session.cpp                                               */
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

#include "epic_active_session.h"

#include "epic_utils.h"

#include "eos_sessions.h"

void EpicActiveSession::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicActiveSession::is_valid);
	ClassDB::bind_method(D_METHOD("copy_info"), &EpicActiveSession::copy_info);
	ClassDB::bind_method(D_METHOD("get_registered_player_count"), &EpicActiveSession::get_registered_player_count);
	ClassDB::bind_method(D_METHOD("get_registered_player_by_index", "index"), &EpicActiveSession::get_registered_player_by_index);
	ClassDB::bind_method(D_METHOD("get_state"), &EpicActiveSession::get_state);
}

void EpicActiveSession::set_handle(EOS_HActiveSession p_handle) {
	if (handle && handle != p_handle) {
		EOS_ActiveSession_Release(handle);
	}
	handle = p_handle;
}

EpicActiveSession::~EpicActiveSession() {
	if (handle) {
		EOS_ActiveSession_Release(handle);
		handle = nullptr;
	}
}

Dictionary EpicActiveSession::copy_info() const {
	Dictionary out;
	if (!handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotFound);
		return out;
	}
	EOS_ActiveSession_CopyInfoOptions opts = {};
	opts.ApiVersion = EOS_ACTIVESESSION_COPYINFO_API_LATEST;
	EOS_ActiveSession_Info *info = nullptr;
	EOS_EResult r = EOS_ActiveSession_CopyInfo(handle, &opts, &info);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success && info) {
		out["session_name"] = String::utf8(info->SessionName ? info->SessionName : "");
		out["local_user_id"] = eos_pui_to_string(info->LocalUserId);
		out["state"] = int(info->State);
		EOS_ActiveSession_Info_Release(info);
	}
	return out;
}

int EpicActiveSession::get_registered_player_count() const {
	if (!handle) {
		return 0;
	}
	EOS_ActiveSession_GetRegisteredPlayerCountOptions opts = {};
	opts.ApiVersion = EOS_ACTIVESESSION_GETREGISTEREDPLAYERCOUNT_API_LATEST;
	return int(EOS_ActiveSession_GetRegisteredPlayerCount(handle, &opts));
}

String EpicActiveSession::get_registered_player_by_index(int p_index) const {
	if (!handle) {
		return String();
	}
	EOS_ActiveSession_GetRegisteredPlayerByIndexOptions opts = {};
	opts.ApiVersion = EOS_ACTIVESESSION_GETREGISTEREDPLAYERBYINDEX_API_LATEST;
	opts.PlayerIndex = uint32_t(p_index);
	return eos_pui_to_string(EOS_ActiveSession_GetRegisteredPlayerByIndex(handle, &opts));
}

int EpicActiveSession::get_state() const {
	Dictionary info = copy_info();
	return info.has("state") ? int(info["state"]) : 0;
}
