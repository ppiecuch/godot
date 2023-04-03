/**************************************************************************/
/*  linkerbnode.cpp                                                       */
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

#include "linkerbnode.h"

BehaviorNode::Status LinkerBNode::_step(const Variant &target, Dictionary &env) {
	if (get_behavior_enable() && link_target) {
		int ret = link_target->step(target, env);
		return (BehaviorNode::Status)ret;
	} else {
		return STATUS_FAILURE;
	}
}

void LinkerBNode::set_link_path(NodePath path) {
	link_path = path;
	update_link_path();
}

bool LinkerBNode::_pre_behavior(const Variant &target, const Dictionary &env) {
	if (link_target)
		return link_target->call("pre_behavior", target, env);
	return false;
}

void LinkerBNode::_reset(const Variant &target) {
	link_target->reset(target);
}

void LinkerBNode::update_link_path() {
	if (is_inside_tree() && link_path != NodePath() && has_node(link_path)) {
		link_target = cast_to<BehaviorNode>(get_node(link_path));
	} else {
		link_target = NULL;
	}
}

void LinkerBNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_ENTER_TREE: {
			update_link_path();
			break;
		}
	}
}

void LinkerBNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_link_target"), &LinkerBNode::get_link_target);

	ClassDB::bind_method(D_METHOD("set_link_path", "link_path"), &LinkerBNode::set_link_path);
	ClassDB::bind_method(D_METHOD("get_link_path"), &LinkerBNode::get_link_path);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "link/path"), "set_link_path", "get_link_path");
}
