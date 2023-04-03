/**************************************************************************/
/*  rope_server.h                                                         */
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

#ifndef ROPE_SERVER_H
#define ROPE_SERVER_H

// Build from 20 commits

#include "scene/2d/node_2d.h"
#include "scene/2d/sprite.h"
#include "scene/main/node.h"

#include <vector>

class RopeServer : public Node {
	GDCLASS(RopeServer, Node)

	std::vector<Node2D *> _ropes;
	float _last_time;
	bool _update_in_editor;

	void _init();
	void _enter_tree();
	void _physics_process(float delta);

	void _start_stop_process();
	void _simulate(Node2D *rope, float delta);

protected:
	void _notification(int what);

	static void _bind_methods();

public:
	void set_update_in_editor(bool value);
	bool get_update_in_editor() const;

	int get_num_ropes() const;
	float get_computation_time() const;

	void register_rope(Node *rope);
	void unregister_rope(Node *rope);

	RopeServer();
	~RopeServer();
};

#endif // ROPE_SERVER_H
