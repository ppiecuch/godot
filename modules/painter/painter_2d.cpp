#include "painter_2d.h"

#include "servers/visual_server.h"

static Vector<Vector2> _to_vector(const PoolVector2Array &p_points) {
	Vector<Vector2> out;
	out.resize(p_points.size());
	PoolVector2Array::Read r = p_points.read();
	for (int i = 0; i < p_points.size(); i++) {
		out.write[i] = r[i];
	}
	return out;
}

Painter2D::Painter2D() {
	batch_flush_scheduled = false;
}

void Painter2D::set_pen(const Ref<PainterPen> &p_pen) {
	current_pen = p_pen;
}

Ref<PainterPen> Painter2D::get_pen() const {
	return current_pen;
}

void Painter2D::set_brush(const Ref<PainterBrush> &p_brush) {
	current_brush = p_brush;
}

Ref<PainterBrush> Painter2D::get_brush() const {
	return current_brush;
}

void Painter2D::paint_save() {
	transform_stack.push_back(current_transform);
}

void Painter2D::paint_restore() {
	if (transform_stack.size() > 0) {
		current_transform = transform_stack[transform_stack.size() - 1];
		transform_stack.remove(transform_stack.size() - 1);
	}
}

void Painter2D::paint_translate(const Vector2 &p_offset) {
	current_transform = current_transform.translated(p_offset);
}

void Painter2D::paint_rotate(real_t p_radians) {
	current_transform = current_transform.rotated(p_radians);
}

void Painter2D::paint_scale(const Vector2 &p_scale) {
	current_transform = current_transform.scaled(p_scale);
}

void Painter2D::paint_reset_transform() {
	current_transform = Transform2D();
}

void Painter2D::_queue(const PainterMesh &p_mesh, const Vector<Color> &p_colors) {
	if (p_mesh.indices.size() == 0) {
		return;
	}
	int base = batch_points.size();
	batch_points.resize(base + p_mesh.vertices.size());
	batch_colors.resize(base + p_mesh.vertices.size());
	for (int i = 0; i < p_mesh.vertices.size(); i++) {
		batch_points.write[base + i] = current_transform.xform(p_mesh.vertices[i]);
		batch_colors.write[base + i] = p_colors[i];
	}
	int index_base = batch_indices.size();
	batch_indices.resize(index_base + p_mesh.indices.size());
	for (int i = 0; i < p_mesh.indices.size(); i++) {
		batch_indices.write[index_base + i] = base + p_mesh.indices[i];
	}

	if (!batch_flush_scheduled) {
		batch_flush_scheduled = true;
		call_deferred("_flush_batch");
	}
}

void Painter2D::_flush_batch() {
	batch_flush_scheduled = false;
	if (batch_indices.size() == 0) {
		return;
	}
	VisualServer::get_singleton()->canvas_item_add_triangle_array(
			get_canvas_item(), batch_indices, batch_points, batch_colors, Vector<Point2>(), Vector<int>(), Vector<float>(),
			RID(), -1, RID(), RID(), false, false);
	batch_points.clear();
	batch_colors.clear();
	batch_indices.clear();
}

void Painter2D::_submit_fill(const PainterMesh &p_mesh) {
	if (p_mesh.indices.size() == 0 || current_brush.is_null() || !current_brush->is_visible()) {
		return;
	}
	// Gradient colors are resolved per vertex in local, pre-transform space, since that's the
	// coordinate space a gradient brush's start/end points are defined against.
	Vector<Color> colors;
	colors.resize(p_mesh.vertices.size());
	for (int i = 0; i < p_mesh.vertices.size(); i++) {
		colors.write[i] = current_brush->color_at(p_mesh.vertices[i]);
	}
	_queue(p_mesh, colors);
}

void Painter2D::_submit_stroke(const PainterMesh &p_mesh, const Color &p_color) {
	Vector<Color> colors;
	colors.resize(p_mesh.vertices.size());
	for (int i = 0; i < p_mesh.vertices.size(); i++) {
		colors.write[i] = p_color;
	}
	_queue(p_mesh, colors);
}

void Painter2D::_stroke_points(const Vector<Vector2> &p_points, bool p_closed) {
	if (current_pen.is_null() || !current_pen->is_visible()) {
		return;
	}

	PainterPen::LineCap cap = current_pen->get_cap();
	PainterPen::LineJoin join = current_pen->get_join();
	PoolRealArray dash = current_pen->get_dash_pattern();

	if (current_pen->has_outline()) {
		real_t outline_total_width = current_pen->get_width() + current_pen->get_outline_width() * 2.0;
		PainterMesh outline_mesh = current_pen->has_dash()
				? PainterTessellator::dashed_polyline(p_points, outline_total_width, dash, cap, join, p_closed)
				: PainterTessellator::thick_polyline(p_points, outline_total_width, cap, join, p_closed);
		_submit_stroke(outline_mesh, current_pen->get_outline_color());
	}

	PainterMesh mesh = current_pen->has_dash()
			? PainterTessellator::dashed_polyline(p_points, current_pen->get_width(), dash, cap, join, p_closed)
			: PainterTessellator::thick_polyline(p_points, current_pen->get_width(), cap, join, p_closed);
	_submit_stroke(mesh, current_pen->get_color());
}

void Painter2D::stroke_line(const Point2 &p_from, const Point2 &p_to) {
	Vector<Vector2> pts;
	pts.push_back(p_from);
	pts.push_back(p_to);
	_stroke_points(pts, false);
}

void Painter2D::stroke_polyline(const PoolVector2Array &p_points) {
	_stroke_points(_to_vector(p_points), false);
}

void Painter2D::stroke_polygon(const PoolVector2Array &p_points) {
	_stroke_points(_to_vector(p_points), true);
}

void Painter2D::fill_polygon(const PoolVector2Array &p_points) {
	_submit_fill(PainterTessellator::filled_polygon(_to_vector(p_points)));
}

void Painter2D::stroke_rect(const Rect2 &p_rect) {
	_stroke_points(PainterTessellator::rounded_rect_points(p_rect, 0.0), true);
}

void Painter2D::fill_rect(const Rect2 &p_rect) {
	_submit_fill(PainterTessellator::filled_rect(p_rect));
}

void Painter2D::stroke_rounded_rect(const Rect2 &p_rect, real_t p_radius) {
	_stroke_points(PainterTessellator::rounded_rect_points(p_rect, p_radius), true);
}

void Painter2D::fill_rounded_rect(const Rect2 &p_rect, real_t p_radius) {
	_submit_fill(PainterTessellator::filled_rounded_rect(p_rect, p_radius));
}

void Painter2D::stroke_circle(const Point2 &p_center, real_t p_radius) {
	_stroke_points(PainterTessellator::arc_points(p_center, p_radius, p_radius, 0, 360), true);
}

void Painter2D::fill_circle(const Point2 &p_center, real_t p_radius) {
	_submit_fill(PainterTessellator::filled_circle(p_center, p_radius));
}

void Painter2D::stroke_ellipse(const Point2 &p_center, real_t p_rx, real_t p_ry) {
	_stroke_points(PainterTessellator::arc_points(p_center, p_rx, p_ry, 0, 360), true);
}

void Painter2D::fill_ellipse(const Point2 &p_center, real_t p_rx, real_t p_ry) {
	_submit_fill(PainterTessellator::filled_ellipse(p_center, p_rx, p_ry));
}

void Painter2D::stroke_arc(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg) {
	_stroke_points(PainterTessellator::arc_points(p_center, p_rx, p_ry, p_start_deg, p_sweep_deg), false);
}

void Painter2D::stroke_pie(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg) {
	Vector<Vector2> pts;
	pts.push_back(p_center);
	Vector<Vector2> arc = PainterTessellator::arc_points(p_center, p_rx, p_ry, p_start_deg, p_sweep_deg);
	for (int i = 0; i < arc.size(); i++) {
		pts.push_back(arc[i]);
	}
	_stroke_points(pts, true);
}

void Painter2D::fill_pie(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg) {
	_submit_fill(PainterTessellator::filled_pie(p_center, p_rx, p_ry, p_start_deg, p_sweep_deg));
}

void Painter2D::stroke_chord(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg) {
	_stroke_points(PainterTessellator::arc_points(p_center, p_rx, p_ry, p_start_deg, p_sweep_deg), true);
}

void Painter2D::fill_chord(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg) {
	_submit_fill(PainterTessellator::filled_chord(p_center, p_rx, p_ry, p_start_deg, p_sweep_deg));
}

void Painter2D::stroke_path(const Ref<PainterPath> &p_path) {
	if (p_path.is_null()) {
		return;
	}
	for (int i = 0; i < p_path->get_subpath_count(); i++) {
		_stroke_points(_to_vector(p_path->get_subpath_points(i)), p_path->is_subpath_closed(i));
	}
}

void Painter2D::fill_path(const Ref<PainterPath> &p_path) {
	if (p_path.is_null()) {
		return;
	}
	for (int i = 0; i < p_path->get_subpath_count(); i++) {
		_submit_fill(PainterTessellator::filled_polygon(_to_vector(p_path->get_subpath_points(i))));
	}
}

void Painter2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_flush_batch"), &Painter2D::_flush_batch);

	ClassDB::bind_method(D_METHOD("set_pen", "pen"), &Painter2D::set_pen);
	ClassDB::bind_method(D_METHOD("get_pen"), &Painter2D::get_pen);
	ClassDB::bind_method(D_METHOD("set_brush", "brush"), &Painter2D::set_brush);
	ClassDB::bind_method(D_METHOD("get_brush"), &Painter2D::get_brush);

	ClassDB::bind_method(D_METHOD("paint_save"), &Painter2D::paint_save);
	ClassDB::bind_method(D_METHOD("paint_restore"), &Painter2D::paint_restore);
	ClassDB::bind_method(D_METHOD("paint_translate", "offset"), &Painter2D::paint_translate);
	ClassDB::bind_method(D_METHOD("paint_rotate", "radians"), &Painter2D::paint_rotate);
	ClassDB::bind_method(D_METHOD("paint_scale", "scale"), &Painter2D::paint_scale);
	ClassDB::bind_method(D_METHOD("paint_reset_transform"), &Painter2D::paint_reset_transform);

	ClassDB::bind_method(D_METHOD("stroke_line", "from", "to"), &Painter2D::stroke_line);
	ClassDB::bind_method(D_METHOD("stroke_polyline", "points"), &Painter2D::stroke_polyline);
	ClassDB::bind_method(D_METHOD("stroke_polygon", "points"), &Painter2D::stroke_polygon);
	ClassDB::bind_method(D_METHOD("fill_polygon", "points"), &Painter2D::fill_polygon);

	ClassDB::bind_method(D_METHOD("stroke_rect", "rect"), &Painter2D::stroke_rect);
	ClassDB::bind_method(D_METHOD("fill_rect", "rect"), &Painter2D::fill_rect);
	ClassDB::bind_method(D_METHOD("stroke_rounded_rect", "rect", "radius"), &Painter2D::stroke_rounded_rect);
	ClassDB::bind_method(D_METHOD("fill_rounded_rect", "rect", "radius"), &Painter2D::fill_rounded_rect);

	ClassDB::bind_method(D_METHOD("stroke_circle", "center", "radius"), &Painter2D::stroke_circle);
	ClassDB::bind_method(D_METHOD("fill_circle", "center", "radius"), &Painter2D::fill_circle);
	ClassDB::bind_method(D_METHOD("stroke_ellipse", "center", "rx", "ry"), &Painter2D::stroke_ellipse);
	ClassDB::bind_method(D_METHOD("fill_ellipse", "center", "rx", "ry"), &Painter2D::fill_ellipse);

	ClassDB::bind_method(D_METHOD("stroke_arc", "center", "rx", "ry", "start_deg", "sweep_deg"), &Painter2D::stroke_arc);
	ClassDB::bind_method(D_METHOD("stroke_pie", "center", "rx", "ry", "start_deg", "sweep_deg"), &Painter2D::stroke_pie);
	ClassDB::bind_method(D_METHOD("fill_pie", "center", "rx", "ry", "start_deg", "sweep_deg"), &Painter2D::fill_pie);
	ClassDB::bind_method(D_METHOD("stroke_chord", "center", "rx", "ry", "start_deg", "sweep_deg"), &Painter2D::stroke_chord);
	ClassDB::bind_method(D_METHOD("fill_chord", "center", "rx", "ry", "start_deg", "sweep_deg"), &Painter2D::fill_chord);

	ClassDB::bind_method(D_METHOD("stroke_path", "path"), &Painter2D::stroke_path);
	ClassDB::bind_method(D_METHOD("fill_path", "path"), &Painter2D::fill_path);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "pen", PROPERTY_HINT_RESOURCE_TYPE, "PainterPen"), "set_pen", "get_pen");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "brush", PROPERTY_HINT_RESOURCE_TYPE, "PainterBrush"), "set_brush", "get_brush");
}
