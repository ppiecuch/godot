/*************************************************************************/
/*  gdgeomgen.h                                                          */
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

#ifndef GD_GEOM_GEN_H
#define GD_GEOM_GEN_H

#include "scene/3d/mesh_instance.h"
#include "scene/3d/spatial.h"

class GdGeomGen : public Spatial {
	GDCLASS(GdGeomGen, Spatial);

public:
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
		GEOM_BOX_MESH,
		GEOM_ROUNDED_BOX_MESH,
		GEOM_SPHERE_MESH,
		GEOM_DISK_MESH,
		GEOM_CYLINDER_MESH,
		GEOM_CAPPED_CYLINDER_MESH,
		GEOM_CONE_MESH,
		GEOM_CAPPED_CONE_MESH,
		GEOM_TUBE_MESH,
		GEOM_CAPPED_TUBE_MESH,
		GEOM_CAPSULE_MESH,
		GEOM_SPHERICAL_CONE_MESH,
		GEOM_CONVEX_POLYGON_MESH,
		GEOM_DODECAHEDRON_MESH,
		GEOM_ICOSAHEDRON_MESH,
		GEOM_ICOSPHERE_MESH,
		GEOM_PLANE_MESH,
		GEOM_SPHERICAL_TRIANGLE_MESH,
		GEOM_SPRING_MESH,
		GEOM_TORUS_KNOT_MESH,
		GEOM_TORUS_MESH,
		GEOM_TRIANGLE_MESH,
		GEOM_BEZIER_MESH,
		GEOM_TEAPOT_MESH,
	};
	const int GEOM_LAST_PRIMITIVE = GEOM_TEAPOT_MESH;

private:
	GeomPrimitive primitive;
	int bezier_shape_num_cp; // 1 dim
	Vector<Vector2> bezier_shape_cp;
	Size2 bezier_mesh_num_cp; // 2 dim
	Vector<Vector3> bezier_mesh_cp;
	bool debug_axis, debug_vertices;

	MeshInstance *_mesh;
	void _update_preview();

protected:
	static void _bind_methods();

	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);

public:
	void set_primitive(int p_geom);
	GeomPrimitive get_primitive() const;
	void set_debug_vertices(bool p_state);
	bool get_debug_vertices() const;
	void set_debug_axis(bool p_state);
	bool get_debug_axis() const;

	GdGeomGen();
	~GdGeomGen();
};
VARIANT_ENUM_CAST(GdGeomGen::GeomPrimitive);

#endif // GD_GEOM_GEN_H
