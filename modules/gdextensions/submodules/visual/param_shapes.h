#ifndef PARAM_SHAPE_H
#define PARAM_SHAPE_H

#include "core/math/vector2.h"
#include "core/variant.h"
#include "scene/2d/canvas_item.h"

struct ShapeOpt {
	bool only_reg_shapes = true;
	bool random_colors = false;
	bool reverse_inout = false;
	bool symmetric_ripple = false;
	int rotate_angle = 0;
	Point2 center(50, 50); // 0..100
	bool debug = false;
};

void draw_param_shape(CanvasItem *canvas, real_t radius1 = 150, real_t radius2 = 50, int n_points = 50, int n_gen_per = 5, const ShapeOpt &opts = ShapeOpt()) {
	ERR_NULL(canvas);
	ERR_FAIL_COND(n_points < 1);

	if (opts.only_reg_shapes) {
		ERR_FAIL_COND(n_gen_per < 1);
		ERR_FAIL_COND(n_points % n_gen_per != 0);
	}

	imgShape.Canvas.Brush.Color = clWhite;
	imgShape.Canvas.Pen.Color = clBlack;
	imgShape.Canvas.Pen.Width = 3;
	imgShape.Canvas.FillRect(imgShape.ClientRect);

	const int w = imgShape.Width;
	const int h = imgShape.Height;

	const int cx = Math::trunc(w * (opts.center.x / 100) + 0.5);
	const int cy = Math::trunc(h * (opts.center.y / 100) + 0.5);

	const real_t dtheta = Math_Tau / n_points;
	const real_t angle = Math_Tau * rotate_angle / 360;

	if (opts.debug) {
		print_line("Parameters:");
		print_line(vformat("n_gen_per: %d, n_points: %d", n_gen_per, n_points));
		print_line(vformat("radius1: %f, Radius2: %f", radius1, radius2));
		print_line(vformat("rotate angle: %f", angle));
		print_line(vformat("symmetric ripple: %s", symmetric_ripple));
		print_line(vformat("reverse in/out: %s", reverse_inout));
		print_line("");
		print_line("Points:");
	}

	// Calculating vertices

	PoolVector2Array p;
	p.resize(n_points);

	for (int k = 0; k < n_points; k++) {
		const real_t theta = k * dtheta;
		real_t ripple = radius2 * Math::cos(n_gen_per * theta);

		if (!symmetric_ripple) {
			ripple = Math::abs(ripple);
		}

		if (chkReverseInOut.Checked) {
			ripple = -1.0 * ripple;
		}

		p[k].x = Math::trunc(cx + (radius1 + ripple) * Math::cos(theta + angle) + 0.5);
		p[k].y = Math::trunc(cy - (radius1 + ripple) * Math::sin(theta + angle) + 0.5);

		if (opts.debug) {
			print_line(vformat(" P[%d]: (%d, %d)", k, p[k].X, p[k].Y));
		}
	}

	// Drawing shapes

	if (chkFilledShapes.Checked) {
		imgShape.Canvas.Brush.Color = Color(Math::randf(), Math::randf(), Math::randf());
		imgShape.Canvas.Pen.Color = Color(Math::randf(), Math::randf(), Math::randf());
		imgShape.Canvas.Pen.Width = 5;

		imgShape.Canvas.Polygon(p);
	} else {
		canvas->move_to(p[k]);
		for (int k = 1; k < n_points; k++) {
			if (opts.random_colors) {
				imgShape.Canvas.Pen.Color = Color(Math::randf(), Math::randf(), Math::randf());
				imgShape.Canvas.Pen.Width = 5;
			}
			canvas->line_to(p[k]);
		}
	}
}

#endif // PARAM_SHAPE_H
