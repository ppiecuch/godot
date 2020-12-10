/*************************************************************************/
/*  bend_deform_2d.h                                                     */
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

#ifndef BENDDEFORM2D_H
#define BENDDEFORM2D_H

#include "scene/2d/mesh_instance_2d.h"
#include "scene/2d/sprite.h"

#include "elastic_simulation.h"


#define simulation_changed "simulation_changed"
#define simulation_update "simulation_update"
#define controller_changed "controller_changed"

class SimulationController2D : public Resource {
	GDCLASS(SimulationController2D, Resource)

private:
	Ref<ElasticSimulation> _sim;
	bool _sim_dirty;
	Node *_sim_owner;

	bool simulation_active;
	bool simulation_fixed_delta;
	real_t simulation_delta;
	real_t simulation_spring_factor;
	real_t simulation_spring_variation;

	Ref<Texture> motion_texture;

	Vector2 _simulation_force_impulse;
	real_t _simulation_force_impulse_duration;

	void _update_simulation();

protected:
	static void _bind_methods();

	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	void set_simulation_state(bool p_state);
	bool is_simulation_active() const;
	void set_simulation_fixed_delta(bool p_state);
	bool is_simulation_fixed_delta() const;
	void set_simulation_delta(real_t p_delta);
	real_t get_simulation_delta() const;
	void set_simulation_spring_factor(real_t p_factor);
	real_t get_simulation_spring_factor() const;
	void set_simulation_spring_variation(real_t p_factor);
	real_t get_simulation_spring_variation() const;
	void apply_simulation_force_impulse(const Vector2 &p_force, real_t p_duration);
	void apply_deform_force_impulse(const Vector2 &p_force, real_t p_duration);
	void reset_simulation();

	Ref<ElasticSimulation> get_simulation() const;
	void simulation_progress(real_t process_delta_time);

	Vector2 get_simulation_force_impulse() const;
	real_t get_simulation_force_impulse_duration() const;

	Node *get_owner() const;
	void set_owner(Node *p_owner);
	bool has_owner() const;

	SimulationController2D();
	~SimulationController2D();
};

class SimulationControllerInstance2D : public Node2D {
	GDCLASS(SimulationControllerInstance2D, Node2D)

private:
	Ref<SimulationController2D> controller;

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	Ref<SimulationController2D> get_controller() const;
	void set_controller(const Ref<SimulationController2D> &p_controller);

	SimulationControllerInstance2D();
	~SimulationControllerInstance2D();
};


class DeformMeshInstance2D : public MeshInstance2D {
	GDCLASS(DeformMeshInstance2D, MeshInstance2D)

private:
	Ref<SimulationController2D> controller;

	int geometry_segments;
	ElasticSimulation::Anchor geometry_anchor;
	bool geometry_size_variation;
	real_t geometry_pixel_unit;
	Vector2 geometry_deform_force;
	bool geometry_debug;

	enum SigOperation {
		SIG_CONNECT,
		SIG_DISCONNECT,
	};

	void _configure_controller(SigOperation p_op);
	void _connect_controller() { _configure_controller(SIG_CONNECT); }
	void _disconnect_controller() { _configure_controller(SIG_DISCONNECT); }

protected:
	static void _bind_methods();

	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);

public:
	Ref<SimulationController2D> get_controller() const;
	void set_controller(const Ref<SimulationController2D> &p_controller);

	DeformMeshInstance2D();
	~DeformMeshInstance2D();
};

class DeformSprite : public Sprite {
	GDCLASS(DeformSprite, Sprite)

private:
	int _sim_id;
	bool _sim_dirty;
	bool _geom_dirty;

	Ref<SimulationController2D> controller;
	int geometry_segments;
	ElasticSimulation::Anchor geometry_anchor;
	bool geometry_size_variation;
	real_t geometry_pixel_unit;
	Vector2 geometry_deform_force;
	bool geometry_debug;

	enum SigOperation {
		SIG_CONNECT,
		SIG_DISCONNECT,
	};

	void _check_parent_simulator();
	void _configure_controller(SigOperation p_op);
	void _connect_controller() { _configure_controller(SIG_CONNECT); }
	void _disconnect_controller() { _configure_controller(SIG_DISCONNECT); }
	Rect2 _get_texture_uv_rect() const;

	Ref<ArrayMesh> _mesh;
	Array _mesh_array;
	void _create_geom();
	void _update_geom();
	void _update_simulation();

protected:
	static void _bind_methods();

#ifdef TOOLS_ENABLED
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
#endif
	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);

public:
	Ref<SimulationController2D> get_controller() const;
	void set_controller(const Ref<SimulationController2D> &p_controller);

	void set_geometry_segments(int p_segments);
	int get_geometry_segments() const;
	void set_geometry_anchor(ElasticSimulation::Anchor p_anchor);
	ElasticSimulation::Anchor get_geometry_anchor() const;
	void set_geometry_size_variation(bool p_factor);
	bool is_geometry_size_variation() const;
	void set_geometry_pixel_unit(real_t p_unit);
	real_t get_geometry_pixel_unit() const;
	void set_geometry_debug(bool p_debug);
	bool get_geometry_debug() const;

	void debug_draw();

	void _on_simulation_update();
	void _on_simulation_changed();

	DeformSprite();
	~DeformSprite();
};

#endif // BENDDEFORM2D_H
