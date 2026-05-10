/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"
#include "core/class_db.h"

#include "bytecode/bytecode_versions.h"
#include "editor/gdre_editor.h"

#ifdef TOOLS_ENABLED
void gdsdecomp_init_callback() {
	EditorNode *editor = EditorNode::get_singleton();
	editor->add_child(memnew(GodotREEditor(editor)));
};
#endif

void register_gdsdecomp_types() {
	ClassDB::register_virtual_class<GDScriptDecomp>();
	register_decomp_versions();

	ClassDB::register_class<GodotREEditorStandalone>();

	ClassDB::register_class<PackDialog>();
	ClassDB::register_class<NewPackDialog>();
	ClassDB::register_class<ScriptCompDialog>();
	ClassDB::register_class<ScriptDecompDialog>();
#ifdef TOOLS_ENABLED
	EditorNode::add_init_callback(&gdsdecomp_init_callback);
#endif
}

void unregister_gdsdecomp_types() {
	//NOP
}
