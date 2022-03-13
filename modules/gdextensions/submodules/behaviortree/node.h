/*************************************************************************/
/*  node.h                                                               */
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

#ifndef BEHAVIOR_TREE_NODE_H
#define BEHAVIOR_TREE_NODE_H

#include "typedef.h"

namespace BehaviorTree {
struct VMRunningData;
}
namespace BehaviorTree {
class VirtualMachine;
}

namespace BehaviorTree {

struct Node {
	virtual ~Node() {}

	virtual void restore_running(VirtualMachine &, IndexType, void *, VMRunningData &) = 0;
	virtual void prepare(VirtualMachine &, IndexType, void *, VMRunningData &) = 0;

	virtual E_State self_update(VirtualMachine &, IndexType, void *, VMRunningData &) = 0;
	virtual E_State child_update(VirtualMachine &, IndexType, void *, E_State child_state, VMRunningData &) = 0;

	virtual void abort(VirtualMachine &, IndexType, void *, VMRunningData &) = 0;
};

class NodeImpl : public Node {
protected:
	virtual void restore_running(VirtualMachine &, IndexType, void *, VMRunningData &) override;
	virtual void prepare(VirtualMachine &, IndexType, void *, VMRunningData &) override;
	virtual void abort(VirtualMachine &, IndexType, void *, VMRunningData &) override;
	virtual E_State self_update(VirtualMachine &, IndexType, void *, VMRunningData &) override;
	virtual E_State child_update(VirtualMachine &, IndexType, void *, E_State, VMRunningData &) override;
};

class Action : public NodeImpl {
protected:
	virtual E_State self_update(VirtualMachine &, IndexType index, void *context, VMRunningData &) override;
	virtual E_State update(IndexType, void *, VMRunningData &);
};

class Decorator : public NodeImpl {
protected:
	virtual E_State self_update(VirtualMachine &, IndexType, void *context, VMRunningData &) override;
	virtual E_State child_update(VirtualMachine &, IndexType, void *context, E_State child_state, VMRunningData &) override;
	virtual E_State pre_update(IndexType, void *, VMRunningData &);
	virtual E_State post_update(IndexType, void *, E_State child_state, VMRunningData &);
};

} //namespace BehaviorTree

#endif
