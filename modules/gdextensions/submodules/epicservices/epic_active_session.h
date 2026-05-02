/**************************************************************************/
/*  epic_active_session.h                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#ifndef EPIC_ACTIVE_SESSION_H
#define EPIC_ACTIVE_SESSION_H

#include "core/reference.h"

#include "eos_sessions_types.h"

class EpicActiveSession : public Reference {
	GDCLASS(EpicActiveSession, Reference);

	EOS_HActiveSession handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HActiveSession p_handle);
	EOS_HActiveSession get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	Dictionary copy_info() const;
	int get_registered_player_count() const;
	String get_registered_player_by_index(int p_index) const;
	int get_state() const;

	EpicActiveSession() {}
	~EpicActiveSession();
};

#endif // EPIC_ACTIVE_SESSION_H
