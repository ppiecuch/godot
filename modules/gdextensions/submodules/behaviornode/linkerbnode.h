/*************************************************************************/
/*  linkerbnode.h                                                        */
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

#ifndef GODOT_MASTER_LINKERBNODE_H
#define GODOT_MASTER_LINKERBNODE_H

#include "behaviornode.h"

class LinkerBNode : public BehaviorNode {
	GDCLASS(LinkerBNode, BehaviorNode);

private:
	BehaviorNode *link_target;
	NodePath link_path;
	void update_link_path();

protected:
	void _notification(int p_notification);
	virtual bool _pre_behavior(const Variant &target, const Dictionary &env);
	virtual Status _step(const Variant &target, Dictionary &env);
	virtual void _reset(const Variant &target);
	static void _bind_methods();

public:
	_FORCE_INLINE_ BehaviorNode *get_link_target() {
		if (!link_target)
			update_link_path();
		return link_target;
	}

	void set_link_path(NodePath path);
	_FORCE_INLINE_ NodePath get_link_path() { return link_path; }
	_FORCE_INLINE_ LinkerBNode() { link_target = NULL; }
};

#endif //GODOT_MASTER_LINKERBNODE_H
