/*************************************************************************/
/*  gdgeomgen.cpp                                                        */
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

#include "gdgeomgen.h"
#include "core/variant.h"

#include "generator/generator.hpp"
using namespace generator;

struct MeshWriter {
	void write_point(const gml::dvec2 &p, const Color &color = { 1, 1, 1 }) {
		points.pos.push_back(Vector3(p.data(), 0));
		points.color.push_back(color);
	}

	void write_point(const gml::dvec3 &p, const Color &color = { 1, 1, 1 }) {
		points.pos.push_back(Vector3(p.data()));
		points.color.push_back(color);
	}

	void write_line(const gml::dvec2 &p1, const gml::dvec2 &p2, const Color &color = { 1, 1, 1 }) {
		if (p1 == p2)
			return;
		lines.pos.push_back(Vector3(p1.data(), 0));
		lines.color.push_back(color);
		lines.pos.push_back(Vector3(p2.data(), 0));
		lines.color.push_back(color);
	}

	void write_line(const gml::dvec3 &p1, const gml::dvec3 &p2, const Color &color = { 1, 1, 1 }) {
		if (p1 == p2)
			return;
		lines.pos.push_back(Vector3(p1.data()));
		lines.color.push_back(color);
		lines.pos.push_back(Vector3(p2.data()));
		lines.color.push_back(color);
	}

	template <typename Path>
	void write_path(const Path &path, bool write_vertices = false, bool write_axis = false) {
		std::vector<PathVertex> vertices{};
		for (const auto &v : path.vertices()) {
			vertices.push_back(v);
		}
		if (write_axis) {
			for (const auto &v : path.vertices()) {
				write_line(v.position, v.position + 0.1 * v.tangent, { 0, 0, 1 });
				write_line(v.position, v.position + 0.1 * v.normal, { 1, 0, 0 });
				write_line(v.position, v.position + 0.1 * v.binormal(), { 0, 1, 0 });
			}
		}
		if (write_vertices) {
			for (const auto &v : path.vertices()) {
				write_point(v.position + 0.001 * v.normal);
			}
		}
		for (const auto &e : path.edges()) {
			write_line(vertices[e.vertices[0]].position, vertices[e.vertices[1]].position);
		}
	}

	template <typename Shape>
	void write_shape(const Shape &shape, bool write_vertices = false, bool write_axis = false) {
		std::vector<ShapeVertex> vertices{};
		for (const auto &vertex : shape.vertices()) {
			vertices.push_back(vertex);
		}
		for (auto e : shape.edges()) {
			write_line(vertices[e.vertices[0]].position, vertices[e.vertices[1]].position, { 0.5, 0.5, 0.5 });
		}
		if (write_axis) {
			for (auto v : vertices) {
				const auto p1 = v.position;
				const auto p2 = v.position + 0.1 * v.tangent;
				const auto p3 = v.position + 0.1 * v.normal();

				write_line(p1, p2, { 0, 1, 0 });
				write_line(p1, p3, { 1, 0, 0 });
			}
		}
		if (write_vertices) {
			for (auto v : shape.vertices()) {
				write_point(v.position);
			}
		}
	}

	template <typename Mesh>
	void write_mesh(const Mesh &mesh) {
		for (const MeshVertex &vertex : mesh.vertices()) {
			verts.pos.push_back(Vector3(vertex.position[0], vertex.position[1], vertex.position[2]));
			verts.norm.push_back(Vector3(vertex.normal[0], vertex.normal[1], vertex.normal[2]));
			verts.tex.push_back(Vector2(vertex.texCoord[0], vertex.texCoord[1]));
		}
		for (const Triangle &triangle : mesh.triangles()) {
			auto t = triangle.vertices;
			index.push_back(t[0]);
			index.push_back(t[1]);
			index.push_back(t[2]);
		}
	}

	struct {
		PoolVector3Array pos;
		PoolVector3Array norm;
		PoolVector2Array tex;
		PoolColorArray color;
	} verts, lines, points;
	PoolIntArray index;
};

static void generate_axis(MeshWriter &writer, Axis axis) {
	Color color{};
	color[static_cast<unsigned>(axis)] = 1.0;

	gml::dvec3 end{};
	end[static_cast<unsigned>(axis)] = 1.5;

	LinePath line{ gml::dvec3{}, end, gml::dvec3{}, 15 };
	auto xx = line.vertices();
	auto prev = xx.generate().position;
	xx.next();
	while (!xx.done()) {
		auto current = xx.generate().position;
		writer.write_line(prev, current, color);
		prev = current;
		xx.next();
	}
}

template <typename Shape>
static void generate_shape(MeshWriter &writer, const Shape &shape, bool write_vertices, bool write_axis) {
	generate_axis(writer, Axis::X);
	generate_axis(writer, Axis::Y);
	writer.write_shape(shape, write_vertices, write_axis);
}

template <typename Path>
static void generate_path(MeshWriter &writer, const Path &path, bool write_vertices, bool write_axis) {
	generate_axis(writer, Axis::X);
	generate_axis(writer, Axis::Y);
	generate_axis(writer, Axis::Z);
	writer.write_path(path, write_vertices, write_axis);
}

template <typename Mesh>
static void generate_mesh(MeshWriter &writer, const Mesh &mesh) {
}

void GdGeomGen::_update_preview() {
	MeshWriter writer;
	switch (primitive) {
		case GEOM_EMPTY_SHAPE:
			generate_shape(writer, CircleShape{}, debug_vertices, debug_axis);
			break;
		case GEOM_LINE_SHAPE:
			generate_shape(writer, LineShape{}, debug_vertices, debug_axis);
			break;
		case GEOM_RECTANGLE_SHAPE:
			generate_shape(writer, RectangleShape{}, debug_vertices, debug_axis);
			break;
		case GEOM_ROUNDED_RECTANGLE_SHAPE:
			generate_shape(writer, RoundedRectangleShape{}, debug_vertices, debug_axis);
			break;
		case GEOM_CIRCLE_SHAPE:
			generate_shape(writer, CircleShape{}, debug_vertices, debug_axis);
			break;
		case GEOM_GRID_SHAPE:
			generate_shape(writer, GridShape{}, debug_vertices, debug_axis);
			break;
		case GEOM_BEZIER_SHAPE:
			generate_shape(writer, BezierShape<4>{ { { -1.0, -1.0 }, { -0.5, 1.0 }, { 0.5, -1.0 }, { 1.0, 1.0 } } }, debug_vertices, debug_axis);
			break;

		case GEOM_EMPTY_PATH:
			generate_path(writer, EmptyPath{}, debug_vertices, debug_axis);
			break;
		case GEOM_LINE_PATH:
			generate_path(writer, LinePath{}, debug_vertices, debug_axis);
			break;
		case GEOM_KNOT_PATH:
			generate_path(writer, KnotPath{}, debug_vertices, debug_axis);
			break;
		case GEOM_HELIX_PATH:
			generate_path(writer, HelixPath{}, debug_vertices, debug_axis);
			break;

		case GEOM_EMPTY_MESH:
			generate_mesh(writer, EmptyMesh{});
			break;
		case GEOM_BOX_MESH:
			generate_mesh(writer, BoxMesh{});
			break;
		case GEOM_ROUNDED_BOX_MESH:
			generate_mesh(writer, RoundedBoxMesh{});
			break;
		case GEOM_CAPPED_CYLINDER_MESH:
			generate_mesh(writer, CappedCylinderMesh{});
			break;
		case GEOM_CAPPED_CONE_MESH:
			generate_mesh(writer, CappedConeMesh{});
			break;
		case GEOM_CAPPED_TUBE_MESH:
			generate_mesh(writer, CappedTubeMesh{});
			break;
		case GEOM_CONE_MESH:
			generate_mesh(writer, ConeMesh{});
			break;
		case GEOM_SPHERICAL_CONE_MESH:
			generate_mesh(writer, SphericalConeMesh{});
			break;
		case GEOM_CAPSULE_MESH:
			generate_mesh(writer, CapsuleMesh{});
			break;
		case GEOM_CONVEX_POLYGON_MESH:
			generate_mesh(writer, ConvexPolygonMesh{});
			break;
		case GEOM_CYLINDER_MESH:
			generate_mesh(writer, CylinderMesh{});
			break;
		case GEOM_DISK_MESH:
			generate_mesh(writer, DiskMesh{});
			break;
		case GEOM_DODECAHEDRON_MESH:
			generate_mesh(writer, DodecahedronMesh{});
			break;
		case GEOM_ICOSAHEDRON_MESH:
			generate_mesh(writer, IcosahedronMesh{});
			break;
		case GEOM_ICOSPHERE_MESH:
			generate_mesh(writer, IcoSphereMesh{});
			break;
		case GEOM_PLANE_MESH:
			generate_mesh(writer, PlaneMesh{});
			break;
		case GEOM_SPHERE_MESH:
			generate_mesh(writer, SphereMesh{});
			break;
		case GEOM_SPHERICAL_TRIANGLE_MESH:
			generate_mesh(writer, SphericalTriangleMesh{});
			break;
		case GEOM_SPRING_MESH:
			generate_mesh(writer, SpringMesh{});
			break;
		case GEOM_TORUS_KNOT_MESH:
			generate_mesh(writer, TorusKnotMesh{});
			break;
		case GEOM_TORUS_MESH:
			generate_mesh(writer, TorusMesh{});
			break;
		case GEOM_TRIANGLE_MESH:
			generate_mesh(writer, generator::TriangleMesh{});
			break;
		case GEOM_TUBE_MESH:
			generate_mesh(writer, TubeMesh{});
			break;
		case GEOM_BEZIER_MESH: {
			const gml::dvec3 cp[4][4] = {
				{ { -1.00, -1.00, 2.66 }, { -0.33, -1.00, 0.66 }, { 0.33, -1.00, -0.66 }, { 1.0, -1.00, 1.33 } },
				{ { -1.00, -0.33, 0.66 }, { -0.33, -0.33, 2.00 }, { 0.33, -0.33, 0.00 }, { 1.0, -0.33, -0.66 } },
				{ { -1.00, 0.33, 2.66 }, { -0.33, 0.33, 0.00 }, { 0.33, 0.33, 2.00 }, { 1.0, 0.33, 2.66 } },
				{ { -1.00, 1.00, -1.33 }, { -0.33, 1.00, -1.33 }, { 0.33, 1.00, 0.00 }, { 1.0, 1.00, -0.66 } }
			};
			generate_mesh(writer, BezierMesh<4, 4>{ cp, { 8, 8 } });
		} break;
		case GEOM_TEAPOT_MESH:
			generate_mesh(writer, TeapotMesh{});
			break;
	}
	// build mesh from data
	if (!_mesh) {
		_mesh = memnew(MeshInstance);
		add_child(_mesh);
	}
	if (writer.verts.pos.size() + writer.lines.pos.size() + writer.points.pos.size()) {
		print_verbose("Building new mesh:");
		print_verbose(" - triangles: " + String::num(writer.verts.pos.size()) + ", " + String::num(writer.verts.tex.size()) + ", " + String::num(writer.verts.norm.size()));
		print_verbose(" - lines: " + String::num(writer.lines.pos.size()) + ", " + String::num(writer.lines.tex.size()) + ", " + String::num(writer.lines.norm.size()));
		print_verbose(" - points: " + String::num(writer.points.pos.size()) + ", " + String::num(writer.points.tex.size()) + ", " + String::num(writer.points.norm.size()));
		Ref<ArrayMesh> mesh = memnew(ArrayMesh);
		if (writer.verts.pos.size()) {
			Array mesh_array;
			mesh_array.resize(VS::ARRAY_MAX);
			mesh_array[VS::ARRAY_VERTEX] = writer.verts.pos;
			if (writer.verts.tex.size())
				mesh_array[VS::ARRAY_TEX_UV] = writer.verts.tex;
			if (writer.verts.norm.size())
				mesh_array[VS::ARRAY_NORMAL] = writer.verts.norm;
			mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array());
		}
		if (writer.lines.pos.size()) {
			Array mesh_array;
			mesh_array.resize(VS::ARRAY_MAX);
			mesh_array[VS::ARRAY_VERTEX] = writer.lines.pos;
			if (writer.lines.tex.size())
				mesh_array[VS::ARRAY_TEX_UV] = writer.lines.tex;
			if (writer.lines.norm.size())
				mesh_array[VS::ARRAY_NORMAL] = writer.lines.norm;
			mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array, Array());
		}
		if (writer.points.pos.size()) {
			Array mesh_array;
			mesh_array.resize(VS::ARRAY_MAX);
			mesh_array[VS::ARRAY_VERTEX] = writer.points.pos;
			if (writer.points.tex.size())
				mesh_array[VS::ARRAY_TEX_UV] = writer.points.tex;
			if (writer.points.norm.size())
				mesh_array[VS::ARRAY_NORMAL] = writer.points.norm;
			mesh->add_surface_from_arrays(Mesh::PRIMITIVE_POINTS, mesh_array, Array());
		}
		_mesh->set_mesh(mesh);
	} else {
		_mesh->set_mesh(nullptr);
	}
}

void GdGeomGen::set_primitive(int p_geom) {
	ERR_FAIL_INDEX(p_geom, GEOM_LAST_PRIMITIVE);
	primitive = (GeomPrimitive)p_geom;
	_update_preview();
}

GdGeomGen::GeomPrimitive GdGeomGen::get_primitive() const {
	return primitive;
}

void GdGeomGen::set_debug_vertices(bool p_state) {
	debug_vertices = p_state;
	_update_preview();
}

bool GdGeomGen::get_debug_vertices() const {
	return debug_vertices;
}

void GdGeomGen::set_debug_axis(bool p_state) {
	debug_axis = p_state;
	_update_preview();
}

bool GdGeomGen::get_debug_axis() const {
	return debug_axis;
}

void GdGeomGen::_notification(int p_what) {
}

void GdGeomGen::_get_property_list(List<PropertyInfo> *p_list) const {
	if (p_list) {
		if (primitive == GEOM_BEZIER_SHAPE) {
			p_list->push_back(PropertyInfo(Variant::INT, "bezier_shape_num_cp", PROPERTY_HINT_RANGE, "1,4"));
			for (int i = 0; i < bezier_shape_num_cp; i++) {
				String prep = "control_point/" + itos(i) + "/";
			}
		} else if (primitive == GEOM_BEZIER_MESH) {
			p_list->push_back(PropertyInfo(Variant::INT, "bezier_mesh_num_cp/rows", PROPERTY_HINT_RANGE, "1,4"));
			p_list->push_back(PropertyInfo(Variant::INT, "bezier_mesh_num_cp/cols", PROPERTY_HINT_RANGE, "1,4"));
			for (int i = 0; i < bezier_mesh_num_cp.x; i++) {
				for (int j = 0; j < bezier_mesh_num_cp.y; j++) {
					String prep = "control_point/" + itos(i) + "x" + itos(j) + "/";
				}
			}
		}
	}
}

void GdGeomGen::_bind_methods() {
	BIND_ENUM_CONSTANT(GEOM_EMPTY_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_LINE_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_RECTANGLE_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_ROUNDED_RECTANGLE_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_CIRCLE_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_GRID_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_BEZIER_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_BEZIER_SHAPE);

	BIND_ENUM_CONSTANT(GEOM_EMPTY_PATH);
	BIND_ENUM_CONSTANT(GEOM_LINE_PATH);
	BIND_ENUM_CONSTANT(GEOM_KNOT_PATH);
	BIND_ENUM_CONSTANT(GEOM_HELIX_PATH);

	BIND_ENUM_CONSTANT(GEOM_EMPTY_MESH);
	BIND_ENUM_CONSTANT(GEOM_BOX_MESH);
	BIND_ENUM_CONSTANT(GEOM_ROUNDED_BOX_MESH);
	BIND_ENUM_CONSTANT(GEOM_SPHERE_MESH);
	BIND_ENUM_CONSTANT(GEOM_DISK_MESH);
	BIND_ENUM_CONSTANT(GEOM_CYLINDER_MESH);
	BIND_ENUM_CONSTANT(GEOM_CAPPED_CYLINDER_MESH);
	BIND_ENUM_CONSTANT(GEOM_CONE_MESH);
	BIND_ENUM_CONSTANT(GEOM_CAPPED_CONE_MESH);
	BIND_ENUM_CONSTANT(GEOM_TUBE_MESH);
	BIND_ENUM_CONSTANT(GEOM_CAPPED_TUBE_MESH);
	BIND_ENUM_CONSTANT(GEOM_CAPSULE_MESH);
	BIND_ENUM_CONSTANT(GEOM_SPHERICAL_CONE_MESH);
	BIND_ENUM_CONSTANT(GEOM_CONVEX_POLYGON_MESH);
	BIND_ENUM_CONSTANT(GEOM_DODECAHEDRON_MESH);
	BIND_ENUM_CONSTANT(GEOM_ICOSAHEDRON_MESH);
	BIND_ENUM_CONSTANT(GEOM_ICOSPHERE_MESH);
	BIND_ENUM_CONSTANT(GEOM_PLANE_MESH);
	BIND_ENUM_CONSTANT(GEOM_SPHERICAL_TRIANGLE_MESH);
	BIND_ENUM_CONSTANT(GEOM_SPRING_MESH);
	BIND_ENUM_CONSTANT(GEOM_TORUS_KNOT_MESH);
	BIND_ENUM_CONSTANT(GEOM_TORUS_MESH);
	BIND_ENUM_CONSTANT(GEOM_TRIANGLE_MESH);
	BIND_ENUM_CONSTANT(GEOM_BEZIER_MESH);
	BIND_ENUM_CONSTANT(GEOM_TEAPOT_MESH);

	ClassDB::bind_method(D_METHOD("set_primitive", "geom"), &GdGeomGen::set_primitive);
	ClassDB::bind_method(D_METHOD("get_primitive"), &GdGeomGen::get_primitive);
	ClassDB::bind_method(D_METHOD("set_debug_vertices", "geom"), &GdGeomGen::set_debug_vertices);
	ClassDB::bind_method(D_METHOD("get_debug_vertices"), &GdGeomGen::get_debug_vertices);
	ClassDB::bind_method(D_METHOD("set_debug_axis", "geom"), &GdGeomGen::set_debug_axis);
	ClassDB::bind_method(D_METHOD("get_debug_axis"), &GdGeomGen::get_debug_axis);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "primitive", PROPERTY_HINT_ENUM, "EmptyShape,LineShape,RectangleShape,RoundRectangleShape,CircleShape,GridShape,BezierShape,EmptyPath, LinePath,KnotPath,HelixPath"), "set_primitive", "get_primitive");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_vertices"), "set_debug_vertices", "get_debug_vertices");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_axis"), "set_debug_axis", "get_debug_axis");
}

GdGeomGen::GdGeomGen() {
	_mesh = nullptr;
	primitive = GEOM_EMPTY_SHAPE;
	bezier_shape_num_cp = 4;
	bezier_mesh_num_cp = Size2(4, 4);
	debug_axis = false;
	debug_vertices = false;
}

GdGeomGen::~GdGeomGen() {
}
