#include "lsystem_grammar.h"

LSystemGrammar::LSystemGrammar(std::string axiom, std::vector<std::string> rul):condition(axiom), rules(rul) {}
LSystemGrammar::LSystemGrammar(std::vector<std::string> rul):rules(rul) {}
LSystemGrammar::LSystemGrammar(std::string axiom):condition(axiom) {}
LSystemGrammar::LSystemGrammar(){}

void LSystemGrammar::add_rule(const std::string &rule) {
	rules.push_back(rule);
}

void LSystemGrammar::set_axiom(const std::string &axiom) {
	condition = axiom;
}

std::string LSystemGrammar::get_result() {
	return condition;
}
