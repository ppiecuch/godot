#ifndef LINEBUILDER2D_H
#define LINEBUILDER2D_H

#include "core/reference.h"
#include "core/variant.h"
#include "scene/2d/line_2d.h"
#include "scene/2d/line_builder.h"

class LineBuilder2D  : public Reference {
    GDCLASS( LineBuilder2D, Reference );

protected:
    static void _bind_methods();

public:
    void set_points(const PoolVector2Array &p_points);
    PoolVector2Array get_points() const;
    void set_default_color(const Color &p_default_color);
    Color get_default_color() const;
    void set_gradient(const Ref<Gradient> &p_gradient);
    Ref<Gradient> get_gradient() const;
    void set_texture_mode(Line2D::LineTextureMode p_texture_mode);
    Line2D::LineTextureMode get_texture_mode() const;
    void set_joint_mode(Line2D::LineJointMode p_joint_mode);
    Line2D::LineJointMode get_joint_mode() const;
    void set_begin_cap_mode(Line2D::LineCapMode p_begin_cap_mode);
    Line2D::LineCapMode get_begin_cap_mode() const;
    void set_end_cap_mode(Line2D::LineCapMode p_end_cap_mode);
    Line2D::LineCapMode get_end_cap_mode() const;
    void set_round_precision(int p_round_precision);
    int get_round_precision() const;
    void set_sharp_limit(float p_sharp_limit);
    float get_sharp_limit() const;
    void set_width(float p_width);
    float get_width() const;
    void set_curve(const Ref<Curve> &p_curve);
    Ref<Curve> get_curve() const;

    PoolIntArray get_indices() const;
    void set_indices(const PoolIntArray &p_indicies);
    PoolVector2Array get_vertices() const;
    void set_vertices(const PoolVector2Array &p_vertices);
    PoolColorArray get_colors() const;
    void set_colors(const PoolColorArray &p_colors);
    PoolVector2Array get_uvs() const;
    void set_uvs(const PoolVector2Array &p_uvs);

private:
    LineBuilder lb;
};

#endif // LINEBUILDER2D_H
