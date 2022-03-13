/*************************************************************************/
/*  composite.cpp                                                        */
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

#include "composite.h"

namespace BehaviorTree {

// Composite
E_State Composite::self_update(VirtualMachine &, IndexType, void *, VMRunningData &) {
	return BH_RUNNING;
}

E_State Selector::child_update(VirtualMachine &vm, IndexType index, void *, E_State child_state, VMRunningData &running_data) {
	if (child_state != BH_FAILURE) {
		vm.move_index_to_node_end(index, running_data);
	}
	return child_state;
}

void Sequence::restore_running(VirtualMachine &vm, IndexType index, void *, VMRunningData &running_data) {
	IndexType child_index = running_data.move_index_to_running_child();
	bool is_child = vm.is_child(index, child_index);
	BT_ASSERT(is_child);
	if (!is_child) {
		vm.move_index_to_node_end(index, running_data);
	}
}

E_State Sequence::child_update(VirtualMachine &vm, IndexType index, void *, E_State child_state, VMRunningData &running_data) {
	if (child_state != BH_SUCCESS) {
		vm.move_index_to_node_end(index, running_data);
	}
	return child_state;
}

} //namespace BehaviorTree
