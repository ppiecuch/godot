/*************************************************************************/
/*  thread_3ds.cpp                                                       */
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

#ifdef _3DS

#include "thread_3ds.h"
#include "core/os/memory.h"
#include "core/script_language.h"
#include "core/ustring.h"
#include "platform/3ds/os_3ds.h"

////////////
/* Thread */
////////////

Thread::ID Thread::main_thread_id = Thread::get_thread_id();
Thread::ID Thread::next_thread_id = 0;
static thread_local Thread::ID caller_id = 0;
static thread_local bool caller_id_cached = false;

void *Thread::thread_callback(void *p_userdata) {
	Thread *t = reinterpret_cast<Thread *>(p_userdata);
	t->id = AtomicIncrement(&next_thread_id);

	caller_id = t->get_id();
	caller_id_cached = true;

	ScriptServer::thread_enter(); //scripts may need to attach a stack

	t->callback(t->user);

	ScriptServer::thread_exit();

	return NULL;
}

void Thread::start(Thread::Callback p_callback, void *p_user, const Thread::Settings &p_settings) {
	// Get base thread priority for relative priority setting
	int32_t priority;
	svcGetThreadPriority(&priority, (Thread::main_thread_id == 0) ? CUR_THREAD_HANDLE : Thread::main_thread_id);

	if (p_settings.priority == PRIORITY_LOW)
		priority++;
	else if (p_settings.priority == PRIORITY_HIGH)
		priority--;

	callback = p_callback;
	user = p_user;

	thread = threadCreate(p_callback, this, 64 * 1024, priority, -1, false);
}

bool Thread::is_started() const {
	return thread != 0;
}

void Thread::wait_to_finish() {
	threadJoin(thread, U64_MAX);
	threadFree(thread);
}

Thread::ID Thread::get_thread_id() {
	if (!threadGetCurrent())
		return CUR_THREAD_HANDLE;
	return threadGetHandle(threadGetCurrent());
}

Thread::ID Thread::get_caller_id() {
	if (likely(caller_id_cached)) {
		return caller_id;
	} else {
		caller_id = Thread::get_thread_id();
		caller_id_cached = true;
		return caller_id;
	}
}

Thread::Thread() {
	thread = 0;
	id = 0;
}

Thread::~Thread() {
}

///////////
/* Mutex */
///////////

Mutex::Mutex(bool p_recursive) :
		is_recursive(p_recursive) {
	if (is_recursive)
		RecursiveLock_Init(&recursiveLock);
	else
		LightLock_Init(&lightLock);
}

Mutex::~Mutex() {
}

void Mutex::lock() const {
	if (is_recursive)
		RecursiveLock_Lock(&recursiveLock);
	else
		LightLock_Lock(&lightLock);
}

void Mutex::unlock() const {
	if (is_recursive)
		RecursiveLock_Unlock(&recursiveLock);
	else
		LightLock_Unlock(&lightLock);
}

Error Mutex::try_lock() const {
	int ret;
	if (is_recursive)
		ret = RecursiveLock_TryLock(&recursiveLock);
	else
		ret = LightLock_TryLock(&lightLock);

	return (ret == 0) ? OK : ERR_BUSY;
}

#endif // _3DS
