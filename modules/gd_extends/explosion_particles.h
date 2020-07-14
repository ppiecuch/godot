#ifndef GODOT_EXPLOSION_PARTICLES_2D_H
#define GODOT_EXPLOSION_PARTICLES_2D_H

#include "core/typedefs.h"
#include "core/object.h"
#include "core/math/vector2.h"
#include "scene/main/timer.h"
#include "scene/2d/mesh_instance_2d.h"


class ExplosionParticles2D : public Node2D {
    GDCLASS(ExplosionParticles2D, Node2D);
private:

    int min_particles_number;
    int max_particles_number;

    float min_particles_gravity;
    float max_particles_gravity;

    float min_particles_velocity;
    float max_particles_velocity;

    int max_particles_position_x;
    int max_particles_position_y;

    int min_particles_size;
    int max_particles_size;

    bool get_random_position;
    bool start_timer;
    float timer_wait_time;
    bool particles_explode;

    String group_name;

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    ExplosionParticles2D();
    ~ExplosionParticles2D();

private:
    void _create_particles();

    Array particles;
    int particles_number;
    Vector2 particles_initial_position;

    struct {
        int w;
        Color c;
    } particles_colors_with_weights[5] = {
        {4, Color::html("#ffffff")},
        {2, Color::html("#000000")},
        {8, Color::html("#ff004d")},
        {8, Color::html("#ffa300")},
        {10, Color::html("#ffec27")}
    };

    Timer *particles_timer = 0;
};

#endif // GODOT_EXPLOSION_PARTICLES_2D_H
