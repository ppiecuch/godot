/**************************************************************************/
/*  gui_parser.h                                                          */
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

#ifndef GUI_PARSER_H
#define GUI_PARSER_H

#include "gui_document.h"
#include "gui_tokenizer.h"

class GUIParser {
	const Vector<GUITokenizer::Token> *tokens;
	int pos;
	String error_message;

	const GUITokenizer::Token &_current() const;
	const GUITokenizer::Token &_peek(int p_offset = 0) const;
	bool _at_end() const;
	bool _match(GUITokenizer::TokenType p_type);
	bool _expect(GUITokenizer::TokenType p_type, const String &p_context);
	void _skip_newlines();

	Error _parse_directives(GUIDocument &r_doc);
	Error _parse_component(GUIComponent &r_comp);
	Error _parse_property_decl(GUIComponent &r_comp);
	Error _parse_signal_decl(GUIComponent &r_comp);
	Error _parse_node(GUINode &r_node, const String &p_component_name);
	Error _parse_node_body(GUINode &r_node, const String &p_component_name);
	Error _parse_property_or_signal(GUINode &r_node);
	Variant _parse_value();

public:
	Error parse(const Vector<GUITokenizer::Token> &p_tokens, GUIDocument &r_document);
	String get_error() const { return error_message; }
};

#endif // GUI_PARSER_H
