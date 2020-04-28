// http://docs.godotengine.org/en/3.0/tutorials/plugins/gdnative/gdnative-cpp-example.html
// https://docs.godotengine.org/en/3.1/tutorials/plugins/gdnative/gdnative-cpp-example.html
// https://godotengine.org/qa/285/procedural-meshes-what-am-i-missing
// http://docs.godotengine.org/en/3.0/tutorials/3d/vertex_displacement_with_shaders.html
// https://godotengine.org/qa/40494/how-to-write-script-to-make-the-sprite-scatter-into-fragments

#include "elastic_node_2d.h"

using namespace godot;

void GDElasticMesh2D::_bind_methods() {
    ClassDB::bind_method("_process", &GDElasticMesh2D::_process);
}

GDElasticMesh2D::GDElasticMesh2D() {
}

GDElasticMesh2D::~GDElasticMesh2D() {
    // add your cleanup here
}

void GDElasticMesh2D::_init() {
    // initialize any variables here
    time_passed = 0.0;
}

void GDElasticMesh2D::_process(float delta) {
    time_passed += delta;

    Vector2 new_position = Vector2(10.0 + (10.0 * sin(time_passed * 2.0)), 10.0 + (10.0 * cos(time_passed * 1.5)));

    set_position(new_position);
}
