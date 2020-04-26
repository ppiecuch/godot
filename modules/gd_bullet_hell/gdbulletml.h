// Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.
// Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// -- Godot Engine <https://godotengine.org>

#pragma once

#include "core/reference.h"

#include "bulletml/include/bulletmlparser-tinyxml.h"
#include "bulletml/include/bulletmlparser.h"
#include "bulletml/include/bulletmlrunner.h"
#include "core/math/random_number_generator.h"

class BulletCommand : public BulletMLRunner {
	BulletCommand(BulletMLParser *parser, int bullet, double direction, double speed) :
		BulletMLRunner(parser),
		bullet(bullet),
		direction(direction),
		speed(speed) {
	}
	
	BulletCommand(BulletMLState *state, int bullet, double direction, double speed) :
		BulletMLRunner(state),
		bullet(bullet),
		direction(direction),
		speed(speed) {
	}

private:	     
	Ref<RandomNumberGenerator> rng;
	int bullet;
	double direction;
	double speed;
	Vector<BulletCommand *> bullets;
	
public:
	virtual double getBulletDirection() override { return direction; };
	virtual double getAimDirection() override { return 0.0; };
	virtual double getBulletSpeed() override { return speed; };
	virtual double getDefaultSpeed() override { return 0.0; };
	virtual double getRank() override { return 0.0; };
	virtual int getTurn() override { return 0; };
	virtual void doVanish() override {
	};
	virtual void createSimpleBullet(double p_direction, double p_speed) override {
		rng.instance();
		rng->randomize();
		speed = p_speed;
		direction = p_direction;
	};
	virtual void createBullet(BulletMLState *state, double p_direction, double p_speed) override {
		rng.instance();
		rng->randomize();
		BulletCommand *runner = new BulletCommand(state, rng->randi(), direction, speed);
        bullets.push_back(runner);
	};
	virtual void doChangeDirection(double) override {}
	virtual void doChangeSpeed(double) override {}
	virtual void doAccelX(double) override {}
	virtual void doAccelY(double) override {}
	virtual double getBulletSpeedX() override { return 0.0; }
	virtual double getBulletSpeedY() override { return 0.0; }
	virtual double getRand() override { return rng->randf(); }
};

class GdBulletml : public Reference {
private:
	GDCLASS(GdBulletml, Reference);

public:
	GdBulletml() {}

	void _init() {}
	static void _register_methods() {}
};
