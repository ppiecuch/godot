/*************************************************************************/
/*  composite.h                                                          */
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

#ifndef BEHAVIOR_TREE_COMPOSITE_H
#define BEHAVIOR_TREE_COMPOSITE_H

#include "virtual_machine.h"

namespace BehaviorTree {

class Composite : public NodeImpl {
protected:
	virtual E_State self_update(VirtualMachine &, IndexType, void *, VMRunningData &) override;
};

class Selector : public Composite {
protected:
	virtual E_State child_update(VirtualMachine &vm, IndexType index, void *, E_State child_state, VMRunningData &) override;
};

class Sequence : public Composite {
protected:
	virtual void restore_running(VirtualMachine &vm, IndexType index, void *context, VMRunningData &) override;
	virtual E_State child_update(VirtualMachine &vm, IndexType index, void *, E_State child_state, VMRunningData &) override;
};

// instead of running children nodes in seperated thread,
// we run chilren nodes step by step without interruption.
template <E_State RESULT_STATE = BH_SUCCESS>
class Parallel : public Composite {
public:
	Parallel() {
		BT_STATIC_ASSERT(RESULT_STATE != BH_ERROR, "RESULT_STATE cannot be BH_ERROR");
	}

protected:
	virtual E_State child_update(VirtualMachine &vm, IndexType index, void *context, E_State child_state, VMRunningData &) override;
};

template <E_State RESULT_STATE>
E_State Parallel<RESULT_STATE>::child_update(VirtualMachine &, IndexType, void *, E_State child_state, VMRunningData &) {
	return child_state == BH_ERROR ? BH_ERROR : RESULT_STATE;
}

} //namespace BehaviorTree

#endif
