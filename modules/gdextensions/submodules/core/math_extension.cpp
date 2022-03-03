/*************************************************************************/
/*  math_extension.cpp                                                   */
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

#include "math_extension.h"

_GodotMathExtension *_GodotMathExtension::singleton = NULL;

void _GodotMathExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("spherical_to_local_position:Vector3", "theta", "phi"), &_GodotMathExtension::spherical_to_local_position);
	ClassDB::bind_method(D_METHOD("quat_from_radians:Quat", "radians"), &_GodotMathExtension::quat_from_radians);

	ClassDB::bind_method(D_METHOD("ease_in", "t"), &_GodotMathExtension::ease_in);
	ClassDB::bind_method(D_METHOD("ease_out", "t"), &_GodotMathExtension::ease_out);
	ClassDB::bind_method(D_METHOD("exponetial", "t"), &_GodotMathExtension::exponetial);
	ClassDB::bind_method(D_METHOD("smooth_step", "t"), &_GodotMathExtension::smooth_step);
	ClassDB::bind_method(D_METHOD("smoother_step", "t"), &_GodotMathExtension::smoother_step);

	ClassDB::bind_method(D_METHOD("camera_get_position_distance", "camera", "pos"), &_GodotMathExtension::camera_get_position_distance);
	//ClassDB::bind_method(D_METHOD("get_2d_position_from_3d_position_with_screen_limits:Vector2", "camera", "position_3d", "screen_size", "screen_center", "screen_mins", "screen_max"), &_GodotMathExtension::get_2d_position_from_3d_position_with_screen_limits);
	ClassDB::bind_method(D_METHOD("get_2d_position_from_3d_position:Vector2", "camera", "position_3d"), &_GodotMathExtension::get_2d_position_from_3d_position);
	ClassDB::bind_method(D_METHOD("clamp_angle", "val", "ang_min", "ang_max"), &_GodotMathExtension::clamp_angle);
	ClassDB::bind_method(D_METHOD("adjust_facing:Vector3", "facing", "target", "step", "adjust_rate", "current_gn"), &_GodotMathExtension::adjust_facing);
	ClassDB::bind_method(D_METHOD("rotate_around:Transform", "transform", "point", "axis", "angle"), &_GodotMathExtension::rotate_around);
	ClassDB::bind_method(D_METHOD("inverse_lerp", "from", "to", "weight"), &_GodotMathExtension::inverse_lerp);
	ClassDB::bind_method(D_METHOD("base_log:float", "float", "float"), &_GodotMathExtension::base_log);
	ClassDB::bind_method(D_METHOD("transform_directon_vector:Vector3", "direction", "basis"), &_GodotMathExtension::transform_directon_vector);

	ClassDB::bind_method(D_METHOD("spatial_set_rotation_quat", "spatial", "quat"), &_GodotMathExtension::spatial_set_rotation_quat);
	ClassDB::bind_method(D_METHOD("spatial_set_rotation_quat_keep_scale", "spatial", "quat"), &_GodotMathExtension::spatial_set_rotation_quat_keep_scale);
	ClassDB::bind_method(D_METHOD("spatial_get_rotation_quat", "spatial"), &_GodotMathExtension::spatial_get_rotation_quat);

	BIND_CONSTANT(GME_MATH_TAU);
}

_GodotMathExtension *_GodotMathExtension::get_singleton() {
	return singleton;
}

Vector3 _GodotMathExtension::spherical_to_local_position(real_t p_theta, real_t p_phi) {
	return GodotMathExtension::spherical_to_local_position(p_theta, p_phi);
}

Quat _GodotMathExtension::quat_from_radians(Vector3 p_radians) {
	return GodotMathExtension::quat_from_radians(p_radians);
}

real_t _GodotMathExtension::ease_in(real_t t) {
	return GodotMathExtension::ease_in(t);
}

real_t _GodotMathExtension::ease_out(real_t t) {
	return GodotMathExtension::ease_out(t);
}

real_t _GodotMathExtension::exponetial(real_t t) {
	return GodotMathExtension::exponetial(t);
}

real_t _GodotMathExtension::smooth_step(real_t t) {
	return GodotMathExtension::smooth_step(t);
}

real_t _GodotMathExtension::smoother_step(real_t t) {
	return GodotMathExtension::smoother_step(t);
}

real_t _GodotMathExtension::camera_get_position_distance(const Object *p_camera, const Vector3 &p_pos) {
	ERR_FAIL_NULL_V(p_camera, 0.0f);
	if (const Camera *camera = cast_to<Camera>(p_camera))
		return GodotMathExtension::camera_get_position_distance(camera, p_pos);
	return 0.0f;
}

Vector2 _GodotMathExtension::get_2d_position_from_3d_position_with_screen_limits(const Object *p_camera, const Vector3 &p_position_3d,
		const Vector2 &screen_size, const Vector2 &screen_center,
		const Vector2 &screen_mins, const Vector2 &screen_max) {
	ERR_FAIL_NULL_V(p_camera, Vector2());
	if (const Camera *camera = cast_to<Camera>(p_camera))
		return GodotMathExtension::get_2d_position_from_3d_position_with_screen_limits(camera, p_position_3d, screen_size, screen_center, screen_mins, screen_max);
	return Vector2();
}

Vector2 _GodotMathExtension::get_2d_position_from_3d_position(const Object *p_camera, const Vector3 &p_position_3d) {
	ERR_FAIL_NULL_V(p_camera, Vector2());
	if (const Camera *camera = cast_to<Camera>(p_camera))
		return GodotMathExtension::get_2d_position_from_3d_position(camera, p_position_3d);
	return Vector2();
}

real_t _GodotMathExtension::clamp_angle(real_t val, real_t ang_min, real_t ang_max) {
	return GodotMathExtension::clamp_angle(val, ang_min, ang_max);
}

Vector3 _GodotMathExtension::adjust_facing(const Vector3 &p_facing, const Vector3 &p_target, const real_t &p_step, const real_t &p_adjust_rate, const Vector3 &p_current_gn) {
	return GodotMathExtension::adjust_facing(p_facing, p_target, p_step, p_adjust_rate, p_current_gn);
}

Transform _GodotMathExtension::rotate_around(Transform p_transform, Vector3 p_point, Vector3 p_axis, real_t p_angle) {
	return GodotMathExtension::rotate_around(p_transform, p_point, p_axis, p_angle);
};

real_t _GodotMathExtension::inverse_lerp(real_t p_from, real_t p_to, real_t p_weight) {
	return GodotMathExtension::inverse_lerp(p_from, p_to, p_weight);
};

float _GodotMathExtension::base_log(float a, float new_base) {
	if (new_base == 1.0) {
		return NAN;
	}
	if (a != 1.0 && (new_base == 0.0 || Math::is_inf(new_base))) {
		return NAN;
	}

	return Math::log(a) / Math::log(new_base);
}

Vector3 _GodotMathExtension::transform_directon_vector(const Vector3 &p_direction, const Basis &p_basis) {
	return GodotMathExtension::transform_directon_vector(p_direction, p_basis);
}

Quat spatial_get_rotation_quat(const Node *spatial) {
	return GodotMathExtension::spatial_get_rotation_quat(spatial);
}

void spatial_set_rotation_quat(Node *spatial, const Quat &rotation) {
	return GodotMathExtension::spatial_set_rotation_quat(spatial, rotation);
}

void spatial_set_rotation_quat_keep_scale(Node *spatial, const Quat &rotation) {
	return GodotMathExtension::spatial_set_rotation_quat_keep_scale(spatial, rotation);
}

_GodotMathExtension::_GodotMathExtension() {
	singleton = this;
}
