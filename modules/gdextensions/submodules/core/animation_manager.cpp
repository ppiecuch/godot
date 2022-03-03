/*************************************************************************/
/*  animation_manager.cpp                                                */
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
// Created by gen on 15-5-2.
//

#include "animation_manager.h"
#include "core/script_language.h"

Variant AnimationManager::getvar(const Variant &p_key, bool *r_valid) const {
	Variant ret;
	if (anim_nodes.has(p_key)) {
		*r_valid = true;
		ret = anim_nodes[p_key];
	} else {
		ret = Node::getvar(p_key, r_valid);
	}
	return ret;
}

void AnimationManager::run() {
	_play();
	if (get_script_instance()) {
		get_script_instance()->call_multilevel("_play");
	}
}

void AnimationManager::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PHYSICS_PROCESS: {
			run();
		};
	}
}

void AnimationManager::play(String p_key, String p_name) {
	current_anims[p_key] = p_name;
}

void AnimationManager::stop(String p_key) {
	if (current_anims.has(p_key))
		current_anims.erase(p_key);
}

void AnimationManager::stop_with_name(String p_key, String p_name) {
	if (current_anims.has(p_key) && current_anims[p_key] == p_name)
		current_anims.erase(p_key);
}

void AnimationManager::stop_all() {
	current_anims.clear();
}

Variant AnimationManager::get_anim(String p_key) {
	if (current_anims.has(p_key)) {
		return current_anims[p_key];
	} else {
		return Variant();
	}
}

void AnimationManager::add_child_notify(Node *p_child) {
	if (p_child->is_class("AnimationPlayer")) {
		anim_nodes[p_child->get_name()] = p_child;
	}
}

void AnimationManager::remove_child_notify(Node *p_child) {
	if (p_child->is_class("AnimationPlayer") && anim_nodes.has(p_child->get_name())) {
		anim_nodes.erase(p_child->get_name());
	}
}

void AnimationManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("run"), &AnimationManager::run);
	ClassDB::bind_method(D_METHOD("get_anim", "key"), &AnimationManager::get_anim);
	ClassDB::bind_method(D_METHOD("stop_all"), &AnimationManager::stop_all);
	ClassDB::bind_method(D_METHOD("stop_with_name", "key", "name"), &AnimationManager::stop_with_name);
	ClassDB::bind_method(D_METHOD("stop", "key"), &AnimationManager::stop);
	ClassDB::bind_method(D_METHOD("play", "key", "name"), &AnimationManager::play);

	BIND_VMETHOD(MethodInfo("_play"));
}
