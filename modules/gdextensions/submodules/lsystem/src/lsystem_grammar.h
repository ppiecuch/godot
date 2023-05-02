#pragma once

#include "lrule.h"

#include <string>
#include <algorithm>
#include <vector>

class LSystemGrammar {
public:
	LSystemGrammar(std::string, std::vector<std::string>);
	LSystemGrammar(std::vector<std::string>);
	LSystemGrammar(std::string);
	LSystemGrammar();
	void add_rule(const std::string &rule);
	void set_axiom(const std::string &axiom);
	std::string get_result();

protected:
	std::string condition; // condition of the lsystem
	std::vector<std::string> rules; // rules of the lsystem
};
