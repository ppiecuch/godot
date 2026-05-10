/**************************************************************************/
/*  spawner_2d.h                                                          */
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

#pragma once

#include "scene/2d/node_2d.h"

class Spawner2D : public Node2D {
	GDCLASS(Spawner2D, Node2D);

public:
	enum ProcessMode {
		PROCESS_PHYSICS,
		PROCESS_IDLE,
	};

private:
	Ref<Resource> resource; // From which a new node is instantiated: scene or script.
	NodePath spawn_path = NodePath(".");

	bool enabled = true;
	bool local = false;

	uint32_t rate = 1; // Objects per time `step`.
	double step = 1.0; // The `rate` amount of objects will be spawned each time step.
	double delay = 0.0; // Before spawning starts.
	// double lifetime = 0.0; // Lifetime of each object before destroying an object.
	uint32_t limit = 0; // Before spawning stops, zero means unlimited.

	// Modify node's transform on spawning.
	bool modify_position = true;
	bool modify_rotation = true;
	bool modify_scale = true;

	ProcessMode process_mode = PROCESS_IDLE;

	// Internal.
	double time = 0.0;
	bool delayed = false;
	uint32_t amount = 0;

protected:
	void _notification(int p_what);
	static void _bind_methods();

	void _process_spawn();
	void _set_process(bool p_process);

public:
	void set_resource(const Ref<Resource> &p_resource);
	Ref<Resource> get_resource() const { return resource; }

	void set_spawn_path(const NodePath &p_path) { spawn_path = p_path; }
	NodePath get_spawn_path() const { return spawn_path; }

	Node *spawn();

	void set_enabled(bool p_enabled);
	bool is_enabled() const { return enabled; }

	void set_local(bool p_local) { local = p_local; }
	bool is_local() const { return local; }

	void set_rate(int p_rate);
	int get_rate() const { return rate; }

	void set_step(double p_step);
	double get_step() const { return step; }

	void set_delay(double p_delay);
	double get_delay() const { return delay; }

	// void set_lifetime(double p_lifetime);
	// double get_lifetime() const { return lifetime; }

	void set_limit(int p_limit);
	int get_limit() const { return limit; }

	void set_modify_position(bool p_enabled) { modify_position = p_enabled; }
	bool is_modifying_position() const { return modify_position; }

	void set_modify_rotation(bool p_enabled) { modify_rotation = p_enabled; }
	bool is_modifying_rotation() const { return modify_rotation; }

	void set_modify_scale(bool p_enabled) { modify_scale = p_enabled; }
	bool is_modifying_scale() const { return modify_scale; }

	void set_process_mode(ProcessMode p_mode);
	ProcessMode get_process_mode() const { return process_mode; };
};

VARIANT_ENUM_CAST(Spawner2D::ProcessMode);
