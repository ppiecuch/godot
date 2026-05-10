/**************************************************************************/
/*  git_api.h                                                             */
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

// Initial implementation based on Godot's GDNative version of the Git plugin
// See https://github.com/godotengine/godot-git-plugin.

#include "editor/editor_vcs_interface.h"
#include "editor/plugins/version_control_editor_plugin.h"

#include "git_common.h"
#include <git2.h>

class EditorVCSInterfaceGit : public EditorVCSInterface {
	GDCLASS(EditorVCSInterfaceGit, EditorVCSInterface);

	static EditorVCSInterfaceGit *singleton;

	git_repository *repo = nullptr;
	bool is_initialized = false;

	void _commit(const String p_msg);
	bool _is_vcs_initialized();
	Array _get_modified_files_data();
	Array _get_file_diff(const String file_path, int area);
	String _get_project_name();
	String _get_vcs_name();
	bool _initialize(const String p_project_root_path);
	bool _shut_down();
	void _stage_file(const String p_file_path);
	void _unstage_file(const String p_file_path);
	void _discard_file(String p_file_path);
	Array _get_previous_commits();
	Array _get_branch_list();
	bool _checkout_branch(String p_branch);
	Dictionary _get_data();
	void _fetch();
	void _pull();
	void _push();
	const char *_get_current_branch_name(bool full_ref);
	void _set_up_credentials(String p_username, String p_password);
	Array _parse_diff(git_diff *diff);
	Array _get_line_diff(String p_file_path, String p_text);

protected:
	static void _bind_methods();

public:
	static EditorVCSInterfaceGit *get_singleton() { return singleton; }
	static void set_singleton(EditorVCSInterfaceGit *p_object) { singleton = p_object; }

	Array diff_contents;

	void create_gitignore_and_gitattributes();

	EditorVCSInterfaceGit() {
		if (!singleton) {
			singleton = this;
		}
	}
	virtual ~EditorVCSInterfaceGit(){};
};
