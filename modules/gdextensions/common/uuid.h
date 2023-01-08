/*************************************************************************/
/*  uuid.h                                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/// UUID generator class.

#pragma once

#ifndef UUID_H
#define UUID_H

#include "core/variant.h"
#include "core/int_types.h"

#include <map>
#include <utility>

class Uuid {
public:
	Uuid();
	Uuid(uint32_t time_low, uint16_t time_mid, uint16_t time_hi_version, uint8_t clock_seq_low, uint8_t clock_seq_hi_variant, uint64_t node);
	String bytes();
	String bytes_le();
	std::map<String, uint64_t> fields();
	String hex();
	std::pair<uint64_t, uint64_t> integer();
	bool is_valid() const;

private:
	static const uint64_t version_ = 1;
	// Store the 128-bit UUID as two 64-bit integers.
	uint64_t upper_ = 0;
	uint64_t lower_ = 0;

	static uint16_t getclockseq();
};

////////////////////////////////////////////////////////////////////////////////
// Generate a UUID from a host ID, sequence number, and the current time.
// If node is not given, getnode() is used to obtain the hardware address. If
// clock_seq is given, it is used as the sequence number; otherwise a random
// 14-bit sequence number is chosen.

Uuid uuid1();
Uuid uuid1(uint64_t node);
Uuid uuid1(uint64_t node, uint16_t clock_seq);

#endif // UUID_H
