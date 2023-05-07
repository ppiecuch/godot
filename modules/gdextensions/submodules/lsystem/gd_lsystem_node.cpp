/**************************************************************************/
/*  gd_lsystem_node.cpp                                                   */
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

#include "gd_lsystem_node.h"

#include "ls/lsystem.h"
#include "ls/ls.cpp"

#include <vector>
#include <stack>

class TurtleGraf {
	real_t step, angle;

	PoolVector2Array vertices; // representation of the script iteration as a set of vertices
	float x = 0; // current position of the turtle
	float y = 0; // current position of the turtle
	float direction = 0; // the position of the turtle's head in space( in degrees )
	bool pen = true;
	std::stack<real_t> save_stack; // stack to store coordinates

	bool get_pen() const { return pen; }
	void set_pen(bool state) { pen = state; }

public:
	void move_to(real_t x1, real_t y1) { // Move the turtle to a point with coordinates (x,y)
		x = x1, y = y1;
	}
	void move(real_t distance) { // Crawl distance steps forward calculate the new coordinates of the turtle
		const real_t dx = x + distance * Math::cos(direction);
		const real_t dy = y + distance * Math::sin(direction);
		if (get_pen()) {
			vertices.push_back(Vector2(x, y));
			vertices.push_back(Vector2(dx, dy));
		}

		x = dx, y = dy;
	}
	void turn_right(real_t angle) { // Turn the turtle to the right side
		direction -= Math::deg2rad(angle);
	}
	void turn_left(real_t angle) { // Turn the turtle to the left side
		direction += Math::deg2rad(angle);
	}
	void save();
	void restore();

	// Lowers the pen-the turtle, after which she leaves a trail when moving
	void pen_down() {
		set_pen(true);
	}

	// Lifts the pen the turtle, after which it ceases to leave a trail when moving
	void pen_up() {
		set_pen(false);
	}

	void init(real_t init_step, real_t init_angle) {
		step = init_step ,angle = init_angle;
	}
	PoolVector2Array interpret(std::string, std::vector<char>);

	TurtleGraf() : direction(Math::deg2rad(-90.0)) {}
};

// LSystemSolver implementation

PoolVector2Array LSystemSolver::interpret() {
	TurtleGraf turtle;
	LSystem ls;
	std::vector<std::string> lrules;
	for (const String &r : rules) {
		lrules.push_back(r.utf8().c_str());
	}
	std::string result = ls.build(axiom.utf8().c_str(), lrules, iterations);
	turtle.init(ls.get_step(), ls.get_angle());
	return turtle.interpret(result, ls.get_constants());
}

LSystemSolver::LSystemSolver() {
	iterations = 1;
}

// Turtle graphics implementation

void TurtleGraf::save() {
	save_stack.push(x);
	save_stack.push(y);
	save_stack.push(direction);
}

void TurtleGraf::restore() {
	direction = save_stack.top();
	save_stack.pop();
	const real_t ny = save_stack.top();
	save_stack.pop();
	const real_t nx = save_stack.top();
	save_stack.pop();
	x = nx;
	y = ny;
}

PoolVector2Array TurtleGraf::interpret(std::string result, std::vector<char> constants) {
	// interpretation of the alphabet in a certain action turtles
	for (unsigned i = 0, size = result.size(); i < size; ++i) {
		switch (result[i]) {
			case '+': {
				turn_left(angle);
			} break;
			case '-': {
				turn_right(angle);
			} break;
			case '[': {
				save();
			} break;
			case ']': {
				restore();
			} break;
			default: {
				if (find(constants.begin(), constants.end(), result[i]) == constants.end()) {
					move(step);
				}
			} break;
		}
	}
	return vertices;
}
