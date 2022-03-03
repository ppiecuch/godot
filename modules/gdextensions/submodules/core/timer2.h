/*************************************************************************/
/*  timer2.h                                                             */
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

//
// Created by gen on 15-4-26.
//

#ifndef GODOT_MASTER_TIMER2_H
#define GODOT_MASTER_TIMER2_H

#include "core/reference.h"
#include "scene/main/node.h"

class TimerObject : public Reference {
	GDCLASS(TimerObject, Reference);

private:
	bool is_cancel = false;

protected:
	static void _bind_methods();

public:
	float time;
	Object *target = NULL;
	String method;

	bool step(float delta);
	void cancel();
};

class TimerNode : public Node {
	GDCLASS(TimerNode, Node);

public:
	Vector<Ref<TimerObject>> timer_objs;
	void check_queue();

protected:
	void _notification(int p_what);
};

class Timer2 : public Object {
	GDCLASS(Timer2, Object);

private:
	TimerNode *timer_node = NULL;

protected:
	static void _bind_methods();
	static Timer2 *singleton;

public:
	static Timer2 *get_singleton();

	Ref<TimerObject> wait(float p_time);
	Ref<TimerObject> wait_trigger(float p_time, Object *p_target, String p_method);
	void _add_node(Object *node);

	Timer2();
	~Timer2();
};

#endif //GODOT_MASTER_TIMER2_H
