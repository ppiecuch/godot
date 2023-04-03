/**************************************************************************/
/*  test_action.cpp                                                       */
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

#include "catch.hpp"
#include "utils.h"

TEST_CASE("Behavior Tree Action", "[bt_act]") {
	VMRunningData data;
	BTStructure structure_data;
	NodeList node_list;
	VirtualMachine vm(node_list, structure_data);
	MockAction action;

	SECTION("S") {
		MockAgent agent;
		agent.data_list.resize(1);
		const MockAgent::NodeData &action_data = agent.data_list[0];
		to_vm(structure_data, node_list, action.inner_node);

		action.update_result = BH_SUCCESS;
		tick_vm(vm, agent, data);
		REQUIRE(action_data.counter.prepare == 1);
		REQUIRE(action_data.counter.abort == 0);
		REQUIRE(action_data.counter.self_update == 1);
		REQUIRE(action_data.counter.child_update == 0);
	}

	SECTION("F") {
		MockAgent agent;
		agent.data_list.resize(1);
		const MockAgent::NodeData &action_data = agent.data_list[0];
		to_vm(structure_data, node_list, action.inner_node);

		action.update_result = BH_FAILURE;
		tick_vm(vm, agent, data);
		REQUIRE(action_data.counter.prepare == 1);
		REQUIRE(action_data.counter.abort == 0);
		REQUIRE(action_data.counter.self_update == 1);
		REQUIRE(action_data.counter.child_update == 0);
	}

	SECTION("R") {
		MockAgent agent;
		agent.data_list.resize(1);
		const MockAgent::NodeData &action_data = agent.data_list[0];
		to_vm(structure_data, node_list, action.inner_node);

		action.update_result = BH_RUNNING;
		tick_vm(vm, agent, data);
		REQUIRE(action_data.counter.prepare == 1);
		REQUIRE(action_data.counter.abort == 0);
		REQUIRE(action_data.counter.self_update == 1);
		REQUIRE(action_data.counter.child_update == 0);
	}
}
