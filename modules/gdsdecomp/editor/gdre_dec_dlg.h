/**************************************************************************/
/*  gdre_dec_dlg.h                                                        */
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

#ifndef GODOT_RE_DEC_DLG_H
#define GODOT_RE_DEC_DLG_H

#include "core/map.h"
#include "core/resource.h"

#include "scene/gui/control.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/item_list.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/text_edit.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#else
#define EDSCALE 1.0
#endif

class ScriptDecompDialog : public AcceptDialog {
	GDCLASS(ScriptDecompDialog, AcceptDialog)

	FileDialog *target_folder_selection;
	FileDialog *file_selection;

	ItemList *file_list;

	Button *add_file;
	Button *remove_file;
	Button *clear_files;

	OptionButton *scrver;

	LineEdit *script_key;
	Label *script_key_error;

	LineEdit *target_dir;
	Button *select_dir;

	void _validate_input();
	void _add_files_pressed();
	void _add_files_request(const PoolVector<String> &p_files);
	void _remove_file_pressed();
	void _clear_pressed();
	void _script_encryption_key_changed(const String &p_key);
	void _dir_select_pressed();
	void _dir_select_request(const String &p_path);
	void _bytcode_changed(int p_id);

protected:
	void _notification(int p_notification);
	static void _bind_methods();

public:
	Vector<String> get_file_list() const;
	String get_target_dir() const;
	Vector<uint8_t> get_key() const;
	int get_bytecode_version() const;

	ScriptDecompDialog();
	~ScriptDecompDialog();
};

#endif
