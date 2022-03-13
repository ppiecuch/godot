/*************************************************************************/
/*  test_decorator.cpp                                                   */
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

#include "catch.hpp"
#include "utils.h"

TEST_CASE("Behavior Tree FaitureDecorator", "[bt_dec]") {
	VMRunningData data;
	BTStructure structure_data;
	NodeList node_list;
	VirtualMachine vm(node_list, structure_data);
	MockDecorator decorator;
	MockAgent agent;
	agent.data_list.resize(1);

	SECTION("+") {
		const MockAgent::NodeData &decorator_data = agent.data_list[0];
		to_vm(structure_data, node_list, decorator.inner_node);

		tick_vm(vm, agent, data);
		REQUIRE(decorator_data.counter.prepare == 1);
		REQUIRE(decorator_data.counter.abort == 0);
		REQUIRE(decorator_data.counter.self_update == 1);
		REQUIRE(decorator_data.counter.child_update == 0);
	}

	MockAction action_foo;
	decorator.inner_node.children.push_back(action_foo.inner_node);
	agent.data_list.resize(2);

	SECTION("+S") {
		const MockAgent::NodeData &decorator_data = agent.data_list[0];
		const MockAgent::NodeData &action_foo_data = agent.data_list[1];
		to_vm(structure_data, node_list, decorator.inner_node);

		action_foo.update_result = BH_SUCCESS;
		tick_vm(vm, agent, data);
		REQUIRE(decorator_data.child_update_result == BH_SUCCESS);
		REQUIRE(decorator_data.counter.prepare == 1);
		REQUIRE(decorator_data.counter.abort == 0);
		REQUIRE(decorator_data.counter.self_update == 1);
		REQUIRE(decorator_data.counter.child_update == 1);
		REQUIRE(action_foo_data.counter.prepare == 1);
		REQUIRE(action_foo_data.counter.abort == 0);
		REQUIRE(action_foo_data.counter.self_update == 1);
		REQUIRE(action_foo_data.counter.child_update == 0);

		tick_vm(vm, agent, data);
		REQUIRE(decorator_data.child_update_result == BH_SUCCESS);
		REQUIRE(decorator_data.counter.prepare == 1);
		REQUIRE(decorator_data.counter.abort == 0);
		REQUIRE(decorator_data.counter.self_update == 1);
		REQUIRE(decorator_data.counter.child_update == 1);
		REQUIRE(action_foo_data.counter.prepare == 1);
		REQUIRE(action_foo_data.counter.abort == 0);
		REQUIRE(action_foo_data.counter.self_update == 1);
		REQUIRE(action_foo_data.counter.child_update == 0);
	}

	SECTION("+F") {
		const MockAgent::NodeData &decorator_data = agent.data_list[0];
		const MockAgent::NodeData &action_foo_data = agent.data_list[1];
		to_vm(structure_data, node_list, decorator.inner_node);

		action_foo.update_result = BH_FAILURE;
		tick_vm(vm, agent, data);
		REQUIRE(decorator_data.child_update_result == BH_FAILURE);
		REQUIRE(decorator_data.counter.prepare == 1);
		REQUIRE(decorator_data.counter.abort == 0);
		REQUIRE(decorator_data.counter.self_update == 1);
		REQUIRE(decorator_data.counter.child_update == 1);
		REQUIRE(action_foo_data.counter.prepare == 1);
		REQUIRE(action_foo_data.counter.abort == 0);
		REQUIRE(action_foo_data.counter.self_update == 1);
		REQUIRE(action_foo_data.counter.child_update == 0);

		tick_vm(vm, agent, data);
		REQUIRE(decorator_data.child_update_result == BH_FAILURE);
		REQUIRE(decorator_data.counter.prepare == 1);
		REQUIRE(decorator_data.counter.abort == 0);
		REQUIRE(decorator_data.counter.self_update == 1);
		REQUIRE(decorator_data.counter.child_update == 1);
		REQUIRE(action_foo_data.counter.prepare == 1);
		REQUIRE(action_foo_data.counter.abort == 0);
		REQUIRE(action_foo_data.counter.self_update == 1);
		REQUIRE(action_foo_data.counter.child_update == 0);
	}

	SECTION("+R") {
		const MockAgent::NodeData &decorator_data = agent.data_list[0];
		const MockAgent::NodeData &action_foo_data = agent.data_list[1];
		to_vm(structure_data, node_list, decorator.inner_node);

		action_foo.update_result = BH_RUNNING;
		tick_vm(vm, agent, data);
		REQUIRE(decorator_data.child_update_result == BH_RUNNING);
		REQUIRE(decorator_data.counter.prepare == 1);
		REQUIRE(decorator_data.counter.abort == 0);
		REQUIRE(decorator_data.counter.self_update == 1);
		REQUIRE(decorator_data.counter.child_update == 1);
		REQUIRE(action_foo_data.counter.prepare == 1);
		REQUIRE(action_foo_data.counter.abort == 0);
		REQUIRE(action_foo_data.counter.self_update == 1);
		REQUIRE(action_foo_data.counter.child_update == 0);

		tick_vm(vm, agent, data);
		REQUIRE(decorator_data.child_update_result == BH_RUNNING);
		REQUIRE(decorator_data.counter.prepare == 0);
		REQUIRE(decorator_data.counter.abort == 0);
		REQUIRE(decorator_data.counter.self_update == 1);
		REQUIRE(decorator_data.counter.child_update == 1);
		REQUIRE(action_foo_data.counter.prepare == 0);
		REQUIRE(action_foo_data.counter.abort == 0);
		REQUIRE(action_foo_data.counter.self_update == 1);
		REQUIRE(action_foo_data.counter.child_update == 0);
	}
}
