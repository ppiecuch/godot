/**************************************************************************/
/*  epic_session_search.h                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/**************************************************************************/

#ifndef EPIC_SESSION_SEARCH_H
#define EPIC_SESSION_SEARCH_H

#include "core/reference.h"

#include "eos_sessions_types.h"

class EpicSessionDetails;

class EpicSessionSearch : public Reference {
	GDCLASS(EpicSessionSearch, Reference);

	EOS_HSessionSearch handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HSessionSearch p_handle);
	EOS_HSessionSearch get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	int set_session_id(const String &p_session_id);
	int set_target_user_id(const String &p_target);
	int set_max_results(int p_max);
	int set_parameter(const Dictionary &p_param);
	int remove_parameter(const String &p_key, int p_comparison_op);

	void find(const Dictionary &p_options); // emits "session_search_find_callback"

	int get_search_result_count();
	Ref<EpicSessionDetails> copy_search_result_by_index(int p_index);

	EpicSessionSearch() {}
	~EpicSessionSearch();
};

#endif // EPIC_SESSION_SEARCH_H
