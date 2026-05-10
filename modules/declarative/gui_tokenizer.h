/**************************************************************************/
/*  gui_tokenizer.h                                                       */
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

#ifndef GUI_TOKENIZER_H
#define GUI_TOKENIZER_H

#include "core/ustring.h"
#include "core/vector.h"

class GUITokenizer {
public:
	enum TokenType {
		// Literals
		TK_IDENTIFIER,
		TK_STRING,
		TK_NUMBER,
		TK_BOOL_TRUE,
		TK_BOOL_FALSE,

		// Keywords
		TK_COMPONENT,
		TK_INHERITS,
		TK_PROPERTY,
		TK_SIGNAL,
		TK_IN,
		TK_OUT,

		// Punctuation
		TK_BRACE_OPEN,
		TK_BRACE_CLOSE,
		TK_PAREN_OPEN,
		TK_PAREN_CLOSE,
		TK_ANGLE_OPEN,
		TK_ANGLE_CLOSE,
		TK_COLON,
		TK_COMMA,
		TK_DOT,
		TK_HASH,
		TK_ARROW, // =>
		TK_MINUS,

		// Directives
		TK_AT_TARGET,
		TK_AT_SCRIPT,
		TK_AT_CHILDREN,

		TK_NEWLINE,
		TK_EOF,
		TK_ERROR,
	};

	struct Token {
		TokenType type;
		String value;
		int line;
		int column;

		Token() :
				type(TK_EOF), line(0), column(0) {}
		Token(TokenType p_type, const String &p_value, int p_line, int p_col) :
				type(p_type), value(p_value), line(p_line), column(p_col) {}
	};

private:
	String source;
	int pos;
	int line;
	int column;
	Vector<Token> tokens;
	String error_message;

	CharType _peek() const;
	CharType _advance();
	bool _at_end() const;
	void _skip_whitespace_and_comments();
	Token _read_string();
	Token _read_number();
	Token _read_identifier_or_keyword();
	Token _read_directive();

public:
	Error tokenize(const String &p_source);

	int get_token_count() const { return tokens.size(); }
	const Token &get_token(int p_index) const { return tokens[p_index]; }
	const Vector<Token> &get_tokens() const { return tokens; }
	String get_error() const { return error_message; }

	static String token_type_name(TokenType p_type);
};

#endif // GUI_TOKENIZER_H
