// Stroke style used by Painter2D. Ported from sdl-painter's Pen class
// (https://github.com/yazilimperver/sdl-painter, MIT license) — see modules/painter/memo.md
// for the origin commit and what was changed during the port.

#ifndef PAINTER_PEN_H
#define PAINTER_PEN_H

#include "core/resource.h"

class PainterPen : public Resource {
	GDCLASS(PainterPen, Resource);

public:
	enum LineCap {
		CAP_BUTT,
		CAP_SQUARE,
		CAP_ROUND,
	};

	enum LineJoin {
		JOIN_ROUND,
		JOIN_MITER,
		JOIN_BEVEL,
	};

	PainterPen();

	void set_color(const Color &p_color);
	Color get_color() const;

	void set_width(real_t p_width);
	real_t get_width() const;

	void set_outline_color(const Color &p_color);
	Color get_outline_color() const;

	void set_outline_width(real_t p_width);
	real_t get_outline_width() const;

	void set_cap(LineCap p_cap);
	LineCap get_cap() const;

	void set_join(LineJoin p_join);
	LineJoin get_join() const;

	void set_dash_pattern(const PoolRealArray &p_dash);
	PoolRealArray get_dash_pattern() const;

	bool has_dash() const;
	bool has_outline() const;
	bool is_visible() const;

protected:
	static void _bind_methods();

private:
	Color color;
	real_t width;
	Color outline_color;
	real_t outline_width;
	LineCap cap;
	LineJoin join;
	PoolRealArray dash_pattern;
};

VARIANT_ENUM_CAST(PainterPen::LineCap);
VARIANT_ENUM_CAST(PainterPen::LineJoin);

#endif // PAINTER_PEN_H
