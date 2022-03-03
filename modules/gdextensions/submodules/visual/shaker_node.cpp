/*************************************************************************/
/*  shaker_node.cpp                                                      */
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

#include "shaker_node.h"

void ShakerNode::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
		} break;
	}
}

void ShakerNode::set_target_node_path(const NodePath &p_node) {
}

void ShakerNode::set_target_group_name(const String &p_name) {
}

void ShakerNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node_path"), &ShakerNode::set_target_node_path);
	ClassDB::bind_method(D_METHOD("set_target_group_name"), &ShakerNode::set_target_group_name);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "affect_position"), "set_affect_position", "get_affect_position");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "affect_rotation"), "set_affect_rotation", "get_affect_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_position_shake"), "set_max_position_shake", "get_max_position_shake");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_rotation_shake"), "set_max_rotation_shake", "get_max_rotation_shake");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trauma_decay"), "set_trauma_decay", "get_trauma_decay");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_time_scale"), "set_use_time_scale", "get_use_time_scale");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node_path"), "set_target_node_path", "get_target_node_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "target_group_name"), "set_target_group_name", "get_target_group_name");
}

ShakerNode::ShakerNode() {
	affect_position = true;
	affect_rotation = true;
	max_rotation_shake = 15;
	max_position_shake = 1;
	trauma_decay = 0.01;
	use_time_scale = true;
}

#if 0 // GDScript original source code
export (bool) var affect_position = true
export (bool) var affect_rotation = true
export (float) var max_rotation_shake: float = 15  # This value is measured in degrees
export (float) var max_position_shake: float = 1  # This value is measured in meters
export (NodePath) var node_path setget set_node_path
export (float) var trauma_decay = 0.01  # How fast should trauma decay over time
export (bool) var use_time_scale = true  # If true, time will affect the shaking

var original_position: Vector3 setget , get_original_position
var original_rotation: Vector3 setget , get_original_rotation
var _elapsed_game_seconds: float = 0
var _noise_generator := OpenSimplexNoise.new()
var _seeds: Array = [
	randi(), randi(), randi(), randi(), randi(), randi(), randi(), randi(), randi()
]
var _trauma_amount: float = 0
var _is_node_rotation_modified: bool = false
var _is_node_position_modified: bool = false

onready var _node: Node = get_node(node_path)


func _process(delta: float) -> void:
	_elapsed_game_seconds += delta
	if _node:
		if _trauma_amount > 0:
			var shake_amount: float = pow(_trauma_amount, 2)
			if affect_rotation:
				if _is_node_rotation_modified == false:
					set_original_rotation(_node)
					_is_node_rotation_modified = true

				if _node is Node2D:
					var rotation_shake: float = _get_shake(
						max_rotation_shake, shake_amount, _seeds[0]
					)
					_node.rotation = get_original_rotation()
					_node.rotation = deg2rad(rotation_shake)

				if _node is Spatial:
					var x_shake: float = _get_shake(max_rotation_shake, shake_amount, _seeds[1])
					var y_shake: float = _get_shake(max_rotation_shake, shake_amount, _seeds[2])
					var z_shake: float = _get_shake(max_rotation_shake, shake_amount, _seeds[3])
					_node.rotation = get_original_rotation()
					_node.rotate_x(deg2rad(x_shake))
					_node.rotate_y(deg2rad(y_shake))
					_node.rotate_z(deg2rad(z_shake))

			if affect_position:
				if _is_node_position_modified == false:
					set_original_position(_node)
					_is_node_position_modified = true

				if _node is Node2D:
					var x_shake: float = _get_shake(max_position_shake, shake_amount, _seeds[4])
					var y_shake: float = _get_shake(max_position_shake, shake_amount, _seeds[5])
					_node.position = get_original_position()
					_node.translate(Vector2(x_shake, y_shake))

				if _node is Spatial:
					var x_shake: float = _get_shake(max_position_shake, shake_amount, _seeds[6])
					var y_shake: float = _get_shake(max_position_shake, shake_amount, _seeds[7])
					var z_shake: float = _get_shake(max_position_shake, shake_amount, _seeds[8])
					_node.translation = get_original_position()
					_node.translate(Vector3(x_shake, y_shake, z_shake))

		else:
#Restore the node 's position to it' s original value if it has been modified
			if _is_node_position_modified:
				if _node is Node2D:
					_node.position = get_original_position()
				if _node is Spatial:
					_node.translation = get_original_position()
				_is_node_position_modified = false
#Restore the node 's rotation to it' s original value if it has been modified
			if _is_node_rotation_modified:
				_node.rotation = get_original_rotation()
				_is_node_rotation_modified = false
#Save the node position and rotation
			set_original_position(_node)
			set_original_rotation(_node)

#Decrease trauma linearly
	_trauma_amount = max(_trauma_amount - trauma_decay * Engine.time_scale, 0)

#Trauma
func add_trauma(amount: float) -> void:
	_trauma_amount = min(_trauma_amount + amount, 1)

#Node path
func set_node_path(new_value: NodePath) -> void:
	node_path = new_value
	_node = get_node(node_path)
	if _node:
		set_original_position(_node)
		set_original_rotation(_node)

#Original position
func set_original_position(node: Node) -> void:
	if node is Node2D:
		original_position.x = node.position.x
		original_position.y = node.position.y
	if node is Spatial:
		original_position = node.translation

func get_original_position():
	if _node is Node2D:
		return Vector2(original_position.x, original_position.y)
	if _node is Spatial:
		return original_position

#Original rotation
func set_original_rotation(node: Node) -> void:
	if node is Node2D:
		original_rotation.x = node.rotation
	if node is Spatial:
		original_rotation = node.rotation

func get_original_rotation():
	if _node is Node2D:
		return original_rotation.x
	if _node is Spatial:
		return original_rotation

#Noise
func _get_noise(x: int) -> float:
	var y: float = _elapsed_game_seconds * 1000 if use_time_scale else randi()
	return _noise_generator.get_noise_2d(x, y)

#Shake
func _get_shake(maximum: float, amount: float, x: int) -> float:
	return maximum * amount * _get_noise(x)

#endif
