/**************************************************************************/
/*  epic_utils.h                                                          */
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

#ifndef EPIC_UTILS_H
#define EPIC_UTILS_H

#include "core/dictionary.h"
#include "core/ustring.h"
#include "core/variant.h"

#include "eos_common.h"

// EOS_EpicAccountId / EOS_ProductUserId / EOS_ContinuanceToken stringification
// helpers. Returns empty String for null / invalid handles.
String eos_eaid_to_string(EOS_EpicAccountId p_id);
String eos_pui_to_string(EOS_ProductUserId p_id);
String eos_continuance_token_to_string(EOS_ContinuanceToken p_token);

EOS_EpicAccountId eos_eaid_from_string(const String &p_str);
EOS_ProductUserId eos_pui_from_string(const String &p_str);

// Dictionary accessors with defaults — for option-struct construction. Dict
// values are passed in from GDScript and may be missing or wrong-typed.
String dict_get_string(const Dictionary &p_dict, const StringName &p_key, const String &p_default = String());
int dict_get_int(const Dictionary &p_dict, const StringName &p_key, int p_default = 0);
int64_t dict_get_int64(const Dictionary &p_dict, const StringName &p_key, int64_t p_default = 0);
bool dict_get_bool(const Dictionary &p_dict, const StringName &p_key, bool p_default = false);
double dict_get_double(const Dictionary &p_dict, const StringName &p_key, double p_default = 0.0);

#endif // EPIC_UTILS_H
