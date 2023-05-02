#include "lsystem.h"

LSystem::LSystem() {}

bool LSystem::check_grammar(std::vector<std::string> rules) {
	std::string r = rules[0];
	short pos = r.find(" (");
	if (std::string::npos != pos) {
		return true;
	} else {
		return false;
	}
}

void LSystem::add_constant(char constant) {
	constants.push_back(constant);
}

void LSystem::set_step(float _step) {
	step = _step;
}

void LSystem::set_angle(float _angle) {
	angle = _angle;
}

void LSystem::build(std::string axiom, std::vector<std::string> rules, int iterations) {
	std::string result;
	if(check_grammar(rules) == 0) {
		StandartGrammar ls(axiom, rules);
		ls.iterate(iterations);
		result = ls.get_result();
	} else {
		StochasticGrammar ls(axiom, rules);
		ls.iterate(iterations);
		result = ls.get_result();
	}

	turtle.init(step, angle);
	turtle.interpret(result, constants);
}

void LSystem::loop() {
	turtle.draw(this);
}
