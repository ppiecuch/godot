/*************************************************************************/
/*  spherical_waves.cpp                                                  */
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

#include "spherical_waves.h"

#include "core/math/math_funcs.h"
#include "core/math/vector3.h"

#include <stdlib.h>

const real_t SphericalWaves::TwoSquareHalf = Math::sqrt(2.0) / 2;

void SphericalWaves::init(const Size2 &p_grid_size, real_t p_spring_constant, real_t p_friction) {
	grid_size = p_grid_size;
	spring_constant = p_spring_constant;
	friction = p_friction;
	int cells = grid_size.x * grid_size.y;
	next_amplitudes = new real_t[cells];
	current_amplitudes = new real_t[cells];
	velocities = new real_t[cells];
	for (int i = 0; i < cells; ++i) {
		current_amplitudes[i] = 0;
		next_amplitudes[i] = 0;
		velocities[i] = 0;
	}
}

SphericalWaves::~SphericalWaves() {
	if (next_amplitudes) {
		delete[] next_amplitudes;
	}
	if (current_amplitudes) {
		delete[] current_amplitudes;
	}
	if (velocities) {
		delete[] velocities;
	}
}

real_t SphericalWaves::get_amplitude(int p_pos_x, int p_pos_y) {
	return next_amplitudes[p_pos_x * grid_size.y + p_pos_y];
}

void SphericalWaves::set_amplitude(int p_pos_x, int p_pos_y, real_t p_value) {
	current_amplitudes[p_pos_x * grid_size.y + p_pos_y] = p_value;
}

void SphericalWaves::update(real_t p_delta) {
	for (int x = 1; x < grid_size.x - 1; ++x) {
		for (int y = 1; y < grid_size.y - 1; ++y) {
			real_t force = 0;
			for (int a = -1; a < 2; ++a) {
				for (int b = -1; b < 2; ++b) {
					real_t difference = current_amplitudes[(x + a) * grid_size.y + y + b] - current_amplitudes[x * grid_size.y + y];
					if (Math::abs(x) + Math::abs(y) == 2) {
						force += TwoSquareHalf * difference;
					} else {
						force += difference;
					}
				}
			}
			real_t velocity = velocities[x * grid_size.y + y];
			force = force * spring_constant - velocity * friction;
			velocity += p_delta * force;

			next_amplitudes[x * grid_size.y + y] = current_amplitudes[x * grid_size.y + y] + velocity * p_delta;
			velocities[x * grid_size.y + y] = velocity;
		}
	}
	for (int i = 0; i < grid_size.x * grid_size.y; ++i) {
		current_amplitudes[i] = next_amplitudes[i];
	}
}

void SphericalWaves::set_nodes(const Array &p_voxels, int p_index) {
	for (int x = 1; x < grid_size.x - 1; ++x) {
		for (int y = 1; y < grid_size.y - 1; ++y) {
			Spatial *node = cast_to<Spatial>(p_voxels[x * grid_size.y + y]);
			Vector3 translation = node->get_translation();
			translation[p_index] = current_amplitudes[x * grid_size.y + y];
			node->set_translation(translation);
		}
	}
}

void SphericalWaves::set_mesh(const Ref<Mesh> &p_mesh) {
	MeshDataTool *data_tool = memnew(MeshDataTool);
	data_tool->create_from_surface(p_mesh, 0);
	for (int i = 0; i < data_tool->get_vertex_count(); ++i) {
		Vector3 vertex = data_tool->get_vertex(i);
		vertex.y = current_amplitudes[int((vertex.x - grid_size.x / 2) * grid_size.y + vertex.z - grid_size.y / 2)];
		data_tool->set_vertex(i, vertex);
	}
	data_tool->commit_to_surface(p_mesh);
}

void SphericalWaves::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "grid_size", "spring_constant", "friction"), &SphericalWaves::init);
	ClassDB::bind_method(D_METHOD("_process", "delta"), &SphericalWaves::update);
	ClassDB::bind_method(D_METHOD("get_amplitude", "pos"), &SphericalWaves::get_amplitude);
	ClassDB::bind_method(D_METHOD("set_amplitude", "pos", "value"), &SphericalWaves::set_amplitude);
	ClassDB::bind_method(D_METHOD("set_nodes", "voxels", "index"), &SphericalWaves::set_nodes);
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &SphericalWaves::set_mesh);
}

SphericalWaves::SphericalWaves() {
	grid_size = Size2(2, 2);
	spring_constant = 5;
	friction = 0.7;
	next_amplitudes = nullptr;
	current_amplitudes = nullptr;
	velocities = nullptr;
}

#if 0 // Example

extends Spatial

export var sizeX = 2
export var sizeZ = 2

var stomplitude = 10
var springConstant = 5
var friction = 0.7

var boxes = []
var waves = SphericalWaves.new()
var mapscene = load("res://groundBox/groundBox.tscn")

var mesh = Mesh.new()
var meshinstance = MeshInstance.new()
var datatool = MeshDataTool.new()

var lastStompTime = 0
var lastStompCenterIndices = null

var stones = []

export(Material) var material = null

func initMesh():
	var material = FixedMaterial.new()
	material.set_parameter(material.PARAM_DIFFUSE, Color(1,0,0,1))
	var surfTool = SurfaceTool.new()
	surfTool.set_material(material)
	surfTool.begin(VS.PRIMITIVE_TRIANGLES)
	for z in range(sizeZ):
		for x in range(sizeX):
			var xCentered = x - sizeX / 2
			var zCentered = z - sizeZ / 2
			surfTool.add_normal(Vector3(0,1,0))
			surfTool.add_vertex(Vector3(xCentered + 0, 0, zCentered + 0))
			surfTool.add_normal(Vector3(0,1,0))
			surfTool.add_vertex(Vector3(xCentered + 1, 0, zCentered + 0))
			surfTool.add_normal(Vector3(0,1,0))
			surfTool.add_vertex(Vector3(xCentered + 0, 0, zCentered + 1))
			surfTool.add_normal(Vector3(0,1,0))
			surfTool.add_vertex(Vector3(xCentered + 1, 0, zCentered + 1))
			surfTool.add_normal(Vector3(0,1,0))
			surfTool.add_vertex(Vector3(xCentered + 0, 0, zCentered + 1))
			surfTool.add_normal(Vector3(0,1,0))
			surfTool.add_vertex(Vector3(xCentered + 1, 0, zCentered + 0))
	surfTool.commit(mesh)
	meshinstance.set_mesh(mesh)
	meshinstance.create_trimesh_collision()
	add_child(meshinstance)

func initNodes():
	for x in range(0, sizeX):
		for z in range(0, sizeZ):
			var pos = Vector3(x - sizeX/2.0, 0, z - sizeZ/2.0)
			var box = mapscene.instance()
			add_child(box)
			box.set_translation(pos)
			boxes.append(box)
	waves.set_nodes(boxes, 1)

func _ready():
	initNodes()
	waves.init(sizeX, sizeZ, springConstant, friction)
	self.set_process(true)

var frame = 0
func _process(deltaT):
	frame = frame + 1
	if frame %50 == 0:
		print(1/deltaT)
	waves.update(deltaT)

#I will build a wall to make games great again
#for x in range(0, sizeX / 2):
#waves.set_amplitude(x, 10, 0)
#waves.set_amplitude(x, 11, 0)
#waves.set_amplitude(x, 13, 0)

	for stonePos in stones:
		waves.set_amplitude(stonePos.x, stonePos.y, 0)

	if OS.get_unix_time()-lastStompTime < 5:
		for i in range(-1, 2):
			waves.set_amplitude(lastStompCenterIndices.x+i, lastStompCenterIndices.y, 0)
			waves.set_amplitude(lastStompCenterIndices.x, lastStompCenterIndices.y+i, 0)

	waves.set_nodes(boxes, 1)

func stomp(position, amplitudeFactor = 1):
	get_node("/root/Spatial/SamplePlayer").play("smash")
	var indexX = int(position.x)+sizeX/2
	var indexZ = int(position.z)+sizeZ/2

	for x in range(-4, 5):
		for z in range(-4, 5):
			if ((indexX + x < 1) or (indexZ + z < 1) or (indexX + x > sizeX-2) or (indexZ + z > sizeZ-2)):
				continue
			var r = sqrt(x*x+z*z)
			if abs(r-4) < 0.5:
				waves.set_amplitude(indexX+x, indexZ+z, stomplitude*amplitudeFactor)
	lastStompTime = OS.get_unix_time()
	lastStompCenterIndices = Vector2(indexX, indexZ)
	return Vector3(-sizeX/2+indexX, position.y, -sizeZ/2+indexZ)

func getHeightAt(position):
	var indexX = int(position.x)+sizeX/2
	var indexZ = int(position.z)+sizeZ/2

	return waves.get_amplitude(indexX, indexZ)

func putStone(position):
	var indexX = int(position.x)+sizeX/2
	var indexZ = int(position.z)+sizeZ/2

	stones.append(Vector2(indexX, indexZ))

#endif // Example
