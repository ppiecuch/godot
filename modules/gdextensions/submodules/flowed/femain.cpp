/*
 * Copyright (c) 2012-2016, Stanislaw Adaszewski
 * All rights reserved.
 */

#include "femain.h"

#include "core/error_macros.h"
#include "common/gd_core.h"

#include "flowed/akima/akima.h"
#include "flowed/nn-c/nn.h"
#include "flowed/qhull/qhull_a.h"

#define PREVIEW_SIZE() Size2(100, 75)

template <typename T> struct LocalAutoVector : public LocalVector<T, size_t> {
	operator T*() { return this->ptr(); }
	operator T*() const { return this->ptr(); }
	LocalAutoVector(size_t p_size, const T *p_src = nullptr) : LocalVector<T, size_t>(p_size, p_src) { }
};

real_t FEFlowElement::max_radius = 32;

Ref<Image> FEMain::akima_generate_flow_map() {
	if (elems.size() < 10) {
		WARN_PRINT("AKIMA Cubic Interpolation requires at least 10 points");
		return Ref<Image>();
	}
	int MD = 1;
	int NDP = elems.size();
	LocalAutoVector<double> XD(NDP);
	LocalAutoVector<double> YD(NDP);
	LocalAutoVector<double> ZD1(NDP);
	LocalAutoVector<double> ZD2(NDP);
	LocalAutoVector<double> ZD3(NDP);
    LocalAutoVector<double> ZD4(NDP);
	int NXI = image->get_width();
	LocalAutoVector<double> XI(NXI);
	int NYI = image->get_height();
	LocalAutoVector<double> YI(NYI);
	LocalAutoVector<double> ZI1(NXI*NYI); // final output X
	LocalAutoVector<double> ZI2(NXI*NYI); // final output Y
	LocalAutoVector<double> ZI3(NXI*NYI); // final output Z
    LocalAutoVector<double> ZI4(NXI*NYI); // final output A
	int IER;
	LocalAutoVector<double> WK(NDP * 17);
	LocalAutoVector<int> IWK(NDP * 25);
	LocalAutoVector<bool> EXTRPI(NXI*NYI);
	LocalAutoVector<int> NEAR(NDP);
	LocalAutoVector<int> NEXT(NDP);
	LocalAutoVector<double> DIST(NDP);

	// memset(WK, 0, NDP * 17 * sizeof(double));
	// memset(IWK, 0, NDP * 25 * sizeof(int));
	// memset(EXTRPI, 0, NXI*NYI*sizeof(int));
	// memset(NEAR, 0, NDP * sizeof(int));
	// memset(NEXT, 0, NDP * sizeof(int));
	// memset(DIST, 0, NDP * sizeof(double));

	for (int i = 0; i < NXI; i++) {
		XI[i] = i;
	}
	for (int i = 0; i < NYI; i++) {
		YI[i] = i;
	}

	for (int i = 0; i < NDP; i++) {
		XD[i] = elems[i]->get_pos().x;
		YD[i] = elems[i]->get_pos().y;
		const Color color = elems[i]->get_color();
		ZD1[i] = color.r;
		ZD2[i] = color.g;
		ZD3[i] = color.b;
        ZD4[i] = color.a;
	}

	sdsf3p_(&MD, &NDP, XD, YD, ZD1, &NXI, XI, &NYI, YI, ZI1, &IER, WK, IWK, EXTRPI, NEAR, NEXT, DIST);

	// memset(WK, 0, NDP * 17 * sizeof(double));
	// memset(IWK, 0, NDP * 25 * sizeof(int));
	// memset(EXTRPI, 0, NXI*NYI*sizeof(int));
	// memset(NEAR, 0, NDP * sizeof(int));
	// memset(NEXT, 0, NDP * sizeof(int));
	// memset(DIST, 0, NDP * sizeof(double));

	sdsf3p_(&MD, &NDP, XD, YD, ZD2, &NXI, XI, &NYI, YI, ZI2, &IER, WK, IWK, EXTRPI, NEAR, NEXT, DIST);

	// memset(WK, 0, NDP * 17 * sizeof(double));
	// memset(IWK, 0, NDP * 25 * sizeof(int));
	// memset(EXTRPI, 0, NXI*NYI*sizeof(int));
	// memset(NEAR, 0, NDP * sizeof(int));
	// memset(NEXT, 0, NDP * sizeof(int));
	// memset(DIST, 0, NDP * sizeof(double));

	sdsf3p_(&MD, &NDP, XD, YD, ZD3, &NXI, XI, &NYI, YI, ZI3, &IER, WK, IWK, EXTRPI, NEAR, NEXT, DIST);
    sdsf3p_(&MD, &NDP, XD, YD, ZD4, &NXI, XI, &NYI, YI, ZI4, &IER, WK, IWK, EXTRPI, NEAR, NEXT, DIST);

	Ref<Image> im = memnew(Image(NXI, NYI, false, Image::FORMAT_RGBA8));
    im->fill(0);
	for (int x = 0; x < NXI; x++) {
		for (int y = 0; y < NYI; y++) {
			const int ofs = y * NXI + x;
			const float red = CLAMP(ZI1[ofs], 0.0, 1.0);
			const float green = CLAMP(ZI2[ofs], 0.0, 1.0);
			const float blue = CLAMP(ZI3[ofs], 0.0, 1.0);
			const float alpha = CLAMP(ZI4[ofs], 0.0, 1.0);

			im->set_pixel(x, y, Color(red, green, blue, alpha));
		}
	}
	return im;
}

Ref<Image> FEMain::nn_generate_flow_map() {
	point *points = new point[elems.size()];
	point *p = points;
	LocalAutoVector<double> zin(elems.size());
	for (const FEElement *elem : elems) {
		p->x = elem->get_pos().x;
		p->y = elem->get_pos().y;
		p->z = 0;
		p++;
	}
	delaunay *d = delaunay_build(elems.size(), points, 0, 0, 0, 0);
	const int w = image->get_width();
	const int h = image->get_height();
	const int N = w * h;
	LocalAutoVector<double> x(N);
	LocalAutoVector<double> y(N);
	LocalAutoVector<double> red(N);
	LocalAutoVector<double> green(N);
	LocalAutoVector<double> blue(N);
    LocalAutoVector<double> alpha(N);
	for (int yy = 0; yy < h; yy++) {
		for (int xx = 0; xx < w; xx++) {
			x[yy*w+xx] = xx;
			y[yy*w+xx] = yy;
		}
	}

	// nn_rule = NON_SIBSONIAN;
	nnai *nn = nnai_build(d, N, x, y);
	nnai_setwmin(nn, -INFINITY);

	for (size_t i = 0; i < elems.size(); i++) { // red
		zin[i] = elems[i]->get_color().r;
	}
	nnai_interpolate(nn, zin, red);

	for (size_t i = 0; i < elems.size(); i++) { // green
		zin[i] = elems[i]->get_color().g;
	}
	nnai_interpolate(nn, zin, green);

	for (size_t i = 0; i < elems.size(); i++) { // blue
		zin[i] = elems[i]->get_color().b;
	}
	nnai_interpolate(nn, zin, blue);

	for (size_t i = 0; i < elems.size(); i++) { // alpha
		zin[i] = elems[i]->get_color().a;
	}
    nnai_interpolate(nn, zin, alpha);

#define CCLAMP(v) ((v) < 0 ? 0 : ((v) > 1 ? 1 : (v)))

	Ref<Image> im = memnew(Image(w, h, false, Image::FORMAT_RGBA8));
    im->fill(0);
	for (int yy = 0; yy < h; yy++) {
		for (int xx = 0; xx < w; xx++) {
			const float r = red[yy*w + xx];
			const float g = green[yy*w + xx];
			const float b = blue[yy*w + xx];
			const float a = alpha[yy*w + xx];
			const Color c(CCLAMP(r), CCLAMP(g), CCLAMP(b), CCLAMP(a));
			im->set_pixel(xx, yy, c);
		}
	}

#undef CCLAMP

	nnai_destroy(nn);
	delaunay_destroy(d);

	return im;
}

void FEMain::generate_grid(int n_cols, int n_rows, int dir_x, int dir_y, int wave_size) {
	ERR_FAIL_COND(n_cols < 1 || n_cols > 1000);
	ERR_FAIL_COND(n_rows < 1 || n_rows > 1000);
	ERR_FAIL_COND(dir_x < -100 || dir_x > 100);
	ERR_FAIL_COND(dir_y < -100 || dir_y > 100);
	ERR_FAIL_COND(wave_size < 0 || wave_size > 255);

	const real_t step_x = image->get_width() / real_t(n_cols);
	const real_t step_y = image->get_height() / real_t(n_rows);
	const real_t dx = dir_x * 2 * FEFlowElement::MaxRadius() / 100.0;
	const real_t dy = dir_y * 2 * FEFlowElement::MaxRadius() / 100.0;

	for (int x = 0; x <= n_cols; x++) {
		for (int y = 0; y <= n_rows; y++) {
			FEFlowElement *elem = memnew(FEFlowElement);
			elem->set_from(Point2(x * step_x, y * step_y));
			elem->set_to(elem->get_from() + Point2(dx, dy));
			elem->set_wave_size(wave_size);
		}
	}
}

void FEMain::generate_gradient_grid(int n_cols, int n_rows, const Color &c) {
	ERR_FAIL_COND(n_cols < 1 || n_cols > 1000);
	ERR_FAIL_COND(n_rows < 1 || n_rows > 1000);

	const real_t step_x = image->get_width() / real_t(n_cols);
	const real_t step_y = image->get_height() / real_t(n_rows);

	for (int x = 0; x <= n_cols; x++) {
		for (int y = 0; y <= n_rows; y++) {
			FEGradientElement *elem = memnew(FEGradientElement());
			elem->set_pos(Point2(x * step_x, y * step_y));
			elem->set_color(c);
		}
	}
}

Ref<Image> FEMain::generate_flow_map() {
	const Size2 s = image->get_size();
	const int w = s.width;
	const int h = s.height;

	LocalAutoVector<double> A(w * h * 4);
	A.fill(-1);

	// Voronoi tesselation
	#pragma omp parallel for
	for (int i = 0; i < elems.size(); i++) {
		Point2 pos(elems[i]->get_pos());
		Color color = elems[i]->get_color();

		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++) {
				const int ofs = (y * w + x) * 4;
				const double l = Vector2(pos - Point2(x, y)).length_squared();
				if (l < A[ofs] || A[ofs] == -1) {
					A[ofs] = l;
					A[ofs + 1] = color.r;
					A[ofs + 2] = color.g;
					A[ofs + 3] = color.b;
				}
			}
	}

	LocalAutoVector<double> B(w * h * 3);

	// Natural neighbor
	#pragma omp parallel for
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int r = 0, count = 0;
			double red = 0, green = 0, blue = 0;
			while (true) {
				int count1 = count;
				for (int x1 = MAX(0, x - r); x1 <= MIN(w-1, x + r); x1++) {
					double l = (x1-x)*(x1-x) + r*r;
					int ofs1 = (MAX(0, y - r) * w + x1) * 4;
					DEV_ASSERT(ofs1 < w*h*4);
					DEV_ASSERT(A[ofs1] != -1);
					if (l <= A[ofs1]) {
						red += A[ofs1+1];
						green += A[ofs1+2];
						blue += A[ofs1+3];
						count++;
					}
					ofs1 = (MIN(h-1, y + r) * w + x1) * 4;
					DEV_ASSERT(ofs1 < w*h*4);
					DEV_ASSERT(A[ofs1] != -1);
					if (l <= A[ofs1]) {
						red += A[ofs1+1];
						green += A[ofs1+2];
						blue += A[ofs1+3];
						count++;
					}
				}
				for (int y1 = MAX(0, y - r); y1 <= MIN(h-1, y + r); y1++) {
					double l = (y1-y)*(y1-y) + r*r;
					int ofs1 = (y1 * w + MAX(0, x - r)) * 4;
					DEV_ASSERT(ofs1 < w*h*4);
					DEV_ASSERT(A[ofs1] != -1);
					if (l <= A[ofs1]) {
						red += A[ofs1+1];
						green += A[ofs1+2];
						blue += A[ofs1+3];
						count++;
					}
					ofs1 = (y1 * w + MIN(w-1, x + r)) * 4;
					DEV_ASSERT(ofs1 < w*h*4);
					DEV_ASSERT(A[ofs1] != -1);
					if (l <= A[ofs1]) {
						red += A[ofs1+1];
						green += A[ofs1+2];
						blue += A[ofs1+3];
						count++;
					}
				}
				if (count1 == count) {
					break;
				}
				r++;
			}
			const int ofs = (y * w + x) * 3;
			B[ofs + 0] = red / count;
			B[ofs + 1] = green / count;
			B[ofs + 2] = blue / count;
		}
	}

	Ref<Image> im = memnew(Image(w, h, false, Image::FORMAT_RGBA8));
	for (int i = 0; i < w * h; i++) {
		const Color c(B[i * 3], B[i * 3 + 1], B[i * 3 + 2], 1);
	}
	return im;
}

Ref<Image> FEMain::fast_generate_flow_map() {
	int numpoints = elems.size();
	int dim = 2;
	boolT ismalloc;
	coordT *points = nullptr;

	qh NOerrexit = False;
	qh_option("delaunay Qbbound-last", nullptr, nullptr);
	qh DELAUNAY = True;      /* 'd'   */
	qh SCALElast = True;     /* 'Qbb' */
	qh KEEPcoplanar = True;  /* 'Qc', to keep coplanars in 'p' */
	qh TRIangulate = True;   /* Qt */
	qh_appendprint(qh_PRINTvertices); /* Fv */
	points = qh_readpoints(&numpoints, &dim, &ismalloc);
	qh_init_B(points, numpoints, dim, ismalloc);
	qh_qhull();

	LocalVector<LocalVector<int>> indices;

	Ref<Image> im = memnew(Image(image->get_width(), image->get_height(), false, Image::FORMAT_RGBA8));
	im->fill(0);

	for (LocalVector<int> I : indices) {
		FEElement *e0 = elems[I[0]];
		FEElement *e1 = elems[I[1]];
		FEElement *e2 = elems[I[2]];

		Color c0 = e0->get_color();
		Color c1 = e1->get_color();
		Color c2 = e2->get_color();

		if (e0->get_passive()) c0 = (e1->get_passive() ? c2 : c1);
		if (e1->get_passive()) c1 = (e0->get_passive() ? c2 : c0);
		if (e2->get_passive()) c2 = (e0->get_passive() ? c1 : c0);

		draw_gouraud_triangle(im, e0->get_pos(), c0, e1->get_pos(), c1, e2->get_pos(), c2);
	}

	return im;
}

void FEMain::generate_preview() {
    Ref<Image> fm(render_final_image(&FEMain::fast_generate_flow_map));
}

Ref<Image> FEMain::render_final_image(Ref<Image>(FEMain::*fe_method)(), bool use_checker_board) {
	Ref<Image> im = use_checker_board ? checker_board() : memnew(Image(image->get_width(), image->get_height(), false, Image::FORMAT_RGBA8));
	im->blit_image((this->*fe_method)());
	return im;
}

Ref<Image> FEMain::checker_board() const {
	const int w = image->get_width();
	const int h = image->get_height();
	Ref<Image> checker_board = memnew(Image(w, h, false, Image::FORMAT_RGBA8));
	checker_board->checker_board(32);
	return checker_board;
}

void FEMain::draw_gouraud_triangle(Ref<Image> &dest, const Point2 &p0, const Color &c0, const Point2 &p1, const Color &c1, const Point2 &p2, const Color &c2) {
	ERR_FAIL_NULL(dest);

#define CCLAMP(x) (isfinite((x)) ? (((x) < 0) ? (0) : (((x) > 1) ? 1 : (x))) : 0)
#define CLAMP2(x,d) (((x) < 0) ? (0) : (((x) >= (d)) ? ((d) - 1) : (x)))

	int x0 = p0.x;
	int y0 = p0.y;
	float r0 = c0.r, g0 = c0.g, b0 = c0.b, a0 = c0.a;
	int x1 = p1.x;
	int y1 = p1.y;
	float r1 = c1.r, g1 = c1.g, b1 = c1.b, a1 = c1.a;
	int x2 = p2.x;
	int y2 = p2.y;
	float r2 = c2.r, g2 = c2.g, b2 = c2.b, a2 = c2.a;

	// Sort our points into order of y:
	//  - 0 top
	//  - 2 middle
	//  - 1 bottom

	const int w = image->get_width();
	const int h = image->get_height();

	if (y1 < y0) {
		SWAP(y1, y0);
		SWAP(x1, x0);
		SWAP(r1, r0); SWAP(g1, g0); SWAP(b1, b0); SWAP(a1, a0);
	}
	if (y2 < y0) {
		SWAP(y2, y0);
		SWAP(x2, x0);
		SWAP(r2, r0); SWAP(g2, g0); SWAP(b2, b0); SWAP(a2, a0);
	}
	if (y1 < y2) {
		SWAP(y2, y1);
		SWAP(x2, x1);
		SWAP(r2, r1); SWAP(g2, g1); SWAP(b2, b1); SWAP(a2, a1);
	}

	float xl_edge = x0; // left edge
	float xr_edge = x0; // right edge

	float dxldy;
	float dxrdy;

	float dxdy1 = (float)(x2-x0)/(y2-y0);
	float dxdy2 = (float)(x1-x0)/(y1-y0);

	float dr1 = (float)(r2-r0)/(y2-y0);
	float dg1 = (float)(g2-g0)/(y2-y0);
	float db1 = (float)(b2-b0)/(y2-y0);
	float da1 = (float)(a2-a0)/(y2-y0);

	float dr2 = (float)(r1-r0)/(y1-y0);
	float dg2 = (float)(g1-g0)/(y1-y0);
	float db2 = (float)(b1-b0)/(y1-y0);
	float da2 = (float)(a1-a0)/(y1-y0);

	float drldy, dgldy, dbldy, daldy;
	float drrdy, dgrdy, dbrdy, dardy;

	if (dxdy1 < dxdy2) {
		dxldy = dxdy1;
		dxrdy = dxdy2;
		drldy = dr1; dgldy = dg1; dbldy = db1; daldy = da1; // left  (r,g,b)
		drrdy = dr2; dgrdy = dg2; dbrdy = db2; dardy = da2; // right (r,g,b)
	} else {
		dxldy = dxdy2;
		dxrdy = dxdy1;
		drldy  = dr2; dgldy = dg2; dbldy = db2; daldy = da2; // left  (r,g,b)
		drrdy  = dr1; dgrdy = dg1; dbrdy = db1; dardy = da1; // right (r,g,b)
	}

	float r_left  = r0, r_right = r0;
	float g_left  = g0, g_right = g0;
	float b_left  = b0, b_right = b0;
	float a_left  = a0, a_right = a0;

	// Top of the triangle
	for(int y = y0; y < y2; y++) {
		float dr = (r_right - r_left)/(xr_edge - xl_edge);
		float dg = (g_right - g_left)/(xr_edge - xl_edge);
		float db = (b_right - b_left)/(xr_edge - xl_edge);
		float da = (a_right - a_left)/(xr_edge - xl_edge);

		float pr = r_left;
		float pg = g_left;
		float pb = b_left;
		float pa = a_left;

		for(int x = xl_edge; x < xr_edge; x++) {
			pr = pr + dr;
			pg = pg + dg;
			pb = pb + db;
			pa = pa + da;

			if (x >= 0 && x < w && y >= 0 && y < h) {
				image->set_pixel(x, y, Color(CCLAMP(pr), CCLAMP(pg), CCLAMP(pb), CCLAMP(pa)));
			}
		} //end for loop x

		xl_edge = xl_edge + dxldy;
		xr_edge = xr_edge + dxrdy;


		r_left  += drldy;
		r_right += drrdy;

		g_left  += dgldy;
		g_right += dgrdy;

		b_left  += dbldy;
		b_right += dbrdy;

		a_left  += daldy;
		a_right += dardy;
	} // end for loop y

	// Bottom half of the triangle
	if (y0 == y2) {
		if (x0 > x2) {
			xr_edge = x0;
			r_right = r0;
			g_right = g0;
			b_right = b0;
			a_right = a0;

			xl_edge = x2;
			r_left = r2;
			g_left = g2;
			b_left = b2;
			a_left = a2;
		} else {
			xr_edge = x2;
			r_right = r2;
			g_right = g2;
			b_right = b2;
			a_right = a2;

			xl_edge = x0;
			r_left = r0;
			g_left = g0;
			b_left = b0;
			a_left = a0;
		}
	}

	if( dxdy1 < dxdy2 ) {
		dxldy = (float)(x2-x1)/(y2-y1);

		drldy  = (r2-r1)/(y2-y1);
		dgldy  = (g2-g1)/(y2-y1);
		dbldy  = (b2-b1)/(y2-y1);
		daldy  = (a2-a1)/(y2-y1);
	} else {
		dxrdy = (float)(x2-x1)/(y2-y1);

		drrdy  = (r2-r1)/(y2-y1);
		dgrdy  = (g2-g1)/(y2-y1);
		dbrdy  = (b2-b1)/(y2-y1);
		dardy  = (a2-a1)/(y2-y1);
	}

	for (int y = y2; y < y1; y++) {
		const float dr = (r_right - r_left)/(xr_edge - xl_edge);
		const float dg = (g_right - g_left)/(xr_edge - xl_edge);
		const float db = (b_right - b_left)/(xr_edge - xl_edge);
		const float da = (a_right - a_left)/(xr_edge - xl_edge);

		float pr = r_left;
		float pg = g_left;
		float pb = b_left;
		float pa = a_left;

		for (int x = xl_edge; x < xr_edge; x++) {
			pr = pr + dr;
			pg = pg + dg;
			pb = pb + db;
			pa = pa + da;

			if (x >= 0 && x < w && y >= 0 && y < h) {
				image->set_pixel(x, y, Color(CCLAMP(pr), CCLAMP(pg), CCLAMP(pb), CCLAMP(pa)));
			}
		} // end for loop x

		xl_edge = xl_edge + dxldy;
		xr_edge = xr_edge + dxrdy;

		r_left  += drldy;
		r_right += drrdy;

		g_left  += dgldy;
		g_right += dgrdy;

		b_left  += dbldy;
		b_right += dbrdy;

		a_left  += daldy;
		a_right += dardy;
	} // end for loop y
#undef CLAMP2
#undef CCLAMP
}

FEMain::FEMain() {
}

FEMain::~FEMain() {
}
