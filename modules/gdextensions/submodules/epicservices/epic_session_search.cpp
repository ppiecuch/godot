/**************************************************************************/
/*  epic_session_search.cpp                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#include "epic_session_search.h"

#include "epic_callback.h"
#include "epic_session_details.h"
#include "epic_utils.h"
#include "gd_epic_services.h"

#include "eos_sessions.h"

void EpicSessionSearch::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicSessionSearch::is_valid);
	ClassDB::bind_method(D_METHOD("set_session_id", "session_id"), &EpicSessionSearch::set_session_id);
	ClassDB::bind_method(D_METHOD("set_target_user_id", "product_user_id"), &EpicSessionSearch::set_target_user_id);
	ClassDB::bind_method(D_METHOD("set_max_results", "max"), &EpicSessionSearch::set_max_results);
	ClassDB::bind_method(D_METHOD("set_parameter", "parameter"), &EpicSessionSearch::set_parameter);
	ClassDB::bind_method(D_METHOD("remove_parameter", "key", "comparison_op"), &EpicSessionSearch::remove_parameter);
	ClassDB::bind_method(D_METHOD("find", "options"), &EpicSessionSearch::find);
	ClassDB::bind_method(D_METHOD("get_search_result_count"), &EpicSessionSearch::get_search_result_count);
	ClassDB::bind_method(D_METHOD("copy_search_result_by_index", "index"), &EpicSessionSearch::copy_search_result_by_index);

	ADD_SIGNAL(MethodInfo("session_search_find_callback", PropertyInfo(Variant::DICTIONARY, "data")));
}

void EpicSessionSearch::set_handle(EOS_HSessionSearch p_handle) {
	if (handle && handle != p_handle) {
		EOS_SessionSearch_Release(handle);
	}
	handle = p_handle;
}

EpicSessionSearch::~EpicSessionSearch() {
	if (handle) {
		EOS_SessionSearch_Release(handle);
		handle = nullptr;
	}
}

int EpicSessionSearch::set_session_id(const String &p_session_id) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_session_id.utf8();
	EOS_SessionSearch_SetSessionIdOptions opts = {};
	opts.ApiVersion = EOS_SESSIONSEARCH_SETSESSIONID_API_LATEST;
	opts.SessionId = cs.length() ? cs.get_data() : nullptr;
	return int(EOS_SessionSearch_SetSessionId(handle, &opts));
}

int EpicSessionSearch::set_target_user_id(const String &p_target) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_SessionSearch_SetTargetUserIdOptions opts = {};
	opts.ApiVersion = EOS_SESSIONSEARCH_SETTARGETUSERID_API_LATEST;
	opts.TargetUserId = eos_pui_from_string(p_target);
	return int(EOS_SessionSearch_SetTargetUserId(handle, &opts));
}

int EpicSessionSearch::set_max_results(int p_max) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_SessionSearch_SetMaxResultsOptions opts = {};
	opts.ApiVersion = EOS_SESSIONSEARCH_SETMAXSEARCHRESULTS_API_LATEST;
	opts.MaxSearchResults = uint32_t(p_max);
	return int(EOS_SessionSearch_SetMaxResults(handle, &opts));
}

int EpicSessionSearch::set_parameter(const Dictionary &p_param) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString key_cs = String(dict_get_string(p_param, "key")).utf8();
	CharString str_cs;
	EOS_Sessions_AttributeData data = {};
	data.ApiVersion = EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST;
	data.Key = key_cs.length() ? key_cs.get_data() : nullptr;
	data.ValueType = EOS_ESessionAttributeType(dict_get_int(p_param, "value_type", int(EOS_ESessionAttributeType::EOS_AT_STRING)));
	if (p_param.has("value")) {
		const Variant &v = p_param["value"];
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
	EOS_SessionSearch_SetParameterOptions opts = {};
	opts.ApiVersion = EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST;
	opts.Parameter = &data;
	opts.ComparisonOp = EOS_EOnlineComparisonOp(dict_get_int(p_param, "comparison_op", int(EOS_EOnlineComparisonOp::EOS_CO_EQUAL)));
	return int(EOS_SessionSearch_SetParameter(handle, &opts));
}

int EpicSessionSearch::remove_parameter(const String &p_key, int p_comparison_op) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_key.utf8();
	EOS_SessionSearch_RemoveParameterOptions opts = {};
	opts.ApiVersion = EOS_SESSIONSEARCH_REMOVEPARAMETER_API_LATEST;
	opts.Key = cs.get_data();
	opts.ComparisonOp = EOS_EOnlineComparisonOp(p_comparison_op);
	return int(EOS_SessionSearch_RemoveParameter(handle, &opts));
}

static void EOS_CALL _on_session_search_find(const EOS_SessionSearch_FindCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicSessionSearch::find(const Dictionary &p_options) {
	if (!handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		EpicServices *es = EpicServices::get_singleton();
		if (es) {
			epic_emit_deferred(es->get_instance_id(), StringName("session_search_find_callback"), payload);
		}
		return;
	}
	EOS_SessionSearch_FindOptions opts = {};
	opts.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));

	EpicServices *es = EpicServices::get_singleton();
	EpicCallback *cb = memnew(EpicCallback(es ? es->get_instance_id() : ObjectID(), StringName("session_search_find_callback")));
	EOS_SessionSearch_Find(handle, &opts, cb, &_on_session_search_find);
}

int EpicSessionSearch::get_search_result_count() {
	if (!handle) {
		return 0;
	}
	EOS_SessionSearch_GetSearchResultCountOptions opts = {};
	opts.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
	return int(EOS_SessionSearch_GetSearchResultCount(handle, &opts));
}

Ref<EpicSessionDetails> EpicSessionSearch::copy_search_result_by_index(int p_index) {
	Ref<EpicSessionDetails> result;
	if (!handle) {
		return result;
	}
	EOS_SessionSearch_CopySearchResultByIndexOptions opts = {};
	opts.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
	opts.SessionIndex = uint32_t(p_index);
	EOS_HSessionDetails details = nullptr;
	if (EOS_SessionSearch_CopySearchResultByIndex(handle, &opts, &details) == EOS_EResult::EOS_Success && details) {
		result.instance();
		result->set_handle(details);
	}
	return result;
}
