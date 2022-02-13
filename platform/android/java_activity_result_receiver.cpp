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


static int uniqueActivityRequestCode() {
	static Mutex mutex;
	static int requestCode = 0x1000; // Reserve all request codes under 0x1000.

	MutexLock locker(mutex);
	if (requestCode == INT_MAX)
		WARN_PRINT("Unique activity request code has wrapped. Unexpected behavior may occur.");
	return requestCode++;
}

class ActivityResultListener {
public:
	virtual ~ActivityResultListener();
	virtual bool handleActivityResult(jint requestCode, jint resultCode, jobject data) = 0;
};

static void registerActivityResultListener(ActivityResultListener *listener);
static void unregisterActivityResultListener(ActivityResultListener *listener);

class ActivityResultReceiverPrivate : public ActivityResultListener {
public:
	ActivityResultReceiver *q;
	mutable Map<int, int> localToGlobalRequestCode;
	mutable Map<int, int> globalToLocalRequestCode;

	static ActivityResultReceiverPrivate *get(ActivityResultReceiver *publicObject) {
		return publicObject->d.get();
	}
	int globalRequestCode(int localRequestCode) const {
		if (!localToGlobalRequestCode.has(localRequestCode)) {
			int globalRequestCode = uniqueActivityRequestCode();
			localToGlobalRequestCode[localRequestCode] = globalRequestCode;
			globalToLocalRequestCode[globalRequestCode] = localRequestCode;
		}
		return localToGlobalRequestCode[localRequestCode];
	}
	bool handleActivityResult(jint requestCode, jint resultCode, jobject data) {
		if (globalToLocalRequestCode.has(requestCode)) {
			q->handleActivityResult(globalToLocalRequestCode[requestCode], resultCode, data);
			return true;
		}
		return false;
	}
};

int ActivityResultReceiver::getGlobalRequestCode(ActivityResultReceiver *publicObject, int requestCode) {
	return publicObject->d->globalRequestCode(requestCode);
}

ActivityResultReceiver::ActivityResultReceiver() : d(new ActivityResultReceiverPrivate) {
	d->q = this;
	registerActivityResultListener(d.get());
}

ActivityResultReceiver::~ActivityResultReceiver() {
	unregisterActivityResultListener(d.get());
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

void processActivityResult(jint requestCode, jint resultCode, jobject data) {
	MutexLock locker(g_listeners.mutex);
	const Vector<ActivityResultListener *> &listeners = g_listeners.listeners;
	for (int i=0; i<listeners.size(); ++i) {
		if (listeners.get(i)->handleActivityResult(requestCode, resultCode, data))
			break;
	}
}
