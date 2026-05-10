/**************************************************************************/
/*  poly_node_2d_editor_plugin.h                                          */
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

#include "core/math/geometry/2d/poly/poly_node_2d.h"
#include "editor/plugins/abstract_polygon_2d_editor.h"

class PolyNode2DEditor : public AbstractPolygon2DEditor {
	GDCLASS(PolyNode2DEditor, AbstractPolygon2DEditor);

	friend class PolyNode2DEditorPlugin;

protected:
	PolyNode2D *node = nullptr;

	enum Menu {
		MENU_OPTION_FLATTEN_OUTLINES = 100,
	};
	MenuButton *options = nullptr;
	Menu selected_menu_item;
	virtual void _menu_option(int p_option);

	static PolyNode2DEditor *singleton;

	virtual void _set_node(Node *p_node);
	virtual Node2D *_get_node() const;

	virtual bool _is_line() const;
	virtual Variant _get_polygon(int p_idx) const;
	virtual void _set_polygon(int p_idx, const Variant &p_polygon) const;
	virtual void _action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon);

public:
	static PolyNode2DEditor *get_singleton() { return singleton; }

	PolyNode2DEditor(EditorNode *p_editor);
};

class PolyNode2DEditorPlugin : public AbstractPolygon2DEditorPlugin {
	GDCLASS(PolyNode2DEditorPlugin, AbstractPolygon2DEditorPlugin);

public:
	virtual void make_visible(bool p_visible);

	PolyNode2DEditorPlugin(EditorNode *p_node);
};
