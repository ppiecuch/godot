/*************************************************************************/
/*  thread_posix.h                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#ifndef THREAD_POSIX_H
#define THREAD_POSIX_H

/**
	@author Juan Linietsky <reduzio@gmail.com>
*/

#if (defined(UNIX_ENABLED) || defined(PTHREAD_ENABLED)) && !defined(NO_THREADS)

#include "core/typedefs.h"

#include <pthread.h>
#include <sys/types.h>

class Thread {

private:
	static pthread_key_t thread_id_key;
	static ID next_thread_id;
	static ID main_thread_id;

	pthread_t pthread;
	pthread_attr_t pthread_attr;
	Callback callback;
	void *user;
	ID id;

public:
	friend class Main;

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

public:
	_FORCE_INLINE_ ID get_id() const { return id; }
	// get the ID of the caller thread
	static ID get_caller_id();
	// get the ID of the main thread
	_FORCE_INLINE_ static ID get_main_id() { return main_thread_id; }

	void start(Callback p_callback, void *p_user, const Settings &p_settings = Settings());
	bool is_started() const;
	Error set_name(const String &p_name);
	///< waits until thread is finished, and deallocates it.
	void wait_to_finish();

	ThreadPosix();
	~ThreadPosix();
};

#endif

#endif // THREAD_POSIX_H
