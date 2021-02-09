/*************************************************************************/
/*  bend_deform_2d.h                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

class SimulationController2D : public Resource {
	GDCLASS(SimulationController2D, Resource)

public:
	enum MotionPacking {
		PACKING_8BIT,
		PACKING_16BIT,
	};

private:
	Ref<ElasticSimulation> _sim;

	bool simulation_active;
	real_t simulation_delta;

	// Using motion texture for vertex displacement:
	// P' = P + WarpingMotionTexture[ StartUV + RandomTrajectoryUV * Time ]

	Ref<Image> motion_image;
	MotionPacking motion_packing;

	Vector2 _simulation_force_impulse;
	real_t _simulation_force_impulse_duration;

	void _update_simulation();

	// points are defined in normalized space here:
	Vector2 _get_motion_normalized_displacement(Point2 p_coord);

protected:
	static void _bind_methods();

public:
	void set_simulation_state(bool p_state);
	bool is_simulation_active() const;
	void set_simulation_delta(real_t p_delta);
	real_t get_simulation_delta() const;
	void apply_simulation_force_impulse(const Vector2 &p_force, real_t p_duration);
	void apply_deform_force(int sim_id, const Vector2 &p_force);
	void reset_simulation();

	Ref<ElasticSimulation> get_simulation() const;

	void simulation_progress();
	Vector2 get_simulation_force_impulse() const;
	real_t get_simulation_force_impulse_duration() const;

	Ref<Image> get_motion_image() const;
	void set_motion_image(const Ref<Image> &p_image);

	void set_motion_packing(MotionPacking p_packing);
	MotionPacking get_motion_packing() const;

	Vector2 get_motion_value(Node *p_node, real_t p_amp, int p_interpolations);

	SimulationController2D();
	~SimulationController2D();
};

VARIANT_ENUM_CAST(SimulationController2D::MotionPacking);

class SimulationControllerInstance2D : public Node2D {
	GDCLASS(SimulationControllerInstance2D, Node2D)

private:
	Ref<SimulationController2D> controller;
	bool motion_debug;

	Ref<ImageTexture> _motion_texture;

	void _draw_debug_marker(const Point2 &p0, real_t dir, int marker_length, int head_length, int head_width, const Color &marker_color1 = Color(1, 1, 0, 1), const Color &marker_color2 = Color(1, 1, 1, 1));

protected:
	static void _bind_methods();

	void _notification(int p_what);

	void _on_controller_changed();

public:
	Ref<SimulationController2D> get_controller() const;
	void set_controller(const Ref<SimulationController2D> &p_controller);
	bool get_motion_debug() const;
	void set_motion_debug(const bool &p_debug);

	SimulationControllerInstance2D();
	~SimulationControllerInstance2D();
};

class DeformMeshInstance2D : public MeshInstance2D {
	GDCLASS(DeformMeshInstance2D, MeshInstance2D)

private:
	int _sim_id;

	Ref<SimulationController2D> controller;

	real_t motion_trajectory_direction;
	Vector2 motion_trajectory_origin;
	real_t motion_amplify;
	int motion_interpolations;
	int geometry_segments;
	ElasticSimulation::Anchor geometry_anchor;
	bool geometry_size_variation;
	real_t geometry_pixel_unit;
	Vector2 geometry_deform_force;
	bool geometry_debug;

protected:
	static void _bind_methods();

	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);

public:
	int get_simulation_id() const;

	real_t get_motion_direction() const;
	void set_motion_direction(real_t p_angle);
	real_t get_motion_direction_degree() const;
	void set_motion_direction_degree(real_t p_degree);
	Vector2 get_motion_trajectory_origin() const;
	void set_motion_trajectory_origin(Vector2 p_origin);
	real_t get_motion_amplify() const;
	void set_motion_amplify(real_t p_amp);
	int get_motion_interpolations() const;
	void set_motion_interpolations(int p_steps);

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

	Vector2 _deform_force;

	Ref<SimulationController2D> controller;
	real_t motion_trajectory_direction;
	Vector2 motion_trajectory_origin;
	real_t motion_amplify;
	int motion_interpolations;
	int geometry_segments;
	ElasticSimulation::Anchor geometry_anchor;
	bool geometry_size_variation;
	real_t geometry_pixel_unit;
	real_t geometry_spring_factor;
	real_t geometry_spring_variation;
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
#endif
	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);

	void _on_simulation_update();
	void _on_simulation_changed();

	void _on_texture_changed();

public:
	int get_simulation_id() const;

	real_t get_motion_direction() const;
	void set_motion_direction(real_t p_angle);
	real_t get_motion_direction_degree() const;
	void set_motion_direction_degree(real_t p_degree);
	Vector2 get_motion_trajectory_origin() const;
	void set_motion_trajectory_origin(Vector2 p_origin);
	real_t get_motion_amplify() const;
	void set_motion_amplify(real_t p_amp);
	int get_motion_interpolations() const;
	void set_motion_interpolations(int p_steps);

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
	void set_geometry_spring_factor(real_t p_factor);
	real_t get_geometry_spring_factor() const;
	void set_geometry_spring_variation(real_t p_factor);
	real_t get_geometry_spring_variation() const;
	void set_geometry_debug(bool p_debug);
	bool get_geometry_debug() const;

	void deform_geometry(const Vector2 &force);

	void debug_draw_geometry();

	DeformSprite();
	~DeformSprite();
};

#endif // BENDDEFORM2D_H
