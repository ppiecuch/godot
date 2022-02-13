/*************************************************************************/
/*  java_activity_result_receiver.h                                      */
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

#include "core/map.h"
#include "core/vector.h"
#include "core/os/mutex.h"

#include "java_activity_result_receiver.h"


static int unique_activity_request_code() {
	static Mutex mutex;
	static int request_code = 0x1000; // Reserve all request codes under 0x1000.

	MutexLock locker(mutex);
	if (request_code == INT_MAX)
		WARN_PRINT("Unique activity request code has wrapped. Unexpected behavior may occur.");
	return request_code++;
}

class ActivityResultListener {
public:
	virtual ~ActivityResultListener();
	virtual bool handleActivityResult(jint request_code, jint result_code, jobject data) = 0;
};

static void registerActivityResultListener(ActivityResultListener *listener);
static void unregisterActivityResultListener(ActivityResultListener *listener);

class ActivityResultReceiver::ActivityResultReceiverPrivate : public ActivityResultListener {
public:
	ActivityResultReceiver *q;
	mutable Map<int, int> local_to_global_request_code;
	mutable Map<int, int> global_to_local_request_code;

	static ActivityResultReceiverPrivate *get(ActivityResultReceiver *public_object) {
		return public_object->imp.get();
	}
	int globalRequestCode(int local_request_code) const {
		if (!local_to_global_request_code.has(local_request_code)) {
			const int global_request_code = unique_activity_request_code();
			local_to_global_request_code[local_request_code] = global_request_code;
			global_to_local_request_code[global_request_code] = local_request_code;
		}
		return local_to_global_request_code.get(local_request_code);
	}
	bool handleActivityResult(jint request_code, jint result_code, jobject data) {
		if (global_to_local_request_code.has(request_code)) {
			q->handleActivityResult(global_to_local_request_code.get(request_code), result_code, data);
			return true;
		}
		return false;
	}
	ActivityResultReceiverPrivate(ActivityResultReceiver *receiver) : q(receiver) { }
};

int ActivityResultReceiver::getGlobalRequestCode(ActivityResultReceiver *public_object, int request_code) {
	return public_object->imp->globalRequestCode(request_code);
}

ActivityResultReceiver::ActivityResultReceiver() : imp(new ActivityResultReceiverPrivate(this)) {
	registerActivityResultListener(imp.get());
}

ActivityResultReceiver::~ActivityResultReceiver() {
	unregisterActivityResultListener(imp.get());
}


namespace {
	struct {
		Mutex mutex;
		Vector<ActivityResultListener *> listeners;
	}  g_listeners;
}

static void registerActivityResultListener(ActivityResultListener *listener) {
	MutexLock locker(g_listeners.mutex);
	g_listeners.listeners.push_back(listener);
}

static void unregisterActivityResultListener(ActivityResultListener *listener) {
	MutexLock locker(g_listeners.mutex);
	g_listeners.listeners.erase(listener);
}

void processActivityResult(jint request_code, jint result_code, jobject data) {
	MutexLock locker(g_listeners.mutex);
	const Vector<ActivityResultListener *> &listeners = g_listeners.listeners;
	for (int i=0; i<listeners.size(); ++i) {
		if (listeners.get(i)->handleActivityResult(request_code, result_code, data))
			break;
	}
}
