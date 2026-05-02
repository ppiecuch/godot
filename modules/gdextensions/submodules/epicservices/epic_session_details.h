/**************************************************************************/
/*  epic_session_details.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#ifndef EPIC_SESSION_DETAILS_H
#define EPIC_SESSION_DETAILS_H

#include "core/reference.h"

#include "eos_sessions_types.h"

class EpicSessionDetails : public Reference {
	GDCLASS(EpicSessionDetails, Reference);

	EOS_HSessionDetails handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HSessionDetails p_handle);
	EOS_HSessionDetails get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	Dictionary copy_info() const;
	int get_session_attribute_count() const;
	Dictionary copy_session_attribute_by_index(int p_index) const;
	Dictionary copy_session_attribute_by_key(const String &p_key) const;

	EpicSessionDetails() {}
	~EpicSessionDetails();
};

#endif // EPIC_SESSION_DETAILS_H
