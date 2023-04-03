/**************************************************************************/
/*  line_builder_2d.h                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef LINEBUILDER2D_H
#define LINEBUILDER2D_H

#include "core/reference.h"
#include "core/variant.h"
#include "scene/2d/line_2d.h"
#include "scene/2d/line_builder.h"

class LineBuilder2D : public Reference {
	GDCLASS(LineBuilder2D, Reference);

protected:
	static void _bind_methods();

public:
	void set_points(const PoolVector2Array &p_points);
	PoolVector2Array get_points() const;
	void set_default_color(const Color &p_default_color);
	Color get_default_color() const;
	void set_gradient(Ref<Gradient> p_gradient);
	Ref<Gradient> get_gradient() const;
	void set_texture_mode(Line2D::LineTextureMode p_texture_mode);
	Line2D::LineTextureMode get_texture_mode() const;
	void set_tile_aspect(float p_tile_aspect);
	float get_tile_aspect() const;
	void set_tile_region(Rect2 p_tile_region);
	Rect2 get_tile_region() const;
	void set_joint_mode(Line2D::LineJointMode p_joint_mode);
	Line2D::LineJointMode get_joint_mode() const;
	void set_begin_cap_mode(Line2D::LineCapMode p_begin_cap_mode);
	Line2D::LineCapMode get_begin_cap_mode() const;
	void set_end_cap_mode(Line2D::LineCapMode p_end_cap_mode);
	Line2D::LineCapMode get_end_cap_mode() const;
	void set_round_precision(int p_round_precision);
	int get_round_precision() const;
	void set_sharp_limit(float p_sharp_limit);
	float get_sharp_limit() const;
	void set_width(float p_width);
	float get_width() const;
	void set_curve(Ref<Curve> p_curve);
	Ref<Curve> get_curve() const;

	PoolIntArray get_indices() const;
	PoolVector2Array get_vertices() const;
	PoolColorArray get_colors() const;
	PoolVector2Array get_uvs() const;

	void build() { lb.build(); }
	void clear_output() { lb.clear_output(); }

	LineBuilder2D();

private:
	LineBuilder lb;
};

#endif // LINEBUILDER2D_H
