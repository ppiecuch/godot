/*************************************************************************/
/*  gd_procedural_mesh.cpp                                               */
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

#include "gd_procedural_mesh.h"
#include "core/variant.h"
#include "scene/main/node.h"

#include "generator/generator.hpp"
using namespace generator;

#include <vector>

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
		std::vector<PathVertex> vertices;
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
		std::vector<ShapeVertex> vertices;
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
			verts.index.push_back(t[0]);
			verts.index.push_back(t[1]);
			verts.index.push_back(t[2]);
		}
	}

	struct {
		PoolVector3Array pos;
		PoolVector3Array norm;
		PoolVector2Array tex;
		PoolColorArray color;
		PoolIntArray index;
	} verts, lines, points;
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

static generator::MeshVertex process_script(Node *mesh_node, generator::MeshVertex mesh_vertex) {
	if (mesh_node && mesh_node->get_script_instance()) {
		Array vertex;
		Variant v = mesh_node->get_script_instance()->call("_process_geom_modifier", vertex);
		if (v.get_type() == Variant::ARRAY) {
			// update mesh vertex
		}
	}
	return mesh_vertex;
}

// Passthrough modifiers
template <typename Mesh>
AxisSwapMesh<Mesh> axisSwapMeshIf(bool cond, Mesh mesh, const ProceduralMesh::GeomAxis axis[]) {
	if (cond)
		return AxisSwapMesh<Mesh>{ std::move(mesh), static_cast<generator::Axis>(axis[0]), static_cast<generator::Axis>(axis[1]), static_cast<generator::Axis>(axis[2]) };
	else
		return AxisSwapMesh<Mesh>{ std::move(mesh) };
}

template <typename Mesh>
FlipMesh<Mesh> flipMeshIf(bool cond, Mesh mesh) {
	if (cond)
		return FlipMesh<Mesh>{ std::move(mesh) };
	else
		return FlipMesh<Mesh>{ &mesh };
}

template <typename Mesh>
RotateMesh<Mesh> rotateMeshIf(bool cond, Mesh mesh, const Quat &rotation) {
	if (cond)
		return RotateMesh<Mesh>{ std::move(mesh), { rotation.w, { rotation.x, rotation.y, rotation.z } } };
	else
		return RotateMesh<Mesh>{ std::move(mesh) };
}

template <typename Mesh>
ScaleMesh<Mesh> scaleMeshIf(bool cond, Mesh mesh, const Vector3 &delta) {
	if (cond)
		return ScaleMesh<Mesh>{ std::move(mesh), { delta.coord } };
	else
		return ScaleMesh<Mesh>{ std::move(mesh) };
}

template <typename Mesh>
SpherifyMesh<Mesh> spherifyMeshIf(bool cond, Mesh mesh, const real_t params[]) {
	if (cond)
		return SpherifyMesh<Mesh>{ std::move(mesh), params[0] /* radius */, params[1] /* factor */ };
	else
		return SpherifyMesh<Mesh>{ std::move(mesh) };
}

template <typename Mesh>
TranslateMesh<Mesh> translateMeshIf(bool cond, Mesh mesh, const Vector3 &delta) {
	if (cond)
		return TranslateMesh<Mesh>{ std::move(mesh), { delta.coord } };
	else
		return TranslateMesh<Mesh>{ std::move(mesh) };
}

template <typename Mesh>
TransformMesh<Mesh> transformMeshIf(bool cond, Mesh mesh, Node *node) {
	if (cond)
		return TransformMesh<Mesh>{ std::move(mesh), [node](MeshVertex &value) {
									   return process_script(node, value);
								   } };
	else
		return TransformMesh<Mesh>{ std::move(mesh) };
}

template <typename Mesh>
UvFlipMesh<Mesh> uvFlipMeshIf(bool cond, Mesh mesh, const bool param[]) {
	if (cond)
		return UvFlipMesh<Mesh>{ std::move(mesh), param[0] /* u */, param[1] /* v */ };
	else
		return UvFlipMesh<Mesh>{ std::move(mesh) };
}

template <typename Mesh>
UvScaleMesh<Mesh> uvScaleMeshIf(bool cond, Mesh mesh, const Vector2 &delta) {
	if (cond)
		return UvScaleMesh<Mesh>{ std::move(mesh), { delta.coord } };
	else
		return UvScaleMesh<Mesh>{ std::move(mesh) };
}

template <typename Mesh>
UvSwapMesh<Mesh> uvSwapMeshIf(bool cond, Mesh mesh) {
	if (cond)
		return UvSwapMesh<Mesh>{ std::move(mesh) };
	else
		return UvSwapMesh<Mesh>{ &mesh };
}
// end.

template <typename Shape>
static void generate_shape(MeshWriter &writer, const Shape &shape, const ProceduralMesh::GeomModifiersStatus &modifiers, bool write_vertices, bool write_axis) {
	if (write_axis) {
		generate_axis(writer, Axis::X);
		generate_axis(writer, Axis::Y);
	}
	writer.write_shape(shape, write_vertices, write_axis);
}

template <typename Path>
static void generate_path(MeshWriter &writer, const Path &path, const ProceduralMesh::GeomModifiersStatus &modifiers, bool write_vertices, bool write_axis) {
	if (write_axis) {
		generate_axis(writer, Axis::X);
		generate_axis(writer, Axis::Y);
		generate_axis(writer, Axis::Z);
	}
	writer.write_path(path, write_vertices, write_axis);
}

template <typename Mesh>
static void generate_mesh(MeshWriter &writer, const Mesh &mesh, const ProceduralMesh::GeomModifiersStatus &modifiers, bool write_axis) {
	const auto xmesh =
			axisSwapMeshIf(modifiers.axis_swap,
					flipMeshIf(modifiers.flip,
							rotateMeshIf(modifiers.translate,
									scaleMeshIf(modifiers.rotate,
											spherifyMeshIf(modifiers.spherify,
													transformMeshIf(modifiers.transform,
															translateMeshIf(modifiers.scale,
																	uvFlipMeshIf(modifiers.uv_flip,
																			uvScaleMeshIf(modifiers.uv_scale,
																					uvSwapMeshIf(modifiers.uv_swap, mesh),
																					modifiers.uv_scale_param),
																			modifiers.uv_flip_param),
																	modifiers.translate_param),
															modifiers.transform_node),
													modifiers.spherify_param),
											modifiers.scale_param),
									Quat(modifiers.rotate_param))),
					modifiers.axis_swap_param);
	if (write_axis) {
		generate_axis(writer, Axis::X);
		generate_axis(writer, Axis::Y);
		generate_axis(writer, Axis::Z);
	}
	writer.write_mesh(xmesh);
}

void ProceduralMesh::_update_preview() {
	MeshWriter writer;
	switch (primitive) {
		case GEOM_EMPTY_SHAPE:
			generate_shape(writer, EmptyShape{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_LINE_SHAPE:
			generate_shape(writer, LineShape{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_RECTANGLE_SHAPE:
			generate_shape(writer, RectangleShape{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_ROUNDED_RECTANGLE_SHAPE:
			generate_shape(writer, RoundedRectangleShape{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_CIRCLE_SHAPE:
			generate_shape(writer, CircleShape{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_GRID_SHAPE:
			generate_shape(writer, GridShape{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_BEZIER_SHAPE:
			switch (bezier_shape_num_cp) {
				case 2:
					generate_shape(writer, BezierShape<2>{ { { bezier_shape_cp[0].coord }, { bezier_shape_cp[1].coord } } }, modifiers, debug_vertices, debug_axis);
					break;
				case 3:
					generate_shape(writer, BezierShape<3>{ { { bezier_shape_cp[0].coord }, { bezier_shape_cp[1].coord }, { bezier_shape_cp[2].coord } } }, modifiers, debug_vertices, debug_axis);
					break;
				case 4:
					generate_shape(writer, BezierShape<4>{ { { bezier_shape_cp[0].coord }, { bezier_shape_cp[1].coord }, { bezier_shape_cp[2].coord }, { bezier_shape_cp[3].coord } } }, modifiers, debug_vertices, debug_axis);
					break;
				default:
					WARN_PRINT("Invalid number of control points: " + String::num(bezier_shape_num_cp));
			}
			break;

		case GEOM_EMPTY_PATH:
			generate_path(writer, EmptyPath{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_LINE_PATH:
			generate_path(writer, LinePath{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_KNOT_PATH:
			generate_path(writer, KnotPath{}, modifiers, debug_vertices, debug_axis);
			break;
		case GEOM_HELIX_PATH:
			generate_path(writer, HelixPath{}, modifiers, debug_vertices, debug_axis);
			break;

		case GEOM_EMPTY_MESH:
			generate_mesh(writer, EmptyMesh{}, modifiers, debug_axis);
			break;
		case GEOM_BOX_MESH:
			generate_mesh(writer, BoxMesh{}, modifiers, debug_axis);
			break;
		case GEOM_ROUNDED_BOX_MESH:
			generate_mesh(writer, RoundedBoxMesh{}, modifiers, debug_axis);
			break;
		case GEOM_CAPPED_CYLINDER_MESH:
			generate_mesh(writer, CappedCylinderMesh{}, modifiers, debug_axis);
			break;
		case GEOM_CAPPED_CONE_MESH:
			generate_mesh(writer, CappedConeMesh{}, modifiers, debug_axis);
			break;
		case GEOM_CAPPED_TUBE_MESH:
			generate_mesh(writer, CappedTubeMesh{}, modifiers, debug_axis);
			break;
		case GEOM_CONE_MESH:
			generate_mesh(writer, ConeMesh{}, modifiers, debug_axis);
			break;
		case GEOM_SPHERICAL_CONE_MESH:
			generate_mesh(writer, SphericalConeMesh{}, modifiers, debug_axis);
			break;
		case GEOM_CAPSULE_MESH:
			generate_mesh(writer, CapsuleMesh{}, modifiers, debug_axis);
			break;
		case GEOM_CONVEX_POLYGON_MESH:
			generate_mesh(writer, ConvexPolygonMesh{}, modifiers, debug_axis);
			break;
		case GEOM_CYLINDER_MESH:
			generate_mesh(writer, CylinderMesh{}, modifiers, debug_axis);
			break;
		case GEOM_DISK_MESH:
			generate_mesh(writer, DiskMesh{}, modifiers, debug_axis);
			break;
		case GEOM_DODECAHEDRON_MESH:
			generate_mesh(writer, DodecahedronMesh{}, modifiers, debug_axis);
			break;
		case GEOM_ICOSAHEDRON_MESH:
			generate_mesh(writer, IcosahedronMesh{}, modifiers, debug_axis);
			break;
		case GEOM_ICOSPHERE_MESH:
			generate_mesh(writer, IcoSphereMesh{}, modifiers, debug_axis);
			break;
		case GEOM_PLANE_MESH:
			generate_mesh(writer, PlaneMesh{}, modifiers, debug_axis);
			break;
		case GEOM_SPHERE_MESH:
			generate_mesh(writer, SphereMesh{}, modifiers, debug_axis);
			break;
		case GEOM_SPHERICAL_TRIANGLE_MESH:
			generate_mesh(writer, SphericalTriangleMesh{}, modifiers, debug_axis);
			break;
		case GEOM_SPRING_MESH:
			generate_mesh(writer, SpringMesh{}, modifiers, debug_axis);
			break;
		case GEOM_TORUS_KNOT_MESH:
			generate_mesh(writer, TorusKnotMesh{}, modifiers, debug_axis);
			break;
		case GEOM_TORUS_MESH:
			generate_mesh(writer, TorusMesh{}, modifiers, debug_axis);
			break;
		case GEOM_TRIANGLE_MESH:
			generate_mesh(writer, generator::TriangleMesh{}, modifiers, debug_axis);
			break;
		case GEOM_TUBE_MESH:
			generate_mesh(writer, TubeMesh{}, modifiers, debug_axis);
			break;
		case GEOM_BEZIER_MESH: {
			const gml::dvec3 cp[4][4] = {
				{ { -1.00, -1.00, 2.66 }, { -0.33, -1.00, 0.66 }, { 0.33, -1.00, -0.66 }, { 1.0, -1.00, 1.33 } },
				{ { -1.00, -0.33, 0.66 }, { -0.33, -0.33, 2.00 }, { 0.33, -0.33, 0.00 }, { 1.0, -0.33, -0.66 } },
				{ { -1.00, 0.33, 2.66 }, { -0.33, 0.33, 0.00 }, { 0.33, 0.33, 2.00 }, { 1.0, 0.33, 2.66 } },
				{ { -1.00, 1.00, -1.33 }, { -0.33, 1.00, -1.33 }, { 0.33, 1.00, 0.00 }, { 1.0, 1.00, -0.66 } }
			};
			if (bezier_mesh_num_cp == Size2i{ 2, 2 }) {
			} else if (bezier_mesh_num_cp == Size2i{ 3, 3 }) {
			}
			generate_mesh(writer, BezierMesh<4, 4>{ cp, { bezier_mesh_num_seg.coord } }, modifiers, debug_axis);
		} break;
		case GEOM_TEAPOT_MESH:
			generate_mesh(writer, TeapotMesh{}, modifiers, debug_axis);
			break;
		default:
			WARN_PRINT("Unknown geometry primitive: " + String::num(primitive));
	}
	// build mesh from data
	clear_surfaces();
	if (writer.verts.pos.size() + writer.lines.pos.size() + writer.points.pos.size()) {
		print_verbose("Building new mesh:");
		print_verbose(" - triangles: " + String::num(writer.verts.pos.size()) + ", t:" + String::num(writer.verts.tex.size()) + ", n:" + String::num(writer.verts.norm.size()) + ", i:" + String::num(writer.verts.index.size()));
		print_verbose(" - lines: " + String::num(writer.lines.pos.size()) + ", t:" + String::num(writer.lines.tex.size()) + ", n:" + String::num(writer.lines.norm.size()) + ", i:" + String::num(writer.lines.index.size()));
		print_verbose(" - points: " + String::num(writer.points.pos.size()) + ", t:" + String::num(writer.points.tex.size()) + ", n:" + String::num(writer.points.norm.size()) + ", i:" + String::num(writer.points.index.size()));
		if (writer.verts.pos.size()) {
			Array mesh_array;
			mesh_array.resize(VS::ARRAY_MAX);
			mesh_array[VS::ARRAY_VERTEX] = writer.verts.pos;
			if (writer.verts.tex.size())
				mesh_array[VS::ARRAY_TEX_UV] = writer.verts.tex;
			if (writer.verts.norm.size())
				mesh_array[VS::ARRAY_NORMAL] = writer.verts.norm;
			if (writer.verts.color.size())
				mesh_array[VS::ARRAY_COLOR] = writer.verts.color;
			if (writer.verts.index.size())
				mesh_array[VS::ARRAY_INDEX] = writer.verts.index;
			add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array());
		}
		if (writer.lines.pos.size()) {
			Array mesh_array;
			mesh_array.resize(VS::ARRAY_MAX);
			mesh_array[VS::ARRAY_VERTEX] = writer.lines.pos;
			if (writer.lines.tex.size())
				mesh_array[VS::ARRAY_TEX_UV] = writer.lines.tex;
			if (writer.lines.norm.size())
				mesh_array[VS::ARRAY_NORMAL] = writer.lines.norm;
			if (writer.lines.color.size())
				mesh_array[VS::ARRAY_COLOR] = writer.lines.color;
			if (writer.lines.index.size())
				mesh_array[VS::ARRAY_INDEX] = writer.lines.index;
			add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array, Array());
		}
		if (writer.points.pos.size()) {
			Array mesh_array;
			mesh_array.resize(VS::ARRAY_MAX);
			mesh_array[VS::ARRAY_VERTEX] = writer.points.pos;
			if (writer.points.tex.size())
				mesh_array[VS::ARRAY_TEX_UV] = writer.points.tex;
			if (writer.points.norm.size())
				mesh_array[VS::ARRAY_NORMAL] = writer.points.norm;
			if (writer.points.color.size())
				mesh_array[VS::ARRAY_COLOR] = writer.points.color;
			if (writer.points.index.size())
				mesh_array[VS::ARRAY_INDEX] = writer.points.index;
			add_surface_from_arrays(Mesh::PRIMITIVE_POINTS, mesh_array, Array());
		}
	}
}

void ProceduralMesh::set_primitive(int p_geom) {
	ERR_FAIL_INDEX(p_geom, GEOM_PRIMITIVES_COUNT);
	primitive = (GeomPrimitive)p_geom;
	_update_preview();
}

ProceduralMesh::GeomPrimitive ProceduralMesh::get_primitive() const {
	return primitive;
}

void ProceduralMesh::set_debug_vertices(bool p_state) {
	debug_vertices = p_state;
	_update_preview();
}

bool ProceduralMesh::get_debug_vertices() const {
	return debug_vertices;
}

void ProceduralMesh::set_debug_axis(bool p_state) {
	debug_axis = p_state;
	_update_preview();
}

bool ProceduralMesh::get_debug_axis() const {
	return debug_axis;
}

void ProceduralMesh::_get_property_list(List<PropertyInfo> *p_list) const {
	if (p_list) {
		if (primitive == GEOM_BEZIER_SHAPE) {
			p_list->push_back(PropertyInfo(Variant::INT, "bezier_shape_num_cp", PROPERTY_HINT_RANGE, "2,4"));
			for (int i = 0; i < bezier_shape_num_cp; i++) {
				String prep = "control_points/" + itos(i) + "/";
				p_list->push_back(PropertyInfo(Variant::VECTOR2, prep));
			}
		} else if (primitive == GEOM_BEZIER_MESH) {
			p_list->push_back(PropertyInfo(Variant::INT, "bezier_mesh_num_cp/rows", PROPERTY_HINT_RANGE, "2,4"));
			p_list->push_back(PropertyInfo(Variant::INT, "bezier_mesh_num_cp/cols", PROPERTY_HINT_RANGE, "2,4"));
			for (int i = 0; i < bezier_mesh_num_cp.x; i++) {
				for (int j = 0; j < bezier_mesh_num_cp.y; j++) {
					String prep = "control_points/" + itos(i) + "x" + itos(j) + "/";
					p_list->push_back(PropertyInfo(Variant::VECTOR3, prep));
				}
			}
			p_list->push_back(PropertyInfo(Variant::INT, "bezier_mesh_num_seg/x", PROPERTY_HINT_RANGE, "2,16"));
			p_list->push_back(PropertyInfo(Variant::INT, "bezier_mesh_num_seg/y", PROPERTY_HINT_RANGE, "2,16"));
		}

		if (primitive >= GEOM_EMPTY_SHAPE && primitive <= GEOM_BEZIER_SHAPE) {
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/axis_swap"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/flip"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/rotate/enable"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "modifiers/rotate/rotation"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/scale/enable"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "modifiers/scale/scaling"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/transform"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/translate"));
		} else if (primitive >= GEOM_EMPTY_PATH && primitive <= GEOM_HELIX_PATH) {
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/axis_swap"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/flip"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/rotate/enable"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "modifiers/rotate/rotation"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/scale/enable"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "modifiers/scale/scaling"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/transform"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/translate"));
		} else if (primitive >= GEOM_EMPTY_MESH && primitive <= GEOM_TEAPOT_MESH) {
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/axis_swap"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/flip"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/rotate/enable"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "modifiers/rotate/rotation"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/scale/enable"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "modifiers/scale/scaling"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/spherify"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/translate"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/uv_flip/enable"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/uv_flip/u"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/uv_flip/v"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/uv_scale/enable"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "modifiers/uv_scale/scaling"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "modifiers/uv_swap"));
		}
	}
}

bool ProceduralMesh::_set(const StringName &p_path, const Variant &p_value) {
	String path = p_path;

	if (path.begins_with("control_points/")) {
		String cp = path.substr(15);
	} else if (path.begins_with("modifiers/")) {
		String modif = path.substr(10);
		if (modif == "axis_swap") {
			modifiers.axis_swap = p_value;
			_update_preview();
		} else if (modif == "flip") {
			modifiers.flip = p_value;
			_update_preview();
		} else if (modif == "rotate/enable") {
			modifiers.rotate = p_value;
			_update_preview();
		} else if (modif == "rotate/rotation") {
			modifiers.rotate_param = p_value;
			_update_preview();
		} else if (modif == "scale/enable") {
			modifiers.scale = p_value;
			_update_preview();
		} else if (modif == "scale/scaling") {
			modifiers.scale_param = p_value;
			_update_preview();
		} else if (modif == "spherify") {
			modifiers.spherify = p_value;
			_update_preview();
		} else if (modif == "transform") {
			modifiers.transform = p_value;
			_update_preview();
		} else if (modif == "translate") {
			modifiers.translate = p_value;
			_update_preview();
		} else if (modif == "uv_flip/enable") {
			modifiers.uv_flip = p_value;
			_update_preview();
		} else if (modif == "uv_flip/u") {
			modifiers.uv_flip_param[0] = p_value;
			_update_preview();
		} else if (modif == "uv_flip/v") {
			modifiers.uv_flip_param[1] = p_value;
			_update_preview();
		} else if (modif == "uv_scale/enable") {
			modifiers.uv_scale = p_value;
			_update_preview();
		} else if (modif == "uv_scale/scaling") {
			modifiers.uv_scale_param = p_value;
			_update_preview();
		} else if (modif == "uv_swap") {
			modifiers.uv_swap = p_value;
			_update_preview();
		} else {
			WARN_PRINT("Unknown modifier: " + modif);
		}
	} else if (path == "bezier_shape_num_cp") {
		const int value = p_value;
		ERR_FAIL_COND_V(value < 2 || value > 4, true);
		bezier_shape_num_cp = value;
		if (bezier_shape_cp.size() < bezier_shape_num_cp) {
			bezier_shape_cp.resize(bezier_shape_num_cp);
		}
	} else if (path == "bezier_shape_cp") {
		ERR_FAIL_COND_V(p_value.get_type() != Variant::ARRAY, true);
		Array cp = p_value;
		ERR_FAIL_COND_V(cp.size() < 2 || cp.size() > 4, true);
		bezier_shape_num_cp = cp.size();
		if (bezier_shape_cp.size() < bezier_shape_num_cp) {
			bezier_shape_cp.resize(bezier_shape_num_cp);
		}
		for (int i = 0; i < cp.size(); i++) {
			bezier_shape_cp.write[i] = cp[i];
		}
	} else if (path == "bezier_mesh_num_seg/x") {
		const int value = p_value;
		ERR_FAIL_COND_V(value < 2 || value > 16, true);
		bezier_mesh_num_seg.x = value;
	} else if (path == "bezier_mesh_num_seg/y") {
		const int value = p_value;
		ERR_FAIL_COND_V(value < 2 || value > 16, true);
		bezier_mesh_num_seg.y = value;
	} else if (path == "bezier_mesh_num_cp/rows") {
		const int value = p_value;
		ERR_FAIL_COND_V(value < 2 || value > 4, true);
		bezier_mesh_num_cp.height = value;
		if (bezier_mesh_cp.size() < bezier_mesh_num_cp.width * bezier_mesh_num_cp.height) {
			bezier_mesh_cp.resize(bezier_mesh_num_cp.width * bezier_mesh_num_cp.height);
		}
	} else if (path == "bezier_mesh_num_cp/cols") {
		const int value = p_value;
		ERR_FAIL_COND_V(value < 2 || value > 4, true);
		bezier_mesh_num_cp.width = value;
		if (bezier_mesh_cp.size() < bezier_mesh_num_cp.width * bezier_mesh_num_cp.height) {
			bezier_mesh_cp.resize(bezier_mesh_num_cp.width * bezier_mesh_num_cp.height);
		}
	} else if (path == "bezier_mesh_cp") {
		ERR_FAIL_COND_V(p_value.get_type() != Variant::ARRAY, true);
		const Array value = p_value;
		for (int y = 0; y < bezier_mesh_num_cp.height; y++) {
			const Array row = value[y];
			for (int x = 0; x < bezier_mesh_num_cp.width; x++) {
				bezier_mesh_cp.write[y * bezier_mesh_num_cp.width + x] = row[x];
			}
		}
	} else {
		return false;
	}

	return true;
}

bool ProceduralMesh::_get(const StringName &p_path, Variant &r_ret) const {
	String path = p_path;

	if (path.begins_with("control_points/")) {
		String cp = path.substr(15);
	} else if (path.begins_with("modifiers/")) {
		String modif = path.substr(10);
		if (modif == "axis_swap") {
			r_ret = modifiers.axis_swap;
		} else if (modif == "flip") {
			r_ret = modifiers.flip;
		} else if (modif == "rotate/enable") {
			r_ret = modifiers.rotate;
		} else if (modif == "rotate/rotation") {
			r_ret = modifiers.rotate_param;
		} else if (modif == "scale/enable") {
			r_ret = modifiers.scale;
		} else if (modif == "scale/scaling") {
			r_ret = modifiers.scale_param;
		} else if (modif == "spherify") {
			r_ret = modifiers.spherify;
		} else if (modif == "transform") {
			r_ret = modifiers.transform;
		} else if (modif == "translate") {
			r_ret = modifiers.translate;
		} else if (modif == "uv_flip/enable") {
			r_ret = modifiers.uv_flip;
		} else if (modif == "uv_flip/u") {
			r_ret = modifiers.uv_flip_param[0];
		} else if (modif == "uv_flip/v") {
			r_ret = modifiers.uv_flip_param[1];
		} else if (modif == "uv_scale/enable") {
			r_ret = modifiers.uv_scale;
		} else if (modif == "uv_scale/scaling") {
			r_ret = modifiers.uv_scale_param;
		} else if (modif == "uv_swap") {
			r_ret = modifiers.uv_swap;
		} else {
			WARN_PRINT("Unknown modifier: " + modif);
		}
	} else if (path == "bezier_shape_num_cp") {
		r_ret = bezier_shape_num_cp;
	} else if (path == "bezier_shape_cp") {
		r_ret = bezier_shape_cp;
	} else if (path == "bezier_mesh_num_seg/x") {
		r_ret = bezier_mesh_num_seg.x;
	} else if (path == "bezier_mesh_num_seg/y") {
		r_ret = bezier_mesh_num_seg.y;
	} else if (path == "bezier_mesh_num_cp/rows") {
		r_ret = bezier_mesh_num_cp.x;
	} else if (path == "bezier_mesh_num_cp/cols") {
		r_ret = bezier_mesh_num_cp.y;
	} else if (path == "bezier_mesh_num_cp") {
		r_ret = Size2(bezier_mesh_num_cp.x, bezier_mesh_num_cp.y);
	} else if (path == "bezier_mesh_cp") {
		Array cp;
		for (int y = 0; y < bezier_mesh_num_cp.height; y++) {
			for (int x = 0; x < bezier_mesh_num_cp.width; x++) {
				Array row;
				row.append(bezier_mesh_cp[y * bezier_mesh_num_cp.width + x]);
				cp.append(row);
			}
		}
		r_ret = cp;
	} else {
		return false;
	}

	return true;
}

void ProceduralMesh::_bind_methods() {
	BIND_ENUM_CONSTANT(GEOM_EMPTY_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_LINE_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_RECTANGLE_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_ROUNDED_RECTANGLE_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_CIRCLE_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_GRID_SHAPE);
	BIND_ENUM_CONSTANT(GEOM_BEZIER_SHAPE);

	BIND_ENUM_CONSTANT(GEOM_EMPTY_PATH);
	BIND_ENUM_CONSTANT(GEOM_LINE_PATH);
	BIND_ENUM_CONSTANT(GEOM_KNOT_PATH);
	BIND_ENUM_CONSTANT(GEOM_HELIX_PATH);

	BIND_ENUM_CONSTANT(GEOM_EMPTY_MESH);
	BIND_ENUM_CONSTANT(GEOM_PLANE_MESH);
	BIND_ENUM_CONSTANT(GEOM_BOX_MESH);
	BIND_ENUM_CONSTANT(GEOM_ROUNDED_BOX_MESH);
	BIND_ENUM_CONSTANT(GEOM_SPHERE_MESH);
	BIND_ENUM_CONSTANT(GEOM_DISK_MESH);
	BIND_ENUM_CONSTANT(GEOM_CYLINDER_MESH);
	BIND_ENUM_CONSTANT(GEOM_CAPPED_CYLINDER_MESH);
	BIND_ENUM_CONSTANT(GEOM_CONE_MESH);
	BIND_ENUM_CONSTANT(GEOM_CAPPED_CONE_MESH);
	BIND_ENUM_CONSTANT(GEOM_SPHERICAL_CONE_MESH);
	BIND_ENUM_CONSTANT(GEOM_TUBE_MESH);
	BIND_ENUM_CONSTANT(GEOM_CAPPED_TUBE_MESH);
	BIND_ENUM_CONSTANT(GEOM_CAPSULE_MESH);
	BIND_ENUM_CONSTANT(GEOM_CONVEX_POLYGON_MESH);
	BIND_ENUM_CONSTANT(GEOM_DODECAHEDRON_MESH);
	BIND_ENUM_CONSTANT(GEOM_ICOSAHEDRON_MESH);
	BIND_ENUM_CONSTANT(GEOM_ICOSPHERE_MESH);
	BIND_ENUM_CONSTANT(GEOM_TRIANGLE_MESH);
	BIND_ENUM_CONSTANT(GEOM_SPHERICAL_TRIANGLE_MESH);
	BIND_ENUM_CONSTANT(GEOM_SPRING_MESH);
	BIND_ENUM_CONSTANT(GEOM_TORUS_KNOT_MESH);
	BIND_ENUM_CONSTANT(GEOM_TORUS_MESH);
	BIND_ENUM_CONSTANT(GEOM_BEZIER_MESH);
	BIND_ENUM_CONSTANT(GEOM_TEAPOT_MESH);

	ClassDB::bind_method(D_METHOD("set_primitive", "geom"), &ProceduralMesh::set_primitive);
	ClassDB::bind_method(D_METHOD("get_primitive"), &ProceduralMesh::get_primitive);
	ClassDB::bind_method(D_METHOD("set_debug_vertices", "geom"), &ProceduralMesh::set_debug_vertices);
	ClassDB::bind_method(D_METHOD("get_debug_vertices"), &ProceduralMesh::get_debug_vertices);
	ClassDB::bind_method(D_METHOD("set_debug_axis", "geom"), &ProceduralMesh::set_debug_axis);
	ClassDB::bind_method(D_METHOD("get_debug_axis"), &ProceduralMesh::get_debug_axis);

	ADD_GROUP("Generator", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "primitive", PROPERTY_HINT_ENUM, "EmptyShape,LineShape,RectangleShape,RoundRectangleShape,CircleShape,GridShape,BezierShape,EmptyPath, LinePath,KnotPath,HelixPath,EmptyMesh,PlaneMesh,BoxMesh,RoundedBoxMesh,SphereMesh,DiskMesh,CylinderMesh,CappedCylinderMesh,ConeMesh,CappedConeMesh,SphericalConeMesh,TubeMesh,CappedTubeMesh,CapsuleMesh,ConvexPolygonMesh,DodecahedronMesh,IcosahedronMesh,IcosphereMesh,TriangleMesh,SphericalTriangleMesh,SpringMesh,TorusKnotMesh,TorusMesh,BezierMesh,TeapotMesh"), "set_primitive", "get_primitive");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_vertices"), "set_debug_vertices", "get_debug_vertices");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_axis"), "set_debug_axis", "get_debug_axis");
}

ProceduralMesh::ProceduralMesh() {
	primitive = GEOM_EMPTY_SHAPE;
	bezier_shape_num_cp = 4;
	bezier_shape_cp.push_back({ -1.0, -1.0 }, { -0.5, 1.0 }, { 0.5, -1.0 }, { 1.0, 1.0 });
	bezier_mesh_num_cp = Size2i(4, 4);
	bezier_mesh_num_seg = Size2i(8, 8);
	bezier_mesh_cp.push_back({ -1.00, -1.00, 2.66 }, { -0.33, -1.00, 0.66 }, { 0.33, -1.00, -0.66 }, { 1.0, -1.00, 1.33 });
	bezier_mesh_cp.push_back({ -1.00, -0.33, 0.66 }, { -0.33, -0.33, 2.00 }, { 0.33, -0.33, 0.00 }, { 1.0, -0.33, -0.66 });
	bezier_mesh_cp.push_back({ -1.00, 0.33, 2.66 }, { -0.33, 0.33, 0.00 }, { 0.33, 0.33, 2.00 }, { 1.0, 0.33, 2.66 });
	bezier_mesh_cp.push_back({ -1.00, 1.00, -1.33 }, { -0.33, 1.00, -1.33 }, { 0.33, 1.00, 0.00 }, { 1.0, 1.00, -0.66 });
	modifiers.axis_swap = modifiers.flip = modifiers.rotate = modifiers.scale = modifiers.spherify = modifiers.transform = modifiers.translate = modifiers.uv_scale = modifiers.uv_swap = false;
	debug_axis = debug_vertices = false;
}

ProceduralMesh::~ProceduralMesh() {
}
