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

#ifndef GDELASTICNODE2D_H
#define GDELASTICNODE2D_H

#include "scene/2d/mesh_instance_2d.h"

namespace godot {

class DeformMeshInstance2D : public MeshInstance2D {
	GDCLASS(DeformMeshInstance2D, MeshInstance2D)

private:
	float time_passed;

private:
	int mesh_segments;
	bool simulation_active;
	Vector2 simulation_force;
	float simulation_delta;
	Vector2 flow_factors;

	bool debug_geometry;
	bool debug_simulation;
	bool debug_output;

public:
	static void _bind_methods();

	DeformMeshInstance2D();
	~DeformMeshInstance2D();

	void _init(); // our initializer called by Godot

	void _process(float delta);
};

} // namespace godot

#endif // GDELASTICNODE2D_H
