/*************************************************************************/
/*  timer2.cpp                                                           */
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

#include "timer2.h"
#include "core/bind/core_bind.h"
#include "core/os/main_loop.h"
#include "core/os/os.h"
#include "scene/main/viewport.h"

bool TimerObject::step(float delta) {
	if (is_cancel)
		return true;
	time -= delta;
	if (time < 0) {
		if (target)
			target->call(method);
		emit_signal("timeout");
		is_cancel = true;
		return false;
	} else {
		return false;
	}
}

void TimerObject::cancel() {
	is_cancel = true;
}

void TimerObject::_bind_methods() {
	ClassDB::bind_method(D_METHOD("cancel"), &TimerObject::cancel);
	ADD_SIGNAL(MethodInfo("timeout"));
}

void TimerNode::check_queue() {
	if (timer_objs.size() > 0 && !is_processing()) {
		set_process(true);
		set_pause_mode(Node::PAUSE_MODE_PROCESS);
	}
}

void TimerNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			float delta = get_process_delta_time();
			for (int n = 0, t = timer_objs.size(); n < t; n++) {
				Ref<TimerObject> obj = timer_objs[n];
				if (obj->step(delta)) {
					timer_objs.remove(n);
					n -= 1;
					t -= 1;
				}
			}
			if (timer_objs.size() == 0) {
				set_process(false);
			}
		};
	}
}

Timer2::Timer2() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "Singleton already exists");
	singleton = this;
}

Timer2::~Timer2() {
	if (timer_node) {
		memdelete(timer_node);
	}
	singleton = nullptr;
}

Ref<TimerObject> Timer2::wait(float p_time) {
	const String timer_key = "new_timer";
	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	SceneTree *tree = cast_to<SceneTree>(main_loop);
	ERR_FAIL_COND_V(tree == nullptr, nullptr);

	Viewport *viewport = tree->get_root();
	ERR_FAIL_COND_V(viewport == nullptr, nullptr);
	if (timer_node == nullptr) {
		timer_node = memnew(TimerNode);
		timer_node->set_name(timer_key);

		Vector<Variant> vector;
		vector.push_back(Variant(timer_node));

		tree->connect(StringName("idle_frame"), this, StringName("_add_node"), vector, 0);
	}

	Ref<TimerObject> obj = memnew(TimerObject);
	obj->time = p_time;
	timer_node->timer_objs.push_back(obj);
	timer_node->check_queue();
	return obj;
}

Ref<TimerObject> Timer2::wait_trigger(float p_time, Object *p_target, String p_method) {
	const String timer_key = "new_timer";
	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	SceneTree *tree = cast_to<SceneTree>(main_loop);
	ERR_FAIL_COND_V(tree == nullptr, nullptr);

	Viewport *viewport = tree->get_root();
	ERR_FAIL_COND_V(viewport == nullptr, nullptr);
	TimerNode *timer_node = nullptr;
	if (timer_node == nullptr) {
		timer_node = memnew(TimerNode);
		timer_node->set_name(timer_key);

		Vector<Variant> vector;
		vector.push_back(Variant(timer_node));

		tree->connect("idle_frame", this, "_add_node", vector, 0);

	} else {
		timer_node = cast_to<TimerNode>(viewport->get_node(timer_key));
	}

	Ref<TimerObject> obj = memnew(TimerObject);
	obj->time = p_time;
	obj->target = p_target;
	obj->method = p_method;
	timer_node->timer_objs.push_back(obj);
	timer_node->check_queue();
	return obj;
}

Timer2 *Timer2::singleton = nullptr;
Timer2 *Timer2::get_singleton() {
	return singleton;
}

void Timer2::_add_node(Object *node) {
	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	SceneTree *tree = cast_to<SceneTree>(main_loop);
	ERR_FAIL_COND(tree == nullptr);

	tree->disconnect("idle_frame", this, "_add_node");
	tree->get_root()->add_child(cast_to<TimerNode>(node));
}

void Timer2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("wait_trigger", "time", "target", "method"), &Timer2::wait_trigger);
	ClassDB::bind_method(D_METHOD("wait", "time"), &Timer2::wait);
	ClassDB::bind_method(D_METHOD("_add_node", "node"), &Timer2::_add_node);
}
