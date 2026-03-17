/**************************************************************************/
/*  tween2.h                                                              */
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

//
// Created by gen on 15-4-26.
//

#ifndef GODOT_MASTER_TWEEN2_H
#define GODOT_MASTER_TWEEN2_H

#include "core/object.h"
#include "core/reference.h"
#include "scene/main/node.h"

class TweenAction;
class TweenNode;

class Tween2 : public Object {
	GDCLASS(Tween2, Object);

private:
	TweenNode *tween_node = NULL;

protected:
	static void _bind_methods();
	static Tween2 *singleton;

public:
	static Tween2 *get_singleton();

	typedef enum {
		TWEEN_EASING_LINEAR,
		TWEEN_EASING_QUADRATIC_IN,
		TWEEN_EASING_QUADRATIC_OUT,
		TWEEN_EASING_QUADRATIC_IN_OUT,
		TWEEN_EASING_CUBIC_IN,
		TWEEN_EASING_CUBIC_OUT,
		TWEEN_EASING_CUBIC_IN_OUT,
		TWEEN_EASING_QUARTIC_IN,
		TWEEN_EASING_QUARTIC_OUT,
		TWEEN_EASING_QUARTIC_IN_OUT,
		TWEEN_EASING_QUINTIC_IN,
		TWEEN_EASING_QUINTIC_OUT,
		TWEEN_EASING_QUINTIC_IN_OUT,
		TWEEN_EASING_SINUSOIDAL_IN,
		TWEEN_EASING_SINUSOIDAL_OUT,
		TWEEN_EASING_SINUSOIDAL_IN_OUT,
		TWEEN_EASING_EXPONENTIAL_IN,
		TWEEN_EASING_EXPONENTIAL_OUT,
		TWEEN_EASING_EXPONENTIAL_IN_OUT,
		TWEEN_EASING_CIRCULAR_IN,
		TWEEN_EASING_CIRCULAR_OUT,
		TWEEN_EASING_CIRCULAR_IN_OUT,
		TWEEN_EASING_ELASTIC_IN,
		TWEEN_EASING_ELASTIC_OUT,
		TWEEN_EASING_ELASTIC_IN_OUT,
		TWEEN_EASING_BACK_IN,
		TWEEN_EASING_BACK_OUT,
		TWEEN_EASING_BACK_IN_OUT,
		TWEEN_EASING_BOUNCE_IN,
		TWEEN_EASING_BOUNCE_OUT,
		TWEEN_EASING_BOUNCE_IN_OUT,
	} Tween_Easing;
	void _add_node(Object *node);
	Ref<TweenAction> to(Object *target, float during);
	void cancel(Object *target);

	Tween2();
	~Tween2();
};

class TweenNode : public Node {
	GDCLASS(TweenNode, Node);

private:
	Vector<Ref<TweenAction>> actions;
	void check_queue();
	friend class Tween2;

protected:
	void _notification(int p_what);
};

class TweenProperty : public Reference {
	GDCLASS(TweenProperty, Reference);

private:
	friend class TweenAction;
	StringName property_name;
	int property_type;
	Variant from_value;
	Variant to_value;

	Variant callback_target;
	float step;
	int step_count;

	Variant interpolation(float per);
	Variant lerp(Variant from, Variant to, float per);
};

class TweenAction : public Reference {
	GDCLASS(TweenAction, Reference);

private:
	typedef enum {
		TWEEN_STATUS_NORMAL,
		TWEEN_STATUS_CANCEL,
		TWEEN_STATUS_END
	} TweenStatus;
	float total_time;
	float delta_time;
	float delay_time;
	Object *target;
	TweenStatus status;
	Tween2::Tween_Easing easing;

	bool step(float delta);

	Vector<Ref<TweenProperty>> properties;
	friend class Tween2;
	friend class TweenNode;
	void set_target(Object *target);
	void _on_target_exit();

protected:
	static void _bind_methods();

public:
	void add_method(const StringName &method_name, const Variant &from, const Variant &to);
	void add_property(const StringName &property_name, const Variant &from, const Variant &to);
	void add_callback(const Variant &target, const StringName &method_name, float step);

	void cancel();
	void end();

	void set_easing(Tween2::Tween_Easing e) {
		easing = e;
	}
	Tween2::Tween_Easing get_easing() { return easing; }

	float get_total_time() { return total_time; }
	float get_delta_time() { return delta_time; }
	float get_delay_time() { return delay_time; }
	void set_delay_time(float dt) {
		delay_time = dt;
	}

	TweenAction() {
		easing = Tween2::TWEEN_EASING_LINEAR;
		status = TWEEN_STATUS_NORMAL;
		delay_time = 0;
		total_time = 0;
		delta_time = 0;
		target = NULL;
	}
	~TweenAction();
};

VARIANT_ENUM_CAST(Tween2::Tween_Easing);

#endif //GODOT_MASTER_TWEEN2_H
