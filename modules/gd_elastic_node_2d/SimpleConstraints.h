/*
 *  SimpleConstraints.h
 *
 *  Created by Pawel Piecuch on 12/19/11.
 *  Copyright 2011 Roche Polska. All rights reserved.
 *
 */

#include <vector>
#include <string>
#include "inc/ptr_vector.h"
#include "inc/basic_math.h"

using std::vector;
typedef basic_math::d2::Vector vec2;

#ifndef NO
# define NO false
#endif
#ifndef YES
# define YES true
#endif

namespace sim3 {

	struct Point;
	typedef stdx::ptr_vector<Point> PointsArray;
	struct Constraint;
	typedef stdx::ptr_vector<Constraint> ConstraintsArray;

	class Simulation3 {

	public:
		float interval;
		PointsArray points;
		ConstraintsArray constraints;

		Simulation3() { }
		void simulate(float delta, vec2 &gravity);
		// --
		void make_beam(float x, float y, float length, int segments);
		void add_beam(const vector<float*> &r1, const vector<float*> &r2, const vector<float*> &p1, const vector<float*> &p2, const vector<float*> &b1, const vector<float*> &b2);
	};

	struct Point {
		vec2 position, previous, acceleration;
		bool fixed;
		const vector<float*> *aliases;

		Point (float x, float y, bool fixed = NO):
		fixed(fixed),
		position(vec2(x, y)),
		previous(vec2(x, y)),
		acceleration(vec2(0, 0)),
		aliases(NULL) { }
		Point (float x, float y, const vector<float*> *aliases, bool fixed = NO):
		fixed(fixed),
		position(vec2(x, y)),
		previous(vec2(x, y)),
		acceleration(vec2(0, 0)),
		aliases(aliases) { }
		void accelerate(const vec2 &v) { acceleration.Add(v); }
		void correct(const vec2 &v) { if (!fixed) position.Add(v); }
		void simulate(float delta) {
			if (!fixed) {
				acceleration.Mul(delta*delta);
				
				const register float x = position.x;
				const register float y = position.y;
				position.Set(x*2-previous.x+acceleration.x, y*2-previous.y+acceleration.y);
				previous.Set(x, y);

				acceleration.Zero();
			}
		}
		void update_aliases() {
			if (aliases) {
				const int cnt = aliases->size();
				const register float x = position.x;
				const register float y = position.y;
				float *const *root = &((*aliases)[0]);
				switch (cnt) {
					case 4: 
						(*root)[0] = (*(root+1))[0] = (*(root+2))[0] = (*(root+3))[0] = x;
						(*root)[1] = (*(root+1))[1] = (*(root+2))[1] = (*(root+3))[1] = y;
						break;
					case 3:
						(*root)[0] = (*(root+1))[0] = (*(root+2))[0] = x;
						(*root)[1] = (*(root+1))[1] = (*(root+2))[1] = y;
						break;
					case 2:
						(*root)[0] = (*(root+1))[0] = x;
						(*root)[1] = (*(root+1))[1] = y;
						break;
					case 1:
						(*root)[0] = x;
						(*root)[1] = y;
						break;
				}
			}
		}
	};

	struct Constraint {
		Point &point1, &point2;
		float target;

		Constraint(Point &point1, Point &point2):point1(point1), point2(point2) {
			target = point1.position.Distance(point2.position);
		}

		void resolve() {
			vec2 direction = point2.position - point1.position;
			const float length = direction.Length();
			const float factor = (length-target)/(length*2.5);	// this should be flexible constraint
			direction.Mul(factor);								// correction factor

			point1.correct(direction);
			direction.Mul(-1);
			point2.correct(direction);
		}
	};
} // namespace sim3
