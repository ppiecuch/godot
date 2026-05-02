/**************************************************************************/
/*  playerdatastorage_interface.cpp                                       */
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

#include "eos_playerdatastorage.h"

#define PDS_GUARD_VOID(SIGNAL)                                                  \
	if (!player_data_storage_handle) {                                          \
		Dictionary payload;                                                     \
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured); \
		_emit_deferred(SIGNAL, payload);                                        \
		return;                                                                 \
	}

// ---- Read transfer state -------------------------------------------------
// EOS streams chunks via per-request callbacks. We accumulate them into a
// PoolByteArray held by a heap-allocated context struct that's freed by the
// completion callback.
struct _PDSReadCtx {
	ObjectID singleton_id;
	StringName signal_name;
	String filename;
	PoolByteArray buffer;
};

struct _PDSWriteCtx {
	ObjectID singleton_id;
	StringName signal_name;
	String filename;
	PoolByteArray buffer;
	int write_offset;
};

// ---- Query / metadata ----------------------------------------------------

static void EOS_CALL _on_pds_query_file(const EOS_PlayerDataStorage_QueryFileCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::playerdatastorage_interface_query_file(const Dictionary &p_options) {
	PDS_GUARD_VOID("playerdatastorage_interface_query_file_callback");
	const CharString filename_cs = dict_get_string(p_options, "filename").utf8();
	EOS_PlayerDataStorage_QueryFileOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_QUERYFILE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Filename = filename_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("playerdatastorage_interface_query_file_callback")));
	EOS_PlayerDataStorage_QueryFile(player_data_storage_handle, &opts, cb, &_on_pds_query_file);
}

static void EOS_CALL _on_pds_query_list(const EOS_PlayerDataStorage_QueryFileListCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["file_count"] = int(data->FileCount);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::playerdatastorage_interface_query_file_list(const Dictionary &p_options) {
	PDS_GUARD_VOID("playerdatastorage_interface_query_file_list_callback");
	EOS_PlayerDataStorage_QueryFileListOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_QUERYFILELIST_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("playerdatastorage_interface_query_file_list_callback")));
	EOS_PlayerDataStorage_QueryFileList(player_data_storage_handle, &opts, cb, &_on_pds_query_list);
}

static Dictionary _pds_meta_to_dict(const EOS_PlayerDataStorage_FileMetadata *m) {
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

Dictionary EpicServices::playerdatastorage_interface_copy_file_metadata_by_filename(const Dictionary &p_options) {
	Dictionary out;
	if (!player_data_storage_handle) {
		return out;
	}
	const CharString filename_cs = dict_get_string(p_options, "filename").utf8();
	EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_COPYFILEMETADATABYFILENAME_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Filename = filename_cs.get_data();
	EOS_PlayerDataStorage_FileMetadata *meta = nullptr;
	if (EOS_PlayerDataStorage_CopyFileMetadataByFilename(player_data_storage_handle, &opts, &meta) == EOS_EResult::EOS_Success && meta) {
		out = _pds_meta_to_dict(meta);
		EOS_PlayerDataStorage_FileMetadata_Release(meta);
	}
	return out;
}

int EpicServices::playerdatastorage_interface_get_file_metadata_count(const Dictionary &p_options) {
	if (!player_data_storage_handle) {
		return 0;
	}
	EOS_PlayerDataStorage_GetFileMetadataCountOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_GETFILEMETADATACOUNT_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	int32_t count = 0;
	EOS_PlayerDataStorage_GetFileMetadataCount(player_data_storage_handle, &opts, &count);
	return int(count);
}

Dictionary EpicServices::playerdatastorage_interface_copy_file_metadata_at_index(const Dictionary &p_options) {
	Dictionary out;
	if (!player_data_storage_handle) {
		return out;
	}
	EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_COPYFILEMETADATAATINDEX_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Index = uint32_t(dict_get_int(p_options, "index", 0));
	EOS_PlayerDataStorage_FileMetadata *meta = nullptr;
	if (EOS_PlayerDataStorage_CopyFileMetadataAtIndex(player_data_storage_handle, &opts, &meta) == EOS_EResult::EOS_Success && meta) {
		out = _pds_meta_to_dict(meta);
		EOS_PlayerDataStorage_FileMetadata_Release(meta);
	}
	return out;
}

static void EOS_CALL _on_pds_duplicate(const EOS_PlayerDataStorage_DuplicateFileCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::playerdatastorage_interface_duplicate_file(const Dictionary &p_options) {
	PDS_GUARD_VOID("playerdatastorage_interface_duplicate_file_callback");
	const CharString src_cs = dict_get_string(p_options, "source_filename").utf8();
	const CharString dst_cs = dict_get_string(p_options, "destination_filename").utf8();
	EOS_PlayerDataStorage_DuplicateFileOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_DUPLICATEFILE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.SourceFilename = src_cs.get_data();
	opts.DestinationFilename = dst_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("playerdatastorage_interface_duplicate_file_callback")));
	EOS_PlayerDataStorage_DuplicateFile(player_data_storage_handle, &opts, cb, &_on_pds_duplicate);
}

static void EOS_CALL _on_pds_delete(const EOS_PlayerDataStorage_DeleteFileCallbackInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::playerdatastorage_interface_delete_file(const Dictionary &p_options) {
	PDS_GUARD_VOID("playerdatastorage_interface_delete_file_callback");
	const CharString filename_cs = dict_get_string(p_options, "filename").utf8();
	EOS_PlayerDataStorage_DeleteFileOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_DELETEFILE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Filename = filename_cs.get_data();
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("playerdatastorage_interface_delete_file_callback")));
	EOS_PlayerDataStorage_DeleteFile(player_data_storage_handle, &opts, cb, &_on_pds_delete);
}

// ---- Streaming Read -------------------------------------------------------

static EOS_PlayerDataStorage_EReadResult EOS_CALL _pds_on_read_chunk(const EOS_PlayerDataStorage_ReadFileDataCallbackInfo *data) {
	_PDSReadCtx *ctx = (_PDSReadCtx *)data->ClientData;
	if (!ctx) {
		return EOS_PlayerDataStorage_EReadResult::EOS_RR_FailRequest;
	}
	const int prev = ctx->buffer.size();
	ctx->buffer.resize(prev + int(data->DataChunkLengthBytes));
	PoolByteArray::Write w = ctx->buffer.write();
	memcpy(w.ptr() + prev, data->DataChunk, data->DataChunkLengthBytes);
	return EOS_PlayerDataStorage_EReadResult::EOS_RR_ContinueReading;
}

static void EOS_CALL _pds_on_read_complete(const EOS_PlayerDataStorage_ReadFileCallbackInfo *data) {
	_PDSReadCtx *ctx = (_PDSReadCtx *)data->ClientData;
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

Ref<EpicPlayerDataStorageFileTransferRequest> EpicServices::playerdatastorage_interface_read_file(const Dictionary &p_options) {
	Ref<EpicPlayerDataStorageFileTransferRequest> result;
	if (!player_data_storage_handle) {
		return result;
	}
	const CharString filename_cs = dict_get_string(p_options, "filename").utf8();
	_PDSReadCtx *ctx = memnew(_PDSReadCtx);
	ctx->singleton_id = get_instance_id();
	ctx->signal_name = StringName("playerdatastorage_interface_read_file_callback");
	ctx->filename = String::utf8(filename_cs.get_data());

	EOS_PlayerDataStorage_ReadFileOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_READFILE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Filename = filename_cs.get_data();
	opts.ReadChunkLengthBytes = uint32_t(dict_get_int(p_options, "read_chunk_length_bytes", 16 * 1024));
	opts.ReadFileDataCallback = &_pds_on_read_chunk;
	opts.FileTransferProgressCallback = nullptr;

	EOS_HPlayerDataStorageFileTransferRequest req = EOS_PlayerDataStorage_ReadFile(player_data_storage_handle, &opts, ctx, &_pds_on_read_complete);
	if (req) {
		result.instance();
		result->set_handle(req);
	} else {
		memdelete(ctx);
	}
	return result;
}

// ---- Streaming Write ------------------------------------------------------

static EOS_PlayerDataStorage_EWriteResult EOS_CALL _pds_on_write_chunk(const EOS_PlayerDataStorage_WriteFileDataCallbackInfo *data, void *out_data_buffer, uint32_t *out_data_written) {
	_PDSWriteCtx *ctx = (_PDSWriteCtx *)data->ClientData;
	if (!ctx) {
		*out_data_written = 0;
		return EOS_PlayerDataStorage_EWriteResult::EOS_WR_FailRequest;
	}
	const uint32_t remaining = uint32_t(ctx->buffer.size()) - uint32_t(ctx->write_offset);
	const uint32_t n = MIN(remaining, data->DataBufferLengthBytes);
	if (n > 0) {
		PoolByteArray::Read r = ctx->buffer.read();
		memcpy(out_data_buffer, r.ptr() + ctx->write_offset, n);
		ctx->write_offset += int(n);
	}
	*out_data_written = n;
	if (ctx->write_offset >= ctx->buffer.size()) {
		return EOS_PlayerDataStorage_EWriteResult::EOS_WR_CompleteRequest;
	}
	return EOS_PlayerDataStorage_EWriteResult::EOS_WR_ContinueWriting;
}

static void EOS_CALL _pds_on_write_complete(const EOS_PlayerDataStorage_WriteFileCallbackInfo *data) {
	_PDSWriteCtx *ctx = (_PDSWriteCtx *)data->ClientData;
	if (!ctx) {
		return;
	}
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
	payload["filename"] = String::utf8(data->Filename ? data->Filename : "");
	epic_emit_deferred(ctx->singleton_id, ctx->signal_name, payload);
	memdelete(ctx);
}

Ref<EpicPlayerDataStorageFileTransferRequest> EpicServices::playerdatastorage_interface_write_file(const Dictionary &p_options) {
	Ref<EpicPlayerDataStorageFileTransferRequest> result;
	if (!player_data_storage_handle) {
		return result;
	}
	const CharString filename_cs = dict_get_string(p_options, "filename").utf8();
	_PDSWriteCtx *ctx = memnew(_PDSWriteCtx);
	ctx->singleton_id = get_instance_id();
	ctx->signal_name = StringName("playerdatastorage_interface_write_file_callback");
	ctx->filename = String::utf8(filename_cs.get_data());
	ctx->buffer = p_options.has("data") ? PoolByteArray(p_options["data"]) : PoolByteArray();
	ctx->write_offset = 0;

	EOS_PlayerDataStorage_WriteFileOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_WRITEFILE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.Filename = filename_cs.get_data();
	opts.ChunkLengthBytes = uint32_t(dict_get_int(p_options, "chunk_length_bytes", 16 * 1024));
	opts.WriteFileDataCallback = &_pds_on_write_chunk;
	opts.FileTransferProgressCallback = nullptr;

	EOS_HPlayerDataStorageFileTransferRequest req = EOS_PlayerDataStorage_WriteFile(player_data_storage_handle, &opts, ctx, &_pds_on_write_complete);
	if (req) {
		result.instance();
		result->set_handle(req);
	} else {
		memdelete(ctx);
	}
	return result;
}

int EpicServices::playerdatastorage_interface_delete_cache(const Dictionary &p_options) {
	if (!player_data_storage_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_PlayerDataStorage_DeleteCacheOptions opts = {};
	opts.ApiVersion = EOS_PLAYERDATASTORAGE_DELETECACHE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	struct Wrap {
		static void EOS_CALL cb(const EOS_PlayerDataStorage_DeleteCacheCallbackInfo *) {}
	};
	return int(EOS_PlayerDataStorage_DeleteCache(player_data_storage_handle, &opts, nullptr, &Wrap::cb));
}
