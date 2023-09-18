/**************************************************************************/
/*  ustring_preprocessor.cpp                                              */
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

#include "core/ustring_preprocessor.h"

#include "common/gd_core.h"
#include "core/char_utils.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

enum class LexType {
	None,

	Whitespace,
	Newline,
	Word,
	Operator,
	String,
};

static const String lex_type_name(LexType type) {
	switch (type) {
		case LexType::Whitespace:
			return "WHITESPACE";
		case LexType::Newline:
			return "NEWLINE";
		case LexType::Word:
			return "WORD";
		case LexType::Operator:
			return "OPERATOR";
		case LexType::String:
			return "STRING";
		default:
			return "NONE";
	}
}

static String lex_char_text(CharType c) {
	if (is_ascii_alphanumeric_char(c)) {
		return string_format("%c", c);
	} else {
		return string_format("\\x%02X", (int)c);
	}
}

static size_t lex(CharType *p, CharType *p_end, LexType &type) {
	type = LexType::None;

	CharType *p_start = p;

	if (*p == '"') {
		type = LexType::String;

		while (p < p_end) {
			p++;
			if (*p == '\\') {
				// Skip next character
				p++;
			} else if (*p == '"') {
				// End of string
				p++;
				break;
			}
		}

	} else {
		bool have_newline_r = false;
		bool have_newline_n = false;
		bool have_comment_begin = false;
		bool have_comment = false;

		while (p < p_end) {
			CharType c = *p;
			const bool is_whitespace = (c == ' ' || c == '\t');
			const bool is_newline = (c == '\r' || c == '\n');
			const bool is_alpha_num = ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_');
			const bool is_operator = (c == '!' || c == '&' || c == '|' || c == '(' || c == ')');
			const bool is_comment = (c == '/');

			if (is_comment && have_comment_begin) {
				have_comment = true;
			} else if (is_comment && !have_comment_begin) {
				have_comment_begin = true;
			} else {
				if (!have_comment && have_comment_begin) {
					have_comment_begin = false; // reset comment
				}
				if (is_newline) {
					// Only handle 1 newline at a time, which can be any combination of \r and \n
					if (c == '\r') {
						if (have_newline_r) {
							break;
						}
						have_newline_r = true;
					}
					if (c == '\n') {
						if (have_newline_n) {
							break;
						}
						have_newline_n = true;
					}
				} else if (have_comment) {
					p++;
					continue;
				}

				if (type == LexType::None) {
					if (is_whitespace) {
						type = LexType::Whitespace;
					} else if (is_newline) {
						type = LexType::Newline;
					} else if (is_alpha_num) {
						type = LexType::Word;
					} else if (is_operator) {
						type = LexType::Operator;
					}
				} else {
					if (type == LexType::Whitespace && !is_whitespace) {
						break;
					} else if (type == LexType::Newline && !is_newline) {
						break;
					} else if (type == LexType::Word && !is_alpha_num) {
						break;
					} else if (type == LexType::Operator && !is_operator) {
						break;
					}
				}
			}
			p++;
		}
	}

	return (p - p_start);
}

static size_t lex_expect(CharType *p, CharType *p_end, LexType expected_type) {
	LexType type;
	const size_t len = lex(p, p_end, type);

	ERR_FAIL_COND_V_MSG(type != expected_type, 0, "Unexpected '" + lex_char_text(*p) + "' of type " + lex_type_name(type) + ", was expecting a " + lex_type_name(expected_type));

	return len;
}

// Same as lex(), except skips whitespace and outputs start and length of the symbol
static size_t lex_next(CharType *p, CharType *p_end, LexType &type, CharType **start, size_t *len) {
	LexType t;
	size_t l = lex(p, p_end, t);
	*len = l;

	if (t == LexType::Whitespace) {
		p += l;
		size_t l2 = lex(p, p_end, t);
		*len = l2;
		l += l2;
	}

	type = t;
	*start = p;
	return l;
}

static const CharType cpp_character = '#';

enum {
	Scope_Passing = (1 << 0), // contents must pass
	Scope_Erasing = (1 << 1), // contents must be erased
	Scope_Else = (1 << 2), // contents are inside of an else directive
	Scope_ElseIf = (1 << 3), // contents are inside of an else if directive
	Scope_Deep = (1 << 4), // contents are deep and should be ignored
};

enum {
	Match_Pass = (1 << 0), // the match passed
	Match_OpAnd = (1 << 1), // should be matched as AND with previous entry
	Match_OpOr = (1 << 2), // should be matched as OR with previous entry
};

void StringProcessor::add_define(const String &name) {
	if (has_define(name)) {
		return;
	}
	defines.push_back(name);
}

void StringProcessor::remove_define(const String &name) { defines.erase(name); }
bool StringProcessor::has_define(const String &name) { return defines.has(name); }

void StringProcessor::set_include_callback(include_callback_t callback) { include_callback = callback; }
void StringProcessor::set_command_callback(command_callback_t callback) { command_callback = callback; }

String StringProcessor::process(String buffer) {
	process(buffer.ptrw(), buffer.size() - 1);
	return buffer;
}

void StringProcessor::process(CharType *buffer, size_t len) {
	ERR_FAIL_COND_MSG(p != nullptr, "Illegal attempt of preprocessor usage while not finished preprocessing!");

	line = 1;
	column = 0;

	p = buffer;
	p_end = buffer + len;

	while (p < p_end) {
		bool is_erasing = false;
		bool is_deep = false;

		if (stack.size() > 0) {
			const uint32_t &scope = stack.top();

			is_erasing = (scope & Scope_Erasing);
			is_deep = (scope & Scope_Deep);
		}

		if (*p == '\n') {
			column = 0;
			line++;
			p++;
		} else {
			if (column++ > 0 || *p != cpp_character) {
				if (is_erasing) {
					*p = ' ';
				}
				p++;
				continue;
			}

			CharType *command_start = p++;

			// expect a command word
			const size_t len_command = lex_expect(p, p_end, LexType::Word);
			if (len_command == 0) {
				continue;
			}

			const String word_command(p, len_command);

			p += len_command;

			if (word_command == "define") { // #define <word>

				if (is_erasing) {
					consume_line(); // just consume the line if we're erasing
				} else {
					const size_t len_command_whitespace = lex_expect(p, p_end, LexType::Whitespace); // Expect some whitespace
					if (len_command_whitespace == 0) {
						continue;
					}
					p += len_command_whitespace;

					const size_t len_define = lex_expect(p, p_end, LexType::Word); // Expect a define word
					if (len_define == 0) {
						continue;
					}

					const String word_define(p, len_define);

					p += len_define;

					add_define(word_define); // add define
					expect_eol(); // expect end of line
				}
			} else if (word_command == "undef") { // #undef <word>
				if (is_erasing) {
					consume_line(); // just consume the line if we're erasing
				} else {
					const size_t len_command_whitespace = lex_expect(p, p_end, LexType::Whitespace); // expect some whitespace
					if (len_command_whitespace == 0) {
						continue;
					}
					p += len_command_whitespace;

					const size_t len_define = lex_expect(p, p_end, LexType::Word); // expect a define word
					if (len_define == 0) {
						continue;
					}

					const String word_define(p, len_define);

					p += len_define;

					remove_define(word_define); // Undefine
					expect_eol(); // expect end of line
				}
			} else if (word_command == "if") { // #if <condition>
				if (is_erasing) {
					stack.push(Scope_Erasing | Scope_Deep);
					consume_line(); // just consume the line and push erasing at deep level
				} else {
					const size_t len_command_whitespace = lex_expect(p, p_end, LexType::Whitespace); // expect some whitespace
					if (len_command_whitespace == 0) {
						continue;
					}
					p += len_command_whitespace;

					const bool condition_passed = test_condition(); // expect a condition

					// Push to the stack
					if (condition_passed) {
						stack.push(Scope_Passing);
					} else {
						stack.push(Scope_Erasing);
					}
				}
			} else if (word_command == "else") { // #else
				if (is_erasing && is_deep) {
					consume_line(); // just consume the line if we're deep
				} else if (stack.size() == 0) { // if the stack is empty, this is an invalid command
					WARN_PRINT("Unexpected #else on line " + itos(line));
					consume_line();
				} else {
					uint32_t &top = stack.top(); // get top of stack

					if (top & Scope_Else) { // error out if we're already in an else directive
						WARN_PRINT("Unexpected #else on line " + itos(line));
					} else {
						if (top & Scope_Passing) {
							top = Scope_Erasing | Scope_Else; // if we're passing, set scope to erasing else
						} else if (top & Scope_Erasing) {
							top = Scope_Passing | Scope_Else; // if we're erasing, set scope to passing else
						}
					}

					expect_eol(); // expect end of line
				}
			} else if (word_command == "elif") { // #elif <condition>
				if (is_erasing && is_deep) {
					consume_line(); // just consume the line if we're deep
				} else if (stack.size() == 0) { // If the stack is empty, this is an invalid command
					WARN_PRINT("Unexpected #elif on line " + itos(line));
					consume_line();
				} else {
					uint32_t &top = stack.top(); // get top of stack

					if (top & Scope_Else) { // error out if we're already in an else directive
						WARN_PRINT("Unexpected #elif on line " + itos(line));
						consume_line();
					} else {
						if (top & Scope_Passing) { // if we're already passing, we'll erase anything below and set the deep flag to ignore the rest
							top = Scope_Erasing | Scope_ElseIf | Scope_Deep;
							consume_line();
						} else {
							const size_t len_command_whitespace = lex_expect(p, p_end, LexType::Whitespace); // Expect some whitespace
							if (len_command_whitespace == 0) {
								continue;
							}
							p += len_command_whitespace;

							// Expect a condition
							const bool condition_passed = test_condition();

							// Update the scope
							if (condition_passed) {
								top = Scope_Passing | Scope_ElseIf;
							} else {
								top = Scope_Erasing | Scope_ElseIf;
							}
						}
					}
				}
			} else if (word_command == "endif") { // #endif
				if (stack.size() == 0) { // if the stack is empty, this is an invalid command
					WARN_PRINT("Unexpected #endif on line " + itos(line));
					consume_line();
				} else {
					expect_eol(); // expect end of line
					stack.pop(); // pop from stack
				}
			} else if (word_command == "include") { // #include <path>
				if (is_erasing) {
					consume_line(); // just consume the line if we're erasing
				} else {
					if (include_callback == nullptr) { // if no callback is set up, just consume the line
						WARN_PRINT("No include callback set up for #include on line " + itos(line));
						consume_line();
					} else {
						const size_t len_command_whitespace = lex_expect(p, p_end, LexType::Whitespace); // Expect some whitespace
						if (len_command_whitespace == 0) {
							continue;
						}
						p += len_command_whitespace;

						const size_t len_path = lex_expect(p, p_end, LexType::String); // expect a string
						if (len_path == 0) {
							continue;
						}

						String path(p + 1, len_path - 2);

						p += len_path;

						if (!include_callback(path)) { // run callback
							WARN_PRINT("Failed to include \"" + path + "\" on line " + itos(line));
						}

						expect_eol(); // expect end of line
					}
				}
			} else if (word_command == "error") { // #error <msg>
				*(command_start - 1) = '.'; // #error -> .error
				consume_line(); // consume message
			} else { // unknown command, it can be handled by the callback, or throw an error
				bool command_found = false;
				const int command_line = line;

				CharType *command_value_start = p;

				consume_line(); // consume until end of line

				if (!is_erasing) { // handle if not erasing
					if (command_callback != nullptr) { // see if there is a custom command callback
						LexType type_command_value;
						size_t len_command_value = lex(command_value_start, p_end, type_command_value);

						if (type_command_value == LexType::Whitespace) { // handle potential whitespace
							command_value_start += len_command_value;
							len_command_value = lex(command_value_start, p_end, type_command_value);
						}

						if (type_command_value == LexType::Newline) {
							command_found = command_callback(word_command, String()); // if end of line, there's no command value
						} else { // If not end of line yet, there's some value
							const String command_value(command_value_start, len_command_value);
							command_found = command_callback(word_command, command_value);
						}
					}

					if (!command_found) {
						WARN_PRINT("Unrecognized preprocessor command \"" + word_command + "\" on line " + itos(command_line));
					}
				}
			}

			overwrite(command_start, p - command_start);
		}
	}

	// if there's something left in the stack, there are unclosed commands (missing #endif etc.)
	if (stack.size() > 0) {
		WARN_PRINT(itos(stack.size()) + " preprocessor scope(s) left unclosed at end of file (did you forget \"#endif\"?)");
	}

	p = p_end = nullptr;
}

bool StringProcessor::test_condition() {
	LocalVector<uint32_t> conditions;

	uint32_t op_flag = 0;

	while (true) {
		LexType type;
		CharType *sym_start;
		size_t sym_length;

		p += lex_next(p, p_end, type, &sym_start, &sym_length);

		if (type == LexType::Newline) {
			line++;
			column = 0;
			break;
		}

		uint32_t cond = 0;
		bool must_equal = true;

		const String operator_text(sym_start, sym_length);

		if (type == LexType::Operator) {
			if (*sym_start == '!') {
				must_equal = false;
			} else if (sym_length == 2 && operator_text == "&&") {
				op_flag = Match_OpAnd;
				continue;
			} else if (sym_length == 2 && operator_text == "||") {
				op_flag = Match_OpOr;
				continue;
			} else {
				WARN_PRINT("Unexpected operator '" + operator_text + "' in condition on line " + itos(line));
			}
			p += lex_next(p, p_end, type, &sym_start, &sym_length);
		}

		if (type != LexType::Word) {
			WARN_PRINT("Unexpected text '" + String(sym_start, sym_length) + "' of type " + lex_type_name(type) + " in condition on line " + itos(line));
		} else {
			const String word(sym_start, sym_length);
			if (has_define(word) == must_equal) {
				cond = Match_Pass;
			}
		}

		conditions.push_back(cond | op_flag);
	}

	// perform AND conditions first
	for (int i = conditions.size() - 1; i >= 1; i--) {
		uint32_t &rhs = conditions[i];
		uint32_t &lhs = conditions[i - 1];

		if (rhs & Match_OpAnd) {
			if ((rhs & Match_Pass) && (lhs & Match_Pass)) {
				lhs |= Match_Pass;
			} else {
				lhs &= ~Match_Pass;
			}
			conditions.remove(i);
		}
	}

	// perform OR conditions last
	for (int i = conditions.size() - 1; i >= 1; i--) {
		uint32_t &rhs = conditions[i];
		uint32_t &lhs = conditions[i - 1];

		if (rhs & Match_OpOr) {
			if ((rhs & Match_Pass) || (lhs & Match_Pass)) {
				lhs |= Match_Pass;
			} else {
				lhs &= ~Match_Pass;
			}
			conditions.remove(i);
		}
	}

	DEV_ASSERT(conditions.size() != 1); // now there should only be 1 condition left

	if (conditions.size() != 1) {
		return false;
	}
	return (conditions[0] & Match_Pass);
}

void StringProcessor::expect_eol() {
	if (p == p_end) { // consider the end of the string as end of line too
		return;
	}
	if (lex_expect(p, p_end, LexType::Newline) == 0) {
		return;
	}
	p++;
	line++;
	column = 0;
}

void StringProcessor::consume_line() {
	LexType type = LexType::None;
	while (type != LexType::Newline) {
		p += lex(p, p_end, type);
	}

	line++;
	column = 0;
}

void StringProcessor::overwrite(CharType *p, size_t len) {
	CharType *p_end = p + len;
	for (; p < p_end; p++) {
		if (*p != '\r' && *p != '\n') {
			*p = ' ';
		}
	}
}

StringProcessor::StringProcessor() {
	p = p_end = nullptr;
}

StringProcessor::StringProcessor(const String &multi_defines) :
		StringProcessor() {
	defines = multi_defines.split(",");
}

#ifdef DOCTEST
TEST_CASE("Preprocessor tokens") {
	SUBCASE("#define") {
		REQUIRE(StringProcessor().process("#define A").trim().empty());
		REQUIRE(StringProcessor().process("#define A\n").trim().empty());
		REQUIRE(StringProcessor("A=1").process("#define A\n").trim().empty());
	}
	SUBCASE("#if") {
		REQUIRE(StringProcessor().process("#if A\nTEST\n#endif").trim().empty());
		REQUIRE(StringProcessor("A").process("#if A\nTEST\n#endif").trim() == "TEST");
	}
}
#endif // DOCTEST
