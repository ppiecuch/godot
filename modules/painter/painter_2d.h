// Immediate-mode 2D vector drawing node. API shape ported from sdl-painter's Painter class
// (https://github.com/yazilimperver/sdl-painter, MIT license) — see modules/painter/memo.md
// for the origin commit and what was changed during the port.
//
// Unlike upstream, there is no Begin()/End() frame loop: draw calls are meant to be issued from
// an overridden _draw() (exactly like CanvasItem's own draw_line/draw_rect/etc.), triggered by
// the usual update() -> NOTIFICATION_DRAW flow, and are submitted straight to VisualServer via
// canvas_item_add_triangle_array (see scene/2d/line_2d.cpp for the same pattern in core).
//
// Stroke-only drawing methods are named stroke_* rather than upstream's draw_* because
// CanvasItem/Node2D already bind draw_line/draw_rect/draw_circle/draw_arc/draw_polyline/
// draw_polygon/translate/rotate/apply_scale with different signatures — ClassDB::bind_method
// silently keeps the first (inherited) binding on a name clash, which would make the Painter2D
// overloads unreachable from GDScript. The local paint transform stack is prefixed paint_* for
// the same reason, and to make clear it's separate from the node's own transform.
//
// Batching (replaces upstream's RenderBatcher): every stroke_*/fill_* call appends into a
// per-node accumulation buffer instead of calling VisualServer directly, and the buffer is
// flushed as a single canvas_item_add_triangle_array call via call_deferred(), so a whole
// _draw() worth of shapes becomes one VisualServer call instead of one per shape. This is safe
// because CanvasItem::_update_callback() calls notification(NOTIFICATION_DRAW), emit_signal
// ("draw"), and the script's _draw() override all synchronously before returning — a
// call_deferred() queued from _notification(NOTIFICATION_DRAW) cannot run until that whole
// stack unwinds, so every draw call the script's _draw() makes is guaranteed to already be in
// the buffer by the time the flush fires. See modules/painter/memo.md.

#ifndef PAINTER_2D_H
#define PAINTER_2D_H

#include "painter_brush.h"
#include "painter_path.h"
#include "painter_pen.h"
#include "painter_tessellator.h"
#include "scene/2d/node_2d.h"

class Painter2D : public Node2D {
	GDCLASS(Painter2D, Node2D);

public:
	Painter2D();

	void set_pen(const Ref<PainterPen> &p_pen);
	Ref<PainterPen> get_pen() const;

	void set_brush(const Ref<PainterBrush> &p_brush);
	Ref<PainterBrush> get_brush() const;

	void paint_save();
	void paint_restore();
	void paint_translate(const Vector2 &p_offset);
	void paint_rotate(real_t p_radians);
	void paint_scale(const Vector2 &p_scale);
	void paint_reset_transform();

	void stroke_line(const Point2 &p_from, const Point2 &p_to);
	void stroke_polyline(const PoolVector2Array &p_points);
	void stroke_polygon(const PoolVector2Array &p_points);
	void fill_polygon(const PoolVector2Array &p_points);

	void stroke_rect(const Rect2 &p_rect);
	void fill_rect(const Rect2 &p_rect);
	void stroke_rounded_rect(const Rect2 &p_rect, real_t p_radius);
	void fill_rounded_rect(const Rect2 &p_rect, real_t p_radius);

	void stroke_circle(const Point2 &p_center, real_t p_radius);
	void fill_circle(const Point2 &p_center, real_t p_radius);
	void stroke_ellipse(const Point2 &p_center, real_t p_rx, real_t p_ry);
	void fill_ellipse(const Point2 &p_center, real_t p_rx, real_t p_ry);

	void stroke_arc(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg);
	void stroke_pie(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg);
	void fill_pie(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg);
	void stroke_chord(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg);
	void fill_chord(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg);

	void stroke_path(const Ref<PainterPath> &p_path);
	void fill_path(const Ref<PainterPath> &p_path);

protected:
	static void _bind_methods();

private:
	Ref<PainterPen> current_pen;
	Ref<PainterBrush> current_brush;
	Transform2D current_transform;
	Vector<Transform2D> transform_stack;

	Vector<Point2> batch_points;
	Vector<Color> batch_colors;
	Vector<int> batch_indices;
	bool batch_flush_scheduled;

	void _queue(const PainterMesh &p_mesh, const Vector<Color> &p_colors);
	void _flush_batch();

	void _submit_fill(const PainterMesh &p_mesh);
	void _submit_stroke(const PainterMesh &p_mesh, const Color &p_color);
	void _stroke_points(const Vector<Vector2> &p_points, bool p_closed);
};

#endif // PAINTER_2D_H
