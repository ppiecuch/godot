/**************************************************************************/
/*  animation_manager.h                                                   */
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

//
// Created by gen on 15-5-2.
//

#ifndef GODOT_MASTER_ANIMATION_MANAGER_H
#define GODOT_MASTER_ANIMATION_MANAGER_H

#include "core/reference.h"
#include "core/ustring.h"
#include "scene/main/node.h"

class AnimationManager : public Node {
	GDCLASS(AnimationManager, Node);

private:
	Dictionary anim_nodes;
	HashMap<String, String> current_anims;
	void reload_child();

protected:
	virtual Variant getvar(const Variant &p_key, bool *r_valid) const;
	virtual void _play() {}
	virtual void add_child_notify(Node *p_child);
	virtual void remove_child_notify(Node *p_child);
	void _notification(int p_what);
	static void _bind_methods();

public:
	void run();

	Variant get_anim(String p_key);
	void play(String p_key, String p_name);
	void stop(String p_key);
	void stop_with_name(String p_key, String p_name);
	void stop_all();

	AnimationManager() { set_physics_process(true); }
};

#endif //GODOT_MASTER_ANIMATION_MANAGER_H
