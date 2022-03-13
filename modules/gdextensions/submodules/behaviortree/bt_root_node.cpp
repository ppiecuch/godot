/*************************************************************************/
/*  bt_root_node.cpp                                                     */
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

#include "core/variant.h"

#include "bt_root_node.h"
#include "bt_utils.h"

BTRootNode::BTRootNode() :
		vm(bt_node_list, bt_structure_data) {
	running_data_list.resize(1);
	BehaviorTree::NodeData node_data;
	node_data.begin = 0;
	node_data.end = 1;
	bt_structure_data.push_back(node_data);
	bt_node_list.push_back(get_behavior_node());
}

void BTRootNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("tick", "context", "index"), &BTRootNode::tick, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("step", "context", "index"), &BTRootNode::step, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("create_running_data"), &BTRootNode::create_running_data);
}

void BTRootNode::tick(Object *context, int index) {
	vm.tick(context, running_data_list.write[index]);
}

void BTRootNode::step(Object *context, int index) {
	vm.step(context, running_data_list.write[index]);
}

int BTRootNode::create_running_data() {
	int running_data_size = running_data_list.size();
	running_data_list.resize(running_data_size + 1);
	return running_data_size;
}

void BTRootNode::add_child_node(BTNode &child, Vector<BehaviorTree::Node *> &node_hierarchy) {
	Vector<BehaviorTree::IndexType> node_hierarchy_index;
	fetch_node_data_list_from_node_hierarchy(node_hierarchy, node_hierarchy_index);

	BehaviorTree::BTStructure temp_bt_structure_data;
	BehaviorTree::NodeList temp_bt_node_list;
	// add new child at the end of its parent.
	int child_index = bt_structure_data[node_hierarchy_index[0]].end;
	create_bt_structure(temp_bt_structure_data, temp_bt_node_list, child, child_index);
	BehaviorTree::IndexType children_count = temp_bt_node_list.size();

	int old_size = bt_structure_data.size();
	int new_size = old_size + children_count;

	bt_structure_data.resize(new_size);
	bt_node_list.resize(new_size);

	for (int i = old_size - 1; i >= child_index; --i) {
		BehaviorTree::NodeData &node_data = bt_structure_data.write[i];
		node_data.begin += children_count;
		node_data.end += children_count;
		bt_structure_data.write[i + children_count] = node_data;
		bt_node_list.write[i + children_count] = bt_node_list[i];
	}

	for (int i = 0; i < children_count; ++i) {
		ERR_FAIL_COND_MSG(child_index + i != temp_bt_structure_data[i].index, "Index of child is not correct.");
		bt_structure_data.write[child_index + i] = temp_bt_structure_data[i];
		bt_node_list.write[child_index + i] = temp_bt_node_list[i];
	}

	int parents_count = node_hierarchy_index.size();
	for (int i = 0; i < parents_count; ++i) {
		bt_structure_data.write[node_hierarchy_index[i]].end += children_count;
	}
}

void BTRootNode::remove_child_node(BTNode &, Vector<BehaviorTree::Node *> &node_hierarchy) {
	Vector<BehaviorTree::IndexType> node_hierarchy_index;
	fetch_node_data_list_from_node_hierarchy(node_hierarchy, node_hierarchy_index);

	BehaviorTree::NodeData child_node_data = bt_structure_data[node_hierarchy_index[0]];
	BehaviorTree::IndexType children_count = child_node_data.end - child_node_data.begin;

	int old_size = bt_structure_data.size();
	int new_size = old_size - children_count;

	for (int i = child_node_data.end; i < old_size; ++i) {
		BehaviorTree::NodeData &node_data = bt_structure_data.write[i];
		node_data.begin -= children_count;
		node_data.end -= children_count;
		bt_structure_data.write[i - children_count] = node_data;
		bt_node_list.write[i - children_count] = bt_node_list[i];
	}

	bt_structure_data.resize(new_size);
	bt_node_list.resize(new_size);

	int parents_count = node_hierarchy_index.size();
	// first one is child itself.
	for (int i = 1; i < parents_count; ++i) {
		bt_structure_data.write[node_hierarchy_index[i]].end -= children_count;
	}
}

void BTRootNode::move_child_node(BTNode &child, Vector<BehaviorTree::Node *> &node_hierarchy) {
	BTNode *parent_node = child.get_parent() ? Object::cast_to<BTNode>(child.get_parent()) : NULL;
	if (!parent_node) {
		ERR_FAIL_MSG("Cannot find a parent node for child.");
		return;
	}

	BehaviorTree::IndexType parent_index = find_node_index_from_node_hierarchy(node_hierarchy);
	BehaviorTree::NodeData parent_node_data = bt_structure_data[parent_index];

	BehaviorTree::BTStructure temp_bt_structure_data;
	BehaviorTree::NodeList temp_bt_node_list;
	create_bt_structure(temp_bt_structure_data, temp_bt_node_list, *parent_node, parent_index);

	if (temp_bt_node_list.size() != parent_node_data.end - parent_node_data.begin ||
			temp_bt_node_list.size() != temp_bt_structure_data.size()) {
		ERR_FAIL_MSG("Move child cannot change total number of node.");
		return;
	}

	for (BehaviorTree::IndexType i = parent_node_data.begin; i < parent_node_data.end; ++i) {
		bt_structure_data.write[i] = temp_bt_structure_data[i - parent_node_data.begin];
		bt_node_list.write[i] = temp_bt_node_list[i - parent_node_data.begin];
	}
}

void BTRootNode::fetch_node_data_list_from_node_hierarchy(
		const Vector<BehaviorTree::Node *> &node_hierarchy,
		Vector<BehaviorTree::IndexType> &node_hierarchy_index) const {
	int node_hierarchy_size = node_hierarchy.size();
	BehaviorTree::IndexType node_data_index = 0;
	node_hierarchy_index.resize(node_hierarchy_size + 1); // plus a root node
	node_hierarchy_index.write[node_hierarchy_size] = 0;

	for (int i = node_hierarchy_size - 1; i >= 0; --i) {
		BehaviorTree::Node *node = node_hierarchy[i];
		node_data_index = find_child_index(node_data_index, node);
		node_hierarchy_index.write[i] = node_data_index;
		ERR_FAIL_COND_MSG(node_data_index != node_hierarchy_index[i], "Cannot find child index.");
	}
}

BehaviorTree::IndexType BTRootNode::find_child_index(BehaviorTree::IndexType parent_index, BehaviorTree::Node *child) const {
	BehaviorTree::IndexType parent_end = bt_structure_data[parent_index].end;
	BehaviorTree::NodeData node_data = bt_structure_data[parent_index + 1];
	while (node_data.end <= parent_end) {
		if (bt_node_list[node_data.index] == child)
			return node_data.index;
		else
			node_data = bt_structure_data[node_data.end];
	}
	return parent_index;
}

BehaviorTree::IndexType BTRootNode::find_node_index_from_node_hierarchy(const Vector<BehaviorTree::Node *> &node_hierarchy) const {
	BehaviorTree::IndexType node_index = 0;
	for (int i = node_hierarchy.size() - 1; i >= 0; --i) {
		BehaviorTree::Node *node = node_hierarchy[i];
		BehaviorTree::IndexType child_node_index = find_child_index(node_index, node);
		ERR_FAIL_COND_V_MSG(node_index != child_node_index, child_node_index, "Cannot find child index.");
		node_index = child_node_index;
	}
	return node_index;
}
