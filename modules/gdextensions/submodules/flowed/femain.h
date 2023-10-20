
/*
 Copyright (c) 2012-2016, Stanislaw Adaszewski
 * All rights reserved.
 */

#ifndef FEMAIN_H
#define FEMAIN_H

#include "core/image.h"
#include "core/reference.h"
#include "core/vector.h"

struct FEElement {
	virtual Point2 get_pos() const = 0;
	virtual Color get_color() const = 0;
	virtual bool get_passive() const = 0;
	virtual void set_passive(bool state) = 0;
	virtual ~FEElement() {}
};

class FEFlowElement : public FEElement {
	static real_t max_radius;

	int wave_size = 0;
	real_t radius = 15;
	bool brushed = false;
	bool passive = false;
	Color color = Color::solid(0.6);

public:
	static real_t MaxRadius() { return max_radius; }

	void set_from(const Point2 &pt);
	Point2 get_from() const;
	void set_to(const Point2 &pt);
	Point2 get_to() const;
	void set_wave_size(int size);
	int get_wave_size() const { return wave_size; }
	void set_passive(bool state);
	bool get_passive() const { return passive; }
	void set_brushed(bool state);
	bool get_brushed() const { return brushed; }

	Point2 get_dir() const;
	Point2 get_pos() const;
	real_t get_radius() const { return radius; }
	Color get_color() const { return color; }

	void align_arrow();
};

class FEGradientElement : public FEElement {
	bool passive = false;
	Color color = Color::solid(0.6);

public:
	void set_color(const Color &c);
	Color get_color() const { return color; }

	void set_passive(bool state);
	bool get_passive() const { return passive; }

	void set_pos(const Point2 &p);
	Point2 get_pos() const;
};

class FEMain : public Reference {
	GDCLASS(FEMain, Reference);

	LocalVector<FEElement*> elems;
	Ref<Image> image;
	String export_fname;
	Color last_color;
	int last_wave_size;
	Color gradient_brush_color;

	Ref<Image> generate_flow_map();
	Ref<Image> fast_generate_flow_map();
	void load_flow_field(const String &file_name);
	Ref<Image> akima_generate_flow_map();
	Ref<Image> nn_generate_flow_map();

	Ref<Image> render_final_image(Ref<Image>(FEMain::*fe_method)(), bool use_checker_board = true);
	void draw_gouraud_triangle(Ref<Image> &dest, const Point2 &p0, const Color &c0, const Point2 &p1, const Color &c1, const Point2 &p2, const Color &c2);
	void generate_grid(int n_cols = 10, int n_rows = 5, int dir_x = 100, int dir_y = 100, int wave_size = 0);
	void generate_gradient_grid(int n_cols = 10, int n_rows = 5, const Color &c = Color::solid(0.6));
	Ref<Image> checker_board() const;

public:
	void generate_preview();

public:
	FEMain();
	~FEMain();
};

#endif // FEMAIN_H
