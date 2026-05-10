/**************************************************************************/
/*  gui_tokenizer.cpp                                                     */
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

#include "gui_tokenizer.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

CharType GUITokenizer::_peek() const {
	if (_at_end())
		return 0;
	return source[pos];
}

CharType GUITokenizer::_advance() {
	CharType c = source[pos];
	pos++;
	column++;
	if (c == '\n') {
		line++;
		column = 1;
	}
	return c;
}

bool GUITokenizer::_at_end() const {
	return pos >= source.length();
}

void GUITokenizer::_skip_whitespace_and_comments() {
	while (!_at_end()) {
		CharType c = _peek();
		if (c == ' ' || c == '\t' || c == '\r') {
			_advance();
		} else if (c == '/' && pos + 1 < source.length() && source[pos + 1] == '/') {
			// Line comment — skip to end of line
			while (!_at_end() && _peek() != '\n') {
				_advance();
			}
		} else {
			break;
		}
	}
}

GUITokenizer::Token GUITokenizer::_read_string() {
	int start_line = line;
	int start_col = column;
	_advance(); // skip opening quote
	String result;
	while (!_at_end() && _peek() != '"') {
		CharType c = _advance();
		if (c == '\\' && !_at_end()) {
			CharType esc = _advance();
			switch (esc) {
				case 'n':
					result += '\n';
					break;
				case 't':
					result += '\t';
					break;
				case '\\':
					result += '\\';
					break;
				case '"':
					result += '"';
					break;
				default:
					result += esc;
					break;
			}
		} else {
			result += c;
		}
	}
	if (_at_end()) {
		return Token(TK_ERROR, "Unterminated string", start_line, start_col);
	}
	_advance(); // skip closing quote
	return Token(TK_STRING, result, start_line, start_col);
}

GUITokenizer::Token GUITokenizer::_read_number() {
	int start_line = line;
	int start_col = column;
	String result;
	bool has_dot = false;
	// Handle negative sign
	if (_peek() == '-') {
		result += String::chr(_advance());
	}
	while (!_at_end()) {
		CharType c = _peek();
		if (c >= '0' && c <= '9') {
			result += String::chr(_advance());
		} else if (c == '.' && !has_dot) {
			has_dot = true;
			result += String::chr(_advance());
		} else {
			break;
		}
	}
	return Token(TK_NUMBER, result, start_line, start_col);
}

GUITokenizer::Token GUITokenizer::_read_identifier_or_keyword() {
	int start_line = line;
	int start_col = column;
	String result;
	while (!_at_end()) {
		CharType c = _peek();
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
			result += String::chr(_advance());
		} else {
			break;
		}
	}
	// Check keywords
	if (result == "component")
		return Token(TK_COMPONENT, result, start_line, start_col);
	if (result == "inherits")
		return Token(TK_INHERITS, result, start_line, start_col);
	if (result == "property")
		return Token(TK_PROPERTY, result, start_line, start_col);
	if (result == "signal")
		return Token(TK_SIGNAL, result, start_line, start_col);
	if (result == "in")
		return Token(TK_IN, result, start_line, start_col);
	if (result == "out")
		return Token(TK_OUT, result, start_line, start_col);
	if (result == "true")
		return Token(TK_BOOL_TRUE, result, start_line, start_col);
	if (result == "false")
		return Token(TK_BOOL_FALSE, result, start_line, start_col);

	return Token(TK_IDENTIFIER, result, start_line, start_col);
}

GUITokenizer::Token GUITokenizer::_read_directive() {
	int start_line = line;
	int start_col = column;
	_advance(); // skip @
	String name;
	while (!_at_end()) {
		CharType c = _peek();
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
			name += String::chr(_advance());
		} else {
			break;
		}
	}
	if (name == "target")
		return Token(TK_AT_TARGET, "@target", start_line, start_col);
	if (name == "script")
		return Token(TK_AT_SCRIPT, "@script", start_line, start_col);
	if (name == "children")
		return Token(TK_AT_CHILDREN, "@children", start_line, start_col);

	return Token(TK_ERROR, "Unknown directive: @" + name, start_line, start_col);
}

Error GUITokenizer::tokenize(const String &p_source) {
	source = p_source;
	pos = 0;
	line = 1;
	column = 1;
	tokens.clear();
	error_message = "";

	while (true) {
		_skip_whitespace_and_comments();
		if (_at_end()) {
			tokens.push_back(Token(TK_EOF, "", line, column));
			break;
		}

		CharType c = _peek();

		if (c == '\n') {
			tokens.push_back(Token(TK_NEWLINE, "\\n", line, column));
			_advance();
			continue;
		}

		if (c == '"') {
			Token tk = _read_string();
			if (tk.type == TK_ERROR) {
				error_message = tk.value + " at line " + itos(tk.line) + ":" + itos(tk.column);
				return ERR_PARSE_ERROR;
			}
			tokens.push_back(tk);
			continue;
		}

		if ((c >= '0' && c <= '9') || (c == '-' && pos + 1 < source.length() && source[pos + 1] >= '0' && source[pos + 1] <= '9')) {
			tokens.push_back(_read_number());
			continue;
		}

		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
			tokens.push_back(_read_identifier_or_keyword());
			continue;
		}

		if (c == '@') {
			Token tk = _read_directive();
			if (tk.type == TK_ERROR) {
				error_message = tk.value + " at line " + itos(tk.line) + ":" + itos(tk.column);
				return ERR_PARSE_ERROR;
			}
			tokens.push_back(tk);
			continue;
		}

		// Single/double character tokens
		int tk_line = line;
		int tk_col = column;
		_advance();

		switch (c) {
			case '{':
				tokens.push_back(Token(TK_BRACE_OPEN, "{", tk_line, tk_col));
				break;
			case '}':
				tokens.push_back(Token(TK_BRACE_CLOSE, "}", tk_line, tk_col));
				break;
			case '(':
				tokens.push_back(Token(TK_PAREN_OPEN, "(", tk_line, tk_col));
				break;
			case ')':
				tokens.push_back(Token(TK_PAREN_CLOSE, ")", tk_line, tk_col));
				break;
			case '<':
				tokens.push_back(Token(TK_ANGLE_OPEN, "<", tk_line, tk_col));
				break;
			case '>':
				tokens.push_back(Token(TK_ANGLE_CLOSE, ">", tk_line, tk_col));
				break;
			case ':':
				tokens.push_back(Token(TK_COLON, ":", tk_line, tk_col));
				break;
			case ',':
				tokens.push_back(Token(TK_COMMA, ",", tk_line, tk_col));
				break;
			case '.':
				tokens.push_back(Token(TK_DOT, ".", tk_line, tk_col));
				break;
			case '#':
				tokens.push_back(Token(TK_HASH, "#", tk_line, tk_col));
				break;
			case '-':
				tokens.push_back(Token(TK_MINUS, "-", tk_line, tk_col));
				break;
			case '=':
				if (!_at_end() && _peek() == '>') {
					_advance();
					tokens.push_back(Token(TK_ARROW, "=>", tk_line, tk_col));
				} else {
					error_message = "Unexpected '=' at line " + itos(tk_line) + ":" + itos(tk_col) + " (did you mean '=>'?)";
					return ERR_PARSE_ERROR;
				}
				break;
			default:
				error_message = "Unexpected character '" + String::chr(c) + "' at line " + itos(tk_line) + ":" + itos(tk_col);
				return ERR_PARSE_ERROR;
		}
	}

	return OK;
}

String GUITokenizer::token_type_name(TokenType p_type) {
	switch (p_type) {
		case TK_IDENTIFIER:
			return "IDENTIFIER";
		case TK_STRING:
			return "STRING";
		case TK_NUMBER:
			return "NUMBER";
		case TK_BOOL_TRUE:
			return "TRUE";
		case TK_BOOL_FALSE:
			return "FALSE";
		case TK_COMPONENT:
			return "COMPONENT";
		case TK_INHERITS:
			return "INHERITS";
		case TK_PROPERTY:
			return "PROPERTY";
		case TK_SIGNAL:
			return "SIGNAL";
		case TK_IN:
			return "IN";
		case TK_OUT:
			return "OUT";
		case TK_BRACE_OPEN:
			return "BRACE_OPEN";
		case TK_BRACE_CLOSE:
			return "BRACE_CLOSE";
		case TK_PAREN_OPEN:
			return "PAREN_OPEN";
		case TK_PAREN_CLOSE:
			return "PAREN_CLOSE";
		case TK_ANGLE_OPEN:
			return "ANGLE_OPEN";
		case TK_ANGLE_CLOSE:
			return "ANGLE_CLOSE";
		case TK_COLON:
			return "COLON";
		case TK_COMMA:
			return "COMMA";
		case TK_DOT:
			return "DOT";
		case TK_HASH:
			return "HASH";
		case TK_ARROW:
			return "ARROW";
		case TK_MINUS:
			return "MINUS";
		case TK_AT_TARGET:
			return "AT_TARGET";
		case TK_AT_SCRIPT:
			return "AT_SCRIPT";
		case TK_AT_CHILDREN:
			return "AT_CHILDREN";
		case TK_NEWLINE:
			return "NEWLINE";
		case TK_EOF:
			return "EOF";
		case TK_ERROR:
			return "ERROR";
	}
	return "UNKNOWN";
}

// --- Doctests ---
#ifdef DOCTEST

TEST_CASE("[GUITokenizer] empty input") {
	GUITokenizer tk;
	CHECK(tk.tokenize("") == OK);
	CHECK(tk.get_token_count() == 1);
	CHECK(tk.get_token(0).type == GUITokenizer::TK_EOF);
}

TEST_CASE("[GUITokenizer] comments are skipped") {
	GUITokenizer tk;
	CHECK(tk.tokenize("// this is a comment\n") == OK);
	// Should have NEWLINE + EOF
	bool found_identifier = false;
	for (int i = 0; i < tk.get_token_count(); i++) {
		if (tk.get_token(i).type == GUITokenizer::TK_IDENTIFIER)
			found_identifier = true;
	}
	CHECK(!found_identifier);
}

TEST_CASE("[GUITokenizer] keywords") {
	GUITokenizer tk;
	CHECK(tk.tokenize("component inherits property signal in out true false") == OK);
	int idx = 0;
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_COMPONENT);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_INHERITS);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_PROPERTY);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_SIGNAL);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_IN);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_OUT);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_BOOL_TRUE);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_BOOL_FALSE);
}

TEST_CASE("[GUITokenizer] string literal") {
	GUITokenizer tk;
	CHECK(tk.tokenize("\"hello world\"") == OK);
	CHECK(tk.get_token(0).type == GUITokenizer::TK_STRING);
	CHECK(tk.get_token(0).value == "hello world");
}

TEST_CASE("[GUITokenizer] string escape sequences") {
	GUITokenizer tk;
	CHECK(tk.tokenize("\"line1\\nline2\\t\\\"quoted\\\"\"") == OK);
	CHECK(tk.get_token(0).type == GUITokenizer::TK_STRING);
	CHECK(tk.get_token(0).value == "line1\nline2\t\"quoted\"");
}

TEST_CASE("[GUITokenizer] number literals") {
	GUITokenizer tk;
	CHECK(tk.tokenize("42 3.14 -7") == OK);
	CHECK(tk.get_token(0).type == GUITokenizer::TK_NUMBER);
	CHECK(tk.get_token(0).value == "42");
	CHECK(tk.get_token(1).type == GUITokenizer::TK_NUMBER);
	CHECK(tk.get_token(1).value == "3.14");
	CHECK(tk.get_token(2).type == GUITokenizer::TK_NUMBER);
	CHECK(tk.get_token(2).value == "-7");
}

TEST_CASE("[GUITokenizer] punctuation") {
	GUITokenizer tk;
	CHECK(tk.tokenize("{ } ( ) < > : , . #") == OK);
	int idx = 0;
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_BRACE_OPEN);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_BRACE_CLOSE);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_PAREN_OPEN);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_PAREN_CLOSE);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_ANGLE_OPEN);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_ANGLE_CLOSE);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_COLON);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_COMMA);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_DOT);
	CHECK(tk.get_token(idx++).type == GUITokenizer::TK_HASH);
}

TEST_CASE("[GUITokenizer] arrow operator") {
	GUITokenizer tk;
	CHECK(tk.tokenize("=>") == OK);
	CHECK(tk.get_token(0).type == GUITokenizer::TK_ARROW);
	CHECK(tk.get_token(0).value == "=>");
}

TEST_CASE("[GUITokenizer] directives") {
	GUITokenizer tk;
	CHECK(tk.tokenize("@target @script @children") == OK);
	CHECK(tk.get_token(0).type == GUITokenizer::TK_AT_TARGET);
	CHECK(tk.get_token(1).type == GUITokenizer::TK_AT_SCRIPT);
	CHECK(tk.get_token(2).type == GUITokenizer::TK_AT_CHILDREN);
}

TEST_CASE("[GUITokenizer] unknown directive error") {
	GUITokenizer tk;
	CHECK(tk.tokenize("@foobar") != OK);
	CHECK(tk.get_error().find("Unknown directive") != -1);
}

TEST_CASE("[GUITokenizer] unterminated string error") {
	GUITokenizer tk;
	CHECK(tk.tokenize("\"unterminated") != OK);
	CHECK(tk.get_error().find("Unterminated string") != -1);
}

TEST_CASE("[GUITokenizer] line tracking") {
	GUITokenizer tk;
	CHECK(tk.tokenize("foo\nbar\nbaz") == OK);
	// foo is on line 1, bar on line 2, baz on line 3
	CHECK(tk.get_token(0).line == 1);
	// token 1 is NEWLINE, token 2 is "bar"
	CHECK(tk.get_token(2).line == 2);
}

TEST_CASE("[GUITokenizer] full component tokenization") {
	GUITokenizer tk;
	String src = "@target: scene\n"
				 "component Foo inherits Control {\n"
				 "  Label #title {\n"
				 "    text: \"Hello\"\n"
				 "  }\n"
				 "}\n";
	CHECK(tk.tokenize(src) == OK);
	// Verify key tokens are present
	bool found_component = false;
	bool found_inherits = false;
	bool found_hash = false;
	bool found_string = false;
	for (int i = 0; i < tk.get_token_count(); i++) {
		if (tk.get_token(i).type == GUITokenizer::TK_COMPONENT)
			found_component = true;
		if (tk.get_token(i).type == GUITokenizer::TK_INHERITS)
			found_inherits = true;
		if (tk.get_token(i).type == GUITokenizer::TK_HASH)
			found_hash = true;
		if (tk.get_token(i).type == GUITokenizer::TK_STRING && tk.get_token(i).value == "Hello")
			found_string = true;
	}
	CHECK(found_component);
	CHECK(found_inherits);
	CHECK(found_hash);
	CHECK(found_string);
}

#endif // DOCTEST
