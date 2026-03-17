/**************************************************************************/
/*  input_storage.h                                                       */
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

#ifndef INPUT_STORAGE_H
#define INPUT_STORAGE_H

#include "core/reference.h"
#include "scene/main/node.h"

#include "queue.h"

class InputNode {
private:
	Array _pressed_actions;
	Array _down_actions;

public:
	void pressed_action(const String &action);
	void down_action(const String &action);

	bool is_pressed(const String &action) const;
	bool is_pressed(const PoolStringArray &actions) const;
	bool is_down(const String &action) const;
	bool is_down(const PoolStringArray &actions) const;
	int queue_down(const PoolStringArray &actions, int offset) const;
};

class InputStorageNode;

class InputStorage : public Object {
	GDCLASS(InputStorage, Object);

private:
	gdext::Queue<InputNode> storage_events;
	InputNode *_this_frame;
	int storage_size;

	friend class InputStorageNode;

	bool _pressed_in_frame(const Variant &events, int frame);
	bool _down_in_frame(const Variant &events, int frame);
	void _add_node(Object *node);
	InputStorageNode *storage_node;

	static InputStorage *singleton;

protected:
	static void _bind_methods();

public:
	static InputStorage *get_singleton();

	void start(PoolStringArray events);
	void close();
	void resume();

	void frame_begin();
	void pressed_event(const String &event);
	void down_event(const String &event);

	int get_storage_size() { return storage_size; }
	void set_storage_size(int p_storage_size) {
		storage_size = p_storage_size;
		storage_events.alloc(storage_size);
	}

	bool pressed_at(const Variant &event, int at_frame = 0);
	int down_frame(const Variant &event, int in_frame = 1);
	bool is_pressed(const Variant &event, int in_frame = 1);
	bool is_down(const Variant &event, int in_frame = 1);
	bool test_down(const PoolStringArray &events, int in_frame = 10);

	InputStorage();
	~InputStorage();
};

class InputStorageNode : public Node {
	GDCLASS(InputStorageNode, Node);

private:
	Vector<String> pressed;
	PoolStringArray events;
	void _update_events();

	friend class InputStorage;

protected:
	virtual void _input(const Ref<InputEvent> &p_event);

	void _notification(int p_what);
	static void _bind_methods();

public:
	InputStorage *_storage;
	const PoolStringArray &get_events() const { return events; }
	void set_events(const PoolStringArray &e) { events = e; }

	int get_storage_size() { return _storage == NULL ? 0 : _storage->storage_size; }
	void set_storage_size(int size) {
		if (_storage != NULL)
			_storage->storage_size = size;
	}

	InputStorageNode() {
		_storage = NULL;

		set_process_input(true);
		set_physics_process(true);
		set_pause_mode(Node::PAUSE_MODE_PROCESS);
	}
	~InputStorageNode() {}
};

#endif // INPUT_STORAGE_H
