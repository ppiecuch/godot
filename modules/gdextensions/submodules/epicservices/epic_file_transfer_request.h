/**************************************************************************/
/*  epic_file_transfer_request.h                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#ifndef EPIC_FILE_TRANSFER_REQUEST_H
#define EPIC_FILE_TRANSFER_REQUEST_H

#include "core/reference.h"

#include "eos_playerdatastorage_types.h"
#include "eos_titlestorage_types.h"

// Two thin wrappers — PDS and TitleStorage have parallel APIs, so we just
// define both classes side by side. The classes own their handles and call
// the matching *_Release in the destructor; cancel/state/filename calls
// route to the SDK through the handle.

class EpicPlayerDataStorageFileTransferRequest : public Reference {
	GDCLASS(EpicPlayerDataStorageFileTransferRequest, Reference);
	EOS_HPlayerDataStorageFileTransferRequest handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HPlayerDataStorageFileTransferRequest p_handle);
	EOS_HPlayerDataStorageFileTransferRequest get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	int get_file_request_state() const;
	String get_filename() const;
	int cancel_request();

	EpicPlayerDataStorageFileTransferRequest() {}
	~EpicPlayerDataStorageFileTransferRequest();
};

class EpicTitleStorageFileTransferRequest : public Reference {
	GDCLASS(EpicTitleStorageFileTransferRequest, Reference);
	EOS_HTitleStorageFileTransferRequest handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HTitleStorageFileTransferRequest p_handle);
	EOS_HTitleStorageFileTransferRequest get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	int get_file_request_state() const;
	String get_filename() const;
	int cancel_request();

	EpicTitleStorageFileTransferRequest() {}
	~EpicTitleStorageFileTransferRequest();
};

#endif // EPIC_FILE_TRANSFER_REQUEST_H
