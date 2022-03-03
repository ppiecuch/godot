/*************************************************************************/
/*  statemachine.cpp                                                     */
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

#include "statemachine.h"

void StateMachine::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_ENTER_TREE: {
			add_child_nodes_as_states();
		}
	}
}

void StateMachine::add_child_nodes_as_states() {
	stateMap.clear();
	for (int i = 0; i < get_child_count(); i++) {
		add_new_state(get_child(i));
	}
}

void StateMachine::add_new_state(Node *newState) {
	ERR_FAIL_NULL(newState);
	ERR_FAIL_COND(newState->get_class() != "State");

	String stateName = newState->get_name();

	ERR_FAIL_COND(stateMap.has(stateName));

	stateMap[stateName] = cast_to<State>(newState);
}

PoolStringArray StateMachine::get_all_state_names() {
	PoolStringArray state_name_list;
	Map<StringName, State *>::Element *currentElement = stateMap.front();
	while (currentElement) {
		state_name_list.append(currentElement->key());
		currentElement = currentElement->next();
	}
	return state_name_list;
}

int StateMachine::get_state_amount() {
	return stateMap.size();
}

State *StateMachine::get_active_state() {
	if (stateList.size() > 0 && currentStackIndex < stateList.size()) {
		return stateList[currentStackIndex];
	}
	ERR_PRINT("Couldn't get the current state.");
	return NULL;
}

void StateMachine::execute_active_state(float deltaTime) {
	ERR_FAIL_NULL(get_active_state());
	get_active_state()->execute(deltaTime);
}

void StateMachine::change_active_state_with_name(const StringName &toStateName) {
	ERR_FAIL_COND_MSG(stateMap.size() == 0, "There are no registered states!");
	ERR_FAIL_COND_MSG(!stateMap.has(toStateName), "There are no registered states!");
	State *toStateNode = stateMap[toStateName];
	change_active_state_with_node(toStateNode);
}

void StateMachine::change_state(State *toState, State *fromState) {
	ERR_FAIL_NULL(toState);
	if (toState == fromState) {
		WARN_PRINT("Tried changing to the already active state. This will exit and immediatlely enter that same state and can lead to unpredicted behaviour");
	}
	if (fromState) {
		fromState->exit(toState);
	}
	toState->enter(fromState);
	emit_signal("changed_state", toState->get_name());
}

void StateMachine::change_active_state_with_node(Node *toState) {
	ERR_FAIL_NULL(toState);
	State *castedToState = cast_to<State>(toState);
	State *fromState = get_active_state();
	delete_from_stack_after_index(currentStackIndex);
	currentStackIndex = stateList.size();
	change_state(castedToState, fromState);
	stateList.push_back(castedToState);
}

void StateMachine::delete_from_stack_after_index(int index) {
	if (stateList.size() == 0) {
		return;
	}
	ERR_FAIL_INDEX(index, stateList.size());
	while (stateList.size() > index + 1) {
		stateList.pop_back();
	}
}

void StateMachine::step_back_state() {
	ERR_FAIL_COND(!can_step_back_state());
	step_through_state_history(BACKWARD);
}

bool StateMachine::can_step_back_state() {
	return currentStackIndex > 0;
}

void StateMachine::step_forward_state() {
	ERR_FAIL_COND(!can_step_forward_state());
	step_through_state_history(FORWARD);
}

bool StateMachine::can_step_forward_state() {
	return currentStackIndex < stateList.size() - 1;
}

void StateMachine::step_through_state_history(history_directions direction) {
	State *fromState = get_active_state();
	switch (direction) {
		case FORWARD: {
			currentStackIndex++;
		} break;
		case BACKWARD: {
			currentStackIndex--;
		} break;
	}
	State *toState = stateList[currentStackIndex];
	change_state(toState, fromState);
}

void StateMachine::_bind_methods() {
	ADD_SIGNAL(MethodInfo("changed_state", PropertyInfo(Variant::STRING, "to_state_name")));

	ClassDB::bind_method(D_METHOD("get_all_state_names"), &StateMachine::get_all_state_names);
	ClassDB::bind_method(D_METHOD("get_state_amount"), &StateMachine::get_state_amount);

	ClassDB::bind_method(D_METHOD("add_state", "state"), &StateMachine::add_new_state);

	ClassDB::bind_method(D_METHOD("get_active_state"), &StateMachine::get_active_state);
	ClassDB::bind_method(D_METHOD("execute_current_state", "delta"), &StateMachine::execute_active_state);

	ClassDB::bind_method(D_METHOD("change_active_state_with_name", "name"), &StateMachine::change_active_state_with_name);
	ClassDB::bind_method(D_METHOD("change_active_state_with_node", "state"), &StateMachine::change_active_state_with_node);

	ClassDB::bind_method(D_METHOD("can_step_back_state"), &StateMachine::can_step_back_state);
	ClassDB::bind_method(D_METHOD("can_step_forward_state"), &StateMachine::can_step_forward_state);
	ClassDB::bind_method(D_METHOD("step_back_state"), &StateMachine::step_back_state);
	ClassDB::bind_method(D_METHOD("step_forward_state"), &StateMachine::step_forward_state);
}

StateMachine::StateMachine() {
}
