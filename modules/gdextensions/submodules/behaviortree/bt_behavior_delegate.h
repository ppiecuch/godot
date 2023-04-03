/**************************************************************************/
/*  bt_behavior_delegate.h                                                */
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

#ifndef BT_BEHAVIOR_DELEGATE
#define BT_BEHAVIOR_DELEGATE

#include "core/script_language.h"
#include "core/variant.h"

#include "bt_node.h"
#include "typedef.h"

class BTNode;

template <typename BEHAVIOR_NODE>
struct BehaviorDelegate : public BEHAVIOR_NODE {
	BTNode &node;
	BehaviorDelegate(BTNode &node_) :
			node(node_) {}

	Variant script_call(StringName method, BehaviorTree::IndexType, void *context);
	Variant script_call(StringName method, BehaviorTree::IndexType, void *context, BehaviorTree::E_State child_state);
};

template <typename BEHAVIOR_NODE>
Variant BehaviorDelegate<BEHAVIOR_NODE>::script_call(StringName method, BehaviorTree::IndexType index, void *context) {
	ERR_FAIL_NULL_V_MSG(context, BehaviorTree::BH_ERROR, "Context cannot be null");
	Variant v(BehaviorTree::BH_SUCCESS);
	ScriptInstance *script = node.get_script_instance();
	if (script && script->has_method(method)) {
		Variant index_var(index);
		Variant context_var(static_cast<Object *>(context));
		const Variant *ptr[2] = { &index_var, &context_var };
		Variant::CallError err;
		v = script->call(method, ptr, 2, err);
	}
	return v;
}

template <typename BEHAVIOR_NODE>
Variant BehaviorDelegate<BEHAVIOR_NODE>::script_call(StringName method, BehaviorTree::IndexType index, void *context, BehaviorTree::E_State child_state) {
	ERR_FAIL_NULL_V_MSG(context, BehaviorTree::BH_ERROR, "Context cannot be null");
	Variant v(static_cast<int>(child_state));
	ScriptInstance *script = node.get_script_instance();
	if (script && script->has_method(method)) {
		Variant index_var(index);
		Variant context_var(static_cast<Object *>(context));
		Variant child_state_var(static_cast<int>(child_state));
		const Variant *ptr[3] = { &index_var, &context_var, &child_state_var };
		Variant::CallError err;
		v = script->call(method, ptr, 3, err);
	}
	return v;
}

#endif // BT_BEHAVIOR_DELEGATE
