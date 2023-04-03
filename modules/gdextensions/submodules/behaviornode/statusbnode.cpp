/**************************************************************************/
/*  statusbnode.cpp                                                       */
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

#include "statusbnode.h"

BehaviorNode::Status StatusBNode::_step(const Variant &target, Dictionary &env) {
	if (!get_behavior_enable() || get_child_count() == 0)
		return STATUS_FAILURE;
	if ((bool)call(StringName("pre_behavior"), target, Variant(env)) && get_child_count() > 0) {
		if (_selected < 0) {
			_selected = 0;
		} else if (_selected >= get_child_count()) {
			_selected = get_child_count() - 1;
		}
		BehaviorNode *b_node = cast_to<BehaviorNode>(get_child(_selected));
		Status childrenStatus = STATUS_FAILURE;
		if (b_node) {
			if (_selected != _old_selected)
				b_node->reset(target);
			childrenStatus = b_node->step(target, env);
		}
		_old_selected = _selected;
		Status status = (Status)((int)call(StringName("behavior"), target, Variant(env)));
		if (status == STATUS_DEPEND_ON_CHILDREN)
			return childrenStatus;
		else
			return status;
	} else {
		return STATUS_FAILURE;
	}
}

void StatusBNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_select", "status"), &StatusBNode::set_select);
	ClassDB::bind_method(D_METHOD("get_select"), &StatusBNode::get_select);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "status/status"), "set_select", "get_select");
}
