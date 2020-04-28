#ifndef GDELASTICNODE2D_H
#define GDELASTICNODE2D_H

#include "scene/2d/sprite.h"
#include "scene/2d/mesh_instance_2d.h"

namespace godot {

class GDElasticMesh2D : public MeshInstance2D {
    GDCLASS(GDElasticMesh2D, MeshInstance2D)

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

    GDElasticMesh2D();
    ~GDElasticMesh2D();

    void _init(); // our initializer called by Godot

    void _process(float delta);
};

class GDElasticSprite : public Sprite {
    GDCLASS(GDElasticSprite, Sprite)

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

    GDElasticSprite();
    ~GDElasticSprite();

    void _init(); // our initializer called by Godot

    void _process(float delta);
};


}

#endif // GDELASTICNODE2D_H
