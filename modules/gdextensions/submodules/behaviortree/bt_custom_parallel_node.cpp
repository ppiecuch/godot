/**************************************************************************/
/*  bt_custom_parallel_node.cpp                                           */
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

#include "bt_custom_parallel_node.h"
#include "bt_string_names.h"

BTCustomParallelNode::BTCustomParallelNode() :
		delegate(*this) {
}

void BTCustomParallelNode::_bind_methods() {
	BIND_VMETHOD(MethodInfo(BTStringNames::get_singleton()->_continue, PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::NIL, "context")));
	BIND_VMETHOD(MethodInfo(BTStringNames::get_singleton()->_prepare, PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::NIL, "context")));
	BIND_VMETHOD(MethodInfo(BTStringNames::get_singleton()->_child_update, PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::NIL, "context"), PropertyInfo(Variant::INT, "child_state")));
	BIND_VMETHOD(MethodInfo(BTStringNames::get_singleton()->_abort, PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::NIL, "context")));
}

void BTCustomParallelNode::Delegate::restore_running(BehaviorTree::VirtualMachine &vm, BehaviorTree::IndexType index, void *context, BehaviorTree::VMRunningData &running_data) {
	super::restore_running(vm, index, context, running_data);
	script_call(BTStringNames::get_singleton()->_continue, index, context);
}

void BTCustomParallelNode::Delegate::prepare(BehaviorTree::VirtualMachine &vm, BehaviorTree::IndexType index, void *context, BehaviorTree::VMRunningData &running_data) {
	super::prepare(vm, index, context, running_data);
	script_call(BTStringNames::get_singleton()->_prepare, index, context);
}

BehaviorTree::E_State BTCustomParallelNode::Delegate::child_update(
		BehaviorTree::VirtualMachine &,
		BehaviorTree::IndexType index,
		void *context,
		BehaviorTree::E_State child_state,
		BehaviorTree::VMRunningData &) {
	Variant result_state = script_call(BTStringNames::get_singleton()->_child_update, index, context, child_state);
	ERR_FAIL_COND_V_MSG(result_state.get_type() != Variant::INT, BehaviorTree::BH_ERROR, "Variant type is not int.");
	return static_cast<BehaviorTree::E_State>(static_cast<int>(result_state));
}

void BTCustomParallelNode::Delegate::abort(BehaviorTree::VirtualMachine &vm, BehaviorTree::IndexType index, void *context, BehaviorTree::VMRunningData &running_data) {
	super::abort(vm, index, context, running_data);
	script_call(BTStringNames::get_singleton()->_abort, index, context);
}
