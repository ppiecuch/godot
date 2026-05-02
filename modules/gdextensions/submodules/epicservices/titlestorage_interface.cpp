/**************************************************************************/
/*  titlestorage_interface.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#include "gd_epic_services.h"

#include "epic_callback.h"
#include "epic_file_transfer_request.h"
#include "epic_utils.h"

#include "eos_titlestorage.h"

#define TS_GUARD_VOID(SIGNAL)                                                   \
	if (!title_storage_handle) {                                                \
		Dictionary payload;                                                     \
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured); \
		_emit_deferred(SIGNAL, payload);                                        \
		return;                                                                 \
	}

struct _TSReadCtx {
	ObjectID singleton_id;
	StringName signal_name;
	String filename;
	PoolByteArray buffer;
};

static void EOS_CALL _on_ts_query_file(const EOS_TitleStorage_QueryFileCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::titlestorage_interface_query_file(const Dictionary &p_options) {
	TS_GUARD_VOID("titlestorage_interface_query_file_callback");
	const CharString filename_cs = dict_get_string(p_options, "filename").utf8();
	EOS_TitleStorage_QueryFileOptions opts = {};
	opts.ApiVersion = EOS_TITLESTORAGE_QUERYFILE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Filename = filename_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("titlestorage_interface_query_file_callback")));
	EOS_TitleStorage_QueryFile(title_storage_handle, &opts, cb, &_on_ts_query_file);
}

static void EOS_CALL _on_ts_query_list(const EOS_TitleStorage_QueryFileListCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["file_count"] = int(data->FileCount);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::titlestorage_interface_query_file_list(const Dictionary &p_options) {
	TS_GUARD_VOID("titlestorage_interface_query_file_list_callback");
	const Array tags = p_options.has("list_of_tags") ? Array(p_options["list_of_tags"]) : Array();
	Vector<CharString> tag_storage;
	tag_storage.resize(tags.size());
	Vector<const char *> tag_ptrs;
	tag_ptrs.resize(tags.size());
	for (int i = 0; i < tags.size(); ++i) {
		tag_storage.write[i] = String(tags[i]).utf8();
		tag_ptrs.write[i] = tag_storage[i].length() ? tag_storage[i].get_data() : "";
	}
	EOS_TitleStorage_QueryFileListOptions opts = {};
	opts.ApiVersion = EOS_TITLESTORAGE_QUERYFILELIST_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.ListOfTags = tag_ptrs.size() ? tag_ptrs.ptrw() : nullptr;
	opts.ListOfTagsCount = uint32_t(tag_ptrs.size());
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("titlestorage_interface_query_file_list_callback")));
	EOS_TitleStorage_QueryFileList(title_storage_handle, &opts, cb, &_on_ts_query_list);
}

static Dictionary _ts_meta_to_dict(const EOS_TitleStorage_FileMetadata *m) {
	Dictionary d;
	if (!m) {
		return d;
	}
	d["filename"] = String::utf8(m->Filename ? m->Filename : "");
	d["file_size_bytes"] = int64_t(m->FileSizeBytes);
	d["unencrypted_data_size_bytes"] = int64_t(m->UnencryptedDataSizeBytes);
	d["md5_hash"] = String::utf8(m->MD5Hash ? m->MD5Hash : "");
	return d;
}

Dictionary EpicServices::titlestorage_interface_copy_file_metadata_by_filename(const Dictionary &p_options) {
	Dictionary out;
	if (!title_storage_handle) {
		return out;
	}
	const CharString filename_cs = dict_get_string(p_options, "filename").utf8();
	EOS_TitleStorage_CopyFileMetadataByFilenameOptions opts = {};
	opts.ApiVersion = EOS_TITLESTORAGE_COPYFILEMETADATABYFILENAME_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Filename = filename_cs.get_data();
	EOS_TitleStorage_FileMetadata *meta = nullptr;
	if (EOS_TitleStorage_CopyFileMetadataByFilename(title_storage_handle, &opts, &meta) == EOS_EResult::EOS_Success && meta) {
		out = _ts_meta_to_dict(meta);
		EOS_TitleStorage_FileMetadata_Release(meta);
	}
	return out;
}

int EpicServices::titlestorage_interface_get_file_metadata_count(const Dictionary &p_options) {
	if (!title_storage_handle) {
		return 0;
	}
	EOS_TitleStorage_GetFileMetadataCountOptions opts = {};
	opts.ApiVersion = EOS_TITLESTORAGE_GETFILEMETADATACOUNT_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	return int(EOS_TitleStorage_GetFileMetadataCount(title_storage_handle, &opts));
}

Dictionary EpicServices::titlestorage_interface_copy_file_metadata_at_index(const Dictionary &p_options) {
	Dictionary out;
	if (!title_storage_handle) {
		return out;
	}
	EOS_TitleStorage_CopyFileMetadataAtIndexOptions opts = {};
	opts.ApiVersion = EOS_TITLESTORAGE_COPYFILEMETADATAATINDEX_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Index = uint32_t(dict_get_int(p_options, "index", 0));
	EOS_TitleStorage_FileMetadata *meta = nullptr;
	if (EOS_TitleStorage_CopyFileMetadataAtIndex(title_storage_handle, &opts, &meta) == EOS_EResult::EOS_Success && meta) {
		out = _ts_meta_to_dict(meta);
		EOS_TitleStorage_FileMetadata_Release(meta);
	}
	return out;
}

static EOS_TitleStorage_EReadResult EOS_CALL _ts_on_read_chunk(const EOS_TitleStorage_ReadFileDataCallbackInfo *data) {
	_TSReadCtx *ctx = (_TSReadCtx *)data->ClientData;
	if (!ctx) {
		return EOS_TitleStorage_EReadResult::EOS_TS_RR_FailRequest;
	}
	const int prev = ctx->buffer.size();
	ctx->buffer.resize(prev + int(data->DataChunkLengthBytes));
	PoolByteArray::Write w = ctx->buffer.write();
	memcpy(w.ptr() + prev, data->DataChunk, data->DataChunkLengthBytes);
	return EOS_TitleStorage_EReadResult::EOS_TS_RR_ContinueReading;
}

static void EOS_CALL _ts_on_read_complete(const EOS_TitleStorage_ReadFileCallbackInfo *data) {
	_TSReadCtx *ctx = (_TSReadCtx *)data->ClientData;
	if (!ctx) {
		return;
	}
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["filename"] = String::utf8(data->Filename ? data->Filename : "");
	payload["data"] = ctx->buffer;
	epic_emit_deferred(ctx->singleton_id, ctx->signal_name, payload);
	memdelete(ctx);
}

Ref<EpicTitleStorageFileTransferRequest> EpicServices::titlestorage_interface_read_file(const Dictionary &p_options) {
	Ref<EpicTitleStorageFileTransferRequest> result;
	if (!title_storage_handle) {
		return result;
	}
	const CharString filename_cs = dict_get_string(p_options, "filename").utf8();
	_TSReadCtx *ctx = memnew(_TSReadCtx);
	ctx->singleton_id = get_instance_id();
	ctx->signal_name = StringName("titlestorage_interface_read_file_callback");
	ctx->filename = String::utf8(filename_cs.get_data());

	EOS_TitleStorage_ReadFileOptions opts = {};
	opts.ApiVersion = EOS_TITLESTORAGE_READFILE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Filename = filename_cs.get_data();
	opts.ReadChunkLengthBytes = uint32_t(dict_get_int(p_options, "read_chunk_length_bytes", 16 * 1024));
	opts.ReadFileDataCallback = &_ts_on_read_chunk;
	opts.FileTransferProgressCallback = nullptr;

	EOS_HTitleStorageFileTransferRequest req = EOS_TitleStorage_ReadFile(title_storage_handle, &opts, ctx, &_ts_on_read_complete);
	if (req) {
		result.instance();
		result->set_handle(req);
	} else {
		memdelete(ctx);
	}
	return result;
}

int EpicServices::titlestorage_interface_delete_cache(const Dictionary &p_options) {
	if (!title_storage_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_TitleStorage_DeleteCacheOptions opts = {};
	opts.ApiVersion = EOS_TITLESTORAGE_DELETECACHE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	struct Wrap {
		static void EOS_CALL cb(const EOS_TitleStorage_DeleteCacheCallbackInfo *) {}
	};
	return int(EOS_TitleStorage_DeleteCache(title_storage_handle, &opts, nullptr, &Wrap::cb));
}
