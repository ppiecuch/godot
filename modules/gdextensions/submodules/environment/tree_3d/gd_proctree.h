/**************************************************************************/
/*  gd_proctree.h                                                         */
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

// Based on gdTree3D by Artyom Bozhko (JekSun97)
// https://github.com/JekSun97/gdTree3D
// proctree.js Copyright (c) 2012, Paul Brunt
// C++ port Copyright (c) 2015, Jari Komppa

#ifndef GD_PROCTREE_H
#define GD_PROCTREE_H

#include "scene/3d/mesh_instance.h"
#include "scene/3d/spatial.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/surface_tool.h"

namespace Proctree {
class Tree;
}

class ProceduralTree3D : public Spatial {
	GDCLASS(ProceduralTree3D, Spatial);

	MeshInstance *trunk_instance;
	MeshInstance *twig_instance;

	// Tree parameters.
	int seed;
	int segments;
	int levels;
	int tree_steps;
	float initial_branch_length;
	float length_falloff_factor;
	float length_falloff_power;
	float clump_max;
	float clump_min;
	float branch_factor;
	float drop_amount;
	float grow_amount;
	float sweep_amount;
	float max_radius;
	float climb_rate;
	float trunk_kink;
	float taper_rate;
	float radius_falloff_rate;
	float twist_rate;
	float trunk_length;
	float v_multiplier;
	float twig_scale;
	bool twig_enabled;

	// Materials.
	Ref<Material> trunk_material;
	Ref<Material> twig_material;

	bool _dirty;
	bool _in_tree;

	void _rebuild();
	void _rebuild_trunk_mesh(Proctree::Tree &tree);
	void _rebuild_twig_mesh(Proctree::Tree &tree);
	void _mark_dirty();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_seed(int p_seed);
	int get_seed() const;

	void set_segments(int p_segments);
	int get_segments() const;

	void set_levels(int p_levels);
	int get_levels() const;

	void set_tree_steps(int p_steps);
	int get_tree_steps() const;

	void set_initial_branch_length(float p_length);
	float get_initial_branch_length() const;

	void set_length_falloff_factor(float p_factor);
	float get_length_falloff_factor() const;

	void set_length_falloff_power(float p_power);
	float get_length_falloff_power() const;

	void set_clump_max(float p_max);
	float get_clump_max() const;

	void set_clump_min(float p_min);
	float get_clump_min() const;

	void set_branch_factor(float p_factor);
	float get_branch_factor() const;

	void set_drop_amount(float p_amount);
	float get_drop_amount() const;

	void set_grow_amount(float p_amount);
	float get_grow_amount() const;

	void set_sweep_amount(float p_amount);
	float get_sweep_amount() const;

	void set_max_radius(float p_radius);
	float get_max_radius() const;

	void set_climb_rate(float p_rate);
	float get_climb_rate() const;

	void set_trunk_kink(float p_kink);
	float get_trunk_kink() const;

	void set_taper_rate(float p_rate);
	float get_taper_rate() const;

	void set_radius_falloff_rate(float p_rate);
	float get_radius_falloff_rate() const;

	void set_twist_rate(float p_rate);
	float get_twist_rate() const;

	void set_trunk_length(float p_length);
	float get_trunk_length() const;

	void set_v_multiplier(float p_multiplier);
	float get_v_multiplier() const;

	void set_twig_scale(float p_scale);
	float get_twig_scale() const;

	void set_twig_enabled(bool p_enabled);
	bool get_twig_enabled() const;

	void set_trunk_material(const Ref<Material> &p_material);
	Ref<Material> get_trunk_material() const;

	void set_twig_material(const Ref<Material> &p_material);
	Ref<Material> get_twig_material() const;

	void generate();

	Ref<ArrayMesh> generate_trunk_mesh() const;
	Ref<ArrayMesh> generate_twig_mesh() const;

	int get_trunk_vertex_count() const;
	int get_trunk_face_count() const;
	int get_twig_vertex_count() const;
	int get_twig_face_count() const;

	ProceduralTree3D();
	~ProceduralTree3D();
};

#endif // GD_PROCTREE_H
