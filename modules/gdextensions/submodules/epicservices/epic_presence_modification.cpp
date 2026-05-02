/**************************************************************************/
/*  epic_presence_modification.cpp                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#include "epic_presence_modification.h"

#include "epic_utils.h"

#include "eos_presence.h"

void EpicPresenceModification::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicPresenceModification::is_valid);
	ClassDB::bind_method(D_METHOD("set_status", "status"), &EpicPresenceModification::set_status);
	ClassDB::bind_method(D_METHOD("set_raw_rich_text", "text"), &EpicPresenceModification::set_raw_rich_text);
	ClassDB::bind_method(D_METHOD("set_data", "kv"), &EpicPresenceModification::set_data);
	ClassDB::bind_method(D_METHOD("delete_data", "keys"), &EpicPresenceModification::delete_data);
	ClassDB::bind_method(D_METHOD("set_join_info", "join_info"), &EpicPresenceModification::set_join_info);
}

void EpicPresenceModification::set_handle(EOS_HPresenceModification p_handle) {
	if (handle && handle != p_handle) {
		EOS_PresenceModification_Release(handle);
	}
	handle = p_handle;
}

EpicPresenceModification::~EpicPresenceModification() {
	if (handle) {
		EOS_PresenceModification_Release(handle);
		handle = nullptr;
	}
}

int EpicPresenceModification::set_status(int p_status) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_PresenceModification_SetStatusOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_SETSTATUS_API_LATEST;
	opts.Status = EOS_Presence_EStatus(p_status);
	return int(EOS_PresenceModification_SetStatus(handle, &opts));
}

int EpicPresenceModification::set_raw_rich_text(const String &p_text) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_text.utf8();
	EOS_PresenceModification_SetRawRichTextOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_SETRAWRICHTEXT_API_LATEST;
	opts.RichText = cs.length() ? cs.get_data() : "";
	return int(EOS_PresenceModification_SetRawRichText(handle, &opts));
}

int EpicPresenceModification::set_data(const Dictionary &p_kv) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const Array keys = p_kv.keys();
	const int n = keys.size();
	if (n == 0) {
		return int(EOS_EResult::EOS_InvalidParameters);
	}
	Vector<CharString> key_storage;
	key_storage.resize(n);
	Vector<CharString> val_storage;
	val_storage.resize(n);
	Vector<EOS_Presence_DataRecord> records;
	records.resize(n);
	for (int i = 0; i < n; ++i) {
		key_storage.write[i] = String(keys[i]).utf8();
		val_storage.write[i] = String(p_kv[keys[i]]).utf8();
		records.write[i].ApiVersion = EOS_PRESENCE_DATARECORD_API_LATEST;
		records.write[i].Key = key_storage[i].length() ? key_storage[i].get_data() : "";
		records.write[i].Value = val_storage[i].length() ? val_storage[i].get_data() : "";
	}
	EOS_PresenceModification_SetDataOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_SETDATA_API_LATEST;
	opts.RecordsCount = uint32_t(n);
	opts.Records = records.ptrw();
	return int(EOS_PresenceModification_SetData(handle, &opts));
}

int EpicPresenceModification::delete_data(const Array &p_keys) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const int n = p_keys.size();
	if (n == 0) {
		return int(EOS_EResult::EOS_InvalidParameters);
	}
	Vector<CharString> key_storage;
	key_storage.resize(n);
	Vector<EOS_PresenceModification_DataRecordId> records;
	records.resize(n);
	for (int i = 0; i < n; ++i) {
		key_storage.write[i] = String(p_keys[i]).utf8();
		records.write[i].ApiVersion = EOS_PRESENCEMODIFICATION_DATARECORDID_API_LATEST;
		records.write[i].Key = key_storage[i].length() ? key_storage[i].get_data() : "";
	}
	EOS_PresenceModification_DeleteDataOptions opts = {};
	opts.ApiVersion = EOS_PRESENCE_DELETEDATA_API_LATEST;
	opts.RecordsCount = uint32_t(n);
	opts.Records = records.ptrw();
	return int(EOS_PresenceModification_DeleteData(handle, &opts));
}

int EpicPresenceModification::set_join_info(const String &p_join_info) {
	if (!handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const CharString cs = p_join_info.utf8();
	EOS_PresenceModification_SetJoinInfoOptions opts = {};
	opts.ApiVersion = EOS_PRESENCEMODIFICATION_SETJOININFO_API_LATEST;
	opts.JoinInfo = cs.length() ? cs.get_data() : nullptr;
	return int(EOS_PresenceModification_SetJoinInfo(handle, &opts));
}
