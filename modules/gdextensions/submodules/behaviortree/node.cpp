/**************************************************************************/
/*  node.cpp                                                              */
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

#include "node.h"
#include "virtual_machine.h"

namespace BehaviorTree {

// NodeImpl
void NodeImpl::restore_running(VirtualMachine &, IndexType, void *, VMRunningData &running_data) {
	running_data.increase_index();
}
void NodeImpl::prepare(VirtualMachine &, IndexType, void *, VMRunningData &running_data) {
	running_data.increase_index();
}
E_State NodeImpl::self_update(VirtualMachine &, IndexType, void *, VMRunningData &) {
	return BH_ERROR;
}
E_State NodeImpl::child_update(VirtualMachine &, IndexType, void *, E_State, VMRunningData &) {
	return BH_ERROR;
}
void NodeImpl::abort(VirtualMachine &, IndexType, void *, VMRunningData &) {}

// Action
E_State Action::self_update(VirtualMachine &, IndexType index, void *context, VMRunningData &running_data) {
	return update(index, context, running_data);
}

E_State Action::update(IndexType, void *, VMRunningData &) {
	return BH_SUCCESS;
}

// Decorator
E_State Decorator::self_update(VirtualMachine &vm, IndexType index, void *context, VMRunningData &running_data) {
	E_State result = pre_update(index, context, running_data);
	if (result == BH_SUCCESS) {
		result = BH_RUNNING;
	} else {
		vm.move_index_to_node_end(index, running_data);
	}
	return result;
}

E_State Decorator::child_update(VirtualMachine &, IndexType index, void *context, E_State child_state, VMRunningData &running_data) {
	return post_update(index, context, child_state, running_data);
}

E_State Decorator::pre_update(IndexType, void *, VMRunningData &) {
	return BH_SUCCESS;
}
E_State Decorator::post_update(IndexType, void *, E_State child_state, VMRunningData &) {
	return child_state;
}

} //namespace BehaviorTree
