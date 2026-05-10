/**************************************************************************/
/*  gdre_pck_dlg.h                                                        */
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

#ifndef GODOT_RE_PCK_DLG_H
#define GODOT_RE_PCK_DLG_H

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

class PackDialog : public AcceptDialog {
	GDCLASS(PackDialog, AcceptDialog)

	FileDialog *target_folder_selection;

	Label *vernfo;
	Label *gennfo;

	Tree *file_list;
	TreeItem *root;

	Label *script_key_error;

	LineEdit *target_dir;
	Button *select_dir;

	bool have_malformed_names;
	bool updating;
	void _update_subitems(TreeItem *p_item, bool p_check, bool p_first = false);
	void _item_edited();

	void _dir_select_pressed();
	void _dir_select_request(const String &p_path);

	void _validate_selection();

	size_t _selected(TreeItem *p_item);
	void _get_selected_files(Vector<String> &p_list, TreeItem *p_item) const;

protected:
	void _notification(int p_notification);
	static void _bind_methods();

public:
	void clear();
	void add_file(const String &p_name, uint64_t p_size, Ref<Texture> p_icon, String p_error, bool p_malformed_name);
	void add_file_to_item(TreeItem *p_item, const String &p_fullname, const String &p_name, uint64_t p_size, Ref<Texture> p_icon, String p_error);
	void set_version(const String &p_version);
	void set_info(const String &p_info);

	Vector<String> get_selected_files() const;
	String get_target_dir() const;

	PackDialog();
	~PackDialog();
};

#endif
