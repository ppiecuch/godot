#include "core/math/vector2.h"
#include "core/reference.h"

#include "ddls_fwd.h"

class DDLS_EntityAI : public Reference {
	const int NUM_SEGMENTS = 6;

	real_t radius;
	real_t radius_squared;
	Point2 pos;
	Vector2 dir_norm;
	real_t angle_fov;
	real_t radius_fov;
	real_t radius_squared_fov;
	DDLSObject approximate_object;

public:
	void build_approximation();
	DDLSObject get_approximate_object();

	real_t get_radius_fov() const { return radius_fov; }
	void set_radius_fov(real_t pradius_fov) {
		radius_fov = pradius_fov;
		radius_squared_fov = radius_fov * radius_fov;
	}

	real_t get_angle_fov() { return angle_fov; }
	void set_angle_fov(real_t p_angle_fov) { angle_fov = p_angle_fov; }

	Point2 get_dir_norm() const { return dir_norm; }
	void set_dir_norm(const Vector2 p_dir) { dir_norm = p_dir; }

	Point2 get_pos() const { return pos; }
	void set_pos(const Point2 &p_pos) { pos = p_pos; }

	real_t get_radius() const { return radius; }
	void set_radius(real_t p_radius) {
		radius = p_radius;
		radius_squared = radius * radius;
	}
	real_t get_radius_squared() const { return radius_squared; }

	DDLS_EntityAI();
};
