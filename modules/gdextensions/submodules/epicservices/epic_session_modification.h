/**************************************************************************/
/*  epic_session_modification.h                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#ifndef EPIC_SESSION_MODIFICATION_H
#define EPIC_SESSION_MODIFICATION_H

#include "core/reference.h"

#include "eos_sessions_types.h"

class EpicSessionModification : public Reference {
	GDCLASS(EpicSessionModification, Reference);

	EOS_HSessionModification handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HSessionModification p_handle);
	EOS_HSessionModification get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	int set_bucket_id(const String &p_bucket_id);
	int set_max_players(int p_count);
	int set_host_address(const String &p_host_address);
	int set_permission_level(int p_level);
	int set_join_in_progress_allowed(bool p_allowed);
	int set_invites_allowed(bool p_allowed);
	int add_attribute(const Dictionary &p_attr);
	int remove_attribute(const String &p_key);

	EpicSessionModification() {}
	~EpicSessionModification();
};

#endif // EPIC_SESSION_MODIFICATION_H
