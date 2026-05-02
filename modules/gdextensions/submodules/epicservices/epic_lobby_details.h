/**************************************************************************/
/*  epic_lobby_details.h                                                  */
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

#ifndef EPIC_LOBBY_DETAILS_H
#define EPIC_LOBBY_DETAILS_H

#include "core/reference.h"

#include "eos_lobby_types.h"

// Reference wrapper around EOS_HLobbyDetails. Calls EOS_LobbyDetails_Release
// in dtor. GDScript can read lobby info (owner, member count, attributes,
// permission level) without holding the raw pointer.
class EpicLobbyDetails : public Reference {
	GDCLASS(EpicLobbyDetails, Reference);

	EOS_HLobbyDetails handle = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_HLobbyDetails p_handle);
	EOS_HLobbyDetails get_handle() const { return handle; }
	bool is_valid() const { return handle != nullptr; }

	String get_lobby_id() const;
	String get_lobby_owner() const;
	Dictionary copy_info() const;

	int get_member_count() const;
	String get_member_by_index(int p_index) const;
	int get_member_attribute_count(const String &p_target_user_id) const;
	Dictionary copy_member_attribute_by_index(const String &p_target_user_id, int p_index) const;
	Dictionary copy_member_attribute_by_key(const String &p_target_user_id, const String &p_key) const;

	int get_attribute_count() const;
	Dictionary copy_attribute_by_index(int p_index) const;
	Dictionary copy_attribute_by_key(const String &p_key) const;

	EpicLobbyDetails() {}
	~EpicLobbyDetails();
};

#endif // EPIC_LOBBY_DETAILS_H
