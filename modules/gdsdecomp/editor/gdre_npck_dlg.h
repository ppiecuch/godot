/**************************************************************************/
/*  gdre_npck_dlg.h                                                       */
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

#ifndef GODOT_RE_NPCK_DLG_H
#define GODOT_RE_NPCK_DLG_H

#include "core/map.h"
#include "core/resource.h"

#include "scene/gui/check_box.h"
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

class NewPackDialog : public AcceptDialog {
	GDCLASS(NewPackDialog, AcceptDialog)

	TextEdit *message;

	SpinBox *ver_base;
	SpinBox *ver_major;
	SpinBox *ver_minor;
	SpinBox *ver_rev;

	LineEdit *wmark;

	CheckBox *emb_chk;
	LineEdit *emb_name;
	Button *emb_button;

	FileDialog *emb_selection;

protected:
	void _val_change(double p_val = 0.0f);
	void _notification(int p_notification);
	static void _bind_methods();

	void _exe_select_pressed();
	void _exe_select_request(const String &p_path);

public:
	void set_message(const String &p_text);

	bool get_is_emb() const;
	String get_emb_source() const;

	int get_version_pack() const;
	int get_version_major() const;
	int get_version_minor() const;
	int get_version_rev() const;
	String get_watermark() const;

	NewPackDialog();
	~NewPackDialog();
};

#endif
