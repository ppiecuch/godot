#ifndef GODOT_DRAW_H
#define GODOT_DRAW_H

#include "core/math/geometry.h"
#include "core/rid.h"

void gd_draw_circle(const RID &p_to_rid, const Point2 &p_pos, float p_radius, int p_vertices, const Color &p_color);
void gd_draw_rect(const RID &p_to_rid, float p_width, float p_height, const Color &p_color);
void gd_draw_arrow(const RID &p_to_rid, const Vector2 &start, const Vector2 &end, const Color &p_color, float p_width);

#endif // GODOT_DRAW_H
