/*************************************************************************/
/*  utils.h                                                              */
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

#ifndef BEHAVIOR_TREE_TEST_UTILS_H
#define BEHAVIOR_TREE_TEST_UTILS_H

#include "composite.h"
#include "virtual_machine.h"

using namespace BehaviorTree;

struct ConstructNode {
	std::vector<ConstructNode> children;
	Node *node;

	ConstructNode(Node *node_ = nullptr) :
			node(node_) {}
};

struct Counter {
	int self_update;
	int child_update;
	int abort;
	int prepare;
};

struct MockAgent {
	// TODO: use a blackboard to store node data?
	struct NodeData {
		Counter counter;
		E_State self_update_result;
		E_State child_update_result;
	};
	std::vector<NodeData> data_list;

	void reset() {
		for (NodeData &data : data_list) {
			data.counter.self_update = 0;
			data.counter.child_update = 0;
			data.counter.abort = 0;
			data.counter.prepare = 0;
			data.self_update_result = BH_ERROR;
			data.child_update_result = BH_ERROR;
		}
	}
};

void to_vm(BTStructure &structure_data, NodeList &node_list, ConstructNode &node);
void tick_vm(VirtualMachine &vm, MockAgent &agent, VMRunningData &running_data);

template <typename T>
struct MockNode : public T {
	ConstructNode inner_node;

	MockNode() { inner_node.node = this; }

	virtual void prepare(VirtualMachine &vm, IndexType index, void *context, VMRunningData &running_data) override {
		MockAgent *agent = static_cast<MockAgent *>(context);
		MockAgent::NodeData &node_data = agent->data_list[index];
		++node_data.counter.prepare;
		T::prepare(vm, index, context, running_data);
	}

	virtual void abort(VirtualMachine &vm, IndexType index, void *context, VMRunningData &running_data) override {
		MockAgent *agent = static_cast<MockAgent *>(context);
		MockAgent::NodeData &node_data = agent->data_list[index];
		++node_data.counter.abort;
		T::abort(vm, index, context, running_data);
	}

	virtual E_State self_update(VirtualMachine &vm, IndexType index, void *context, VMRunningData &running_data) override {
		MockAgent *agent = static_cast<MockAgent *>(context);
		MockAgent::NodeData &node_data = agent->data_list[index];
		++node_data.counter.self_update;
		node_data.self_update_result = T::self_update(vm, index, context, running_data);
		return node_data.self_update_result;
	}

	virtual E_State child_update(VirtualMachine &vm, IndexType index, void *context, E_State child_state, VMRunningData &running_data) override {
		MockAgent *agent = static_cast<MockAgent *>(context);
		MockAgent::NodeData &node_data = agent->data_list[index];
		++node_data.counter.child_update;
		node_data.child_update_result = T::child_update(vm, index, context, child_state, running_data);
		return node_data.child_update_result;
	}
};

struct MockAction : public MockNode<Action> {
	E_State update_result;
	MockAction() :
			update_result(BH_SUCCESS) {}
	MockAction(E_State result) :
			update_result(result) {}

	virtual E_State update(IndexType, void *, VMRunningData &) override {
		return update_result;
	}
};

struct MockDecorator : public MockNode<Decorator> {};
struct MockSelector : public MockNode<Selector> {};
struct MockSequence : public MockNode<Sequence> {};
struct MockParallel : public MockNode<Parallel<BH_SUCCESS>> {};

#endif
