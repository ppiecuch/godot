/**************************************************************************/
/*  epic_lobby_search.cpp                                                 */
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

#include "epic_lobby_search.h"

#include "epic_callback.h"
#include "epic_lobby_details.h"
#include "epic_utils.h"
#include "gd_epic_services.h"

#include "eos_lobby.h"

void EpicLobbySearch::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicLobbySearch::is_valid);
	ClassDB::bind_method(D_METHOD("set_lobby_id", "lobby_id"), &EpicLobbySearch::set_lobby_id);
	ClassDB::bind_method(D_METHOD("set_target_user_id", "product_user_id"), &EpicLobbySearch::set_target_user_id);
	ClassDB::bind_method(D_METHOD("set_max_results", "max"), &EpicLobbySearch::set_max_results);
	ClassDB::bind_method(D_METHOD("set_parameter", "parameter"), &EpicLobbySearch::set_parameter);
	ClassDB::bind_method(D_METHOD("remove_parameter", "key", "comparison_op"), &EpicLobbySearch::remove_parameter);
	ClassDB::bind_method(D_METHOD("find", "options"), &EpicLobbySearch::find);
	ClassDB::bind_method(D_METHOD("get_search_result_count"), &EpicLobbySearch::get_search_result_count);
	ClassDB::bind_method(D_METHOD("copy_search_result_by_index", "index"), &EpicLobbySearch::copy_search_result_by_index);

	ADD_SIGNAL(MethodInfo("lobby_search_find_callback", PropertyInfo(Variant::DICTIONARY, "data")));
}

void EpicLobbySearch::set_handle(EOS_HLobbySearch p_handle) {
	if (handle && handle != p_handle) {
		EOS_LobbySearch_Release(handle);
	}
	handle = p_handle;
}

EpicLobbySearch::~EpicLobbySearch() {
	if (handle) {
		EOS_LobbySearch_Release(handle);
		handle = nullptr;
	}
}

int EpicLobbySearch::set_lobby_id(const String &p_lobby_id) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_lobby_id.utf8();
	EOS_LobbySearch_SetLobbyIdOptions opts = {};
	opts.ApiVersion = EOS_LOBBYSEARCH_SETLOBBYID_API_LATEST;
	opts.LobbyId = cs.length() ? cs.get_data() : nullptr;
	return int(EOS_LobbySearch_SetLobbyId(handle, &opts));
}

int EpicLobbySearch::set_target_user_id(const String &p_target) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_LobbySearch_SetTargetUserIdOptions opts = {};
	opts.ApiVersion = EOS_LOBBYSEARCH_SETTARGETUSERID_API_LATEST;
	opts.TargetUserId = eos_pui_from_string(p_target);
	return int(EOS_LobbySearch_SetTargetUserId(handle, &opts));
}

int EpicLobbySearch::set_max_results(int p_max) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_LobbySearch_SetMaxResultsOptions opts = {};
	opts.ApiVersion = EOS_LOBBYSEARCH_SETMAXRESULTS_API_LATEST;
	opts.MaxResults = uint32_t(p_max);
	return int(EOS_LobbySearch_SetMaxResults(handle, &opts));
}

int EpicLobbySearch::set_parameter(const Dictionary &p_param) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString key_cs = String(dict_get_string(p_param, "key")).utf8();
	CharString str_cs;
	EOS_Lobby_AttributeData data = {};
	data.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
	data.Key = key_cs.length() ? key_cs.get_data() : nullptr;
	data.ValueType = EOS_EAttributeType(dict_get_int(p_param, "value_type", int(EOS_EAttributeType::EOS_AT_STRING)));
	if (p_param.has("value")) {
		const Variant &v = p_param["value"];
		switch (data.ValueType) {
			case EOS_EAttributeType::EOS_AT_BOOLEAN:
				data.Value.AsBool = bool(v) ? EOS_TRUE : EOS_FALSE;
				break;
			case EOS_EAttributeType::EOS_AT_INT64:
				data.Value.AsInt64 = int64_t(v);
				break;
			case EOS_EAttributeType::EOS_AT_DOUBLE:
				data.Value.AsDouble = double(v);
				break;
			case EOS_EAttributeType::EOS_AT_STRING:
			default:
				str_cs = String(v).utf8();
				data.Value.AsUtf8 = str_cs.get_data();
				break;
		}
	}
	EOS_LobbySearch_SetParameterOptions opts = {};
	opts.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;
	opts.Parameter = &data;
	opts.ComparisonOp = EOS_EOnlineComparisonOp(dict_get_int(p_param, "comparison_op", int(EOS_EOnlineComparisonOp::EOS_CO_EQUAL)));
	return int(EOS_LobbySearch_SetParameter(handle, &opts));
}

int EpicLobbySearch::remove_parameter(const String &p_key, int p_comparison_op) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString key_cs = p_key.utf8();
	EOS_LobbySearch_RemoveParameterOptions opts = {};
	opts.ApiVersion = EOS_LOBBYSEARCH_REMOVEPARAMETER_API_LATEST;
	opts.Key = key_cs.get_data();
	opts.ComparisonOp = EOS_EOnlineComparisonOp(p_comparison_op);
	return int(EOS_LobbySearch_RemoveParameter(handle, &opts));
}

static void EOS_CALL _on_lobby_search_find(const EOS_LobbySearch_FindCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicLobbySearch::find(const Dictionary &p_options) {
	if (!handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		EpicServices *es = EpicServices::get_singleton();
		if (es) {
			epic_emit_deferred(es->get_instance_id(), StringName("lobby_search_find_callback"), payload);
		}
		return;
	}
	EOS_LobbySearch_FindOptions opts = {};
	opts.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));

	// Find emits on the EpicServices singleton (and on this Search wrapper's "find" signal too).
	EpicServices *es = EpicServices::get_singleton();
	EpicCallback *cb = memnew(EpicCallback(es ? es->get_instance_id() : ObjectID(), StringName("lobby_search_find_callback")));
	EOS_LobbySearch_Find(handle, &opts, cb, &_on_lobby_search_find);
}

int EpicLobbySearch::get_search_result_count() {
	if (!handle) {
		return 0;
	}
	EOS_LobbySearch_GetSearchResultCountOptions opts = {};
	opts.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
	return int(EOS_LobbySearch_GetSearchResultCount(handle, &opts));
}

Ref<EpicLobbyDetails> EpicLobbySearch::copy_search_result_by_index(int p_index) {
	Ref<EpicLobbyDetails> result;
	if (!handle) {
		return result;
	}
	EOS_LobbySearch_CopySearchResultByIndexOptions opts = {};
	opts.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
	opts.LobbyIndex = uint32_t(p_index);
	EOS_HLobbyDetails details = nullptr;
	if (EOS_LobbySearch_CopySearchResultByIndex(handle, &opts, &details) == EOS_EResult::EOS_Success && details) {
		result.instance();
		result->set_handle(details);
	}
	return result;
}
