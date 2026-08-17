/**************************************************************************/
/*  trackergen_sfx_ring.h                                                 */
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

#ifndef TRACKERGEN_SFX_RING_H
#define TRACKERGEN_SFX_RING_H

#include "core/typedefs.h"

#include <atomic>

// A single SFX trigger request handed from the game (main) thread to the audio
// thread. POD, so it copies trivially into the ring.
struct SfxTrigger {
	int inst_index; // index into the stream's sfx_kit
	float freq;
	float velocity;
};

// Lock-free single-producer / single-consumer ring: the game thread pushes()
// triggers, the audio thread pop()s them in mix(). No allocation, no locks — the
// only correct way to signal the audio callback. CAP must be a power of two.
class SfxRing {
	enum { CAP = 64 };
	SfxTrigger buf[CAP];
	std::atomic<uint32_t> head; // consumer (audio thread)
	std::atomic<uint32_t> tail; // producer (game thread)

public:
	// Producer side (game thread). Returns false if full (event dropped).
	bool push(const SfxTrigger &p_e) {
		const uint32_t t = tail.load(std::memory_order_relaxed);
		const uint32_t next = (t + 1) & (CAP - 1);
		if (next == head.load(std::memory_order_acquire)) {
			return false; // full
		}
		buf[t] = p_e;
		tail.store(next, std::memory_order_release);
		return true;
	}

	// Consumer side (audio thread). Returns false if empty.
	bool pop(SfxTrigger &r_e) {
		const uint32_t h = head.load(std::memory_order_relaxed);
		if (h == tail.load(std::memory_order_acquire)) {
			return false; // empty
		}
		r_e = buf[h];
		head.store((h + 1) & (CAP - 1), std::memory_order_release);
		return true;
	}

	SfxRing() :
			head(0), tail(0) {}
};

#endif // TRACKERGEN_SFX_RING_H
