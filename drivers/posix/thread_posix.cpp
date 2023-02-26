/**************************************************************************/
/*  thread_posix.cpp                                                      */
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

#if (defined(UNIX_ENABLED) || defined(PTHREAD_ENABLED)) && !defined(NO_THREADS)

#include <pthread.h>

#ifdef PTHREAD_BSD_SET_NAME
#include <pthread_np.h>
#endif

#include "core/os/memory.h"
#include "core/safe_refcount.h"
#include "core/script_language.h"
#include "core/ustring.h"
#include "thread_posix.h"

#if defined(__psp__)
#include <pte_osal.h>
#define ATOMIC_INCREMENT pte_osAtomicIncrement
#elif defined(__psp2__)
#include <atomic>
#define ATOMIC_INCREMENT(p) __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST)
#else
#define ATOMIC_INCREMENT atomic_increment
#endif

static void _thread_id_key_destr_callback(void *p_value) {
	memdelete(static_cast<PosixThread::ID *>(p_value));
}

static pthread_key_t _create_thread_id_key() {
	pthread_key_t key;
	pthread_key_create(&key, &_thread_id_key_destr_callback);
	return key;
}

pthread_key_t PosixThread::thread_id_key = _create_thread_id_key();
PosixThread::ID PosixThread::main_thread_id = PosixThread::get_thread_id();
PosixThread::ID PosixThread::next_thread_id = 0;
static thread_local PosixThread::ID caller_id = 0;
static thread_local bool caller_id_cached = false;

void *PosixThread::thread_callback(void *p_userdata) {
	PosixThread *t = reinterpret_cast<PosixThread *>(p_userdata);
	t->id = ATOMIC_INCREMENT(&next_thread_id);
	pthread_setspecific(thread_id_key, (void *)memnew(ID(t->id)));

	caller_id = t->get_id();
	caller_id_cached = true;

	ScriptServer::thread_enter(); //scripts may need to attach a stack

	t->callback(t->user);

	ScriptServer::thread_exit();

	return NULL;
}

void PosixThread::start(PosixThread::Callback p_callback, void *p_user, const Settings &p_settings) {
	if (pthread != 0) {
#ifdef DEBUG_ENABLED
		WARN_PRINT("A Thread object has been re-started without wait_to_finish() having been called on it. Please do so to ensure correct cleanup of the thread.");
#endif
		pthread_detach(pthread);
		pthread = 0;
	}

	callback = p_callback;
	user = p_user;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setstacksize(&pthread_attr, 256 * 1024);

	pthread_create(&pthread, &pthread_attr, thread_callback, this);
}

PosixThread::ID PosixThread::get_thread_id() {
	void *value = pthread_getspecific(thread_id_key);

	if (value)
		return *static_cast<ID *>(value);

	ID new_id = ATOMIC_INCREMENT(&next_thread_id);
	pthread_setspecific(thread_id_key, (void *)memnew(ID(new_id)));
	return new_id;
}

PosixThread::ID PosixThread::get_caller_id() {
	if (likely(caller_id_cached)) {
		return caller_id;
	} else {
		caller_id = PosixThread::get_thread_id();
		caller_id_cached = true;
		return caller_id;
	}
}

bool PosixThread::is_started() const {
	return pthread != 0;
}

void PosixThread::wait_to_finish() {
	pthread_join(pthread, NULL);
	pthread = 0;
}

Error PosixThread::set_name(const String &p_name) {
#ifdef PTHREAD_NO_RENAME
	return ERR_UNAVAILABLE;

#else

#ifdef PTHREAD_RENAME_SELF

	// check if thread is the same as caller
	int err = pthread_setname_np(p_name.utf8().get_data());

#else

	pthread_t running_thread = pthread_self();
#ifdef PTHREAD_BSD_SET_NAME
	pthread_set_name_np(running_thread, p_name.utf8().get_data());
	int err = 0; // Open/FreeBSD ignore errors in this function
#else
	int err = pthread_setname_np(running_thread, p_name.utf8().get_data());
#endif // PTHREAD_BSD_SET_NAME

#endif // PTHREAD_RENAME_SELF

	return err == 0 ? OK : ERR_INVALID_PARAMETER;

#endif // PTHREAD_NO_RENAME
}

PosixThread::PosixThread() {
	pthread = 0;
}

PosixThread::~PosixThread() {
}

#endif
