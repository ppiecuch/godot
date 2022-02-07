/*************************************************************************/
/*  gd_procedural_mesh.h                                                 */
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

#ifndef GD_PROCEDURAL_MESH_H
#define GD_PROCEDURAL_MESH_H

#include "scene/resources/mesh.h"

// Simple runtime preview generation:
// ----------------------------------
// func _ready():
//     remove_all_child()
//     randomize()
//     textures.shuffle() # randomize textures list
//     for y in range(0, 5):
//         for x in range(0, 7):
//             var mesh_inst = MeshInstance.new()
//             mesh_inst.translation = Vector3(x * space, y * space, 0) - origin
//             var mesh = ProceduralMesh.new()
//             var index:int = y * 7 + x
//             mesh.primitive = 35 if index == 11 else index # Show teapot instead of the EmptyMesh
//             var mat = SpatialMaterial.new()
//             mat.albedo_texture = load(textures[index])
//             mat.params_cull_mode = SpatialMaterial.CULL_DISABLED
//             if mesh.get_surface_count() > 0:
//                 mesh.surface_set_material(0, mat)
//             mesh_inst.mesh = mesh
//             add_child(mesh_inst)

class ProceduralMesh : public ArrayMesh {
	GDCLASS(ProceduralMesh, ArrayMesh);

public:
	enum GeomAxis {
		GEOM_AXIS_X,
		GEOM_AXIS_Y,
		GEOM_AXIS_Z,
	};

	// NOTE: Order is important here
	enum GeomPrimitive {
		// Shapes
		GEOM_EMPTY_SHAPE,
		GEOM_LINE_SHAPE,
		GEOM_RECTANGLE_SHAPE,
		GEOM_ROUNDED_RECTANGLE_SHAPE,
		GEOM_CIRCLE_SHAPE,
		GEOM_GRID_SHAPE,
		GEOM_BEZIER_SHAPE,
		// Paths
		GEOM_EMPTY_PATH,
		GEOM_LINE_PATH,
		GEOM_KNOT_PATH,
		GEOM_HELIX_PATH,
		// Meshes
		GEOM_EMPTY_MESH,
		GEOM_PLANE_MESH,
		GEOM_BOX_MESH,
		GEOM_ROUNDED_BOX_MESH,
		GEOM_SPHERE_MESH,
		GEOM_DISK_MESH,
		GEOM_CYLINDER_MESH,
		GEOM_CAPPED_CYLINDER_MESH,
		GEOM_CONE_MESH,
		GEOM_CAPPED_CONE_MESH,
		GEOM_SPHERICAL_CONE_MESH,
		GEOM_TUBE_MESH,
		GEOM_CAPPED_TUBE_MESH,
		GEOM_CAPSULE_MESH,
		GEOM_CONVEX_POLYGON_MESH,
		GEOM_DODECAHEDRON_MESH,
		GEOM_ICOSAHEDRON_MESH,
		GEOM_ICOSPHERE_MESH,
		GEOM_TRIANGLE_MESH,
		GEOM_SPHERICAL_TRIANGLE_MESH,
		GEOM_SPRING_MESH,
		GEOM_TORUS_KNOT_MESH,
		GEOM_TORUS_MESH,
		GEOM_BEZIER_MESH,
		GEOM_TEAPOT_MESH,
		//
		GEOM_PRIMITIVES_COUNT
	};

	enum GeomModifiers {
		// Shape modifiers
		MODIF_AXISSWAP_SHAPE,
		MODIF_FLIP_SHAPE,
		MODIF_MERGE_SHAPE,
		MODIF_REPEAT_SHAPE,
		MODIF_ROTATE_SHAPE,
		MODIF_SCALE_SHAPE,
		MODIF_SUBDIVIDE_SHAPE,
		MODIF_TRANSFORM_SHAPE,
		MODIF_TRANSLATE_SHAPE,
		// Path modifiers
		MODIF_AXISSWAP_PATH,
		MODIF_FLIP_PATH,
		MODIF_MERGE_PATH,
		MODIF_REPEAT_PATH,
		MODIF_ROTATE_PATH,
		MODIF_SCALE_PATH,
		MODIF_SUBDIVIDE_PATH,
		MODIF_TRANSFORM_PATH,
		MODIF_TRANSLATE_PATH,
		// Mesh modifiers
		MODIF_AXISSWAP_MESH,
		MODIF_EXTRUDE_MESH,
		MODIF_FLIP_MESH,
		MODIF_LATHE_MESH,
		MODIF_MERGE_MESH,
		MODIF_REPEAT_MESH,
		MODIF_ROTATE_MESH,
		MODIF_SCALE_MESH,
		MODIF_SPHERIFY_MESH,
		MODIF_SUBDIVIDE_MESH,
		MODIF_TRANSFORM_MESH,
		MODIF_TRANSLATE_MESH,
		MODIF_UVSWAP_MESH,
		//
		MODIF_GEOM_COUNT
	};

	struct GeomModifiersStatus {
		bool axis_swap;
		GeomAxis axis_swap_param[3];
		bool flip;
		bool rotate;
		Vector3 rotate_param;
		bool scale;
		Vector3 scale_param{ 1, 1, 1 };
		bool spherify;
		real_t spherify_param[2] = { 0, 0 };
		bool transform;
		Node *transform_node = nullptr;
		bool translate;
		Vector3 translate_param;
		bool uv_flip;
		bool uv_flip_param[2] = { false, false };
		bool uv_scale;
		Vector2 uv_scale_param{ 1, 1 };
		bool uv_swap;
	};

private:
	GeomPrimitive primitive;
	int bezier_shape_num_cp; // 1 dim
	Vector<Vector2> bezier_shape_cp;
	Size2i bezier_mesh_num_cp; // 2 dim
	Size2i bezier_mesh_num_seg;
	Vector<Vector3> bezier_mesh_cp; // row major
	GeomModifiersStatus modifiers;
	bool debug_axis, debug_vertices;

	void _update_preview();

protected:
	static void _bind_methods();

	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _set(const StringName &p_path, const Variant &p_value);
	bool _get(const StringName &p_path, Variant &r_ret) const;

public:
	void set_primitive(int p_geom);
	GeomPrimitive get_primitive() const;
	void set_debug_vertices(bool p_state);
	bool get_debug_vertices() const;
	void set_debug_axis(bool p_state);
	bool get_debug_axis() const;

	ProceduralMesh();
	~ProceduralMesh();
};
VARIANT_ENUM_CAST(ProceduralMesh::GeomPrimitive);
VARIANT_ENUM_CAST(ProceduralMesh::GeomModifiers);

#endif // GD_PROCEDURAL_MESH_H
