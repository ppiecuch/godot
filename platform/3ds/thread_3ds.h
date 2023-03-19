/**************************************************************************/
/*  thread_3ds.h                                                          */
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

#ifndef THREAD_3DS_H
#define THREAD_3DS_H

#include "core/error_list.h"
#include "core/typedefs.h"

extern "C" {
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#define Thread Thread3DS
#include <3ds/thread.h>
#undef Thread
#include <3ds/types.h>
}

class String;

class Thread {
	friend class Main;

public:
	typedef void (*Callback)(void *p_userdata);

	typedef uint64_t ID;

	enum Priority {
		PRIORITY_LOW,
		PRIORITY_NORMAL,
		PRIORITY_HIGH
	};

	struct Settings {
		Priority priority;
		Settings() { priority = PRIORITY_NORMAL; }
	};

	static void *thread_callback(void *p_userdata);
	static ID get_thread_id();
	static Error set_name(const String &p_name) { return ERR_UNAVAILABLE; }

	void start(Callback p_callback, void *p_user, const Settings &p_settings = Settings());
	bool is_started() const;
	void wait_to_finish();
	ID get_id() const { return id; }
	// get the ID of the caller thread
	static ID get_caller_id();
	// get the ID of the main thread
	_FORCE_INLINE_ static ID get_main_id() { return main_thread_id; }

	Thread();
	~Thread();

private:
	Thread3DS thread;
	Callback callback;
	void *user;
	ID id;

	static ID next_thread_id;
	static ID main_thread_id;
};

class Mutex {
	const bool is_recursive;
	mutable LightLock lightLock;
	mutable RecursiveLock recursiveLock;

public:
	void lock() const;
	void unlock() const;
	Error try_lock() const;

	Mutex(bool p_recursive = true);
	~Mutex();
};

class Semaphore {
public:
	Error wait() const { return OK; };
	Error post() const { return OK; };
	int get() const { return 0; }; ///< get semaphore value
};

class RWLock {
public:
	void read_lock() const {}
	void read_unlock() const {}
	Error read_try_lock() const { return OK; }

	void write_lock() {}
	void write_unlock() {}
	Error write_try_lock() { return OK; }
};

#endif // THREAD_3DS_H
