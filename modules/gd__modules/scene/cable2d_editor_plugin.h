#ifndef CABLE_2D_EDITOR_PLUGIN_H
#define CABLE_2D_EDITOR_PLUGIN_H

#include "editor/plugins/abstract_polygon_2d_editor.h"
#include "cable2d.h"

class Cable2DEditor : public AbstractPolygon2DEditor {

	GDCLASS(Cable2DEditor, AbstractPolygon2DEditor);

	Cable2D *node;

protected:
	virtual Node2D *_get_node() const;
	virtual void _set_node(Node *p_line);

	virtual bool _is_line() const;
	virtual Variant _get_polygon(int p_idx) const;
	virtual void _set_polygon(int p_idx, const Variant &p_polygon) const;
	virtual void _action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon);

public:
	Cable2DEditor(EditorNode *p_editor);
};

class Cable2DEditorPlugin : public AbstractPolygon2DEditorPlugin {

	GDCLASS(Cable2DEditorPlugin, AbstractPolygon2DEditorPlugin);

public:
	Cable2DEditorPlugin(EditorNode *p_node);
};

#endif // CABLE_2D_EDITOR_PLUGIN_H
