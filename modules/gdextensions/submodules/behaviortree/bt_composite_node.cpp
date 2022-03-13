/*************************************************************************/
/*  bt_composite_node.cpp                                                */
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

#include "bt_composite_node.h"

void BTCompositeNode::add_child_node(BTNode &child, Vector<BehaviorTree::Node *> &node_hierarchy) {
	BTNode *p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	ERR_FAIL_NULL_MSG(p_parent, "Parent node is not a BTNode.");
	if (p_parent) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->add_child_node(child, node_hierarchy);
	}
}

void BTCompositeNode::remove_child_node(BTNode &child, Vector<BehaviorTree::Node *> &node_hierarchy) {
	BTNode *p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	if (p_parent) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->remove_child_node(child, node_hierarchy);
	}
}

void BTCompositeNode::move_child_node(BTNode &child, Vector<BehaviorTree::Node *> &node_hierarchy) {
	BTNode *p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	if (p_parent) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->move_child_node(child, node_hierarchy);
	}
}
