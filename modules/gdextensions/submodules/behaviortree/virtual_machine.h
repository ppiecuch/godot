/**************************************************************************/
/*  virtual_machine.h                                                     */
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

#ifndef BEHAVIOR_TREE_VIRTUAL_MACHINE_H
#define BEHAVIOR_TREE_VIRTUAL_MACHINE_H

#include "node.h"

namespace BehaviorTree {

struct VMRunningData {
	struct RunningNode {
		Node *node;
		NodeData data;
	};

	BTVector<RunningNode> running_nodes;
	BTVector<IndexType> this_tick_running;
	BTVector<IndexType> last_tick_running;
	IndexType index_marker;

	void tick_begin();
	void tick_end();

	void sort_last_running_nodes();
	bool is_current_node_running_on_last_tick() const;
	void pop_last_running_behavior();
	void add_running_node(Node *node, NodeData node_data);

	IndexType move_index_to_running_child();

	inline void increase_index() { ++index_marker; }
};

class VirtualMachine {
private:
	NodeList &node_list;
	const BTStructure &structure_data;

public:
	VirtualMachine(NodeList &node_list_, const BTStructure &structure_data_) :
			node_list(node_list_), structure_data(structure_data_) {}

	// execute the whole behavior tree.
	void tick(void *context, VMRunningData &running_data);
	// running behavior tree step by step.
	void step(void *context, VMRunningData &running_data);

	void move_index_to_node_end(IndexType index, VMRunningData &running_data);

	inline NodeData get_node_data(IndexType index) const { return structure_data[index]; }

	bool is_child(IndexType parent_index, IndexType child_index) const;

private:
	void cancel_skipped_behaviors(void *context, VMRunningData &running_data);
	void cancel_behavior(void *context, VMRunningData &running_data);
	void run_composites(E_State state, void *context, VMRunningData &running_data);
	E_State run_action(Node &node, void *context, VMRunningData &running_data);
};

} //namespace BehaviorTree

#endif
