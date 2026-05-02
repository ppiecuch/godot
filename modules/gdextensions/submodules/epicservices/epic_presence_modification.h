/**************************************************************************/
/*  epic_presence_modification.h                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#ifndef EPIC_PRESENCE_MODIFICATION_H
#define EPIC_PRESENCE_MODIFICATION_H

#include "core/reference.h"

#include "eos_presence_types.h"

class EpicPresenceModification : public Reference {
	GDCLASS(EpicPresenceModification, Reference);

	EOS_HPresenceModification handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HPresenceModification p_handle);
	EOS_HPresenceModification get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	int set_status(int p_status);
	int set_raw_rich_text(const String &p_text);
	int set_data(const Dictionary &p_kv); // {key:value, ...} string-only
	int delete_data(const Array &p_keys);
	int set_join_info(const String &p_join_info);

	EpicPresenceModification() {}
	~EpicPresenceModification();
};

#endif // EPIC_PRESENCE_MODIFICATION_H
