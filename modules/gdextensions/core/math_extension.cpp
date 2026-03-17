/**************************************************************************/
/*  math_extension.cpp                                                    */
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

#include "math_extension.h"

GodotMathExtension *GodotMathExtension::singleton = nullptr;

void GodotMathExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("spherical_to_local_position", "theta", "phi"), &GodotMathExtension::spherical_to_local_position);
	ClassDB::bind_method(D_METHOD("quat_from_radians:Quat", "radians"), &GodotMathExtension::quat_from_radians);

	ClassDB::bind_method(D_METHOD("ease_in", "t"), &GodotMathExtension::ease_in);
	ClassDB::bind_method(D_METHOD("ease_out", "t"), &GodotMathExtension::ease_out);
	ClassDB::bind_method(D_METHOD("exponetial", "t"), &GodotMathExtension::exponetial);
	ClassDB::bind_method(D_METHOD("smooth_step", "t"), &GodotMathExtension::smooth_step);
	ClassDB::bind_method(D_METHOD("smoother_step", "t"), &GodotMathExtension::smoother_step);

	ClassDB::bind_method(D_METHOD("camera_get_position_distance", "camera", "pos"), &GodotMathExtension::camera_get_position_distance);
	ClassDB::bind_method(D_METHOD("get_2d_position_from_3d_position_with_screen_limits", "camera", "position_3d", "screen_info"), &GodotMathExtension::get_2d_position_from_3d_position_with_screen_limits);
	ClassDB::bind_method(D_METHOD("get_2d_position_from_3d_position", "camera", "position_3d"), &GodotMathExtension::get_2d_position_from_3d_position);
	ClassDB::bind_method(D_METHOD("clamp_angle", "val", "ang_min", "ang_max"), &GodotMathExtension::clamp_angle);
	ClassDB::bind_method(D_METHOD("adjust_facing", "facing", "target", "step", "adjust_rate", "current_gn"), &GodotMathExtension::adjust_facing);
	ClassDB::bind_method(D_METHOD("rotate_around:Transform", "transform", "point", "axis", "angle"), &GodotMathExtension::rotate_around);
	ClassDB::bind_method(D_METHOD("inverse_lerp", "from", "to", "weight"), &GodotMathExtension::inverse_lerp);
	ClassDB::bind_method(D_METHOD("base_log", "float", "float"), &GodotMathExtension::base_log);
	ClassDB::bind_method(D_METHOD("transform_directon_vector", "direction", "basis"), &GodotMathExtension::transform_directon_vector);

	ClassDB::bind_method(D_METHOD("spatial_set_rotation_quat", "spatial", "quat"), &GodotMathExtension::spatial_set_rotation_quat);
	ClassDB::bind_method(D_METHOD("spatial_set_rotation_quat_keep_scale", "spatial", "quat"), &GodotMathExtension::spatial_set_rotation_quat_keep_scale);
	ClassDB::bind_method(D_METHOD("spatial_get_rotation_quat", "spatial"), &GodotMathExtension::spatial_get_rotation_quat);

	BIND_CONSTANT(GME_MATH_TAU);
}

GodotMathExtension *GodotMathExtension::get_singleton() {
	return singleton;
}

Vector3 GodotMathExtension::spherical_to_local_position(real_t p_theta, real_t p_phi) {
	return MathExtension::spherical_to_local_position(p_theta, p_phi);
}

Quat GodotMathExtension::quat_from_radians(Vector3 p_radians) {
	return MathExtension::quat_from_radians(p_radians);
}

real_t GodotMathExtension::ease_in(real_t t) {
	return MathExtension::ease_in(t);
}

real_t GodotMathExtension::ease_out(real_t t) {
	return MathExtension::ease_out(t);
}

real_t GodotMathExtension::exponetial(real_t t) {
	return MathExtension::exponetial(t);
}

real_t GodotMathExtension::smooth_step(real_t t) {
	return MathExtension::smooth_step(t);
}

real_t GodotMathExtension::smoother_step(real_t t) {
	return MathExtension::smoother_step(t);
}

real_t GodotMathExtension::camera_get_position_distance(const Object *p_camera, const Vector3 &p_pos) {
	ERR_FAIL_NULL_V(p_camera, 0);
	if (const Camera *camera = cast_to<Camera>(p_camera)) {
		return MathExtension::camera_get_position_distance(camera, p_pos);
	}
	return 0;
}

Vector2 GodotMathExtension::get_2d_position_from_3d_position_with_screen_limits(const Object *p_camera, const Vector3 &p_position_3d, const Dictionary &screen_info) {
	ERR_FAIL_NULL_V(p_camera, Vector2());
	if (const Camera *camera = cast_to<Camera>(p_camera)) {
		const Vector2 &screen_size = screen_info["screen_size"];
		const Vector2 &screen_center = screen_info["screen_center"];
		const Vector2 &screen_mins = screen_info["screen_mins"];
		const Vector2 &screen_max = screen_info["screen_max"];
		return MathExtension::get_2d_position_from_3d_position_with_screen_limits(camera, p_position_3d, screen_size, screen_center, screen_mins, screen_max);
	}
	return Vector2();
}

Vector2 GodotMathExtension::get_2d_position_from_3d_position(const Object *p_camera, const Vector3 &p_position_3d) {
	ERR_FAIL_NULL_V(p_camera, Vector2());
	if (const Camera *camera = cast_to<Camera>(p_camera)) {
		return MathExtension::get_2d_position_from_3d_position(camera, p_position_3d);
	}
	return Vector2();
}

real_t GodotMathExtension::clamp_angle(real_t val, real_t ang_min, real_t ang_max) {
	return MathExtension::clamp_angle(val, ang_min, ang_max);
}

Vector3 GodotMathExtension::adjust_facing(const Vector3 &p_facing, const Vector3 &p_target, const real_t &p_step, const real_t &p_adjust_rate, const Vector3 &p_current_gn) {
	return MathExtension::adjust_facing(p_facing, p_target, p_step, p_adjust_rate, p_current_gn);
}

Transform GodotMathExtension::rotate_around(Transform p_transform, Vector3 p_point, Vector3 p_axis, real_t p_angle) {
	return MathExtension::rotate_around(p_transform, p_point, p_axis, p_angle);
};

real_t GodotMathExtension::inverse_lerp(real_t p_from, real_t p_to, real_t p_weight) {
	return MathExtension::inverse_lerp(p_from, p_to, p_weight);
};

float GodotMathExtension::base_log(float a, float new_base) {
	if (new_base == 1) {
		return NAN;
	}
	if (a != 1 && (new_base == 0 || Math::is_inf(new_base))) {
		return NAN;
	}

	return Math::log(a) / Math::log(new_base);
}

Vector3 GodotMathExtension::transform_directon_vector(const Vector3 &p_direction, const Basis &p_basis) {
	return MathExtension::transform_directon_vector(p_direction, p_basis);
}

Quat GodotMathExtension::spatial_get_rotation_quat(const Node *spatial) {
	return MathExtension::spatial_get_rotation_quat(spatial);
}

void GodotMathExtension::spatial_set_rotation_quat(Node *spatial, const Quat &rotation) {
	return MathExtension::spatial_set_rotation_quat(spatial, rotation);
}

void GodotMathExtension::spatial_set_rotation_quat_keep_scale(Node *spatial, const Quat &rotation) {
	return MathExtension::spatial_set_rotation_quat_keep_scale(spatial, rotation);
}

GodotMathExtension::GodotMathExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "Singleton already exists");
	singleton = this;
}
