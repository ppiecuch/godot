/**************************************************************************/
/*  scatter_multi_mesh_2d.h                                               */
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

#ifndef SCATTER_MULTI_MESH_2D_H
#define SCATTER_MULTI_MESH_2D_H

#include "core/local_vector.h"
#include "scene/2d/multimesh_instance_2d.h"

class ScatterMultiMesh2D : public MultiMeshInstance2D {
	GDCLASS(ScatterMultiMesh2D, MultiMeshInstance2D)

	int num_layers;
	struct LayerInfo {
		int num_instances = 10;
		bool show_debug_area = false;
		Ref<Image> mask;
	};
	LocalVector<LayerInfo> layers_info;

	Vector3 offset_position; // Add an offset to the placed instances.
	Vector3 offset_rotation; // Add a rotation offset to the placed instances.
	Vector3 base_scale; // Change the base scale of the instanced meshes.
	Vector3 min_random_size; // The minimum random size for each instance.
	Vector3 max_random_size; // The maximum random size for each instance.
	Vector3 random_rotation; // Rotate each instance by a random amount between.

	bool show_debug_area;

	void _create_debug_area();
	void _delete_debug_area();

	void scatter();

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	void set_num_layers(int p_num_layers);
	int get_num_layers() const;

	ScatterMultiMesh2D();
	~ScatterMultiMesh2D();
};

#endif // SCATTER_MULTI_MESH_2D_H
