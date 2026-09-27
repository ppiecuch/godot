#include "painter_pen.h"

PainterPen::PainterPen() {
	color = Color(0, 0, 0, 1);
	width = 1.0;
	outline_color = Color(0, 0, 0, 0);
	outline_width = 0.0;
	cap = CAP_BUTT;
	join = JOIN_ROUND;
}

void PainterPen::set_color(const Color &p_color) {
	color = p_color;
	emit_changed();
}

Color PainterPen::get_color() const {
	return color;
}

void PainterPen::set_width(real_t p_width) {
	width = MAX(0.0, p_width);
	emit_changed();
}

real_t PainterPen::get_width() const {
	return width;
}

void PainterPen::set_outline_color(const Color &p_color) {
	outline_color = p_color;
	emit_changed();
}

Color PainterPen::get_outline_color() const {
	return outline_color;
}

void PainterPen::set_outline_width(real_t p_width) {
	outline_width = MAX(0.0, p_width);
	emit_changed();
}

real_t PainterPen::get_outline_width() const {
	return outline_width;
}

void PainterPen::set_cap(LineCap p_cap) {
	cap = p_cap;
	emit_changed();
}

PainterPen::LineCap PainterPen::get_cap() const {
	return cap;
}

void PainterPen::set_join(LineJoin p_join) {
	join = p_join;
	emit_changed();
}

PainterPen::LineJoin PainterPen::get_join() const {
	return join;
}

void PainterPen::set_dash_pattern(const PoolRealArray &p_dash) {
	dash_pattern = p_dash;
	emit_changed();
}

PoolRealArray PainterPen::get_dash_pattern() const {
	return dash_pattern;
}

bool PainterPen::has_dash() const {
	return dash_pattern.size() > 0;
}

bool PainterPen::has_outline() const {
	return outline_width > 0.0 && outline_color.a > 0.0;
}

bool PainterPen::is_visible() const {
	return width > 0.0 && color.a > 0.0;
}

void PainterPen::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_color", "color"), &PainterPen::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &PainterPen::get_color);

	ClassDB::bind_method(D_METHOD("set_width", "width"), &PainterPen::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &PainterPen::get_width);

	ClassDB::bind_method(D_METHOD("set_outline_color", "color"), &PainterPen::set_outline_color);
	ClassDB::bind_method(D_METHOD("get_outline_color"), &PainterPen::get_outline_color);

	ClassDB::bind_method(D_METHOD("set_outline_width", "width"), &PainterPen::set_outline_width);
	ClassDB::bind_method(D_METHOD("get_outline_width"), &PainterPen::get_outline_width);

	ClassDB::bind_method(D_METHOD("set_cap", "cap"), &PainterPen::set_cap);
	ClassDB::bind_method(D_METHOD("get_cap"), &PainterPen::get_cap);

	ClassDB::bind_method(D_METHOD("set_join", "join"), &PainterPen::set_join);
	ClassDB::bind_method(D_METHOD("get_join"), &PainterPen::get_join);

	ClassDB::bind_method(D_METHOD("set_dash_pattern", "dash"), &PainterPen::set_dash_pattern);
	ClassDB::bind_method(D_METHOD("get_dash_pattern"), &PainterPen::get_dash_pattern);

	ClassDB::bind_method(D_METHOD("has_dash"), &PainterPen::has_dash);
	ClassDB::bind_method(D_METHOD("has_outline"), &PainterPen::has_outline);
	ClassDB::bind_method(D_METHOD("is_visible"), &PainterPen::is_visible);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "outline_color"), "set_outline_color", "get_outline_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "outline_width"), "set_outline_width", "get_outline_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cap", PROPERTY_HINT_ENUM, "Butt,Square,Round"), "set_cap", "get_cap");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "join", PROPERTY_HINT_ENUM, "Round,Miter,Bevel"), "set_join", "get_join");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_REAL_ARRAY, "dash_pattern"), "set_dash_pattern", "get_dash_pattern");

	BIND_ENUM_CONSTANT(CAP_BUTT);
	BIND_ENUM_CONSTANT(CAP_SQUARE);
	BIND_ENUM_CONSTANT(CAP_ROUND);

	BIND_ENUM_CONSTANT(JOIN_ROUND);
	BIND_ENUM_CONSTANT(JOIN_MITER);
	BIND_ENUM_CONSTANT(JOIN_BEVEL);
}
