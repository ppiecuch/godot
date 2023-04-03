/**************************************************************************/
/*  bt_custom_parallel_node.h                                             */
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

#ifndef BT_CUSTOM_PARALLEL_NODE_H
#define BT_CUSTOM_PARALLEL_NODE_H

#include "bt_behavior_delegate.h"
#include "bt_composite_node.h"

class BTCustomParallelNode : public BTCompositeNode {
	GDCLASS(BTCustomParallelNode, BTCompositeNode);

	struct Delegate : public BehaviorDelegate<BehaviorTree::Composite> {
		typedef BehaviorDelegate<BehaviorTree::Composite> super;

		Delegate(BTCustomParallelNode &node_) :
				super(node_) {}

		virtual void restore_running(BehaviorTree::VirtualMachine &, BehaviorTree::IndexType index, void *context, BehaviorTree::VMRunningData &running_data) override;
		virtual void prepare(BehaviorTree::VirtualMachine &, BehaviorTree::IndexType index, void *context, BehaviorTree::VMRunningData &running_data) override;
		virtual BehaviorTree::E_State child_update(BehaviorTree::VirtualMachine &, BehaviorTree::IndexType, void *, BehaviorTree::E_State child_state, BehaviorTree::VMRunningData &running_data) override;
		virtual void abort(BehaviorTree::VirtualMachine &, BehaviorTree::IndexType, void *, BehaviorTree::VMRunningData &running_data) override;
	};
	Delegate delegate;

	virtual BehaviorTree::Node *get_behavior_node() override { return &delegate; }

public:
	BTCustomParallelNode();

protected:
	static void _bind_methods();
};

#endif // BT_CUSTOM_PARALLEL_NODE_H
