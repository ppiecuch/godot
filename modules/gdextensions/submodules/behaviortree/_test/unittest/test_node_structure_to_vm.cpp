/**************************************************************************/
/*  test_node_structure_to_vm.cpp                                         */
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

TEST_CASE("Node Structure", "[nodestructure]") {
	BTStructure structure_data;
	NodeList node_list;
	VirtualMachine vm(node_list, structure_data);

	SECTION("single node") {
		ConstructNode node;
		to_vm(structure_data, node_list, node);
		REQUIRE(structure_data.size() == 1);
		REQUIRE(structure_data[0].begin == 0);
		REQUIRE(structure_data[0].end == 1);
	}

	SECTION("several children") {
		ConstructNode node;
		node.children.resize(10);
		to_vm(structure_data, node_list, node);
		REQUIRE(structure_data.size() == 11);
		REQUIRE(structure_data[0].begin == 0);
		REQUIRE(structure_data[0].end == 11);
		for (size_t i = 1; i < structure_data.size(); i++) {
			auto node_data = structure_data[i];
			REQUIRE(node_data.begin == i);
			REQUIRE(node_data.end == i + 1);
		}
	}

	SECTION("several hierachy") {
		ConstructNode root;
		root.children.resize(1);
		auto iter = root.children.begin();
		for (int i = 0; i < 20; i++) {
			iter->children.resize(1);
			iter = iter->children.begin();
		}
		to_vm(structure_data, node_list, root);
		REQUIRE(structure_data.size() == 22);
		for (size_t i = 0; i < structure_data.size(); i++) {
			auto node_data = structure_data[i];
			REQUIRE(node_data.begin == i);
			REQUIRE(node_data.end == 22);
		}
	}

	SECTION("complex node tree") {
		ConstructNode root;
		root.children.resize(4);
		root.children[0].children.resize(3);
		root.children[0].children[0].children.resize(2);
		root.children[0].children[0].children[0].children.resize(1);
		root.children[0].children[1].children.resize(1);
		root.children[1].children.resize(2);
		root.children[1].children[0].children.resize(1);
		root.children[2].children.resize(1);
		to_vm(structure_data, node_list, root);
		REQUIRE(structure_data.size() == 16);
		for (size_t i = 0; i < structure_data.size(); i++)
			REQUIRE(structure_data[i].begin == i);
		REQUIRE(structure_data[0].end == 16);
		REQUIRE(structure_data[1].end == 9);
		REQUIRE(structure_data[2].end == 6);
		REQUIRE(structure_data[3].end == 5);
		REQUIRE(structure_data[4].end == 5);
		REQUIRE(structure_data[5].end == 6);
		REQUIRE(structure_data[6].end == 8);
		REQUIRE(structure_data[7].end == 8);
		REQUIRE(structure_data[8].end == 9);
		REQUIRE(structure_data[9].end == 13);
		REQUIRE(structure_data[10].end == 12);
		REQUIRE(structure_data[11].end == 12);
		REQUIRE(structure_data[12].end == 13);
		REQUIRE(structure_data[13].end == 15);
		REQUIRE(structure_data[14].end == 15);
		REQUIRE(structure_data[15].end == 16);
	}
}
