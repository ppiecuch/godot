// CPU-side shape tessellation used by Painter2D. Ported from sdl-painter's Tessellator class
// (https://github.com/yazilimperver/sdl-painter, MIT license) — see modules/painter/memo.md
// for the origin commit and what was changed during the port (Godot Vector2/Color types
// instead of sdl-painter's own Point/Vertex; Godot's Triangulate for concave fill instead of
// a ported ear-clipper, with a duplicate-point pre-pass added in filled_polygon() to match
// sdl-painter's RemoveDuplicatePoints; stroking delegates to Godot's own Clipper-backed
// Geometry::offset_polyline_2d instead of a hand-rolled quad-per-segment tessellator — see
// memo.md for why).
//
// Output is plain (points, triangle indices) so callers can submit it directly via
// VisualServer::canvas_item_add_triangle_array, the same way scene/2d/line_2d.cpp does.

#ifndef PAINTER_TESSELLATOR_H
#define PAINTER_TESSELLATOR_H

#include "core/math/rect2.h"
#include "core/math/vector2.h"
#include "core/vector.h"
#include "painter_pen.h"

struct PainterMesh {
	Vector<Vector2> vertices;
	Vector<int> indices;

	void append(const PainterMesh &p_other) {
		int base = vertices.size();
		for (int i = 0; i < p_other.vertices.size(); i++) {
			vertices.push_back(p_other.vertices[i]);
		}
		for (int i = 0; i < p_other.indices.size(); i++) {
			indices.push_back(base + p_other.indices[i]);
		}
	}
};

class PainterTessellator {
public:
	// Fills.
	static PainterMesh filled_rect(const Rect2 &p_rect);
	static PainterMesh filled_circle(const Point2 &p_center, real_t p_radius);
	static PainterMesh filled_ellipse(const Point2 &p_center, real_t p_rx, real_t p_ry);
	static PainterMesh filled_polygon(const Vector<Vector2> &p_points);
	static PainterMesh filled_pie(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg);
	static PainterMesh filled_chord(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg);
	static PainterMesh filled_rounded_rect(const Rect2 &p_rect, real_t p_radius);

	// Point builders (also useful for stroking with draw_polyline/draw_polygon).
	static Vector<Vector2> arc_points(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg);
	static Vector<Vector2> rounded_rect_points(const Rect2 &p_rect, real_t p_radius);

	// Strokes.
	static PainterMesh thick_polyline(const Vector<Vector2> &p_points, real_t p_width, PainterPen::LineCap p_cap, PainterPen::LineJoin p_join, bool p_closed);
	static PainterMesh dashed_polyline(const Vector<Vector2> &p_points, real_t p_width, const PoolRealArray &p_dash, PainterPen::LineCap p_cap, PainterPen::LineJoin p_join, bool p_closed);

private:
	static int _circle_segments(real_t p_radius);
};

#endif // PAINTER_TESSELLATOR_H
