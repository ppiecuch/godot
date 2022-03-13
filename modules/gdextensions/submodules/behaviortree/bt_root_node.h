/*************************************************************************/
/*  bt_root_node.h                                                       */
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

#ifndef BT_ROOT_NODE
#define BT_ROOT_NODE

#include "bt_decorator_node.h"

class BTRootNode : public BTDecoratorNode {
	GDCLASS(BTRootNode, BTDecoratorNode);

	BehaviorTree::BTStructure bt_structure_data;
	BehaviorTree::NodeList bt_node_list;
	BehaviorTree::VirtualMachine vm;
	Vector<BehaviorTree::VMRunningData> running_data_list;

public:
	BTRootNode();

	virtual void add_child_node(BTNode &child, Vector<BehaviorTree::Node *> &node_hierarchy) override;
	virtual void remove_child_node(BTNode &child, Vector<BehaviorTree::Node *> &node_hierarchy) override;
	virtual void move_child_node(BTNode &child, Vector<BehaviorTree::Node *> &node_hierarchy) override;

	int create_running_data();
	void tick(Object *context, int index = 0);
	void step(Object *context, int index = 0);

protected:
	static void _bind_methods();

private:
	void fetch_node_data_list_from_node_hierarchy(
			const Vector<BehaviorTree::Node *> &node_hierarchy,
			Vector<BehaviorTree::IndexType> &node_hierarchy_index) const;

	BehaviorTree::IndexType find_child_index(BehaviorTree::IndexType parent_index, BehaviorTree::Node *child) const;
	BehaviorTree::IndexType find_node_index_from_node_hierarchy(const Vector<BehaviorTree::Node *> &node_hierarchy) const;
};

#endif
