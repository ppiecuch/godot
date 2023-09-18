/**************************************************************************/
/*  lsystem_grammar.h                                                     */
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

#pragma once

#include "ls/lrule.h"

#include <algorithm>
#include <string>
#include <vector>

class LSystemGrammar {
protected:
	std::string condition; // condition of the lsystem
	std::vector<std::string> rules; // rules of the lsystem

public:
	void add_rule(const std::string &rule) {
		rules.push_back(rule);
	}
	void set_axiom(const std::string &axiom) {
		condition = axiom;
	}
	std::string get_result() {
		return condition;
	}

	LSystemGrammar(std::string axiom, std::vector<std::string> rul) :
			condition(axiom), rules(rul) {}
	LSystemGrammar(std::vector<std::string> rul) :
			rules(rul) {}
	LSystemGrammar(std::string axiom) :
			condition(axiom) {}
	LSystemGrammar() {}
};
