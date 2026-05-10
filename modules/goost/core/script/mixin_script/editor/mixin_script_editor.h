/**************************************************************************/
/*  mixin_script_editor.h                                                 */
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

#include "../mixin_script.h"

#include "editor/plugins/script_editor_plugin.h"
#include "scene/gui/button.h"
#include "scene/gui/center_container.h"

class MixinScriptEditor : public ScriptEditorBase {
	GDCLASS(MixinScriptEditor, ScriptEditorBase);

	enum {
		BUTTON_EDIT,
		BUTTON_MOVE_UP,
		BUTTON_MOVE_DOWN,
		BUTTON_REMOVE,
	};

	Ref<MixinScript> script;

	VBoxContainer *vbox = nullptr;

	Button *add_mixin_button = nullptr;
	Tree *tree_mixins = nullptr;
	PanelContainer *panel_mixins = nullptr;

	bool update_queued = false;
	void _update_list();

	void _on_editor_script_changed(Ref<Script> p_script);

	void _on_add_mixin_pressed();
	void _on_mixin_created(Ref<Script> p_script);
	void _on_mixin_creation_closed();
	void _on_mixin_button_pressed(Object *p_item, int p_column, int p_button);
	void _on_mixin_activated();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	virtual void add_syntax_highlighter(SyntaxHighlighter *p_highlighter);
	virtual void set_syntax_highlighter(SyntaxHighlighter *p_highlighter);

	virtual void apply_code();
	virtual RES get_edited_resource() const;
	virtual void set_edited_resource(const RES &p_res);
	virtual void enable_editor();
	virtual Vector<String> get_functions();
	virtual void reload_text();
	virtual String get_name();
	virtual Ref<Texture> get_icon();
	virtual bool is_unsaved();
	virtual Variant get_edit_state();
	virtual void set_edit_state(const Variant &p_state);
	virtual void goto_line(int p_line, bool p_with_error = false);
	virtual void set_executing_line(int p_line);
	virtual void clear_executing_line();
	virtual void trim_trailing_whitespace();
	virtual void insert_final_newline();
	virtual void convert_indent_to_spaces();
	virtual void convert_indent_to_tabs();
	virtual void ensure_focus();
	virtual void tag_saved_version();
	virtual void reload(bool p_soft);
	virtual void get_breakpoints(List<int> *p_breakpoints);
	virtual void add_callback(const String &p_function, PoolStringArray p_args);
	virtual void update_settings();
	virtual bool show_members_overview();
	virtual void set_debugger_active(bool p_active);
	virtual void set_tooltip_request_func(String p_method, Object *p_obj);
	virtual Control *get_edit_menu();
	virtual void clear_edit_menu();
	virtual bool can_lose_focus_on_node_selection() { return false; }
	virtual void validate();

	void queue_update();

	MixinScriptEditor();
	~MixinScriptEditor();
};
