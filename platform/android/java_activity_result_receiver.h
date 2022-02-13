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

#ifndef JAVA_ACTIVITY_RESULT_RECEIVER_H
#define JAVA_ACTIVITY_RESULT_RECEIVER_H

#include <jni.h>
#include <memory>

class ActivityResultReceiverPrivate;
class ActivityResultReceiver {

	std::shared_ptr<ActivityResultReceiverPrivate> d;

	friend class ActivityResultReceiverPrivate;
public:
	virtual void handleActivityResult(int receiverRequestCode, int resultCode, const jobject &data) = 0;

	static int getGlobalRequestCode(ActivityResultReceiver *publicObject, int requestCode);

	ActivityResultReceiver();
	~ActivityResultReceiver();
};

extern "C" void processActivityResult(jint requestCode, jint resultCode, jobject data);

#endif // JAVA_ACTIVITY_RESULT_RECEIVER_H
