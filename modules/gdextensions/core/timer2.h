/**************************************************************************/
/*  timer2.h                                                              */
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

#ifndef TIMER2_H
#define TIMER2_H

#include "core/reference.h"
#include "scene/main/node.h"

// A single countdown handle returned by Timer2::wait*().
// Hold a Ref<TimerObject> to keep it alive; cancel() or let it expire.
class TimerObject : public Reference {
	GDCLASS(TimerObject, Reference);
	friend class Timer2;
	friend class TimerNode;

private:
	float wait_time = 0.0f; // original duration set at creation
	float time_left = 0.0f; // remaining seconds (counts down)
	bool cancelled = false;
	bool paused = false;
	bool repeating = false;
	ObjectID target_id = 0; // safe handle — survives target deletion
	StringName method;

public:
	// Internal init called by Timer2 (and doctests). Not exposed to GDScript.
	void _init(float p_time, Object *p_target, const StringName &p_method, bool p_repeat);

protected:
	static void _bind_methods();

public:
	// Called by TimerNode each frame. Returns true when the entry should be removed.
	bool step(float delta);

	void cancel();
	void reset(); // restart from original wait_time, clears cancel
	void reset_to(float p_time); // reset with a new duration

	void set_paused(bool p_paused);
	bool is_paused() const { return paused; }

	float get_time_left() const { return time_left; }
	float get_wait_time() const { return wait_time; }
	bool is_cancelled() const { return cancelled; }
	bool is_repeating() const { return repeating; }
};

// Internal scene-tree node that ticks all live TimerObjects each process frame.
// Not exposed to GDScript.
class TimerNode : public Node {
	GDCLASS(TimerNode, Node);

private:
	Vector<Ref<TimerObject>> timer_objs;

	void _ensure_processing();

protected:
	void _notification(int p_what);

public:
	void add_timer(Ref<TimerObject> p_timer);
	void cancel_all();
	int get_timer_count() const { return timer_objs.size(); }
};

// Singleton that creates and manages lightweight scene-tree timers.
// No NOTIFICATION_PROCESS needed in the caller — timers self-drive via TimerNode.
//
// Example — fire-and-forget:
//   Timer2::get_singleton()->wait(2.0f)->connect("timeout", this, "_on_delay_done");
//
// Example — trigger a method after a delay:
//   Timer2::get_singleton()->wait_trigger(1.5f, this, "_spawn_enemy");
//
// Example — repeating heartbeat:
//   Ref<TimerObject> t = Timer2::get_singleton()->wait_repeat(0.5f);
//   t->connect("timeout", this, "_heartbeat");
//   // later: t->cancel();
class Timer2 : public Object {
	GDCLASS(Timer2, Object);

private:
	TimerNode *timer_node = nullptr;
	bool node_in_tree = false;

	// Creates timer_node if needed and schedules its deferred insertion into the root.
	void _ensure_node();
	// Common factory used by all wait* variants.
	Ref<TimerObject> _make_timer(float p_time, Object *p_target, const StringName &p_method, bool p_repeat);

protected:
	static void _bind_methods();
	static Timer2 *singleton;

public:
	static Timer2 *get_singleton();

	// Returns a TimerObject that fires "timeout" after p_time seconds.
	Ref<TimerObject> wait(float p_time);

	// Same as wait() but also calls p_method on p_target when the timer fires.
	Ref<TimerObject> wait_trigger(float p_time, Object *p_target, const String &p_method);

	// Repeating variant of wait() — re-arms automatically after each timeout.
	Ref<TimerObject> wait_repeat(float p_time);

	// Repeating variant of wait_trigger().
	Ref<TimerObject> wait_repeat_trigger(float p_time, Object *p_target, const String &p_method);

	// Cancel and remove all pending timers.
	void cancel_all();

	// Number of active (non-cancelled) timers currently ticking.
	int get_pending_count() const;

	// Internal: called deferred to add timer_node to the root viewport.
	void _add_node(Object *p_node);

	Timer2();
	~Timer2();
};

#endif // TIMER2_H
