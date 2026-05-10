/**************************************************************************/
/*  gui_loader.h                                                          */
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

#ifndef GUI_LOADER_H
#define GUI_LOADER_H

#include "core/reference.h"
#include "gui_document.h"
#include "gui_parser.h"
#include "gui_scene_generator.h"
#include "gui_tokenizer.h"

class GUILoader : public Reference {
	GDCLASS(GUILoader, Reference);

	String last_error;

	Error _parse_source(const String &p_source, GUIDocument &r_doc);
	Node *_build_node_tree(const GUINode &p_node, Node *p_root) const;

protected:
	static void _bind_methods();

public:
	// Compile .gui source text to .tscn text.
	String compile_to_scene(const String &p_source);

	// Load .gui file from path and return a live Node tree.
	Node *load_file(const String &p_path);

	// Load .gui source text and return a live Node tree.
	Node *load_source(const String &p_source);

	String get_last_error() const { return last_error; }
};

#endif // GUI_LOADER_H
