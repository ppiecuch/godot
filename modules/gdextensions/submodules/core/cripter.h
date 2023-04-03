/**************************************************************************/
/*  cripter.h                                                             */
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

#ifndef CRIPTER_H
#define CRIPTER_H

#include "core/reference.h"
#include "core/variant.h"

class Cripter : public Reference {
	GDCLASS(Cripter, Reference);

private:
	PoolByteArray encode_var(const Variant p_data) const;
	Variant decode_var(const PoolByteArray p_data) const;
	PoolByteArray char2pool(const uint8_t *p_in, const size_t p_size) const;

protected:
	static void _bind_methods();

public:
	//CBC
	PoolByteArray encrypt_byte_CBC(const PoolByteArray p_input, const String p_key) const;
	PoolByteArray decrypt_byte_CBC(const PoolByteArray p_input, const String p_key) const;
	PoolByteArray encrypt_var_CBC(const Variant p_input, const String p_key) const;
	Variant decrypt_var_CBC(const PoolByteArray p_input, const String p_key) const;
	//GCM
	PoolByteArray encrypt_byte_GCM(const PoolByteArray p_input, const String p_key, const String p_add = "") const;
	PoolByteArray decrypt_byte_GCM(const PoolByteArray p_input, const String p_key, const String p_add = "") const;
	PoolByteArray encrypt_var_GCM(const Variant p_input, const String p_key, const String p_add = "") const;
	Variant decrypt_var_GCM(const PoolByteArray p_input, const String p_key, const String p_add = "") const;
	//RSA
	PoolByteArray encrypt_byte_RSA(const PoolByteArray p_input, String p_key_path) const;
	PoolByteArray decrypt_byte_RSA(const PoolByteArray p_input, const String p_key_path, const String p_password) const;
	PoolByteArray encrypt_var_RSA(const Variant p_input, const String p_key_path) const;
	Variant decrypt_var_RSA(const PoolByteArray p_input, const String p_key_path, const String p_password) const;

	Cripter();
};

#endif // CRIPTER_H
