/**************************************************************************/
/*  epic_continuance_token.h                                              */
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

#ifndef EPIC_CONTINUANCE_TOKEN_H
#define EPIC_CONTINUANCE_TOKEN_H

#include "core/reference.h"

#include "eos_common.h"

// Reference-wrapped EOS_ContinuanceToken so GDScript can hold the opaque
// pointer between an Auth/Connect login callback (which received the
// token) and a follow-up LinkAccount / CreateUser call. Tokens cannot be
// reconstructed from their string form — the caller must keep the live
// instance.
class EpicContinuanceToken : public Reference {
	GDCLASS(EpicContinuanceToken, Reference);

	EOS_ContinuanceToken token = nullptr;

protected:
	static void _bind_methods();

public:
	void set_handle(EOS_ContinuanceToken p_handle) { token = p_handle; }
	EOS_ContinuanceToken get_handle() const { return token; }
	bool is_valid() const { return token != nullptr; }
	String to_string_value() const;

	EpicContinuanceToken() {}
};

#endif // EPIC_CONTINUANCE_TOKEN_H
