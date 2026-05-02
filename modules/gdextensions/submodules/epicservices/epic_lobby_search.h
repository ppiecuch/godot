/**************************************************************************/
/*  epic_lobby_search.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef EPIC_LOBBY_SEARCH_H
#define EPIC_LOBBY_SEARCH_H

#include "core/reference.h"

#include "eos_lobby_types.h"

class EpicLobbyDetails;

class EpicLobbySearch : public Reference {
	GDCLASS(EpicLobbySearch, Reference);

	EOS_HLobbySearch handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HLobbySearch p_handle);
	EOS_HLobbySearch get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	int set_lobby_id(const String &p_lobby_id);
	int set_target_user_id(const String &p_target_product_user_id);
	int set_max_results(int p_max);
	// Parameter dict: { key, value, value_type, comparison_op }
	int set_parameter(const Dictionary &p_param);
	int remove_parameter(const String &p_key, int p_comparison_op);

	void find(const Dictionary &p_options); // emits "lobby_search_find_callback"

	int get_search_result_count();
	Ref<EpicLobbyDetails> copy_search_result_by_index(int p_index);

	EpicLobbySearch() {}
	~EpicLobbySearch();
};

#endif // EPIC_LOBBY_SEARCH_H
