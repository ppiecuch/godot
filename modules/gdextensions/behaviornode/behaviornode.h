/*************************************************************************/
/*  behaviornode.h                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef BEHAVIOR_NODE_H
#define BEHAVIOR_NODE_H

#include "core/ustring.h"
#include "scene/main/node.h"

class BehaviorNode : public Node {
	GDCLASS(BehaviorNode, Node);

public:
	enum BNodeType {
		TYPE_SEQUENCE,
		TYPE_CONDITION
	};
	enum Status {
		STATUS_DEPEND_ON_CHILDREN,
		STATUS_RUNNING,
		STATUS_FAILURE,
		STATUS_CONTINUE
	};

private:
	NodePath _focus_node_path;
	BNodeType _behavior_node_type;
	bool _behavior_enable;
	bool _will_focus;

protected:
	virtual Status _traversal_children(const Variant &target, const Dictionary &env);

	void _script_reset(const Variant &target);

	virtual bool _pre_behavior(const Variant &target, const Dictionary &env) { return true; }
	virtual Status _behavior(const Variant &target, Dictionary env) { return STATUS_DEPEND_ON_CHILDREN; }
	virtual Status _step(const Variant &target, Dictionary &env);
	virtual void _reset(const Variant &target);
	virtual void _on_notify(const Variant &from, const StringName &key, const Variant &value) {}
	static void _bind_methods();

public:
	void set_focus();
	void clear_focus();

	void send_notify(const Variant &from, const StringName &key, const Variant &value);

	_FORCE_INLINE_ bool get_behavior_enable() { return _behavior_enable; }
	_FORCE_INLINE_ void set_behavior_enable(bool e) { _behavior_enable = e; }

	int get_behavior_node_type() { return _behavior_node_type; }
	void set_behavior_node_type(int behavior_node_type) { _behavior_node_type = (BNodeType)behavior_node_type; }

	_FORCE_INLINE_ bool get_will_focus() { return _will_focus; }
	_FORCE_INLINE_ void set_will_focus(bool focus) { _will_focus = focus; }

	Status step(const Variant &target, Dictionary env);
	void reset(const Variant &target);

	BehaviorNode() {
		_behavior_enable = true;
		_will_focus = false;
		_behavior_node_type = TYPE_SEQUENCE;
	}
	~BehaviorNode() {}
};

VARIANT_ENUM_CAST(BehaviorNode::BNodeType);
VARIANT_ENUM_CAST(BehaviorNode::Status);

#endif // BEHAVIOR_NODE_H
