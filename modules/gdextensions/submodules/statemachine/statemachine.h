/*************************************************************************/
/*  statemachine.h                                                       */
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

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "core/list.h"
#include "core/map.h"
#include "scene/main/node.h"
#include "state.h"

class StateMachine : public Node {
	GDCLASS(StateMachine, Node);

private:
	enum history_directions {
		FORWARD,
		BACKWARD
	};

	Map<StringName, State *> stateMap;
	List<State *> stateList;
	int currentStackIndex;

	void add_child_nodes_as_states();
	void change_state(State *toState, State *fromState);
	void delete_from_stack_after_index(int index);
	void step_through_state_history(history_directions direction);

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	StateMachine();

	void add_new_state(Node *newState);
	PoolStringArray get_all_state_names();
	int get_state_amount();

	State *get_active_state();
	void change_active_state_with_name(const StringName &toStateName);
	void change_active_state_with_node(Node *toState);

	bool can_step_back_state();
	bool can_step_forward_state();
	void step_back_state();
	void step_forward_state();

	void execute_active_state(float deltaTime);
};

#endif // STATEMACHINE_H
