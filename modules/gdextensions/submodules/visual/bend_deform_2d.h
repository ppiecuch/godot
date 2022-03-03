/*************************************************************************/
/*  bend_deform_2d.h                                                     */
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

#ifndef BEND_DEFORM_2D_H
#define BEND_DEFORM_2D_H

#include "scene/2d/mesh_instance_2d.h"
#include "scene/2d/sprite.h"

#include "modules/opensimplex/open_simplex_noise.h"

#include "elastic_simulation.h"

class SimulationController2D : public Resource {
	GDCLASS(SimulationController2D, Resource)

public:
	enum SimulationPrecision {
		PRECISION_LOW,
		PRECISION_MEDIUM,
		PRECISION_HIGH,
		SimulationPrecisionCount,
	};

private:
	Ref<ElasticSimulation> _sim;
	real_t _time_progress;

	bool simulation_paused;
	SimulationPrecision simulation_precision;
	Vector2 simulation_force;

	bool noise_modulation;
	int noise_time_scale;
	int noise_pixel_resolution;
	Ref<OpenSimplexNoise> _noise;

	Vector<Object *> _get_connected_nodes() const;

	const real_t _simulation_delta[SimulationPrecisionCount] = { 0.1, 0.05, 0.01 };

protected:
	static void _bind_methods();

public:
	void set_simulation_pause(bool p_state);
	bool is_simulation_paused() const;

	void set_simulation_precision(SimulationPrecision p_precision);
	SimulationPrecision get_simulation_precision() const;

	void set_simulation_force(const Vector2 &p_force);
	Vector2 get_simulation_force() const;

	void set_noise_modulation(bool p_state);
	bool is_noise_modulation_active() const;
	void set_noise_pixel_resolution(int p_res);
	int get_noise_pixel_resolution() const;
	void set_noise_time_scale(int p_scale);
	int get_noise_time_scale() const;

	Vector2 get_current_noise_modulation(const Vector2 &pos) const; // return [angle,magnitude]

	Ref<ElasticSimulation> get_simulation() const;

	void reset_simulation();
	void simulation_progress(real_t p_delta);

	Vector2 get_simulation_force_for_node(Node *p_node);
	bool add_simulation_force_for_node(Node *p_node, std::map<int, Vector2> &p_forces);

	SimulationController2D();
	~SimulationController2D();
};

VARIANT_ENUM_CAST(SimulationController2D::SimulationPrecision);

class SimulationControllerDebugInstance2D : public CanvasItem {
	GDCLASS(SimulationControllerDebugInstance2D, CanvasItem)

private:
	int cell_size;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	String get_configuration_warning() const;

public:
#ifdef TOOLS_ENABLED
	void _edit_set_position(const Point2 &p_position);
	Point2 _edit_get_position() const;
	void _edit_set_scale(const Size2 &p_scale);
	Size2 _edit_get_scale() const;
#endif

	Transform2D get_transform() const;

	int get_cell_size() const;
	void set_cell_size(int p_size);

	SimulationControllerDebugInstance2D();
};

class SimulationControllerInstance2D : public Node {
	GDCLASS(SimulationControllerInstance2D, Node)

private:
	Ref<SimulationController2D> controller;

	SimulationControllerDebugInstance2D *_debug_node;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _on_controller_changed();

public:
	Ref<SimulationController2D> get_controller() const;
	void set_controller(const Ref<SimulationController2D> &p_controller);

	bool get_debug_controller() const;
	void set_debug_controller(bool p_debug);

	SimulationControllerInstance2D();
	~SimulationControllerInstance2D();
};

class ElasticMeshInstance2D : public MeshInstance2D {
	GDCLASS(ElasticMeshInstance2D, MeshInstance2D)

private:
	int _sim_id;

	bool sprite_simulation_pause;
	Ref<SimulationController2D> controller;

	Vector2 noise_scale;
	int geometry_segments;
	ElasticSimulation::Anchor geometry_anchor;
	bool geometry_size_variation;
	real_t geometry_pixel_unit;
	bool geometry_debug;

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	int get_simulation_id() const;

	Ref<SimulationController2D> get_controller() const;
	void set_controller(const Ref<SimulationController2D> &p_controller);
	bool is_sprite_simulation_paused() const;
	void set_sprite_simulation_pause(bool p_state);
	Vector2 get_noise_scale() const;
	void set_noise_scale(Vector2 p_scale);

	int get_motion_interpolations() const;
	void set_motion_interpolations(int p_steps);

	ElasticMeshInstance2D();
	~ElasticMeshInstance2D();
};

class ElasticSprite : public Sprite {
	GDCLASS(ElasticSprite, Sprite)

private:
	int _sim_id;
	bool _sim_dirty;
	bool _geom_dirty;

	bool sprite_simulation_pause;
	Ref<SimulationController2D> controller;
	Vector2 noise_scale;
	int geometry_segments;
	ElasticSimulation::Anchor geometry_anchor;
	bool geometry_enable_deformation;
	bool geometry_size_variation;
	real_t geometry_pixel_unit;
	real_t geometry_stiffness;
	bool physics_variation;
	bool geometry_debug;

	bool _is_parent_controller() const;
	void _check_parent_controller();
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
	virtual void _edit_set_rect(const Rect2 &p_rect);
#endif
	void _notification(int p_what);

	void _on_simulation_update();
	void _on_simulation_changed();

	void _on_texture_changed();

public:
	int get_simulation_id() const;

	Ref<SimulationController2D> get_controller() const;
	void set_controller(const Ref<SimulationController2D> &p_controller);
	bool is_sprite_simulation_paused() const;
	void set_sprite_simulation_pause(bool p_state);
	Vector2 get_noise_scale() const;
	void set_noise_scale(Vector2 p_scale);

	bool is_geometry_deformation_enabled() const;
	void set_geometry_enable_deformation(bool p_state);
	void set_geometry_segments(int p_segments);
	int get_geometry_segments() const;
	void set_geometry_anchor(ElasticSimulation::Anchor p_anchor);
	ElasticSimulation::Anchor get_geometry_anchor() const;
	void set_geometry_size_variation(bool p_factor);
	bool is_geometry_size_variation() const;
	void set_geometry_pixel_unit(real_t p_unit);
	real_t get_geometry_pixel_unit() const;
	void set_geometry_stiffness(real_t p_stiffness);
	real_t get_geometry_stiffness() const;
	void set_physics_variation(bool p_variation);
	bool is_physics_variation() const;
	void set_geometry_debug(bool p_debug);
	bool get_geometry_debug() const;

	void debug_draw_geometry();

	ElasticSprite();
	~ElasticSprite();
};

#endif // BEND_DEFORM_2D_H
