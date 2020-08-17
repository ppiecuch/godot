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


}

#endif // GDELASTICNODE2D_H
