/**************************************************************************/
/*  epic_utils.cpp                                                        */
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

#include "epic_utils.h"

String eos_eaid_to_string(EOS_EpicAccountId p_id) {
	if (!p_id || EOS_EpicAccountId_IsValid(p_id) == EOS_FALSE) {
		return String();
	}
	char buf[EOS_EPICACCOUNTID_MAX_LENGTH + 1] = { 0 };
	int32_t len = sizeof(buf);
	if (EOS_EpicAccountId_ToString(p_id, buf, &len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf);
}

String eos_pui_to_string(EOS_ProductUserId p_id) {
	if (!p_id || EOS_ProductUserId_IsValid(p_id) == EOS_FALSE) {
		return String();
	}
	char buf[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
	int32_t len = sizeof(buf);
	if (EOS_ProductUserId_ToString(p_id, buf, &len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(buf);
}

String eos_continuance_token_to_string(EOS_ContinuanceToken p_token) {
	if (!p_token) {
		return String();
	}
	int32_t len = 0;
	// Probe required length; SDK fills InOutBufferLength with required size.
	EOS_ContinuanceToken_ToString(p_token, nullptr, &len);
	if (len <= 0) {
		return String();
	}
	CharString cs;
	cs.resize(len);
	if (EOS_ContinuanceToken_ToString(p_token, cs.ptrw(), &len) != EOS_EResult::EOS_Success) {
		return String();
	}
	return String::utf8(cs.get_data());
}

EOS_EpicAccountId eos_eaid_from_string(const String &p_str) {
	if (p_str.empty()) {
		return nullptr;
	}
	CharString cs = p_str.utf8();
	return EOS_EpicAccountId_FromString(cs.get_data());
}

EOS_ProductUserId eos_pui_from_string(const String &p_str) {
	if (p_str.empty()) {
		return nullptr;
	}
	CharString cs = p_str.utf8();
	return EOS_ProductUserId_FromString(cs.get_data());
}

String dict_get_string(const Dictionary &p_dict, const StringName &p_key, const String &p_default) {
	if (!p_dict.has(p_key)) {
		return p_default;
	}
	return String(p_dict[p_key]);
}

int dict_get_int(const Dictionary &p_dict, const StringName &p_key, int p_default) {
	if (!p_dict.has(p_key)) {
		return p_default;
	}
	return int(p_dict[p_key]);
}

int64_t dict_get_int64(const Dictionary &p_dict, const StringName &p_key, int64_t p_default) {
	if (!p_dict.has(p_key)) {
		return p_default;
	}
	return int64_t(p_dict[p_key]);
}

bool dict_get_bool(const Dictionary &p_dict, const StringName &p_key, bool p_default) {
	if (!p_dict.has(p_key)) {
		return p_default;
	}
	return bool(p_dict[p_key]);
}

double dict_get_double(const Dictionary &p_dict, const StringName &p_key, double p_default) {
	if (!p_dict.has(p_key)) {
		return p_default;
	}
	return double(p_dict[p_key]);
}
