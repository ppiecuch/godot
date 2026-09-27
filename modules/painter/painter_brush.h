// Fill style used by Painter2D. Ported from sdl-painter's Brush class
// (https://github.com/yazilimperver/sdl-painter, MIT license) — see modules/painter/memo.md
// for the origin commit and what was changed during the port.
//
// Unlike upstream, gradient colors are resolved to per-vertex colors by Painter2D at
// tessellation time by sampling color_at() in local (pre-transform) space, rather than
// upstream's triangle-subdivision-at-gradient-boundary approach — see memo.md.

#ifndef PAINTER_BRUSH_H
#define PAINTER_BRUSH_H

#include "core/resource.h"

class PainterBrush : public Resource {
	GDCLASS(PainterBrush, Resource);

public:
	enum FillType {
		FILL_SOLID,
		FILL_LINEAR_GRADIENT,
		FILL_RADIAL_GRADIENT,
	};

	PainterBrush();

	void set_fill_type(FillType p_type);
	FillType get_fill_type() const;

	void set_color(const Color &p_color);
	Color get_color() const;

	void set_color2(const Color &p_color);
	Color get_color2() const;

	void set_point1(const Vector2 &p_point);
	Vector2 get_point1() const;

	void set_point2(const Vector2 &p_point);
	Vector2 get_point2() const;

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	// Convenience setters mirroring sdl-painter's Brush::LinearGradient/RadialGradient factories.
	void set_linear_gradient(const Vector2 &p_start, const Vector2 &p_end, const Color &p_from, const Color &p_to);
	void set_radial_gradient(const Vector2 &p_center, real_t p_radius, const Color &p_from, const Color &p_to);

	bool is_gradient() const;
	bool is_visible() const;

	// Evaluates the brush at a point given in the same local space as the shape being filled.
	Color color_at(const Vector2 &p_local_point) const;

protected:
	static void _bind_methods();

private:
	FillType fill_type;
	Color color;
	Color color2;
	Vector2 point1;
	Vector2 point2;
	real_t radius;
};

VARIANT_ENUM_CAST(PainterBrush::FillType);

#endif // PAINTER_BRUSH_H
