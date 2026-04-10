/**************************************************************************/
/*  timer2.cpp                                                            */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "core/object.h"
#include "core/os/main_loop.h"
#include "core/os/os.h"
#include "scene/main/viewport.h"
#include "timer2.h"

// ── TimerObject ───────────────────────────────────────────────────────────────

void TimerObject::_init(float p_time, Object *p_target, const StringName &p_method, bool p_repeat) {
	wait_time = p_time;
	time_left = p_time;
	target_id = p_target ? p_target->get_instance_id() : 0;
	method = p_method;
	repeating = p_repeat;
	cancelled = false;
}

bool TimerObject::step(float delta) {
	if (cancelled) {
		return true; // signal TimerNode to remove this entry
	}
	if (paused) {
		return false;
	}

	time_left -= delta;

	if (time_left <= 0.0f) {
		// Invoke the optional target callback.
		if (target_id != 0) {
			Object *target = ObjectDB::get_instance(target_id);
			if (target) {
				target->call(method);
			}
		}
		emit_signal("timeout");

		if (repeating) {
			// Advance by wait_time to handle frames where delta >> wait_time.
			time_left += wait_time;
			return false; // keep in list
		} else {
			cancelled = true;
			return true; // remove immediately (fixed: was returning false, leaving a dead entry for one extra frame)
		}
	}

	return false;
}

void TimerObject::cancel() {
	cancelled = true;
}

void TimerObject::reset() {
	time_left = wait_time;
	cancelled = false;
}

void TimerObject::reset_to(float p_time) {
	wait_time = p_time;
	time_left = p_time;
	cancelled = false;
}

void TimerObject::set_paused(bool p_paused) {
	paused = p_paused;
}

void TimerObject::_bind_methods() {
	ClassDB::bind_method(D_METHOD("cancel"), &TimerObject::cancel);
	ClassDB::bind_method(D_METHOD("reset"), &TimerObject::reset);
	ClassDB::bind_method(D_METHOD("reset_to", "time"), &TimerObject::reset_to);
	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &TimerObject::set_paused);
	ClassDB::bind_method(D_METHOD("is_paused"), &TimerObject::is_paused);
	ClassDB::bind_method(D_METHOD("get_time_left"), &TimerObject::get_time_left);
	ClassDB::bind_method(D_METHOD("get_wait_time"), &TimerObject::get_wait_time);
	ClassDB::bind_method(D_METHOD("is_cancelled"), &TimerObject::is_cancelled);
	ClassDB::bind_method(D_METHOD("is_repeating"), &TimerObject::is_repeating);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "paused"), "set_paused", "is_paused");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "time_left"), "", "get_time_left");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wait_time"), "", "get_wait_time");

	ADD_SIGNAL(MethodInfo("timeout"));
}

// ── TimerNode ─────────────────────────────────────────────────────────────────

void TimerNode::_ensure_processing() {
	if (timer_objs.size() > 0 && !is_processing()) {
		set_process(true);
		set_pause_mode(Node::PAUSE_MODE_PROCESS);
	}
}

void TimerNode::add_timer(Ref<TimerObject> p_timer) {
	timer_objs.push_back(p_timer);
	_ensure_processing();
}

void TimerNode::cancel_all() {
	// Make local copies before mutating: Godot 3.x COW Vector::operator[] is const.
	for (int i = 0; i < timer_objs.size(); i++) {
		Ref<TimerObject> obj = timer_objs[i];
		obj->cancel();
	}
	timer_objs.clear();
	set_process(false);
}

void TimerNode::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			float delta = get_process_delta_time();
			for (int n = 0, t = timer_objs.size(); n < t; n++) {
				Ref<TimerObject> obj = timer_objs[n]; // local copy: COW Vector gives const ref via []
				if (obj->step(delta)) {
					timer_objs.remove(n);
					n -= 1;
					t -= 1;
				}
			}
			if (timer_objs.empty()) {
				set_process(false);
			}
		} break;
	}
}

// ── Timer2 ────────────────────────────────────────────────────────────────────

Timer2 *Timer2::singleton = nullptr;

Timer2 *Timer2::get_singleton() {
	return singleton;
}

Timer2::Timer2() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "Timer2 singleton already exists");
	singleton = this;
}

Timer2::~Timer2() {
	if (timer_node && !node_in_tree) {
		// Not yet parented to the scene tree — safe to delete directly.
		memdelete(timer_node);
	}
	// If node_in_tree, the SceneTree root owns it and will free it on exit.
	timer_node = nullptr;
	singleton = nullptr;
}

void Timer2::_ensure_node() {
	if (timer_node != nullptr) {
		return;
	}
	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	SceneTree *tree = cast_to<SceneTree>(main_loop);
	ERR_FAIL_COND(tree == nullptr);
	ERR_FAIL_COND(tree->get_root() == nullptr);

	timer_node = memnew(TimerNode);
	timer_node->set_name("Timer2Node");
	// Defer the actual add_child so we never mutate the tree mid-frame.
	call_deferred("_add_node", timer_node);
}

void Timer2::_add_node(Object *p_node) {
	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	SceneTree *tree = cast_to<SceneTree>(main_loop);
	ERR_FAIL_COND(tree == nullptr);

	TimerNode *tn = cast_to<TimerNode>(p_node);
	ERR_FAIL_COND(tn == nullptr);
	tree->get_root()->add_child(tn);
	node_in_tree = true;
}

Ref<TimerObject> Timer2::_make_timer(float p_time, Object *p_target, const StringName &p_method, bool p_repeat) {
	_ensure_node();
	ERR_FAIL_COND_V_MSG(timer_node == nullptr, nullptr, "Timer2: no scene tree available");

	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(p_time, p_target, p_method, p_repeat);
	timer_node->add_timer(obj);
	return obj;
}

Ref<TimerObject> Timer2::wait(float p_time) {
	return _make_timer(p_time, nullptr, StringName(), false);
}

Ref<TimerObject> Timer2::wait_trigger(float p_time, Object *p_target, const String &p_method) {
	return _make_timer(p_time, p_target, StringName(p_method), false);
}

Ref<TimerObject> Timer2::wait_repeat(float p_time) {
	return _make_timer(p_time, nullptr, StringName(), true);
}

Ref<TimerObject> Timer2::wait_repeat_trigger(float p_time, Object *p_target, const String &p_method) {
	return _make_timer(p_time, p_target, StringName(p_method), true);
}

void Timer2::cancel_all() {
	if (timer_node) {
		timer_node->cancel_all();
	}
}

int Timer2::get_pending_count() const {
	return timer_node ? timer_node->get_timer_count() : 0;
}

void Timer2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("wait", "time"), &Timer2::wait);
	ClassDB::bind_method(D_METHOD("wait_trigger", "time", "target", "method"), &Timer2::wait_trigger);
	ClassDB::bind_method(D_METHOD("wait_repeat", "time"), &Timer2::wait_repeat);
	ClassDB::bind_method(D_METHOD("wait_repeat_trigger", "time", "target", "method"), &Timer2::wait_repeat_trigger);
	ClassDB::bind_method(D_METHOD("cancel_all"), &Timer2::cancel_all);
	ClassDB::bind_method(D_METHOD("get_pending_count"), &Timer2::get_pending_count);
	ClassDB::bind_method(D_METHOD("_add_node", "node"), &Timer2::_add_node);
}

// ── Doctests ──────────────────────────────────────────────────────────────────

#ifdef DOCTEST

// ── TimerObject::step ─────────────────────────────────────────────────────────

TEST_CASE("[TimerObject] initial state") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(2.0f, nullptr, StringName(), false);

	CHECK(obj->get_wait_time() == doctest::Approx(2.0f));
	CHECK(obj->get_time_left() == doctest::Approx(2.0f));
	CHECK_FALSE(obj->is_cancelled());
	CHECK_FALSE(obj->is_paused());
	CHECK_FALSE(obj->is_repeating());
}

TEST_CASE("[TimerObject] step counts down time_left") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(1.0f, nullptr, StringName(), false);

	obj->step(0.3f);
	CHECK(obj->get_time_left() == doctest::Approx(0.7f));
	obj->step(0.3f);
	CHECK(obj->get_time_left() == doctest::Approx(0.4f));
}

TEST_CASE("[TimerObject] step returns false before timeout") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(1.0f, nullptr, StringName(), false);

	CHECK_FALSE(obj->step(0.5f));
	CHECK_FALSE(obj->step(0.49f));
}

TEST_CASE("[TimerObject] step returns true immediately on timeout (no extra frame)") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(1.0f, nullptr, StringName(), false);

	obj->step(0.5f); // not yet
	bool done = obj->step(0.6f); // total > 1.0 → fires
	CHECK(done); // must be true on the timeout frame itself
	CHECK(obj->is_cancelled());
}

TEST_CASE("[TimerObject] step clamps time_left on timeout") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(0.1f, nullptr, StringName(), false);

	// Large delta overshoots
	bool done = obj->step(1.0f);
	CHECK(done);
	CHECK(obj->get_time_left() <= 0.0f);
}

TEST_CASE("[TimerObject] cancel causes immediate removal on next step") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(10.0f, nullptr, StringName(), false);

	obj->cancel();
	CHECK(obj->is_cancelled());
	CHECK(obj->step(0.0f)); // cancelled → remove
}

TEST_CASE("[TimerObject] cancel before first step") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(5.0f, nullptr, StringName(), false);

	obj->cancel();
	CHECK(obj->step(0.001f)); // still removed immediately
}

TEST_CASE("[TimerObject] reset restores time_left and clears cancel") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(1.0f, nullptr, StringName(), false);

	obj->step(0.8f);
	obj->cancel();
	CHECK(obj->is_cancelled());

	obj->reset();
	CHECK_FALSE(obj->is_cancelled());
	CHECK(obj->get_time_left() == doctest::Approx(1.0f));
	CHECK_FALSE(obj->step(0.5f)); // ticking again
}

TEST_CASE("[TimerObject] reset_to changes wait_time and restarts") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(1.0f, nullptr, StringName(), false);

	obj->reset_to(3.0f);
	CHECK(obj->get_wait_time() == doctest::Approx(3.0f));
	CHECK(obj->get_time_left() == doctest::Approx(3.0f));
	CHECK_FALSE(obj->is_cancelled());
}

TEST_CASE("[TimerObject] paused — step does not advance time") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(1.0f, nullptr, StringName(), false);

	obj->set_paused(true);
	CHECK(obj->is_paused());
	CHECK_FALSE(obj->step(999.0f)); // huge delta, still not done
	CHECK(obj->get_time_left() == doctest::Approx(1.0f)); // unchanged

	obj->set_paused(false);
	CHECK(obj->step(1.1f)); // now fires
}

TEST_CASE("[TimerObject] repeating — does not cancel after first timeout") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(1.0f, nullptr, StringName(), true);

	CHECK(obj->is_repeating());

	// First timeout: step returns false (stays in list), time_left advances by wait_time
	bool removed = obj->step(1.1f);
	CHECK_FALSE(removed);
	CHECK_FALSE(obj->is_cancelled());
	// time_left wrapped: -0.1 + 1.0 = 0.9
	CHECK(obj->get_time_left() == doctest::Approx(0.9f));

	// Second fire
	removed = obj->step(1.0f);
	CHECK_FALSE(removed);
	CHECK(obj->get_time_left() == doctest::Approx(-0.1f + 1.0f)); // == 0.9 again approximately
}

TEST_CASE("[TimerObject] repeating — cancel stops repetition") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(0.5f, nullptr, StringName(), true);

	obj->step(0.6f); // fires once, still alive
	CHECK_FALSE(obj->is_cancelled());

	obj->cancel();
	CHECK(obj->step(0.0f)); // now removed
}

TEST_CASE("[TimerObject] wait_time preserved after multiple resets") {
	Ref<TimerObject> obj = memnew(TimerObject);
	obj->_init(2.0f, nullptr, StringName(), false);

	obj->step(1.5f);
	obj->reset();
	CHECK(obj->get_wait_time() == doctest::Approx(2.0f));
	CHECK(obj->get_time_left() == doctest::Approx(2.0f));

	obj->reset(); // idempotent
	CHECK(obj->get_time_left() == doctest::Approx(2.0f));
}

// ── TimerNode ─────────────────────────────────────────────────────────────────

TEST_CASE("[TimerNode] get_timer_count reflects add and expiry") {
	// TimerNode is a Node; instantiate without scene tree for structural tests.
	TimerNode *node = memnew(TimerNode);

	CHECK(node->get_timer_count() == 0);

	Ref<TimerObject> t1 = memnew(TimerObject);
	t1->_init(1.0f, nullptr, StringName(), false);
	node->add_timer(t1);
	CHECK(node->get_timer_count() == 1);

	Ref<TimerObject> t2 = memnew(TimerObject);
	t2->_init(0.5f, nullptr, StringName(), false);
	node->add_timer(t2);
	CHECK(node->get_timer_count() == 2);

	memdelete(node);
}

TEST_CASE("[TimerNode] cancel_all clears all timers") {
	TimerNode *node = memnew(TimerNode);

	for (int i = 0; i < 3; i++) {
		Ref<TimerObject> t = memnew(TimerObject);
		t->_init(float(i + 1), nullptr, StringName(), false);
		node->add_timer(t);
	}
	CHECK(node->get_timer_count() == 3);

	node->cancel_all();
	CHECK(node->get_timer_count() == 0);

	memdelete(node);
}

// ── Timer2 singleton (structural, no scene tree) ──────────────────────────────

TEST_CASE("[Timer2] get_pending_count is zero when no node") {
	// Timer2 singleton is created by the engine; just verify the API
	// compiles and the count starts at zero without a tree.
	Timer2 t2;
	CHECK(t2.get_pending_count() == 0);
}

TEST_CASE("[Timer2] cancel_all is safe when no timers exist") {
	Timer2 t2;
	t2.cancel_all(); // must not crash
	CHECK(t2.get_pending_count() == 0);
}

#endif // DOCTEST
