/**************************************************************************/
/*  cable_2d_editor_plugin.cpp                                            */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "cable_2d_editor_plugin.h"

Node2D *Cable2DEditor::_get_node() const {
	return node;
}

void Cable2DEditor::_set_node(Node *p_line) {
	node = cast_to<Cable2D>(p_line);
}

bool Cable2DEditor::_is_line() const {
	return true;
}

Variant Cable2DEditor::_get_polygon(int p_idx) const {
	return _get_node()->get("points");
}

void Cable2DEditor::_set_polygon(int p_idx, const Variant &p_polygon) const {
	_get_node()->set("points", p_polygon);
}

void Cable2DEditor::_action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon) {
	Node2D *node = _get_node();
	undo_redo->add_do_method(node, "set_points", p_polygon);
	undo_redo->add_undo_method(node, "set_points", p_previous);
}

Cable2DEditor::Cable2DEditor(EditorNode *p_editor) :
		AbstractPolygon2DEditor(p_editor) {
}

Cable2DEditorPlugin::Cable2DEditorPlugin(EditorNode *p_node) :
		AbstractPolygon2DEditorPlugin(p_node, memnew(Cable2DEditor(p_node)), "Cable2D") {
}

// -- Tests --

#ifdef DOCTEST

TEST_CASE("[Cable2DEditor] Cable2D points accessible via Variant property interface") {
	// _get_polygon/_set_polygon use node->get/set("points") — verify the round-trip
	Cable2D cable;

	PoolVector<Vector2> pts;
	pts.push_back(Vector2(10, 20));
	pts.push_back(Vector2(30, 40));
	cable.set("points", pts);

	PoolVector<Vector2> got = cable.get("points");
	REQUIRE(got.size() == 2);
	CHECK(got[0].x == doctest::Approx(10));
	CHECK(got[0].y == doctest::Approx(20));
	CHECK(got[1].x == doctest::Approx(30));
	CHECK(got[1].y == doctest::Approx(40));
}

TEST_CASE("[Cable2DEditor] set_points via Variant updates rendered points") {
	Cable2D cable;
	cable.set_segments(2);

	PoolVector<Vector2> pts;
	pts.push_back(Vector2(0, 0));
	pts.push_back(Vector2(100, 0));
	cable.set("points", pts);

	// Confirm rendered subdivision happened
	CHECK(cable.get_rendered_points().size() == 3);
}

TEST_CASE("[Cable2DEditor] overwriting points via Variant replaces previous") {
	Cable2D cable;

	PoolVector<Vector2> first;
	first.push_back(Vector2(1, 2));
	first.push_back(Vector2(3, 4));
	cable.set("points", first);

	PoolVector<Vector2> second;
	second.push_back(Vector2(5, 6));
	second.push_back(Vector2(7, 8));
	second.push_back(Vector2(9, 10));
	cable.set("points", second);

	PoolVector<Vector2> got = cable.get("points");
	REQUIRE(got.size() == 3);
	CHECK(got[2].x == doctest::Approx(9));
	CHECK(got[2].y == doctest::Approx(10));
}

#endif
