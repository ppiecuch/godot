/**************************************************************************/
/*  memory_advisor.cpp                                                    */
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

#include "memory_advisor.h"

#include "../agdk_manager.h"
#include "core/print_string.h"

#include <dlfcn.h>

bool MemoryAdvisor::_load_functions() {
	void *lib = dlopen("libmemory_advice.so", RTLD_NOW);
	if (!lib) {
		return false;
	}

	_fn_init = (PFN_MemoryAdvice_init)dlsym(lib, "MemoryAdvice_init");
	_fn_getMemoryState = (PFN_MemoryAdvice_getMemoryState)dlsym(lib, "MemoryAdvice_getMemoryState");

	return _fn_init && _fn_getMemoryState;
}

Error MemoryAdvisor::init(JNIEnv *p_env, jobject p_activity) {
	if (_initialized) {
		return OK;
	}

	if (!_load_functions()) {
		print_line("AGDK Memory: Memory Advice library not available.");
		return ERR_UNAVAILABLE;
	}

	int result = _fn_init(p_env, p_activity);
	if (result != 0) {
		print_line("AGDK Memory: Failed to initialize Memory Advice API.");
		return ERR_CANT_CREATE;
	}

	_initialized = true;
	_last_poll_usec = 0;
	_cached_state = 0; // OK
	_last_emitted_state = 0;

	print_line("AGDK Memory: Advisor initialized.");
	return OK;
}

void MemoryAdvisor::update() {
	if (!_initialized || !_fn_getMemoryState) {
		return;
	}

	uint64_t now = OS::get_singleton()->get_ticks_usec();
	if (now - _last_poll_usec < POLL_INTERVAL_USEC) {
		return;
	}
	_last_poll_usec = now;

	int state = _fn_getMemoryState();
	_cached_state = state;

	if (state != _last_emitted_state) {
		_last_emitted_state = state;
		AGDKManager *mgr = AGDKManager::get_singleton();
		if (mgr) {
			mgr->emit_signal("memory_state_changed", state);
		}
	}
}

int MemoryAdvisor::get_memory_state() const {
	return _cached_state;
}

bool MemoryAdvisor::is_initialized() const {
	return _initialized;
}

MemoryAdvisor::MemoryAdvisor() {
	_initialized = false;
	_cached_state = 0;
	_last_emitted_state = 0;
	_last_poll_usec = 0;
	_fn_init = nullptr;
	_fn_getMemoryState = nullptr;
}

MemoryAdvisor::~MemoryAdvisor() {
	// No explicit destroy needed for Memory Advice API.
}
