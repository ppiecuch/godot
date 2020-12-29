/*************************************************************************/
/*  elastic_simulation.cpp                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include <algorithm>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/variant.h"

#include "elastic_simulation.h"

const real_t REAL_MIN = std::numeric_limits<real_t>::min();
const real_t REAL_MAX = std::numeric_limits<real_t>::max();

#ifndef NO
#define NO false
#endif
#ifndef YES
#define YES true
#endif

// https://math.stackexchange.com/questions/13404/mapping-irregular-quadrilateral-to-a-rectangle
// https://github.com/softfab/tshirt/commit/56a824b06d98cd3744a6fa5ad1bd5a5ef20b6581
// https://github.com/brianshaler/holidayjs/blob/bb6f7dcc1e1bbf8ac7e41e638655a285be450d68/public/vendor/phaser/wip/verlet/AngleConstraint.js
// * https://github.com/onidev/OniDev-Examples/tree/master/Various/Verlet

namespace sim3 {

struct Point;
typedef std::unique_ptr<Point> Point_r;
typedef std::vector<Point_r> PointsArray;
struct PointParam;
typedef std::vector<PointParam> PointParamsArray;
struct DistanceConstraint;
typedef std::list<DistanceConstraint> DConstraintsArray;
typedef std::vector<DistanceConstraint> DConstraintsVector;
struct AngularConstraint;
typedef std::list<AngularConstraint> AConstraintsArray;
typedef std::vector<AngularConstraint> AConstraintsVector;

typedef std::vector<Vector2> Vec2Vector;

struct PointParam {
	std::vector<real_t> bern_poly_pack[2]; // bernstein polynomial packing
	Vector2 p0, p; // orign and point after applying s,t,u to p0, should result in original point
	real_t s, t; // distances along S/T/U axes
};

struct Point {
	bool fixed;
	Point2 rest;
	Point2 position;
	Point2 previous;
	Point2 control;
	Vector2 acceleration;

	Point(const Vector2 xy, bool fixed = NO) :
			fixed(fixed), rest(xy), position(xy), previous(xy), acceleration(0, 0) {}
	Point(real_t x, real_t y, bool fixed = NO) :
			fixed(fixed), rest(x, y), position(x, y), previous(x, y), acceleration(0, 0) {}
	void accelerate(const Vector2 &v) { acceleration += v; }
	void correct(const Vector2 &v) {
		if (!fixed) position += v;
	}
	void simulate(real_t delta) {
		if (!fixed) {
			acceleration *= delta * delta;

			Vector2 next_position = 2 * position - previous + acceleration;
			previous = position;
			position = next_position;

			acceleration = Vector2();
		}
	}
	void deform(real_t delta) {
		if (!fixed) {
			acceleration *= delta * delta;

			Vector2 next_position = rest + acceleration;
			previous = rest;
			position = next_position;

			acceleration = Vector2();
		}
	}
	void reset() {
		position = previous = rest;
	}
};

inline Point2 operator+(const Point &pt, const Point2 &vec) {
	return pt.position + vec;
}
inline Point2 operator+(const Point *pt, const Point2 &vec) {
	return pt->position + vec;
}

inline Point_r make_point_r(const Vector2 &xy, bool fixed = NO) {
	return std::unique_ptr<Point>(new Point(xy, fixed));
}

struct DistanceConstraint {
	const real_t factor_base = 1.5;

	simid_t sim_id;
	Point &point1, &point2;
	real_t target;
	real_t spring_factor;

	DistanceConstraint(simid_t sim_id, Point &point1, Point &point2, real_t factor = 0.0) :
			sim_id(sim_id), point1(point1), point2(point2), spring_factor(factor_base + factor_base * CLAMP(factor, 0, 1)) { target = point1.position.distance_to(point2.position); }

	void resolve() {
		const Vector2 direction = point2.position - point1.position;
		const real_t length = direction.length();
		const real_t factor = (length - target) / (length * spring_factor);
		const Vector2 correction = direction * factor;

		point1.correct(correction);
		point2.correct(-correction);
	}
};

struct AngularConstraint {
	const real_t factor_base = 1.5;

	int sim_id;
	Point &lpoint, &point, &rpoint;
	real_t target;
	real_t spring_factor;

	AngularConstraint(int sim_id, Point &lpoint, Point &point, Point &rpoint, real_t factor = 0.0) :
			sim_id(sim_id), lpoint(lpoint), point(point), rpoint(rpoint), target(point.position.angle_between(lpoint.position, rpoint.position)), spring_factor(factor_base + factor_base * CLAMP(factor, 0, 1)) {}

	void resolve() {
		const real_t angle = point.position.angle_between(lpoint.position, rpoint.position);
		real_t diff = angle - target;

		if (diff <= -Math_PI) {
			diff += (2 * Math_PI);
		} else if (diff >= Math_PI) {
			diff -= (2 * Math_PI);
		}

		diff *= spring_factor; // correction factor

		lpoint.position = lpoint.position.rotated_around(point.position, diff);
		rpoint.position = rpoint.position.rotated_around(point.position, -diff);
		point.position = point.position.rotated_around(lpoint.position, diff);
		point.position = point.position.rotated_around(rpoint.position, -diff);
	}
};

class Simulation {

public:
	real_t interval;
	std::vector<PointsArray> points;
	DConstraintsArray d_constraints;
	AConstraintsArray a_constraints;

	Simulation() {}
	void simulate(real_t delta, const Vector2 &force) {
		for (DistanceConstraint &c : d_constraints) {
			c.resolve();
		}
		for (PointsArray &arr : points) {
			for (Point_r &point : arr) {
				point->accelerate(force);
				point->simulate(delta);
			}
		}
	}
	void deform(int sim_id, real_t delta, const Vector2 &force) {
		for (Point_r &point : points[sim_id]) {
			point->accelerate(force);
			point->deform(delta);
		}
	}
	void reset() {
		for (PointsArray &arr : points) {
			for (Point_r &point : arr) {
				point->reset();
			}
		}
	}

	inline Point *add_point(int sim_id, const Point2 &xy, bool fixed = NO) {
		points[sim_id].push_back(make_point_r(xy, fixed));
		return points[sim_id].back().get();
	}
	inline Point *add_point(int sim_id, const real_t &x, const real_t &y, bool fixed = NO) {
		points[sim_id].push_back(make_point_r(Point2(x, y), fixed));
		return points[sim_id].back().get();
	}

	void update_geom(int sim_id, const Transform2D &transform) {
		for (Point_r &point : points[sim_id]) {
			point->rest = transform.xform(point->rest);
			point->position = transform.xform(point->position);
			point->previous = transform.xform(point->previous);
		}
	}
	// segment:
	// ========
	//  |        |
	//  |        |
	// (2)------(3)  (0)----(2)----
	//  |        |    |      |
	//  |        |    |      |
	// (0)------(1)  (1)----(3)----
	//  top/bottom    left/right
	void build_geom(int sim_id, const Point2 &starting, const Point2 &opposite, const Vec2Vector &steps, real_t spring_factor, real_t spring_variation) {

		/* root */
		Point *p1 = add_point(sim_id, starting, YES);
		Point *p2 = add_point(sim_id, starting + opposite, YES);

		const int segments = steps.size();

		const real_t spring_change = (spring_variation * spring_factor) / segments;

		for (int i = 0; i < segments; i++) {
			Point *n1 = add_point(sim_id, p1 + steps[i]);
			Point *n2 = add_point(sim_id, p2 + steps[i]);

			d_constraints.push_back(DistanceConstraint(sim_id, *p1, *n1, spring_factor - i * spring_change)); // [n1][n2]
			d_constraints.push_back(DistanceConstraint(sim_id, *p2, *n2, spring_factor - i * spring_change)); //  |  /|
			d_constraints.push_back(DistanceConstraint(sim_id, *n1, *n2, spring_factor - i * spring_change)); //  | / |
			d_constraints.push_back(DistanceConstraint(sim_id, *p1, *n2, spring_factor - i * spring_change)); // [p1][p2]

			p1 = n1;
			p2 = n2;
		}
	}
	int make_geom(const Point2 &starting, const Point2 &opposite, const Vec2Vector &steps, real_t spring_factor, real_t spring_variation) {

		const int sim_id = points.size();

		points.push_back(PointsArray());

		build_geom(sim_id, starting, opposite, steps, spring_factor, spring_variation);

		return sim_id;
	}
	void update_geom(simid_t sim_id, const Point2 &starting, const Point2 &opposite, const Vec2Vector &steps, real_t spring_factor, real_t spring_variation) {

		d_constraints.remove_if([sim_id](DistanceConstraint &c) { return c.sim_id == sim_id; });
		points[sim_id] = PointsArray();

		build_geom(sim_id, starting, opposite, steps, spring_factor, spring_variation);
	}
	void remove_geom(simid_t sim_id) {
		d_constraints.remove_if([sim_id](DistanceConstraint &c) { return c.sim_id == sim_id; });
		points[sim_id] = PointsArray();
	}
};

// Mesh FFD 2D support

class FfdSimulation {

	Vector2 S, T; // Local coordinate system
	int Sc, Tc; // Number of controls for S, T respectively.  (Sc, Tc must be >= 1)
	PointParamsArray vparams; // mesh vertex info and parameters

	// Calculate a binomial coefficient using the multiplicative formula
	real_t binomial(int n, int k) const {
		real_t total = 1.0f;
		for (int i = 1; i <= k; ++i) {
			total *= (n - (k - 1)) / real_t(i);
		}
		return total;
	}
	// Calculate a bernstein polynomial
	real_t bernstein_polynomial(int n, int v, real_t x) const { return binomial(n, v) * powf(x, v) * powf(1.0f - x, n - v); }
	// Calculate local coordinates
	void calculate_st(const Vector2 &max, const Vector2 &min) {
		S = Vector2(max.x - min.x, 0.0f);
		T = Vector2(0.0f, max.y - min.y);
	}
	void calculate_trivariate_bernstein_polynomial(const Vector2 &p0, const PoolVector3Array &mesh, PointParamsArray &vertex_params) {
		vertex_params.clear();
		for (int v = 0; v < mesh.size(); v++) {
			Vector2 diff = Vector2(mesh[v].x, mesh[v].y) - p0;

			PointParam tmp;
			tmp.s = (diff / S).x;
			tmp.t = (diff / T).y;
			tmp.p = p0 + (tmp.s * S) + (tmp.t * T);
			tmp.p0 = p0;
			// Pre-calculate bernstein polynomial expansion. It only needs to be done once per parameterization
			for (int i = 0; i <= Sc; i++) {
				for (int j = 0; j <= Tc; j++) {
					tmp.bern_poly_pack[1].push_back(bernstein_polynomial(Tc, j, tmp.t));
				}
				tmp.bern_poly_pack[0].push_back(bernstein_polynomial(Sc, i, tmp.s));
			}
			vertex_params.push_back(tmp);
#ifdef DEBUG
			if (tmp.p.distance_to(points[v].position) > 0.001f) {
				PRINT_WARN(vformat("Warning, vtx[%d] does not match it's parameterization.", v));
			}
#endif
		}
	}
	void create_control_points(const Vector2 &orig, PointsArray &points) {
		const Vector2 st = S + T;
		for (Point_r &pt : points) {
			pt->control = (pt->position - orig) / st;
		}
	}
	void parameterize_mesh(const PoolVector3Array &mesh, PointsArray &points) {
		Vector2 min(REAL_MAX, REAL_MAX);
		Vector2 max(REAL_MIN, REAL_MIN);
		for (int v = 0; v < mesh.size(); ++v) {
			const Vector2 vxt(mesh[v].x, mesh[v].y);
			max = Vector2::max(vxt, max);
			min = Vector2::min(vxt, min);
		}
		calculate_st(max, min);
		calculate_trivariate_bernstein_polynomial(min, mesh, vparams);
		create_control_points(min, points);
	}
};
} // namespace sim3

int ElasticSimulation::make_sim(const Size2 &p_rect, int p_segments, bool p_dynamic_split, Anchor p_anchor, real_t p_spring_factor, real_t p_spring_variation) {

	ERR_FAIL_COND_V(p_segments <= 0, -1);
	ERR_FAIL_COND_V(p_spring_factor < 0, -1);

	const bool vert = p_anchor == SIM_ANCHOR_BOTTOM || p_anchor == SIM_ANCHOR_TOP;
	const bool horiz = p_anchor == SIM_ANCHOR_LEFT || p_anchor == SIM_ANCHOR_RIGHT;

	const Vector2 HX(p_rect.width / (horiz ? 2 : 1), 0);
	const Vector2 HY(0, p_rect.height / (vert ? 2 : 1));
	const Vector2 DX(p_rect.width / (horiz ? p_segments : 1), 0);
	const Vector2 DY(0, p_rect.height / (vert ? p_segments : 1));

	sim3::Vec2Vector steps;
	Vector2 starting, opposite, step;
	if (p_dynamic_split && p_segments > 1) {
		switch (p_anchor) {
			case SIM_ANCHOR_TOP: {
				step = HY;
				opposite = DX;
			} break;
			case SIM_ANCHOR_BOTTOM: {
				starting.y += p_rect.height;
				step = -HY;
				opposite = DX;
			} break;
			case SIM_ANCHOR_LEFT: {
				step = HX;
				opposite = DY;
			} break;
			case SIM_ANCHOR_RIGHT: {
				starting.x += p_rect.width;
				step = -HX;
				opposite = DY;
			} break;
			default: {
				WARN_PRINT("Invalid anchor value.");
				return -1;
			}
		}
		for (int s = 0; s < p_segments - 1; ++s) {
			if (s == p_segments - 2) {
				// last two segments
				steps.push_back(1.5 * step);
				steps.push_back(0.5 * step);
			} else
				steps.push_back(step);
			step /= 2;
		}
	} else {
		switch (p_anchor) {
			case SIM_ANCHOR_TOP: {
				step = DY;
				opposite = DX;
			} break;
			case SIM_ANCHOR_BOTTOM: {
				starting.y += p_rect.height;
				step = -DY;
				opposite = DX;
			} break;
			case SIM_ANCHOR_LEFT: {
				step = DX;
				opposite = DY;
			} break;
			case SIM_ANCHOR_RIGHT: {
				starting.x += p_rect.width;
				step = -DX;
				opposite = DY;
			} break;
			default: {
				WARN_PRINT("Invalid anchor value.");
				return -1;
			}
		}
		steps = sim3::Vec2Vector(p_segments, step);
	}
	return _sim->make_geom(starting, opposite, steps, p_spring_factor, p_spring_variation);
}

void ElasticSimulation::update_sim(simid_t sim_id, const Size2 &p_rect, int p_segments, bool p_dynamic_split, Anchor p_anchor, real_t p_spring_factor, real_t p_spring_variation) {

	ERR_FAIL_INDEX(sim_id, _sim->points.size());
	ERR_FAIL_COND(p_segments <= 0);

	const bool vert = p_anchor == SIM_ANCHOR_BOTTOM || p_anchor == SIM_ANCHOR_TOP;
	const bool horiz = p_anchor == SIM_ANCHOR_LEFT || p_anchor == SIM_ANCHOR_RIGHT;

	const Vector2 HX(p_rect.x / (horiz ? 2 : 1), 0);
	const Vector2 HY(0, p_rect.y / (vert ? 2 : 1));
	const Vector2 DX(p_rect.x / (horiz ? p_segments : 1), 0);
	const Vector2 DY(0, p_rect.y / (vert ? p_segments : 1));

	sim3::Vec2Vector steps;
	Vector2 starting, opposite, step;
	if (p_dynamic_split && p_segments > 1) {
		switch (p_anchor) {
			case SIM_ANCHOR_TOP: {
				step = HY;
				opposite = DX;
			} break;
			case SIM_ANCHOR_BOTTOM: {
				starting.y += p_rect.y;
				step = -HY;
				opposite = DX;
			} break;
			case SIM_ANCHOR_LEFT: {
				step = HX;
				opposite = DY;
			} break;
			case SIM_ANCHOR_RIGHT: {
				starting.x += p_rect.x;
				step = -HX;
				opposite = DY;
			} break;
			default: {
				WARN_PRINT("Invalid anchor value.");
				return;
			}
		}
		for (int s = 0; s < p_segments - 1; ++s) {
			if (s == p_segments - 2) {
				// last two segments
				steps.push_back(1.5 * step);
				steps.push_back(0.5 * step);
			} else
				steps.push_back(step);
			step /= 2;
		}
	} else {
		switch (p_anchor) {
			case SIM_ANCHOR_TOP: {
				step = DY;
				opposite = DX;
			} break;
			case SIM_ANCHOR_BOTTOM: {
				starting.y += p_rect.y;
				step = -DY;
				opposite = DX;
			} break;
			case SIM_ANCHOR_LEFT: {
				step = DX;
				opposite = DY;
			} break;
			case SIM_ANCHOR_RIGHT: {
				starting.x += p_rect.x;
				step = -DX;
				opposite = DY;
			} break;
			default: {
				WARN_PRINT("Invalid anchor value.");
				return;
			}
		}
		steps = sim3::Vec2Vector(p_segments, step);
	}
	return _sim->update_geom(sim_id, starting, opposite, steps, p_spring_factor, p_spring_variation);
}

void ElasticSimulation::remove_sim(simid_t sim_id) {
	ERR_FAIL_INDEX(sim_id, _sim->points.size());

	_sim->remove_geom(sim_id);
}

void ElasticSimulation::reset_sim() {
	_sim->reset();
}

int ElasticSimulation::get_sim_position_count(simid_t sim_id) const {
	ERR_FAIL_INDEX_V(sim_id, _sim->points.size(), 0);
	return _sim->points[sim_id].size();
}

Vector2 ElasticSimulation::get_sim_position_at(simid_t sim_id, unsigned int p_index) const {
	ERR_FAIL_INDEX_V(sim_id, _sim->points.size(), Vector2());
	ERR_FAIL_INDEX_V(p_index, _sim->points[sim_id].size(), Vector2());
	return _sim->points[sim_id][p_index]->position;
}

bool ElasticSimulation::is_sim_point_fixed(simid_t sim_id, unsigned int p_index) const {
	ERR_FAIL_INDEX_V(sim_id, _sim->points.size(), false);
	ERR_FAIL_INDEX_V(p_index, _sim->points[sim_id].size(), false);
	return _sim->points[sim_id][p_index]->fixed;
}

inline static sim3::DConstraintsVector _filter_constrains(simid_t sim_id, const sim3::DConstraintsArray &constraints) {
	sim3::DConstraintsVector filter;
	std::copy_if(constraints.begin(), constraints.end(), std::back_inserter(filter), [sim_id](const sim3::DistanceConstraint &c) { return c.sim_id == sim_id; });
	return filter;
}

int ElasticSimulation::get_sim_constraint_count(simid_t sim_id) const {
	ERR_FAIL_INDEX_V(sim_id, _sim->points.size(), 0);
	sim3::DConstraintsVector filter = _filter_constrains(sim_id, _sim->d_constraints);
	return filter.size();
}

ElasticSimulation::Constraint ElasticSimulation::get_sim_constraint_at(simid_t sim_id, unsigned int p_index) const {
	ERR_FAIL_INDEX_V(sim_id, _sim->points.size(), ElasticSimulation::Constraint());
	sim3::DConstraintsVector filter = _filter_constrains(sim_id, _sim->d_constraints);
	ERR_FAIL_INDEX_V(p_index, filter.size(), Constraint());
	const sim3::DistanceConstraint &c = filter[p_index];
	return (ElasticSimulation::Constraint){
		c.point1.position,
		c.point2.position,
		(c.target - c.point1.position.distance_to(c.point2.position)) / c.target
	};
}

void ElasticSimulation::simulate(float p_delta, const Vector2 &p_impulse) {
	time_passed += p_delta;
	_sim->simulate(p_delta, p_impulse);
}

void ElasticSimulation::deform(simid_t sim_id, float p_delta, const Vector2 &p_impulse) {
	_sim->deform(sim_id, p_delta, p_impulse);
}

ElasticSimulation::ElasticSimulation() {
	time_passed = 0.0;
	_sim = std::unique_ptr<sim3::Simulation>(new sim3::Simulation());
}

ElasticSimulation::~ElasticSimulation() {
	_sim.release();
}
