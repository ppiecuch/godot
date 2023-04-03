/**************************************************************************/
/*  timebnode.h                                                           */
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

#ifndef TIME_B_NODE_H
#define TIME_B_NODE_H

#include "behaviornode.h"

class TimerBNode : public BehaviorNode {
	GDCLASS(TimerBNode, BehaviorNode);

private:
	float delay;
	bool _cancel;

protected:
	bool timeout;
	float _time;

	virtual Status _step(const Variant &target, Dictionary &env);
	virtual Status _behavior(const Variant &target, Dictionary env);

	void _script_timeout_behavior(const Variant &target, Dictionary &env);
	virtual void _timeout_behavior(const Variant &target, Dictionary &env) {}

	void _script_cancel_behavior(const Variant &target, Dictionary &env);
	virtual void _cancel_behavior(const Variant &target, Dictionary &env) {}

	void _script_during_behavior(const Variant &target, Dictionary &env);
	virtual void _during_behavior(const Variant &target, Dictionary &env) {}
	virtual void _reset(const Variant &target);

	static void _bind_methods();

public:
	void set_delay(float t) { delay = t; }
	float get_delay() { return delay; }

	float get_time() { return _time; }
	void time_out() { _time = 0; }
	void cancel() { _cancel = true; }

	void recount();
	void recount_to(float t);

	TimerBNode() :
			BehaviorNode() {
		delay = 1;
		_time = 0;
		timeout = true;
		set_will_focus(true);
	}
	~TimerBNode() {}
};

#endif
