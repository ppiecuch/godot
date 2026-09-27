#include "painter_brush.h"

PainterBrush::PainterBrush() {
	fill_type = FILL_SOLID;
	color = Color(0, 0, 0, 1);
	color2 = Color(1, 1, 1, 1);
	radius = 0.0;
}

void PainterBrush::set_fill_type(FillType p_type) {
	fill_type = p_type;
	emit_changed();
}

PainterBrush::FillType PainterBrush::get_fill_type() const {
	return fill_type;
}

void PainterBrush::set_color(const Color &p_color) {
	color = p_color;
	emit_changed();
}

Color PainterBrush::get_color() const {
	return color;
}

void PainterBrush::set_color2(const Color &p_color) {
	color2 = p_color;
	emit_changed();
}

Color PainterBrush::get_color2() const {
	return color2;
}

void PainterBrush::set_point1(const Vector2 &p_point) {
	point1 = p_point;
	emit_changed();
}

Vector2 PainterBrush::get_point1() const {
	return point1;
}

void PainterBrush::set_point2(const Vector2 &p_point) {
	point2 = p_point;
	emit_changed();
}

Vector2 PainterBrush::get_point2() const {
	return point2;
}

void PainterBrush::set_radius(real_t p_radius) {
	radius = MAX(0.0, p_radius);
	emit_changed();
}

real_t PainterBrush::get_radius() const {
	return radius;
}

void PainterBrush::set_linear_gradient(const Vector2 &p_start, const Vector2 &p_end, const Color &p_from, const Color &p_to) {
	fill_type = FILL_LINEAR_GRADIENT;
	point1 = p_start;
	point2 = p_end;
	color = p_from;
	color2 = p_to;
	emit_changed();
}

void PainterBrush::set_radial_gradient(const Vector2 &p_center, real_t p_radius, const Color &p_from, const Color &p_to) {
	fill_type = FILL_RADIAL_GRADIENT;
	point1 = p_center;
	radius = MAX(0.0, p_radius);
	color = p_from;
	color2 = p_to;
	emit_changed();
}

bool PainterBrush::is_gradient() const {
	return fill_type != FILL_SOLID;
}

bool PainterBrush::is_visible() const {
	return color.a > 0.0 || color2.a > 0.0;
}

Color PainterBrush::color_at(const Vector2 &p_local_point) const {
	switch (fill_type) {
		case FILL_LINEAR_GRADIENT: {
			Vector2 axis = point2 - point1;
			real_t len_sq = axis.length_squared();
			real_t t = len_sq > CMP_EPSILON ? (p_local_point - point1).dot(axis) / len_sq : 0.0;
			return color.linear_interpolate(color2, CLAMP(t, 0.0, 1.0));
		}
		case FILL_RADIAL_GRADIENT: {
			real_t t = radius > CMP_EPSILON ? (p_local_point - point1).length() / radius : 0.0;
			return color.linear_interpolate(color2, CLAMP(t, 0.0, 1.0));
		}
		case FILL_SOLID:
		default: {
			return color;
		}
	}
}

void PainterBrush::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_fill_type", "type"), &PainterBrush::set_fill_type);
	ClassDB::bind_method(D_METHOD("get_fill_type"), &PainterBrush::get_fill_type);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &PainterBrush::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &PainterBrush::get_color);

	ClassDB::bind_method(D_METHOD("set_color2", "color"), &PainterBrush::set_color2);
	ClassDB::bind_method(D_METHOD("get_color2"), &PainterBrush::get_color2);

	ClassDB::bind_method(D_METHOD("set_point1", "point"), &PainterBrush::set_point1);
	ClassDB::bind_method(D_METHOD("get_point1"), &PainterBrush::get_point1);

	ClassDB::bind_method(D_METHOD("set_point2", "point"), &PainterBrush::set_point2);
	ClassDB::bind_method(D_METHOD("get_point2"), &PainterBrush::get_point2);

	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &PainterBrush::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &PainterBrush::get_radius);

	ClassDB::bind_method(D_METHOD("set_linear_gradient", "start", "end", "from_color", "to_color"), &PainterBrush::set_linear_gradient);
	ClassDB::bind_method(D_METHOD("set_radial_gradient", "center", "radius", "from_color", "to_color"), &PainterBrush::set_radial_gradient);

	ClassDB::bind_method(D_METHOD("is_gradient"), &PainterBrush::is_gradient);
	ClassDB::bind_method(D_METHOD("is_visible"), &PainterBrush::is_visible);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "fill_type", PROPERTY_HINT_ENUM, "Solid,LinearGradient,RadialGradient"), "set_fill_type", "get_fill_type");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color2"), "set_color2", "get_color2");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "point1"), "set_point1", "get_point1");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "point2"), "set_point2", "get_point2");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "radius"), "set_radius", "get_radius");

	BIND_ENUM_CONSTANT(FILL_SOLID);
	BIND_ENUM_CONSTANT(FILL_LINEAR_GRADIENT);
	BIND_ENUM_CONSTANT(FILL_RADIAL_GRADIENT);
}
