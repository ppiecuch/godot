/**************************************************************************/
/*  stats_interface.cpp                                                   */
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

#include "eos_stats.h"

static void EOS_CALL _on_stats_ingest(const EOS_Stats_IngestStatCompleteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::stats_interface_ingest_stat(const Dictionary &p_options) {
	if (!stats_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("stats_interface_ingest_stat_callback", payload);
		return;
	}
	const Array stats_arr = p_options.has("stats") ? Array(p_options["stats"]) : Array();
	const int n = stats_arr.size();
	Vector<CharString> name_storage;
	name_storage.resize(n);
	Vector<EOS_Stats_IngestData> data_arr;
	data_arr.resize(n);
	for (int i = 0; i < n; ++i) {
		const Dictionary stat = Dictionary(stats_arr[i]);
		name_storage.write[i] = String(dict_get_string(stat, "stat_name")).utf8();
		data_arr.write[i].ApiVersion = EOS_STATS_INGESTDATA_API_LATEST;
		data_arr.write[i].StatName = name_storage[i].length() ? name_storage[i].get_data() : "";
		data_arr.write[i].IngestAmount = int32_t(dict_get_int(stat, "ingest_amount", 0));
	}
	EOS_Stats_IngestStatOptions opts = {};
	opts.ApiVersion = EOS_STATS_INGESTSTAT_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	opts.Stats = data_arr.size() ? data_arr.ptrw() : nullptr;
	opts.StatsCount = uint32_t(n);
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("stats_interface_ingest_stat_callback")));
	EOS_Stats_IngestStat(stats_handle, &opts, cb, &_on_stats_ingest);
}

static void EOS_CALL _on_stats_query(const EOS_Stats_OnQueryStatsCompleteCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["target_user_id"] = eos_pui_to_string(data->TargetUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::stats_interface_query_stats(const Dictionary &p_options) {
	if (!stats_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("stats_interface_query_stats_callback", payload);
		return;
	}
	const Array stat_names = p_options.has("stat_names") ? Array(p_options["stat_names"]) : Array();
	Vector<CharString> name_storage;
	name_storage.resize(stat_names.size());
	Vector<const char *> name_ptrs;
	name_ptrs.resize(stat_names.size());
	for (int i = 0; i < stat_names.size(); ++i) {
		name_storage.write[i] = String(stat_names[i]).utf8();
		name_ptrs.write[i] = name_storage[i].length() ? name_storage[i].get_data() : "";
	}
	EOS_Stats_QueryStatsOptions opts = {};
	opts.ApiVersion = EOS_STATS_QUERYSTATS_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	opts.StartTime = int64_t(dict_get_int64(p_options, "start_time", EOS_STATS_TIME_UNDEFINED));
	opts.EndTime = int64_t(dict_get_int64(p_options, "end_time", EOS_STATS_TIME_UNDEFINED));
	opts.StatNames = name_ptrs.size() ? name_ptrs.ptrw() : nullptr;
	opts.StatNamesCount = uint32_t(name_ptrs.size());
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("stats_interface_query_stats_callback")));
	EOS_Stats_QueryStats(stats_handle, &opts, cb, &_on_stats_query);
}

int EpicServices::stats_interface_get_stats_count(const Dictionary &p_options) {
	if (!stats_handle) {
		return 0;
	}
	EOS_Stats_GetStatCountOptions opts = {};
	opts.ApiVersion = EOS_STATS_GETSTATCOUNT_API_LATEST;
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	return int(EOS_Stats_GetStatsCount(stats_handle, &opts));
}

static Dictionary _stat_to_dict(const EOS_Stats_Stat *stat) {
	Dictionary d;
	if (!stat) {
		return d;
	}
	d["name"] = String::utf8(stat->Name ? stat->Name : "");
	d["start_time"] = int64_t(stat->StartTime);
	d["end_time"] = int64_t(stat->EndTime);
	d["value"] = int64_t(stat->Value);
	return d;
}

Dictionary EpicServices::stats_interface_copy_stat_by_index(const Dictionary &p_options) {
	Dictionary out;
	if (!stats_handle) {
		return out;
	}
	EOS_Stats_CopyStatByIndexOptions opts = {};
	opts.ApiVersion = EOS_STATS_COPYSTATBYINDEX_API_LATEST;
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	opts.StatIndex = uint32_t(dict_get_int(p_options, "stat_index", 0));
	EOS_Stats_Stat *stat = nullptr;
	if (EOS_Stats_CopyStatByIndex(stats_handle, &opts, &stat) == EOS_EResult::EOS_Success && stat) {
		out = _stat_to_dict(stat);
		EOS_Stats_Stat_Release(stat);
	}
	return out;
}

Dictionary EpicServices::stats_interface_copy_stat_by_name(const Dictionary &p_options) {
	Dictionary out;
	if (!stats_handle) {
		return out;
	}
	const CharString name_cs = dict_get_string(p_options, "name").utf8();
	EOS_Stats_CopyStatByNameOptions opts = {};
	opts.ApiVersion = EOS_STATS_COPYSTATBYNAME_API_LATEST;
	opts.TargetUserId = eos_pui_from_string(dict_get_string(p_options, "target_user_id"));
	opts.Name = name_cs.get_data();
	EOS_Stats_Stat *stat = nullptr;
	if (EOS_Stats_CopyStatByName(stats_handle, &opts, &stat) == EOS_EResult::EOS_Success && stat) {
		out = _stat_to_dict(stat);
		EOS_Stats_Stat_Release(stat);
	}
	return out;
}
