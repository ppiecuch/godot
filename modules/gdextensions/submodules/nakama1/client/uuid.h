//
// uuid.hpp
//
// Header file for the UUID generator class.
//

#pragma once

#ifndef UUID_H
#define UUID_H

#include "core/variant.h"

#include <inttypes.h>
#include <utility>
#include <map>

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
