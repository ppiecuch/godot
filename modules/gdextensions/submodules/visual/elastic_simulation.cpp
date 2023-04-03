/**************************************************************************/
/*  elastic_simulation.cpp                                                */
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

// Reference:
// ----------
// https://math.stackexchange.com/questions/13404/mapping-irregular-quadrilateral-to-a-rectangle
// https://github.com/softfab/tshirt/commit/56a824b06d98cd3744a6fa5ad1bd5a5ef20b6581
// https://github.com/brianshaler/holidayjs/blob/bb6f7dcc1e1bbf8ac7e41e638655a285be450d68/public/vendor/phaser/wip/verlet/AngleConstraint.js
// * https://github.com/onidev/OniDev-Examples/tree/master/Various/Verlet
// https://math.stackexchange.com/questions/129854/convex-quadrilateral-test

typedef std::vector<Vector2> Vector2Array;

namespace sim3 {

struct Particle;
typedef std::unique_ptr<Particle> Particle_r;
typedef std::vector<Particle_r> ParticlesArray;
struct ParticleParam;
typedef std::vector<ParticleParam> ParticleParamsArray;
struct DistanceConstraint;
typedef std::vector<DistanceConstraint> DConstraintsArray;

struct ParticleParam {
	std::vector<real_t> bern_poly_pack[2]; // bernstein polynomial packing
	Vector2 p0, p; // orign and point after applying s,t,u to p0, should result in original point
	real_t s, t; // distances along S/T/U axes
};

struct Particle {
	bool fixed;
	Point2 rest;
	Point2 position, previous;
	Point2 control;
	real_t mass;

	Particle(const Point2 &xy, bool fixed = NO) :
			fixed(fixed), mass(1) { rest = position = previous = xy; }
	Particle(const Point2 &xy, real_t mass) :
			fixed(NO), mass(mass) { rest = position = previous = xy; }
	void correct(const Vector2 &v) {
		if (!fixed)
			position += v;
	}
	void simulate(real_t delta, const Vector2 &force) {
		if (!fixed) {
			const Vector2 acceleration = force * mass * delta * delta;
			const Point2 next_position = 2 * position - previous + acceleration;
			previous = position;
			position = next_position;
		}
	}
	void reset() { position = previous = rest; }
};

inline Point2 operator+(const Particle &pt, const Point2 &vec) {
	return pt.position + vec;
}
inline Point2 operator+(const Particle *pt, const Point2 &vec) {
	return pt->position + vec;
}

inline Particle_r make_particle_r(const Point2 &xy, bool fixed = NO) {
	return std::unique_ptr<Particle>(new Particle(xy, fixed));
}
inline Particle_r make_particle_r(const Point2 &xy, real_t mass) {
	return std::unique_ptr<Particle>(new Particle(xy, mass));
}

inline Point2 middle_point(const Point2 &a, const Point2 &b) {
	return (a + b) / 2;
}

struct DistanceConstraint {
	const real_t MinDistStiffnessFactor = 0.4;
	const real_t MaxDistStiffnessFactor = 0.7;

	simid_t sim_id;
	Particle &point1, &point2;
	real_t target;
	real_t stiffness;

	DistanceConstraint(simid_t sim_id, Particle &point1, Particle &point2, real_t factor = 0.0) :
			sim_id(sim_id), point1(point1), point2(point2) {
		target = point1.position.distance_to(point2.position);
		stiffness = Math::map1(factor, 0, 1, MinDistStiffnessFactor, MaxDistStiffnessFactor);
	}

	void resolve() {
		const Vector2 direction = point2.position - point1.position;
		const real_t length = direction.length();
		const real_t factor = stiffness * (1 - target / length);
		const Vector2 correction = direction * factor;

		point1.correct(correction);
		point2.correct(-correction);
	}
};

struct Sim {
	enum State {
		StateRunning,
		StatePaused,
		StateEmpty,
	};
	State state;
	int orientation;
	ParticlesArray particles;
	DConstraintsArray constraints;
};

// Simple beam simulation

class Simulation {
private:
	float _ALWAYS_INLINE_ _deform_angle(int orientation, float angle) {
		// converting atan2 result to: 0 .. 180|180 .. 0
		if (angle < 0)
			angle = -angle;
		// return always-positive deform angle
		return Math::abs(orientation + angle);
	}

public:
	std::vector<Sim> simulations;

	real_t angle_limit = 30;

	Simulation() {}
	void simulate(real_t delta, const Vector2 &force) {
		for (Sim &sim : simulations) {
			if (sim.state != Sim::StateRunning) {
				continue;
			}
			for (DistanceConstraint &c : sim.constraints) {
				c.resolve();
			}
			ParticlesArray &particles = sim.particles;
			const int pcnt = particles.size();
			for (int p = 0; p < pcnt - 2; p += 2) {
				const Point2 &p1 = middle_point(particles[p]->position, particles[p + 1]->position);
				const Point2 &p2 = middle_point(particles[p + 2]->position, particles[p + 3]->position);
				const real_t curr_deform = _deform_angle(sim.orientation, Math::rad2deg((p2 - p1).angle()));
				if (curr_deform > 90) {
					// failed constraint
					particles[p]->reset();
					particles[p + 1]->reset();
				}
				const real_t delta_corrected = delta * (curr_deform > angle_limit ? 0.1 : 1);
				particles[p]->simulate(delta_corrected, force);
				particles[p + 1]->simulate(delta_corrected, force);
				if (p == pcnt - 4) {
					particles[p + 2]->simulate(delta_corrected, force);
					particles[p + 3]->simulate(delta_corrected, force);
				}
			}
		}
	}
	void simulate(real_t delta, const std::vector<Vector2> &forces) {
		for (size_t f = 0; f < simulations.size(); f++) {
			Sim &sim = simulations[f];
			if (sim.state != Sim::StateRunning) {
				continue;
			}
			for (DistanceConstraint &c : sim.constraints) {
				c.resolve();
			}
			ParticlesArray &particles = sim.particles;
			const Vector2 &force = forces[f];
			const int pcnt = particles.size();
			for (int p = 0; p < pcnt - 2; p += 2) {
				const Point2 &p1 = middle_point(particles[p]->position, particles[p + 1]->position);
				const Point2 &p2 = middle_point(particles[p + 2]->position, particles[p + 3]->position);
				const real_t curr_deform = _deform_angle(sim.orientation, Math::rad2deg((p2 - p1).angle()));
				if (curr_deform < angle_limit) {
					particles[p]->simulate(delta, force);
					particles[p + 1]->simulate(delta, force);
					if (p == pcnt - 4) {
						particles[p + 2]->simulate(delta, force);
						particles[p + 3]->simulate(delta, force);
					}
				} else {
					// do not process simulation if segment is too bend
					break;
				}
			}
		}
	}
	void reset() {
		for (Sim &sim : simulations) {
			for (Particle_r &p : sim.particles) {
				p->reset();
			}
		}
	}

	inline Particle *add_point(int sim_id, const Point2 &xy, bool fixed = NO) {
		simulations[sim_id].particles.push_back(make_particle_r(xy, fixed));
		return simulations[sim_id].particles.back().get();
	}
	inline Particle *add_point(int sim_id, const Point2 &xy, real_t mass) {
		simulations[sim_id].particles.push_back(make_particle_r(xy, mass));
		return simulations[sim_id].particles.back().get();
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
	void build_geom(int sim_id, const Point2 &starting, const Point2 &opposite, const Vector2Array &steps, real_t stiffness, bool variation) {
		Sim &sim = simulations[sim_id];

		const int segments = steps.size();
		const real_t physics_variation_change = 0.4;

		real_t mass = 1;

		/* root */
		Particle *p1 = add_point(sim_id, starting, YES);
		Particle *p2 = add_point(sim_id, starting + opposite, YES);

		for (int i = 0; i < segments; i++) {
			Particle *n1 = add_point(sim_id, p1 + steps[i], mass);
			Particle *n2 = add_point(sim_id, p2 + steps[i], mass);

			sim.constraints.push_back(DistanceConstraint(sim_id, *p1, *n1, stiffness)); // [n1]--[n2]
			sim.constraints.push_back(DistanceConstraint(sim_id, *p2, *n2, stiffness)); //  |  __/|
			sim.constraints.push_back(DistanceConstraint(sim_id, *n1, *n2, stiffness)); //  | /   |
			sim.constraints.push_back(DistanceConstraint(sim_id, *p1, *n2, stiffness)); // [p1]  [p2]

			p1 = n1;
			p2 = n2;

			if (variation) {
				mass *= physics_variation_change;
				stiffness *= physics_variation_change;
			}
		}
	}
	int make_geom(int orientation, const Point2 &starting, Point2 &opposite, const Vector2Array &steps, real_t stiffness, bool variation) {
		const int sim_id = simulations.size();

		simulations.push_back(Sim{ Sim::StateRunning, orientation });
		build_geom(sim_id, starting, opposite, steps, stiffness, variation);

		return sim_id;
	}
	void update_geom(simid_t sim_id, int orientation, const Point2 &starting, const Point2 &opposite, const Vector2Array &steps, real_t stiffness, bool variation) {
		simulations[sim_id] = Sim{ simulations[sim_id].state, simulations[sim_id].orientation };
		build_geom(sim_id, starting, opposite, steps, stiffness, variation);
	}
	void remove_geom(simid_t sim_id) {
		simulations[sim_id] = Sim{ Sim::StateEmpty };
	}
};

// Mesh FFD 2D deformation

class MeshDeformation {
	Vector2 S, T; // Local coordinate system
	int Sc, Tc; // Number of controls for S, T respectively.  (Sc, Tc must be >= 1)
	ParticleParamsArray vparams; // mesh vertex info and parameters

	// Calculate a binomial coefficient using the multiplicative formula
	real_t binomial(int n, int k) const {
		real_t total = 1;
		for (int i = 1; i <= k; ++i) {
			total *= (n - (k - 1)) / real_t(i);
		}
		return total;
	}
	real_t bernstein_polynomial(int n, int v, real_t x) const { return binomial(n, v) * Math::pow(x, v) * Math::pow(1 - x, n - v); } // Calculate a bernstein polynomial
	void calculate_st(const Vector2 &max, const Vector2 &min) {
		S = Vector2(max.x - min.x, 0);
		T = Vector2(0, max.y - min.y);
	} // Calculate local coordinates
	void calculate_trivariate_bernstein_polynomial(const Vector2 &p0, const PoolVector3Array &mesh, ParticleParamsArray &vertex_params) {
		vertex_params.clear();
		for (int v = 0; v < mesh.size(); v++) {
			Vector2 diff = Vector2(mesh[v].x, mesh[v].y) - p0;

			ParticleParam tmp;
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
			if (tmp.p.distance_to(points[v].position) > 0.001) {
				PRINT_WARN(vformat("Warning, vtx[%d] does not match it's parameterization.", v));
			}
#endif
		}
	}
	void create_control_points(const Vector2 &orig, ParticlesArray &points) {
		const Vector2 st = S + T;
		for (Particle_r &pt : points) {
			pt->control = (pt->position - orig) / st;
		}
	}
	void parameterize_mesh(const PoolVector3Array &mesh, ParticlesArray &points) {
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

static Vector2Array _sim_geometry(const Size2 &p_rect, int p_segments, bool p_dynamic_split, ElasticSimulation::Anchor p_anchor, Vector2 &starting, Vector2 &opposite) {
	const bool vert = p_anchor == ElasticSimulation::SIM_ANCHOR_BOTTOM || p_anchor == ElasticSimulation::SIM_ANCHOR_TOP;
	const bool horiz = p_anchor == ElasticSimulation::SIM_ANCHOR_LEFT || p_anchor == ElasticSimulation::SIM_ANCHOR_RIGHT;

	const Vector2 HX(p_rect.width / (horiz ? 2 : 1), 0);
	const Vector2 HY(0, p_rect.height / (vert ? 2 : 1));
	const Vector2 DX(p_rect.width / (horiz ? p_segments : 1), 0);
	const Vector2 DY(0, p_rect.height / (vert ? p_segments : 1));

	Vector2Array steps;
	Vector2 step;
	if (p_dynamic_split && p_segments > 1) {
		switch (p_anchor) {
			case ElasticSimulation::SIM_ANCHOR_TOP: {
				step = HY;
				opposite = DX;
			} break;
			case ElasticSimulation::SIM_ANCHOR_BOTTOM: {
				starting.y += p_rect.height;
				step = -HY;
				opposite = DX;
			} break;
			case ElasticSimulation::SIM_ANCHOR_LEFT: {
				step = HX;
				opposite = DY;
			} break;
			case ElasticSimulation::SIM_ANCHOR_RIGHT: {
				starting.x += p_rect.width;
				step = -HX;
				opposite = DY;
			} break;
			default: {
				WARN_PRINT("Invalid anchor value.");
				return steps;
			}
		}
		for (int s = 0; s < p_segments - 1; ++s) {
			if (s == p_segments - 2) {
				// last two segments
				steps.push_back(step);
				steps.push_back(step);
			} else {
				steps.push_back(step);
			}
			step /= 2;
		}
	} else {
		switch (p_anchor) {
			case ElasticSimulation::SIM_ANCHOR_TOP: {
				step = DY;
				opposite = DX;
			} break;
			case ElasticSimulation::SIM_ANCHOR_BOTTOM: {
				starting.y += p_rect.height;
				step = -DY;
				opposite = DX;
			} break;
			case ElasticSimulation::SIM_ANCHOR_LEFT: {
				step = DX;
				opposite = DY;
			} break;
			case ElasticSimulation::SIM_ANCHOR_RIGHT: {
				starting.x += p_rect.width;
				step = -DX;
				opposite = DY;
			} break;
			default: {
				WARN_PRINT("Invalid anchor value.");
				return steps;
			}
		}
		steps = Vector2Array(p_segments, step);
	}

	return steps;
}

int _ALWAYS_INLINE_ _get_orientation(ElasticSimulation::Anchor anchor) {
	switch (anchor) {
		case ElasticSimulation::SIM_ANCHOR_TOP:
		case ElasticSimulation::SIM_ANCHOR_BOTTOM:
			return -90;
		case ElasticSimulation::SIM_ANCHOR_LEFT:
			return 0;
		case ElasticSimulation::SIM_ANCHOR_RIGHT:
			return -180;
		default: {
			WARN_PRINT("Invalid anchor value.");
			return 0;
		}
	}
}

int ElasticSimulation::make_sim(const Size2 &p_rect, int p_segments, bool p_dynamic_split, Anchor p_anchor, real_t p_stiffness_factor, bool p_variation) {
	ERR_FAIL_COND_V(p_segments < 1, -1);

#ifdef DEBUG_ENABLED
	if (p_stiffness_factor < 0 || p_stiffness_factor > 1) {
		WARN_PRINT("Optimal stiffness factor should be in range 0 .. 1");
	}
#endif

	Vector2 starting, opposite;
	const Vector2Array steps = _sim_geometry(p_rect, p_segments, p_dynamic_split, p_anchor, starting, opposite);
	ERR_FAIL_COND_V(starting.distance_to(opposite) <= 1, -1);
	return _sim->make_geom(_get_orientation(p_anchor), starting, opposite, steps, p_stiffness_factor, p_variation);
}

void ElasticSimulation::update_sim(simid_t p_sim_id, const Size2 &p_rect, int p_segments, bool p_dynamic_split, Anchor p_anchor, real_t p_stiffness_factor, bool p_variation) {
	ERR_FAIL_INDEX(p_sim_id, _sim->simulations.size());
	ERR_FAIL_COND(p_segments < 0);

#ifdef DEBUG_ENABLED
	if (p_stiffness_factor < 0 || p_stiffness_factor > 1) {
		WARN_PRINT("Optimal stiffness factor should be in range 0 .. 1");
	}
#endif

	Vector2 starting, opposite;
	const Vector2Array steps = _sim_geometry(p_rect, p_segments, p_dynamic_split, p_anchor, starting, opposite);
	ERR_FAIL_COND(starting.distance_to(opposite) <= 1);
	_sim->update_geom(p_sim_id, _get_orientation(p_anchor), starting, opposite, steps, p_stiffness_factor, p_variation);
}

void ElasticSimulation::set_sim_state(simid_t p_sim_id, State p_state) {
	ERR_FAIL_INDEX(p_sim_id, _sim->simulations.size());
	ERR_FAIL_INDEX(p_state, SimStateCount);

	sim3::Sim &sim = _sim->simulations[p_sim_id];
	switch (p_state) {
		case SIM_STATE_PAUSED: {
			sim.state = sim3::Sim::StatePaused;
		} break;
		case SIM_STATE_RUNNING: {
			sim.state = sim3::Sim::StateRunning;
		} break;
		default: {
			WARN_PRINT("Invalid state value");
		}
	}
}

ElasticSimulation::State ElasticSimulation::get_sim_state(simid_t p_sim_id) const {
	ERR_FAIL_INDEX_V(p_sim_id, _sim->simulations.size(), SIM_STATE_REMOVED);
	switch (_sim->simulations[p_sim_id].state) {
		case sim3::Sim::StateRunning:
			return SIM_STATE_RUNNING;
		case sim3::Sim::StatePaused:
			return SIM_STATE_PAUSED;
		default:
			return SIM_STATE_REMOVED;
	}
}

void ElasticSimulation::remove_sim(simid_t p_sim_id) {
	ERR_FAIL_INDEX(p_sim_id, _sim->simulations.size());

	_sim->remove_geom(p_sim_id);
}

void ElasticSimulation::reset_sim() {
	_sim->reset();
}

int ElasticSimulation::get_sim_particles_count(simid_t p_sim_id) const {
	ERR_FAIL_INDEX_V(p_sim_id, _sim->simulations.size(), 0);

	return _sim->simulations[p_sim_id].particles.size();
}

Vector2 ElasticSimulation::get_sim_particle_pos(simid_t p_sim_id, unsigned int p_index) const {
	ERR_FAIL_INDEX_V(p_sim_id, _sim->simulations.size(), Vector2());
	ERR_FAIL_INDEX_V(p_index, _sim->simulations[p_sim_id].particles.size(), Vector2());

	return _sim->simulations[p_sim_id].particles[p_index]->position;
}

real_t ElasticSimulation::get_sim_particle_mass(simid_t p_sim_id, unsigned int p_index) const {
	ERR_FAIL_INDEX_V(p_sim_id, _sim->simulations.size(), 1);
	ERR_FAIL_INDEX_V(p_index, _sim->simulations[p_sim_id].particles.size(), 1);

	return _sim->simulations[p_sim_id].particles[p_index]->mass;
}

bool ElasticSimulation::is_sim_particle_fixed(simid_t p_sim_id, unsigned int p_index) const {
	ERR_FAIL_INDEX_V(p_sim_id, _sim->simulations.size(), false);
	ERR_FAIL_INDEX_V(p_index, _sim->simulations[p_sim_id].particles.size(), false);

	return _sim->simulations[p_sim_id].particles[p_index]->fixed;
}

int ElasticSimulation::get_sim_constraint_count(simid_t p_sim_id) const {
	ERR_FAIL_INDEX_V(p_sim_id, _sim->simulations.size(), 0);

	return _sim->simulations[p_sim_id].constraints.size();
}

ElasticSimulation::Constraint ElasticSimulation::get_sim_constraint_at(simid_t p_sim_id, unsigned int p_index) const {
	ERR_FAIL_INDEX_V(p_sim_id, _sim->simulations.size(), ElasticSimulation::Constraint());
	ERR_FAIL_INDEX_V(p_index, _sim->simulations[p_sim_id].constraints.size(), ElasticSimulation::Constraint());

	const sim3::DistanceConstraint &c = _sim->simulations[p_sim_id].constraints[p_index];
	return ElasticSimulation::Constraint{
		c.point1.position,
		c.point2.position,
		Math::map1(c.point1.position.distance_to(c.point2.position) / c.target, 0.8, 1.2, -1, 1)
	};
}

void ElasticSimulation::simulate_all(float p_delta, const Vector2 &p_force) {
	time_passed += p_delta;
	_sim->simulate(p_delta, p_force);
}

void ElasticSimulation::simulate(float p_delta, const std::map<simid_t, Vector2> &p_forces) {
	std::vector<Vector2> forces(_sim->simulations.size());
	for (const auto &f : p_forces) {
		ERR_FAIL_INDEX(f.first, _sim->simulations.size());
		forces[f.first] = f.second;
	}
	time_passed += p_delta;
	_sim->simulate(p_delta, forces);
}

void ElasticSimulation::_bind_methods() {
	BIND_ENUM_CONSTANT(SIM_ANCHOR_LEFT);
	BIND_ENUM_CONSTANT(SIM_ANCHOR_RIGHT);
	BIND_ENUM_CONSTANT(SIM_ANCHOR_TOP);
	BIND_ENUM_CONSTANT(SIM_ANCHOR_BOTTOM);

	BIND_ENUM_CONSTANT(SIM_STATE_RUNNING);
	BIND_ENUM_CONSTANT(SIM_STATE_PAUSED);
	BIND_ENUM_CONSTANT(SIM_STATE_REMOVED);
}

ElasticSimulation::ElasticSimulation() {
	time_passed = 0.0;
	_sim = std::unique_ptr<sim3::Simulation>(new sim3::Simulation());
}

ElasticSimulation::~ElasticSimulation() {
	if (sim3::Simulation *sim = _sim.release())
		delete sim;
}
