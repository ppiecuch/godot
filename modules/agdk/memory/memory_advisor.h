/**************************************************************************/
/*  memory_advisor.h                                                      */
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

#ifndef MEMORY_ADVISOR_H
#define MEMORY_ADVISOR_H

#include "core/os/os.h"
#include "core/variant.h"

#include <jni.h>

class MemoryAdvisor {
	bool _initialized;
	int _cached_state;
	int _last_emitted_state;
	uint64_t _last_poll_usec;

	// Poll every 2 seconds.
	static const uint64_t POLL_INTERVAL_USEC = 2000000ULL;

	// Function pointers loaded via dlsym.
	typedef int (*PFN_MemoryAdvice_init)(JNIEnv *, jobject);
	typedef int (*PFN_MemoryAdvice_getMemoryState)();

	PFN_MemoryAdvice_init _fn_init;
	PFN_MemoryAdvice_getMemoryState _fn_getMemoryState;

	bool _load_functions();

public:
	Error init(JNIEnv *p_env, jobject p_activity);
	void update();

	int get_memory_state() const;
	bool is_initialized() const;

	MemoryAdvisor();
	~MemoryAdvisor();
};

#endif // MEMORY_ADVISOR_H
