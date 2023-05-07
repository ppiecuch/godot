#include "core/reference.h"
#include "core/vector.h"
#include "core/math/transform_2d.h"

#include "data.h"

class DDLS_Object : public Reference {
	unsigned id;

	Transform2D matrix;
	Vector<real_t> coordinates;
	DDLSConstraintShape constraint_shape;

	real_t pivot_x, pivot_y;

	real_t scale_x, scale_y;
	real_t rotation;
	real_t x, y;

	bool has_changed;

public:
	unsigned get_id() const { return id; }

	void update_values_from_matrix() { }

	void updatematrix_from_values() {
		matrix = Transform2D();
		matrix.translate(-pivot_x, -pivot_y);
		matrix.scale(Size2(scale_x, scale_y));
		matrix.rotate(rotation);
		matrix.translate(x, y);
	}

	real_t get_pivot_x() const { return pivot_x; }
	void set_pivot_x(real_t p_value) {
		pivot_x = p_value;
		has_changed = true;
	}

	real_t get_pivot_y() const { return pivot_y; }
	void set_pivot_y(real_t p_value) {
		pivot_y = p_value;
		has_changed = true;
	}

	real_t get_scale_x() const { return scale_x; }
	void set_scale_x(real_t p_value) {
		if (scale_x != p_value) {
			scale_x = p_value;
			has_changed = true;
		}
	}
	real_t get_scale_y() const { return scale_y; }
	void set_scale_y(real_t p_value) {
		if (scale_y != p_value) {
			scale_y = p_value;
			has_changed = true;
		}
	}

	real_t get_rotation() const { return rotation; }
	void set_rotation(real_t p_value) {
		if (rotation != p_value) {
			rotation = p_value;
			has_changed = true;
		}
	}

	real_t get_x() const { return x; }
	void set_x(real_t p_value) {
		if (x != p_value) {
			x = p_value;
			has_changed = true;
		}
	}
	real_t get_y() const { return y; }
	void set_y(real_t p_value) {
		if (y != p_value) {
			y = p_value;
			has_changed = true;
		}
	}

	Transform2D get_matrix() const { return matrix; }
	void set_matrix(const Transform2D &value) {
		matrix = value;
		has_changed = true;
	}

	Vector<real_t> get_coordinates() const { return coordinates; }
	void set_coordinates(Vector<real_t> p_coordinates) {
		coordinates = p_coordinates;
		has_changed = true;
	}

	DDLSConstraintShape get_constraint_shape() const;
	void set_constraint_shape(DDLSConstraintShape p_shape);

	bool is_changed() const { return has_changed; }
	void set_changed(bool p_state) { has_changed = p_state; }

	Vector<DDLSEdge> get_edges() const;

	DDLS_Object();
};
