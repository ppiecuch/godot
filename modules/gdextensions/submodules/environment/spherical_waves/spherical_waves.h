/**************************************************************************/
/*  spherical_waves.h                                                     */
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

#ifndef SPHERICAL_WAVES_H
#define SPHERICAL_WAVES_H

#include "core/object.h"
#include "core/reference.h"
#include "core/variant.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/spatial.h"
#include "scene/main/node.h"
#include "scene/resources/mesh.h"
#include "scene/resources/mesh_data_tool.h"

class SphericalWaves : public Reference {
	GDCLASS(SphericalWaves, Reference);

	real_t *current_amplitudes;
	real_t *next_amplitudes;
	real_t *velocities;

	Size2i grid_size;
	real_t spring_constant, friction;

	static const real_t TwoSquareHalf;

protected:
	static void _bind_methods();

public:
	void init(const Size2 &p_grid_size, real_t p_spring_constant, real_t p_friction);
	void update(real_t p_delta);

	real_t get_amplitude(int p_pos_x, int p_pos_y);
	void set_amplitude(int p_pos_x, int p_pos_y, real_t p_value);
	void set_nodes(const Array &p_voxels, int p_index);
	void set_mesh(const Ref<Mesh> &p_mesh);

	SphericalWaves();
	~SphericalWaves();
};

#endif // SPHERICAL_WAVES_H
