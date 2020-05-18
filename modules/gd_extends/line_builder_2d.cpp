#include "line_builder_2d.h"

#define CopyArray(fr, to) {        \
    to.resize(fr.size());          \
    for(int i=0; i<fr.size(); ++i) \
        to.set(i, fr[i]);          \
}

void LineBuilder2D::set_points(const PoolVector2Array &p_points) {
    lb.points.clear();
    lb.points.resize(p_points.size());
    for(int i=0; i<lb.points.size(); ++i)
        lb.points.ref(i) = p_points[i];
}
PoolVector2Array LineBuilder2D::get_points() const {
    PoolVector2Array ret;
    CopyArray(lb.points, ret);
    return ret;
}
void LineBuilder2D::set_default_color(const Color &p_default_color) { }
Color LineBuilder2D::get_default_color() const { return lb.default_color; }
void LineBuilder2D::set_gradient(const Ref<Gradient> &p_gradient) { }
Ref<Gradient> LineBuilder2D::get_gradient() const { return lb.gradient; }
void LineBuilder2D::set_texture_mode(Line2D::LineTextureMode p_texture_mode) { }
Line2D::LineTextureMode LineBuilder2D::get_texture_mode() const { return lb.texture_mode; }
void LineBuilder2D::set_joint_mode(Line2D::LineJointMode p_joint_mode) { }
Line2D::LineJointMode LineBuilder2D::get_joint_mode() const { return lb.joint_mode; }
void LineBuilder2D::set_begin_cap_mode(Line2D::LineCapMode p_begin_cap_mode) { }
Line2D::LineCapMode LineBuilder2D::get_begin_cap_mode() const { return lb.begin_cap_mode; }
void LineBuilder2D::set_end_cap_mode(Line2D::LineCapMode p_end_cap_mode) { }
Line2D::LineCapMode LineBuilder2D::get_end_cap_mode() const { return lb.end_cap_mode; }
void LineBuilder2D::set_round_precision(int p_round_precision) { }
int LineBuilder2D::get_round_precision() const { return lb.round_precision; }
void LineBuilder2D::set_sharp_limit(float p_sharp_limit) { }
float LineBuilder2D::get_sharp_limit() const { return lb.sharp_limit; }
void LineBuilder2D::set_width(float p_width) { }
float LineBuilder2D::get_width() const { return lb.width; }
void LineBuilder2D::set_curve(const Ref<Curve> &p_curve) { }
Ref<Curve> LineBuilder2D::get_curve() const { return lb.curve; }


PoolIntArray LineBuilder2D::get_indices() const {
    PoolIntArray ret;
    CopyArray(lb.indices, ret);
    return ret;
}
void LineBuilder2D::set_indices(const PoolIntArray &p_indicies) { }
PoolVector2Array LineBuilder2D::get_vertices() const {
    PoolVector2Array ret;
    CopyArray(lb.vertices, ret);
    return ret;
}
void LineBuilder2D::set_vertices(const PoolVector2Array &p_vertices) { }
PoolColorArray LineBuilder2D::get_colors() const {
    PoolColorArray ret;
    CopyArray(lb.colors, ret);
    return ret;
}
void LineBuilder2D::set_colors(const PoolColorArray &p_colors) { }
PoolVector2Array LineBuilder2D::get_uvs() const {
    PoolVector2Array ret;
    CopyArray(lb.uvs, ret);
    return ret;
}
void LineBuilder2D::set_uvs(const PoolVector2Array &p_uvs) { }


void LineBuilder2D::_bind_methods() {

    ClassDB::bind_method("set_points", &LineBuilder2D::set_points);
    ClassDB::bind_method("get_points", &LineBuilder2D::get_points);
    ClassDB::bind_method("set_sharp_limit", &LineBuilder2D::set_sharp_limit);
    ClassDB::bind_method("get_sharp_limit", &LineBuilder2D::get_sharp_limit);

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "points"), "set_points", "get_points");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharp_limit", PROPERTY_HINT_RANGE, "0,10,0.1"), "set_sharp_limit", "get_sharp_limit");
}
