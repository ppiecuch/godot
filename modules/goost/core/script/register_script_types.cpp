/**************************************************************************/
/*  register_script_types.cpp                                             */
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

#include "register_script_types.h"

#include "mixin_script/editor/mixin_script_editor.h"
#include "mixin_script/editor/mixin_script_editor_plugin.h"
#include "mixin_script/mixin_script.h"

#include "core/script_language.h"
#include "editor/editor_node.h"
#include "editor/plugins/script_editor_plugin.h"

#include "classes_enabled.gen.h"

namespace goost {

static MixinScriptLanguage *script_mixin_script = nullptr;

#if defined(TOOLS_ENABLED) && defined(GOOST_MixinScript)
static ScriptEditorBase *create_editor(const RES &p_resource) {
	if (Object::cast_to<MixinScript>(*p_resource)) {
		return memnew(MixinScriptEditor);
	}
	return nullptr;
}

static void mixin_script_register_editor_callback() {
	ScriptEditor::register_create_script_editor_function(create_editor);
}
#endif

void register_script_types() {
#ifdef GOOST_MixinScript
	script_mixin_script = memnew(MixinScriptLanguage);
	ScriptServer::register_language(script_mixin_script);
	ClassDB::register_class<MixinScript>();
	ClassDB::register_class<Mixin>();

#ifdef TOOLS_ENABLED
	EditorNode::add_plugin_init_callback(mixin_script_register_editor_callback);
	EditorPlugins::add_by_type<MixinScriptEditorPlugin>();
#endif
#endif // GOOST_MixinScript
}

void unregister_script_types() {
#ifdef GOOST_MixinScript
	if (script_mixin_script) {
		ScriptServer::unregister_language(script_mixin_script);
		memdelete(script_mixin_script);
	}
#endif
}

} // namespace goost
