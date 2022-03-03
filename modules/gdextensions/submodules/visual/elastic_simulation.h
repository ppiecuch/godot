/*************************************************************************/
/*  elastic_simulation.h                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef ELASTICSIMULATION_H
#define ELASTICSIMULATION_H

#include <map>
#include <memory>

#include "core/class_db.h"
#include "core/reference.h"

typedef unsigned int simid_t;

namespace sim3 {
class Simulation;
}

class ElasticSimulation : public Reference {
	GDCLASS(ElasticSimulation, Reference);

private:
	float time_passed;

protected:
	static void _bind_methods();

public:
	enum Anchor {
		SIM_ANCHOR_LEFT,
		SIM_ANCHOR_RIGHT,
		SIM_ANCHOR_TOP,
		SIM_ANCHOR_BOTTOM,
		SimAnchorCount,
	};

	enum State {
		SIM_STATE_RUNNING,
		SIM_STATE_PAUSED,
		SIM_STATE_REMOVED,
		SimStateCount,
	};

	float simulation_delta;
	Vector2 flow_factors;

	std::unique_ptr<sim3::Simulation> _sim;

	int make_sim(const Size2 &p_rect, int p_segments, bool p_dynamic_split, Anchor p_anchor, real_t p_stiffness_factor = 0.0, bool p_variation = false);
	void update_sim(simid_t p_sim_id, const Size2 &p_rect, int p_segments, bool p_dynamic_split, Anchor p_anchor, real_t p_stiffness_factor = 0.0, bool p_variation = false);
	void set_sim_state(simid_t p_sim_id, State p_state);
	State get_sim_state(simid_t p_sim_id) const;
	void remove_sim(simid_t p_sim_id);
	void reset_sim();
	int get_sim_particles_count(simid_t p_sim_id) const;
	Vector2 get_sim_particle_pos(simid_t p_sim_id, unsigned int p_index) const;
	real_t get_sim_particle_mass(simid_t p_sim_id, unsigned int p_index) const;
	bool is_sim_particle_fixed(simid_t p_sim_id, unsigned int p_index) const;
	void simulate(real_t p_delta, const std::map<simid_t, Vector2> &p_forces);
	void simulate_all(real_t p_delta, const Vector2 &p_force);

	struct Constraint {
		Vector2 begin, end;
		real_t deviation; // -1 .. 1
	};
	int get_sim_constraint_count(simid_t p_sim_id) const;
	Constraint get_sim_constraint_at(simid_t p_sim_id, unsigned int p_index) const;

	ElasticSimulation();
	~ElasticSimulation();
};

VARIANT_ENUM_CAST(ElasticSimulation::Anchor);
VARIANT_ENUM_CAST(ElasticSimulation::State);

#endif // ELASTICSIMULATION_H
