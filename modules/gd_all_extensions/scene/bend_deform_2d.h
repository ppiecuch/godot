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


class ElasticSimulator2D : public Node2D {
	GDCLASS(ElasticSimulator2D, Node2D)

private:
	Ref<ElasticSimualtion> sim;
	Vector2 force_impulse;

	bool _enable_simulation;

public:
	static void _bind_methods();

	void _notification(int p_what);

	Ref<ElasticSimualtion> get_simulator() const;

	ElasticSimulator2D();
	~ElasticSimulator2D();

};

class DeformMeshInstance2D : public MeshInstance2D {
	GDCLASS(DeformMeshInstance2D, MeshInstance2D)

private:
	Ref<ElasticSimualtion> _sim;
	bool _enable_simulation;

public:
	static void _bind_methods();

	void _notification(int p_what);

	DeformMeshInstance2D();
	~DeformMeshInstance2D();

};

class DeformSprite : public Sprite {
	GDCLASS(DeformSprite, Sprite)

private:
	Ref<ElasticSimualtion> _sim;
	int _sim_id;
	bool _sim_dirty;
	bool _sim_shared;
	bool _geom_dirty;

	bool simulation_active;
	bool simulation_override_delta;
	real_t simulation_delta;
	real_t simulation_pixel_scale;
	int simulation_geom_segments;
	ElasticSimualtion::Anchor simulation_geom_anchor;
	real_t simulation_spring_factor;
	Vector2 simulation_force_impulse;
	bool simulation_debug;

	void _check_parent_simulator();
	void _update_simulation();

	Ref<ArrayMesh> _mesh;
	void _create_geom();
	void _update_geom();

protected:
	static void _bind_methods();

	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);

public:
	void set_simulation_state(bool p_state);
	bool is_simulation_active() const;
	void reset_simulation();
	void set_simulation_geom_anchor(ElasticSimualtion::Anchor p_anchor);
	ElasticSimualtion::Anchor get_simulation_geom_anchor() const;
	void set_simulation_geom_segments(int p_segments);
	int get_simulation_geom_segments() const;
	void set_simulation_force_impulse(const Vector2 &p_force);
	Vector2 get_simulation_force_impulse() const;
	void set_simulation_override_delta(bool p_state);
	bool is_simulation_override_delta() const;
	void set_simulation_delta(real_t p_delta);
	real_t get_simulation_delta() const;
	void set_simulation_pixel_scale(real_t p_scale);
	real_t get_simulation_pixel_scale() const;
	void set_simulation_spring_factor(real_t p_factor);
	real_t get_simulation_spring_factor() const;
	void set_simulation_debug(bool p_debug);
	bool get_simulation_debug() const;

	void debug_draw();

	DeformSprite();
	~DeformSprite();
};

#endif // BENDDEFORM2D_H
