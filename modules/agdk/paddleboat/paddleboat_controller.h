/**************************************************************************/
/*  paddleboat_controller.h                                               */
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

#ifndef PADDLEBOAT_CONTROLLER_H
#define PADDLEBOAT_CONTROLLER_H

#include "core/os/mutex.h"
#include "core/variant.h"

#include <jni.h>
#include <paddleboat/paddleboat.h>

class PaddleboatController {
	bool _initialized;
	Mutex _mutex;

	struct ConnectionEvent {
		int32_t index;
		bool connected;
	};
	Vector<ConnectionEvent> _pending_events;

	static void _status_callback(const int32_t p_controller_index, const Paddleboat_ControllerStatus p_controller_status, void *p_user_data);

public:
	Error init(JNIEnv *p_env, jobject p_context);
	void update();
	void destroy();

	void on_pause();
	void on_resume();

	int get_controller_count() const;
	Dictionary get_controller_info(int p_index) const;
	Dictionary get_controller_data(int p_index) const;

	bool is_initialized() const;

	PaddleboatController();
	~PaddleboatController();
};

#endif // PADDLEBOAT_CONTROLLER_H
