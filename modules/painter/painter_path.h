// Path builder used by Painter2D. Ported from sdl-painter's Path class
// (https://github.com/yazilimperver/sdl-painter, MIT license) — see modules/painter/memo.md
// for the origin commit and what was changed during the port.
//
// Like upstream, quadratic/cubic Bezier segments are flattened to line segments immediately
// on insertion (no control points are retained). The adaptive segment-count heuristic here is
// a simplified approximation of upstream's curvature-bound formula — see memo.md.

#ifndef PAINTER_PATH_H
#define PAINTER_PATH_H

#include "core/resource.h"
#include "core/vector.h"

class PainterPath : public Resource {
	GDCLASS(PainterPath, Resource);

public:
	PainterPath();

	void move_to(const Vector2 &p_point);
	void line_to(const Vector2 &p_point);
	void quad_to(const Vector2 &p_control, const Vector2 &p_end);
	void cubic_to(const Vector2 &p_control1, const Vector2 &p_control2, const Vector2 &p_end);
	void close();
	void clear();

	bool is_empty() const;
	Vector2 get_current_point() const;

	int get_subpath_count() const;
	PoolVector2Array get_subpath_points(int p_index) const;
	bool is_subpath_closed(int p_index) const;

protected:
	static void _bind_methods();

private:
	struct SubPath {
		PoolVector2Array points;
		bool closed = false;
	};

	Vector<SubPath> subpaths;
	Vector2 current_point;
	Vector2 subpath_start;
	bool has_open_subpath;

	void _ensure_subpath();
};

#endif // PAINTER_PATH_H
