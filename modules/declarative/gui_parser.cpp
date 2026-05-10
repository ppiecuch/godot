/**************************************************************************/
/*  gui_parser.cpp                                                        */
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

#include "gui_parser.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

const GUITokenizer::Token &GUIParser::_current() const {
	return tokens->get(pos);
}

const GUITokenizer::Token &GUIParser::_peek(int p_offset) const {
	int idx = pos + p_offset;
	if (idx >= tokens->size()) {
		return tokens->get(tokens->size() - 1); // EOF
	}
	return tokens->get(idx);
}

bool GUIParser::_at_end() const {
	return pos >= tokens->size() || _current().type == GUITokenizer::TK_EOF;
}

bool GUIParser::_match(GUITokenizer::TokenType p_type) {
	if (!_at_end() && _current().type == p_type) {
		pos++;
		return true;
	}
	return false;
}

bool GUIParser::_expect(GUITokenizer::TokenType p_type, const String &p_context) {
	if (_at_end() || _current().type != p_type) {
		const GUITokenizer::Token &tk = _current();
		error_message = "Expected " + GUITokenizer::token_type_name(p_type) +
				" in " + p_context + ", got " + GUITokenizer::token_type_name(tk.type) +
				" ('" + tk.value + "') at line " + itos(tk.line) + ":" + itos(tk.column);
		return false;
	}
	pos++;
	return true;
}

void GUIParser::_skip_newlines() {
	while (!_at_end() && _current().type == GUITokenizer::TK_NEWLINE) {
		pos++;
	}
}

// Parse @target and @script directives at file top.
Error GUIParser::_parse_directives(GUIDocument &r_doc) {
	_skip_newlines();

	while (!_at_end()) {
		if (_current().type == GUITokenizer::TK_AT_TARGET) {
			pos++;
			if (!_expect(GUITokenizer::TK_COLON, "@target"))
				return ERR_PARSE_ERROR;
			_skip_newlines();
			if (!_expect(GUITokenizer::TK_IDENTIFIER, "@target value"))
				return ERR_PARSE_ERROR;
			String val = tokens->get(pos - 1).value;
			if (val == "scene") {
				r_doc.target = GUIDocument::TARGET_SCENE;
			} else if (val == "gdscript") {
				r_doc.target = GUIDocument::TARGET_GDSCRIPT;
			} else if (val == "runtime") {
				r_doc.target = GUIDocument::TARGET_RUNTIME;
			} else {
				error_message = "Unknown target '" + val + "', expected scene|gdscript|runtime";
				return ERR_PARSE_ERROR;
			}
			_skip_newlines();
		} else if (_current().type == GUITokenizer::TK_AT_SCRIPT) {
			pos++;
			if (!_expect(GUITokenizer::TK_COLON, "@script"))
				return ERR_PARSE_ERROR;
			_skip_newlines();
			if (!_expect(GUITokenizer::TK_STRING, "@script path"))
				return ERR_PARSE_ERROR;
			r_doc.script_path = tokens->get(pos - 1).value;
			_skip_newlines();
		} else {
			break;
		}
	}

	return OK;
}

// Parse: component Name inherits Base { ... }
Error GUIParser::_parse_component(GUIComponent &r_comp) {
	if (!_expect(GUITokenizer::TK_COMPONENT, "component declaration"))
		return ERR_PARSE_ERROR;
	if (!_expect(GUITokenizer::TK_IDENTIFIER, "component name"))
		return ERR_PARSE_ERROR;
	r_comp.name = tokens->get(pos - 1).value;

	if (!_expect(GUITokenizer::TK_INHERITS, "component inherits"))
		return ERR_PARSE_ERROR;
	if (!_expect(GUITokenizer::TK_IDENTIFIER, "base class name"))
		return ERR_PARSE_ERROR;
	r_comp.inherits = tokens->get(pos - 1).value;

	_skip_newlines();
	if (!_expect(GUITokenizer::TK_BRACE_OPEN, "component body"))
		return ERR_PARSE_ERROR;
	_skip_newlines();

	// Set up the root node to match the inherited type.
	r_comp.root.type = r_comp.inherits;
	r_comp.root.name = r_comp.name;

	// Parse component body: property decls, signal decls, and child nodes.
	while (!_at_end() && _current().type != GUITokenizer::TK_BRACE_CLOSE) {
		_skip_newlines();
		if (_at_end() || _current().type == GUITokenizer::TK_BRACE_CLOSE)
			break;

		// Property declaration: [in|out|in-out] property<Type> name: default
		if (_current().type == GUITokenizer::TK_PROPERTY ||
				_current().type == GUITokenizer::TK_IN ||
				_current().type == GUITokenizer::TK_OUT) {
			Error err = _parse_property_decl(r_comp);
			if (err != OK)
				return err;
		}
		// Signal declaration: signal name(args)
		else if (_current().type == GUITokenizer::TK_SIGNAL) {
			Error err = _parse_signal_decl(r_comp);
			if (err != OK)
				return err;
		}
		// @children slot
		else if (_current().type == GUITokenizer::TK_AT_CHILDREN) {
			pos++;
			GUINode slot;
			slot.is_children_slot = true;
			slot.type = "Control";
			slot.name = "_slot";
			r_comp.root.children.push_back(slot);
			_skip_newlines();
		}
		// Child node: IdentifierType [#name] { ... }
		else if (_current().type == GUITokenizer::TK_IDENTIFIER) {
			GUINode child;
			Error err = _parse_node(child, r_comp.name);
			if (err != OK)
				return err;
			r_comp.root.children.push_back(child);
		} else {
			error_message = "Unexpected token '" + _current().value + "' in component body at line " +
					itos(_current().line) + ":" + itos(_current().column);
			return ERR_PARSE_ERROR;
		}
	}

	if (!_expect(GUITokenizer::TK_BRACE_CLOSE, "component body end"))
		return ERR_PARSE_ERROR;
	return OK;
}

// Parse: [in|out|in-out] property<Type> name [: default]
Error GUIParser::_parse_property_decl(GUIComponent &r_comp) {
	GUIPropertyDecl decl;
	decl.visibility = GUIPropertyDecl::VIS_PRIVATE;

	// Parse optional visibility prefix
	if (_current().type == GUITokenizer::TK_IN) {
		pos++;
		_skip_newlines();
		if (_current().type == GUITokenizer::TK_MINUS) {
			pos++;
			if (!_expect(GUITokenizer::TK_OUT, "in-out visibility"))
				return ERR_PARSE_ERROR;
			decl.visibility = GUIPropertyDecl::VIS_IN_OUT;
		} else {
			decl.visibility = GUIPropertyDecl::VIS_IN;
		}
	} else if (_current().type == GUITokenizer::TK_OUT) {
		pos++;
		decl.visibility = GUIPropertyDecl::VIS_OUT;
	}

	_skip_newlines();
	if (!_expect(GUITokenizer::TK_PROPERTY, "property declaration"))
		return ERR_PARSE_ERROR;

	// Parse <Type>
	if (!_expect(GUITokenizer::TK_ANGLE_OPEN, "property type"))
		return ERR_PARSE_ERROR;
	if (!_expect(GUITokenizer::TK_IDENTIFIER, "property type name"))
		return ERR_PARSE_ERROR;
	decl.type = tokens->get(pos - 1).value;
	if (!_expect(GUITokenizer::TK_ANGLE_CLOSE, "property type close"))
		return ERR_PARSE_ERROR;

	// Parse name
	if (!_expect(GUITokenizer::TK_IDENTIFIER, "property name"))
		return ERR_PARSE_ERROR;
	decl.name = tokens->get(pos - 1).value;

	// Optional default value
	if (!_at_end() && _current().type == GUITokenizer::TK_COLON) {
		pos++;
		_skip_newlines();
		decl.default_value = _parse_value();
	}

	r_comp.property_decls.push_back(decl);
	_skip_newlines();
	return OK;
}

// Parse: signal name[(arg: Type, ...)]
Error GUIParser::_parse_signal_decl(GUIComponent &r_comp) {
	GUISignalDecl decl;

	if (!_expect(GUITokenizer::TK_SIGNAL, "signal declaration"))
		return ERR_PARSE_ERROR;
	if (!_expect(GUITokenizer::TK_IDENTIFIER, "signal name"))
		return ERR_PARSE_ERROR;
	decl.name = tokens->get(pos - 1).value;

	// Optional arguments
	if (!_at_end() && _current().type == GUITokenizer::TK_PAREN_OPEN) {
		pos++;
		while (!_at_end() && _current().type != GUITokenizer::TK_PAREN_CLOSE) {
			if (!_expect(GUITokenizer::TK_IDENTIFIER, "signal arg name"))
				return ERR_PARSE_ERROR;
			String arg_name = tokens->get(pos - 1).value;
			String arg_type = "Variant";
			if (!_at_end() && _current().type == GUITokenizer::TK_COLON) {
				pos++;
				_skip_newlines();
				if (!_expect(GUITokenizer::TK_IDENTIFIER, "signal arg type"))
					return ERR_PARSE_ERROR;
				arg_type = tokens->get(pos - 1).value;
			}
			decl.args.push_back(Pair<String, String>(arg_name, arg_type));
			if (!_at_end() && _current().type == GUITokenizer::TK_COMMA) {
				pos++;
				_skip_newlines();
			}
		}
		if (!_expect(GUITokenizer::TK_PAREN_CLOSE, "signal args close"))
			return ERR_PARSE_ERROR;
	}

	r_comp.signal_decls.push_back(decl);
	_skip_newlines();
	return OK;
}

// Parse: TypeName [#name] { body }
Error GUIParser::_parse_node(GUINode &r_node, const String &p_component_name) {
	if (!_expect(GUITokenizer::TK_IDENTIFIER, "node type"))
		return ERR_PARSE_ERROR;
	r_node.type = tokens->get(pos - 1).value;

	// Optional #name
	if (!_at_end() && _current().type == GUITokenizer::TK_HASH) {
		pos++;
		if (!_expect(GUITokenizer::TK_IDENTIFIER, "node name"))
			return ERR_PARSE_ERROR;
		r_node.name = tokens->get(pos - 1).value;
	} else {
		// Auto-generate name from type
		r_node.name = r_node.type;
	}

	_skip_newlines();
	if (!_expect(GUITokenizer::TK_BRACE_OPEN, "node body"))
		return ERR_PARSE_ERROR;

	Error err = _parse_node_body(r_node, p_component_name);
	if (err != OK)
		return err;

	if (!_expect(GUITokenizer::TK_BRACE_CLOSE, "node body end"))
		return ERR_PARSE_ERROR;
	_skip_newlines();
	return OK;
}

// Parse node body: mix of properties, signal connections, @children, and child nodes.
Error GUIParser::_parse_node_body(GUINode &r_node, const String &p_component_name) {
	_skip_newlines();
	while (!_at_end() && _current().type != GUITokenizer::TK_BRACE_CLOSE) {
		_skip_newlines();
		if (_at_end() || _current().type == GUITokenizer::TK_BRACE_CLOSE)
			break;

		// @children
		if (_current().type == GUITokenizer::TK_AT_CHILDREN) {
			pos++;
			GUINode slot;
			slot.is_children_slot = true;
			slot.type = "Control";
			slot.name = "_slot";
			r_node.children.push_back(slot);
			_skip_newlines();
			continue;
		}

		// Must be an identifier — could be property, signal connection, or child node.
		if (_current().type != GUITokenizer::TK_IDENTIFIER) {
			error_message = "Unexpected token '" + _current().value + "' in node body at line " +
					itos(_current().line) + ":" + itos(_current().column);
			return ERR_PARSE_ERROR;
		}

		// Look ahead to distinguish:
		// - property:     ident : value
		// - signal conn:  ident => target.method()
		// - child node:   ident [#name] {
		//
		// We need to look past potential #name to find { for child nodes.

		int lookahead = 1;
		// Skip newlines in lookahead
		while (pos + lookahead < tokens->size() && _peek(lookahead).type == GUITokenizer::TK_NEWLINE) {
			lookahead++;
		}

		GUITokenizer::TokenType next_type = _peek(lookahead).type;

		if (next_type == GUITokenizer::TK_COLON) {
			// Property assignment: name: value
			Error err = _parse_property_or_signal(r_node);
			if (err != OK)
				return err;
		} else if (next_type == GUITokenizer::TK_ARROW) {
			// Signal connection: signal_name => target.method()
			Error err = _parse_property_or_signal(r_node);
			if (err != OK)
				return err;
		} else if (next_type == GUITokenizer::TK_BRACE_OPEN || next_type == GUITokenizer::TK_HASH) {
			// Child node
			GUINode child;
			Error err = _parse_node(child, p_component_name);
			if (err != OK)
				return err;
			r_node.children.push_back(child);
		} else {
			error_message = "Unexpected token after identifier '" + _current().value +
					"' at line " + itos(_current().line) + ":" + itos(_current().column) +
					" (got " + GUITokenizer::token_type_name(next_type) + ")";
			return ERR_PARSE_ERROR;
		}
	}
	return OK;
}

// Parse either a property assignment or signal connection.
// property:  name: value
// signal:    signal_name => target.method()
Error GUIParser::_parse_property_or_signal(GUINode &r_node) {
	String name = _current().value;
	pos++; // consume identifier
	_skip_newlines();

	if (_current().type == GUITokenizer::TK_COLON) {
		// Property assignment
		pos++; // consume :
		_skip_newlines();

		GUIProperty prop;
		prop.name = name;

		// Check if this is a binding (identifier possibly with dots) or a literal
		if (_current().type == GUITokenizer::TK_IDENTIFIER) {
			// Could be a binding like root.title, or a constructor like Vector2(...)
			String first = _current().value;
			pos++;

			if (!_at_end() && _current().type == GUITokenizer::TK_DOT) {
				// Binding: root.title or root.something.else
				String binding = first;
				while (!_at_end() && _current().type == GUITokenizer::TK_DOT) {
					pos++; // consume .
					if (!_expect(GUITokenizer::TK_IDENTIFIER, "binding path"))
						return ERR_PARSE_ERROR;
					binding += "." + tokens->get(pos - 1).value;
				}
				prop.binding_expr = binding;
			} else if (!_at_end() && _current().type == GUITokenizer::TK_PAREN_OPEN) {
				// Constructor call like Vector2(100, 50) — store as binding for now
				String expr = first + "(";
				pos++; // consume (
				int depth = 1;
				while (!_at_end() && depth > 0) {
					if (_current().type == GUITokenizer::TK_PAREN_OPEN)
						depth++;
					if (_current().type == GUITokenizer::TK_PAREN_CLOSE)
						depth--;
					if (depth > 0) {
						expr += _current().value;
						pos++;
					}
				}
				if (!_expect(GUITokenizer::TK_PAREN_CLOSE, "constructor close"))
					return ERR_PARSE_ERROR;
				expr += ")";
				prop.binding_expr = expr;
			} else {
				// Plain identifier — treat as string enum or binding
				prop.binding_expr = first;
			}
		} else if (_current().type == GUITokenizer::TK_STRING) {
			prop.value = Variant(_current().value);
			pos++;
		} else if (_current().type == GUITokenizer::TK_NUMBER) {
			String num_str = _current().value;
			pos++;
			if (num_str.find(".") != -1) {
				prop.value = Variant(num_str.to_double());
			} else {
				prop.value = Variant(num_str.to_int());
			}
		} else if (_current().type == GUITokenizer::TK_BOOL_TRUE) {
			prop.value = Variant(true);
			pos++;
		} else if (_current().type == GUITokenizer::TK_BOOL_FALSE) {
			prop.value = Variant(false);
			pos++;
		} else if (_current().type == GUITokenizer::TK_MINUS) {
			// Negative number
			pos++;
			if (!_expect(GUITokenizer::TK_NUMBER, "negative number"))
				return ERR_PARSE_ERROR;
			String num_str = "-" + tokens->get(pos - 1).value;
			if (num_str.find(".") != -1) {
				prop.value = Variant(num_str.to_double());
			} else {
				prop.value = Variant(num_str.to_int());
			}
		} else {
			error_message = "Unexpected value token '" + _current().value + "' for property '" + name +
					"' at line " + itos(_current().line);
			return ERR_PARSE_ERROR;
		}

		r_node.properties.push_back(prop);
		_skip_newlines();
		return OK;

	} else if (_current().type == GUITokenizer::TK_ARROW) {
		// Signal connection: signal_name => target.method()
		pos++; // consume =>
		_skip_newlines();

		GUISignalConnection conn;
		conn.signal_name = name;

		// Parse target.method()
		if (!_expect(GUITokenizer::TK_IDENTIFIER, "signal target"))
			return ERR_PARSE_ERROR;
		conn.target = tokens->get(pos - 1).value;

		if (!_expect(GUITokenizer::TK_DOT, "signal target.method"))
			return ERR_PARSE_ERROR;
		if (!_expect(GUITokenizer::TK_IDENTIFIER, "signal method name"))
			return ERR_PARSE_ERROR;
		conn.method = tokens->get(pos - 1).value;

		// Consume optional ()
		if (!_at_end() && _current().type == GUITokenizer::TK_PAREN_OPEN) {
			pos++;
			if (!_expect(GUITokenizer::TK_PAREN_CLOSE, "signal method parens"))
				return ERR_PARSE_ERROR;
		}

		r_node.connections.push_back(conn);
		_skip_newlines();
		return OK;
	}

	error_message = "Expected ':' or '=>' after '" + name + "' at line " + itos(_current().line);
	return ERR_PARSE_ERROR;
}

Variant GUIParser::_parse_value() {
	if (_current().type == GUITokenizer::TK_STRING) {
		Variant v = Variant(_current().value);
		pos++;
		return v;
	}
	if (_current().type == GUITokenizer::TK_NUMBER) {
		String s = _current().value;
		pos++;
		if (s.find(".") != -1)
			return Variant(s.to_double());
		return Variant(s.to_int());
	}
	if (_current().type == GUITokenizer::TK_BOOL_TRUE) {
		pos++;
		return Variant(true);
	}
	if (_current().type == GUITokenizer::TK_BOOL_FALSE) {
		pos++;
		return Variant(false);
	}
	if (_current().type == GUITokenizer::TK_MINUS) {
		pos++;
		if (_current().type == GUITokenizer::TK_NUMBER) {
			String s = "-" + _current().value;
			pos++;
			if (s.find(".") != -1)
				return Variant(s.to_double());
			return Variant(s.to_int());
		}
	}
	return Variant();
}

Error GUIParser::parse(const Vector<GUITokenizer::Token> &p_tokens, GUIDocument &r_document) {
	tokens = &p_tokens;
	pos = 0;
	error_message = "";

	Error err = _parse_directives(r_document);
	if (err != OK)
		return err;

	// Parse components
	while (!_at_end()) {
		_skip_newlines();
		if (_at_end())
			break;

		if (_current().type == GUITokenizer::TK_COMPONENT) {
			GUIComponent comp;
			err = _parse_component(comp);
			if (err != OK)
				return err;
			r_document.components.push_back(comp);
		} else {
			error_message = "Expected 'component' declaration, got '" + _current().value +
					"' at line " + itos(_current().line);
			return ERR_PARSE_ERROR;
		}
		_skip_newlines();
	}

	if (r_document.components.empty()) {
		error_message = "No components found in .gui file";
		return ERR_PARSE_ERROR;
	}

	return OK;
}

// --- Helper for parser tests ---
#ifdef DOCTEST
static Error _tokenize_and_parse(const String &p_source, GUIDocument &r_doc) {
	GUITokenizer tk;
	Error err = tk.tokenize(p_source);
	if (err != OK)
		return err;
	GUIParser parser;
	return parser.parse(tk.get_tokens(), r_doc);
}

// --- Doctests ---

TEST_CASE("[GUIParser] minimal component") {
	GUIDocument doc;
	String src = "@target: scene\ncomponent Foo inherits Control {\n}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.target == GUIDocument::TARGET_SCENE);
	CHECK(doc.components.size() == 1);
	CHECK(doc.components[0].name == "Foo");
	CHECK(doc.components[0].inherits == "Control");
}

TEST_CASE("[GUIParser] directives") {
	GUIDocument doc;
	String src = "@target: gdscript\n@script: \"my_script.gd\"\ncomponent X inherits Control {\n}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.target == GUIDocument::TARGET_GDSCRIPT);
	CHECK(doc.script_path == "my_script.gd");
}

TEST_CASE("[GUIParser] runtime target") {
	GUIDocument doc;
	String src = "@target: runtime\ncomponent X inherits Control {\n}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.target == GUIDocument::TARGET_RUNTIME);
}

TEST_CASE("[GUIParser] child node with properties") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits VBoxContainer {\n"
				 "  Label {\n"
				 "    text: \"Hello\"\n"
				 "    align: 1\n"
				 "  }\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.components[0].root.children.size() == 1);
	const GUINode &label = doc.components[0].root.children[0];
	CHECK(label.type == "Label");
	CHECK(label.properties.size() == 2);
	CHECK(label.properties[0].name == "text");
	CHECK(String(label.properties[0].value) == "Hello");
	CHECK(label.properties[1].name == "align");
	CHECK(int(label.properties[1].value) == 1);
}

TEST_CASE("[GUIParser] named node with hash") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  Button #my_btn {\n"
				 "    text: \"Click\"\n"
				 "  }\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.components[0].root.children[0].name == "my_btn");
	CHECK(doc.components[0].root.children[0].type == "Button");
}

TEST_CASE("[GUIParser] signal connection") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  Button {\n"
				 "    pressed => root.on_click()\n"
				 "  }\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	const GUINode &btn = doc.components[0].root.children[0];
	CHECK(btn.connections.size() == 1);
	CHECK(btn.connections[0].signal_name == "pressed");
	CHECK(btn.connections[0].target == "root");
	CHECK(btn.connections[0].method == "on_click");
}

TEST_CASE("[GUIParser] property binding") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  in property<String> title: \"Hi\"\n"
				 "  Label {\n"
				 "    text: root.title\n"
				 "  }\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.components[0].property_decls.size() == 1);
	CHECK(doc.components[0].property_decls[0].name == "title");
	CHECK(doc.components[0].property_decls[0].visibility == GUIPropertyDecl::VIS_IN);
	CHECK(doc.components[0].property_decls[0].type == "String");
	const GUINode &label = doc.components[0].root.children[0];
	CHECK(label.properties[0].is_binding());
	CHECK(label.properties[0].binding_expr == "root.title");
}

TEST_CASE("[GUIParser] signal declaration") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  signal clicked(x: int, y: int)\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.components[0].signal_decls.size() == 1);
	CHECK(doc.components[0].signal_decls[0].name == "clicked");
	CHECK(doc.components[0].signal_decls[0].args.size() == 2);
	CHECK(doc.components[0].signal_decls[0].args[0].first == "x");
	CHECK(doc.components[0].signal_decls[0].args[0].second == "int");
}

TEST_CASE("[GUIParser] @children slot") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component Card inherits PanelContainer {\n"
				 "  VBoxContainer {\n"
				 "    @children\n"
				 "  }\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	const GUINode &vbox = doc.components[0].root.children[0];
	CHECK(vbox.children.size() == 1);
	CHECK(vbox.children[0].is_children_slot);
	CHECK(vbox.children[0].name == "_slot");
}

TEST_CASE("[GUIParser] nested nodes") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits VBoxContainer {\n"
				 "  HBoxContainer {\n"
				 "    Button { text: \"A\" }\n"
				 "    Button { text: \"B\" }\n"
				 "  }\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	const GUINode &hbox = doc.components[0].root.children[0];
	CHECK(hbox.type == "HBoxContainer");
	CHECK(hbox.children.size() == 2);
	CHECK(String(hbox.children[0].properties[0].value) == "A");
	CHECK(String(hbox.children[1].properties[0].value) == "B");
}

TEST_CASE("[GUIParser] boolean property values") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  LineEdit {\n"
				 "    secret: true\n"
				 "    editable: false\n"
				 "  }\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	const GUINode &le = doc.components[0].root.children[0];
	CHECK(bool(le.properties[0].value) == true);
	CHECK(bool(le.properties[1].value) == false);
}

TEST_CASE("[GUIParser] in-out property visibility") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  in-out property<int> count: 0\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.components[0].property_decls[0].visibility == GUIPropertyDecl::VIS_IN_OUT);
}

TEST_CASE("[GUIParser] out property visibility") {
	GUIDocument doc;
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  out property<bool> valid: false\n"
				 "}\n";
	CHECK(_tokenize_and_parse(src, doc) == OK);
	CHECK(doc.components[0].property_decls[0].visibility == GUIPropertyDecl::VIS_OUT);
}

TEST_CASE("[GUIParser] error on empty file") {
	GUIDocument doc;
	GUITokenizer tk;
	CHECK(tk.tokenize("") == OK);
	GUIParser parser;
	CHECK(parser.parse(tk.get_tokens(), doc) != OK);
	CHECK(parser.get_error().find("No components") != -1);
}

TEST_CASE("[GUIParser] error on missing inherits") {
	GUIDocument doc;
	String src = "@target: scene\ncomponent Foo {\n}\n";
	CHECK(_tokenize_and_parse(src, doc) != OK);
}

#endif // DOCTEST
