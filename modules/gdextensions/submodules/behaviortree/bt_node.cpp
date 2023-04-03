/**************************************************************************/
/*  bt_node.cpp                                                           */
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

#include "bt_node.h"

void BTNode::move_child_notify(Node *p_child) {
	BTNode *p_btnode = Object::cast_to<BTNode>(p_child);
	ERR_FAIL_NULL_MSG(p_btnode, "Child node is not a BTNode.");

	if (p_btnode) {
		Vector<BehaviorTree::Node *> node_hierarchy;
		move_child_node(*p_btnode, node_hierarchy);
	}
}

void BTNode::add_child_notify(Node *p_child) {
	BTNode *p_btnode = Object::cast_to<BTNode>(p_child);
	ERR_FAIL_NULL_MSG(p_btnode, "Child node is not a BTNode.");
	if (p_btnode) {
		Vector<BehaviorTree::Node *> node_hierarchy;
		add_child_node(*p_btnode, node_hierarchy);
	}
}

void BTNode::remove_child_notify(Node *p_child) {
	BTNode *p_btnode = Object::cast_to<BTNode>(p_child);
	ERR_FAIL_NULL_MSG(p_btnode, "Child node is not a BTNode.");
	if (p_btnode) {
		Vector<BehaviorTree::Node *> node_hierarchy;
		node_hierarchy.push_back(p_btnode->get_behavior_node());
		remove_child_node(*p_btnode, node_hierarchy);
	}
}

void BTNode::_bind_methods() {
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "BH_SUCCESS", BehaviorTree::BH_SUCCESS);
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "BH_FAILURE", BehaviorTree::BH_FAILURE);
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "BH_RUNNING", BehaviorTree::BH_RUNNING);
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "BH_ERROR", BehaviorTree::BH_ERROR);
}
