/*************************************************************************/
/*  bt_utils.cpp                                                         */
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

#include "bt_utils.h"
#include "bt_node.h"

using namespace BehaviorTree;

static void create_structure_impl(BTStructure &structure_data, NodeList &node_list, BTNode &node, int &index) {
	BT_ASSERT(index < INDEX_TYPE_MAX);
	node_list.push_back(node.get_behavior_node());
	NodeData node_data;
	node_data.begin = index++;
	structure_data.push_back(node_data);
	NodeData &current_node_data = structure_data.back();
	int children_count = node.get_child_count();
	for (int i = 0; i < children_count; ++i) {
		BTNode *p_bt_node = Object::cast_to<BTNode>(node.get_child(i));
		if (p_bt_node)
			create_structure_impl(structure_data, node_list, *p_bt_node, index);
	}
	current_node_data.end = index;
}

void create_bt_structure(BTStructure &structure_data, NodeList &node_list, BTNode &node, int begin) {
	structure_data.clear();
	node_list.clear();
	create_structure_impl(structure_data, node_list, node, begin);
}
