#include "cable2d_editor_plugin.h"

Node2D *Cable2DEditor::_get_node() const {

	return node;
}

void Cable2DEditor::_set_node(Node *p_line) {

	node = Object::cast_to<Cable2D>(p_line);
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
	node = NULL;
}

Cable2DEditorPlugin::Cable2DEditorPlugin(EditorNode *p_node) :
		AbstractPolygon2DEditorPlugin(p_node, memnew(Cable2DEditor(p_node)), "Cable2D") {
}
