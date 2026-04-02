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
