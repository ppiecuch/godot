/**************************************************************************/
/*  epic_file_transfer_request.cpp                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#include "epic_file_transfer_request.h"

#include "eos_playerdatastorage.h"
#include "eos_titlestorage.h"

// ---- PlayerDataStorage ---------------------------------------------------

void EpicPlayerDataStorageFileTransferRequest::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicPlayerDataStorageFileTransferRequest::is_valid);
	ClassDB::bind_method(D_METHOD("get_file_request_state"), &EpicPlayerDataStorageFileTransferRequest::get_file_request_state);
	ClassDB::bind_method(D_METHOD("get_filename"), &EpicPlayerDataStorageFileTransferRequest::get_filename);
	ClassDB::bind_method(D_METHOD("cancel_request"), &EpicPlayerDataStorageFileTransferRequest::cancel_request);
}

void EpicPlayerDataStorageFileTransferRequest::set_handle(EOS_HPlayerDataStorageFileTransferRequest p_handle) {
	if (handle && handle != p_handle) {
		EOS_PlayerDataStorageFileTransferRequest_Release(handle);
	}
	handle = p_handle;
}

EpicPlayerDataStorageFileTransferRequest::~EpicPlayerDataStorageFileTransferRequest() {
	if (handle) {
		EOS_PlayerDataStorageFileTransferRequest_Release(handle);
		handle = nullptr;
	}
}

int EpicPlayerDataStorageFileTransferRequest::get_file_request_state() const {
	if (!handle) {
		return int(EOS_EResult::EOS_NotFound);
	}
	return int(EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState(handle));
}

String EpicPlayerDataStorageFileTransferRequest::get_filename() const {
	if (!handle) {
		return String();
	}
	int32_t out_len = 0;
	if (EOS_PlayerDataStorageFileTransferRequest_GetFilename(handle, 0, nullptr, &out_len) != EOS_EResult::EOS_LimitExceeded) {
		// First call with NULL buffer should return EOS_LimitExceeded with required size in out_len.
		if (out_len <= 0) {
			return String();
		}
	}
	if (out_len <= 0) {
		return String();
	}
	CharString buf;
	buf.resize(out_len);
	if (EOS_PlayerDataStorageFileTransferRequest_GetFilename(handle, uint32_t(out_len), buf.ptrw(), &out_len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf.get_data());
}

int EpicPlayerDataStorageFileTransferRequest::cancel_request() {
	if (!handle) {
		return int(EOS_EResult::EOS_NotFound);
	}
	return int(EOS_PlayerDataStorageFileTransferRequest_CancelRequest(handle));
}

// ---- TitleStorage --------------------------------------------------------

void EpicTitleStorageFileTransferRequest::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &EpicTitleStorageFileTransferRequest::is_valid);
	ClassDB::bind_method(D_METHOD("get_file_request_state"), &EpicTitleStorageFileTransferRequest::get_file_request_state);
	ClassDB::bind_method(D_METHOD("get_filename"), &EpicTitleStorageFileTransferRequest::get_filename);
	ClassDB::bind_method(D_METHOD("cancel_request"), &EpicTitleStorageFileTransferRequest::cancel_request);
}

void EpicTitleStorageFileTransferRequest::set_handle(EOS_HTitleStorageFileTransferRequest p_handle) {
	if (handle && handle != p_handle) {
		EOS_TitleStorageFileTransferRequest_Release(handle);
	}
	handle = p_handle;
}

EpicTitleStorageFileTransferRequest::~EpicTitleStorageFileTransferRequest() {
	if (handle) {
		EOS_TitleStorageFileTransferRequest_Release(handle);
		handle = nullptr;
	}
}

int EpicTitleStorageFileTransferRequest::get_file_request_state() const {
	if (!handle) {
		return int(EOS_EResult::EOS_NotFound);
	}
	return int(EOS_TitleStorageFileTransferRequest_GetFileRequestState(handle));
}

String EpicTitleStorageFileTransferRequest::get_filename() const {
	if (!handle) {
		return String();
	}
	int32_t out_len = 0;
	EOS_TitleStorageFileTransferRequest_GetFilename(handle, 0, nullptr, &out_len);
	if (out_len <= 0) {
		return String();
	}
	CharString buf;
	buf.resize(out_len);
	if (EOS_TitleStorageFileTransferRequest_GetFilename(handle, uint32_t(out_len), buf.ptrw(), &out_len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf.get_data());
}

int EpicTitleStorageFileTransferRequest::cancel_request() {
	if (!handle) {
		return int(EOS_EResult::EOS_NotFound);
	}
	return int(EOS_TitleStorageFileTransferRequest_CancelRequest(handle));
}
