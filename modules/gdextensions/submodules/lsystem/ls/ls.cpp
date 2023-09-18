/**************************************************************************/
/*  ls.cpp                                                                */
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

#include "ls/details/standart_grammar.h"
#include "ls/details/stochastic_grammar.h"
#include "ls/lsystem.h"

#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

/// LSystem

bool LSystem::check_grammar(std::vector<std::string> rules) {
	std::string r = rules[0];
	short pos = r.find(" (");
	if (std::string::npos != pos) {
		return true;
	} else {
		return false;
	}
}

std::string LSystem::build(std::string axiom, std::vector<std::string> rules, int iterations) {
	std::string result;
	if (check_grammar(rules) == 0) {
		StandartGrammar ls(axiom, rules);
		ls.iterate(iterations);
		result = ls.get_result();
	} else {
		StochasticGrammar ls(axiom, rules);
		ls.iterate(iterations);
		result = ls.get_result();
	}
	return result;
}

/// StandartGrammar

void StandartGrammar::iterate(const int iterations) {
	std::vector<LRule> rul = get_rules(rules);
	for (unsigned i = 0; i < iterations; ++i) {
		std::string new_cond;
		for (unsigned j = 0; j < condition.size(); ++j) {
			std::string cur;
			cur += condition[j];
			std::string replacement = cur;
			for (auto r : rul) {
				if (cur == r.variable) {
					replacement = r.rule;
					break;
				}
			}
			new_cond += replacement;
		}
		condition = new_cond;
	}
}

std::vector<LRule> StandartGrammar::get_rules(std::vector<std::string> &rul) {
	std::vector<LRule> v;
	for (auto r : rul) {
		auto pos = r.find(" => ");
		if (pos != -1) {
			LRule n(r.substr(0, pos), r.substr(pos + 4));
			v.push_back(n);
		}
	}
	return v;
}

/// StochasticGrammar

void StochasticGrammar::iterate(int iterations = 1) {
	std::vector<LRule> r = get_rules(rules);
	std::map<float, LRule> rul = build_rule_range(r);
	srand(time(nullptr));
	for (unsigned i = 0; i < iterations; ++i) {
		std::string new_cond;
		float random = ((float)rand() / (float)RAND_MAX);
		auto randomRule = rul.lower_bound(random);
		for (unsigned j = 0; j < condition.size(); ++j) {
			std::string cur;
			cur += condition[j];
			std::string replacement = cur;
			if (randomRule->second.variable == cur) {
				replacement = randomRule->second.rule;
			}
			new_cond += replacement;
		}
		condition = new_cond;
	}
}

std::vector<LRule> StochasticGrammar::get_rules(std::vector<std::string> &rul) {
	std::vector<LRule> v;
	for (auto r : rul) {
		const auto pos1 = r.find(" => ");
		const auto pos2 = r.find(" (");
		const auto pos3 = r.find(")");
		if (pos1 != -1 && pos2 != -1) {
			std::string p = r.substr(pos2 + 3, pos3);
			LRule n(r.substr(0, pos1), r.substr(pos1 + 4, pos2 - pos1 - 4), stof(p));
			v.push_back(n);
		}
	}
	return v;
}

std::map<float, LRule> StochasticGrammar::build_rule_range(const std::vector<LRule> &rules) {
	std::map<float, LRule> rules_with_probability;
	float probability = 0;
	for (auto r : rules) {
		probability += r.probability;
		rules_with_probability.insert(std::make_pair(probability, r));
	}
	return rules_with_probability;
}
