/**************************************************************************/
/*  rope_server.cpp                                                       */
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

// Reference:
// ----------
// 1. https://godotengine.org/qa/339/does-gdscript-have-method-to-execute-string-code-exec-python

#include "core/engine.h"
#include "core/math/vector2.h"
#include "core/os/os.h"
#include "core/variant.h"
#include "scene/resources/curve.h"

#include "rope_server.h"

#include <algorithm>

static float get_point_perc(int index, const PoolVector2Array &points) {
	return index / (points.size() > 0 ? float(points.size() - 1) : 0);
}

static Vector2 damp_vec(Vector2 value, float damping_factor, float delta) {
	return value.linear_interpolate(value.ZERO, 1 - Math::exp(-damping_factor * delta));
}

void RopeServer::_init() {
	_last_time = 0;
	_update_in_editor = false;
}

void RopeServer::_enter_tree() {
	_start_stop_process();
}

void RopeServer::_physics_process(float delta) {
	emit_signal("on_pre_update");
	uint64_t start = OS::get_singleton()->get_ticks_usec();

	for (Node2D *rope : _ropes) {
		_simulate(rope, delta);
	}
	_last_time = (OS::get_singleton()->get_ticks_usec() - start) / 1000.;
	emit_signal("on_post_update");
}

void RopeServer::register_rope(Node *rope) {
	if (Node2D *r = cast_to<Node2D>(rope)) {
		_ropes.emplace_back(r);
		_start_stop_process();
	} else {
		WARN_PRINT("Not a Node2D");
	}
}

void RopeServer::unregister_rope(Node *rope) {
	if (Node2D *r = cast_to<Node2D>(rope)) {
		if (_ropes.empty() || r != _ropes.back()) {
			auto it = std::find(_ropes.begin(), _ropes.end(), r);
			if (it == _ropes.end()) {
				WARN_PRINT("Unregistering non-registered Rope");
				return;
			}
			(*it) = _ropes.back(); // Swap and pop
		}
		_ropes.pop_back();
		_start_stop_process();
	} else {
		WARN_PRINT("Not a Node2D");
	}
}

void RopeServer::set_update_in_editor(bool value) {
	_update_in_editor = value;
	_start_stop_process();
}

bool RopeServer::get_update_in_editor() const {
	return _update_in_editor;
}

void RopeServer::_start_stop_process() {
	_last_time = 0;
	set_physics_process(!_ropes.empty() && (!Engine::get_singleton()->is_editor_hint() || get_update_in_editor()));
}

void RopeServer::_simulate(Node2D *rope, float delta) {
	PoolVector2Array points = rope->call("get_points");
	if (points.size() < 2) {
		return;
	}
	PoolVector2Array oldpoints = rope->call("get_old_points");
	Ref<Curve> damping_curve = rope->get("damping_curve");
	const float gravity = rope->get("gravity");
	const float damping = rope->get("damping");
	const float stiffness = rope->get("stiffness");
	const int num_constraint_iterations = rope->get("num_constraint_iterations");
	const PoolRealArray seg_lengths = rope->call("get_segment_lengths");
	Vector2 parent_seg_dir = rope->get_global_transform().basis_xform(Vector2::DOWN).normalized();
	Vector2 last_stiffness_force;

	// Simulate
	for (size_t i = 1; i < points.size(); ++i) {
		Vector2 vel = points[i] - oldpoints[i];
		const float dampmult = damping_curve.is_valid() ? damping_curve->interpolate_baked(get_point_perc(i, points)) : 1;

		if (stiffness > 0) {
			///  |  parent_seg_dir     --->  parent_seg_tangent
			///  |                     \
			///  V                      \   seg_dir
			///  \  seg_dir              V
			///   \
			///    V
			const Vector2 seg_dir = (points[i] - points[i - 1]).normalized();
			const Vector2 parent_seg_tangent = parent_seg_dir.tangent();
			const float angle = seg_dir.angle_to(parent_seg_dir);

			// The force directs orthogonal to the current segment
			// TODO: Ask a physicist if this is physically correct.
			const Vector2 force_dir = seg_dir.tangent();

			// Scale the force the further the segment bends.
			// angle is signed and can be used to determine the force direction
			// TODO: Ask a physicist if this is physically correct.
			last_stiffness_force += force_dir * (-angle / Math_PI) * stiffness;
			vel += last_stiffness_force;
			parent_seg_dir = seg_dir;
		}

		oldpoints.set(i, points[i]);
		points.set(i, points[i] + damp_vec(vel, damping * dampmult, delta) + Vector2(0, gravity * delta));
	}

	// Constraint
	for (int _ = 0; _ < num_constraint_iterations; ++_) {
		points.set(0, rope->get_global_position());
		points.set(1, points[0] + (points[1] - points[0]).normalized() * seg_lengths[0]);

		for (size_t i = 1; i < points.size() - 1; ++i) {
			const Vector2 diff = points[i + 1] - points[i];
			const float distance = diff.length();
			const Vector2 dir = diff / distance;
			const float error = (seg_lengths[i] - distance) * 0.5;
			points.set(i, points[i] - error * dir);
			points.set(i + 1, points[i + 1] + error * dir);
		}
	}

	rope->call("set_points", points);
	rope->call("set_old_points", oldpoints);
}

float RopeServer::get_computation_time() const {
	return _last_time;
}

int RopeServer::get_num_ropes() const {
	return _ropes.size();
}

void RopeServer::_notification(int what) {
	switch (what) {
		case NOTIFICATION_READY: {
			_init();
		} break;
		case NOTIFICATION_ENTER_TREE: {
			_enter_tree();
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			_physics_process(get_process_delta_time());
		} break;
	}
}

void RopeServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_rope", "rope"), &RopeServer::register_rope);
	ClassDB::bind_method(D_METHOD("unregister_rope", "rope"), &RopeServer::unregister_rope);
	ClassDB::bind_method(D_METHOD("get_num_ropes"), &RopeServer::get_num_ropes);
	ClassDB::bind_method(D_METHOD("get_computation_time"), &RopeServer::get_computation_time);
	ClassDB::bind_method(D_METHOD("set_update_in_editor", "state"), &RopeServer::set_update_in_editor);
	ClassDB::bind_method(D_METHOD("get_update_in_editor"), &RopeServer::get_update_in_editor);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "update_in_editor"), "set_update_in_editor", "get_update_in_editor");

	ADD_SIGNAL(MethodInfo("on_pre_update"));
	ADD_SIGNAL(MethodInfo("on_post_update"));
}

RopeServer::RopeServer() {
	_update_in_editor = false;
}

RopeServer::~RopeServer() {}
