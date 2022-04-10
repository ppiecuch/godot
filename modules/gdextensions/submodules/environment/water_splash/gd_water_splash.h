/*************************************************************************/
/*  gd_water_splash.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

// -*- C++ -*-
//

#ifndef GD_WATER_SPLASH_H
#define GD_WATER_SPLASH_H

#include "scene/2d/area_2d.h"
#include "scene/2d/canvas_modulate.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/rectangle_shape_2d.h"
#include "scene/resources/shape_2d.h"
#include "scene/resources/texture.h"

class GdWaterSplashColumn : public Area2D {
	GDCLASS(GdWaterSplashColumn, Area2D);

protected:
	static void _bind_methods();

public:
	real_t target_height_;
	real_t height_;
	real_t speed_;
	Vector2 drag_;

	void update(real_t &tension, real_t &damping);
	void body_enter_shape(int body_id, Object *body, int body_shape, int area_shape);

	GdWaterSplashColumn(const Vector2 &, const Vector2 &delta, const Vector2 &drag);
};

class GdWaterSplash : public Node2D {
	GDCLASS(GdWaterSplash, Node2D);

	Vector<GdWaterSplashColumn *> columns_;

	Rect2 rect_;
	uint32_t ncols_;
	uint32_t resolution_;
	Color color_;
	real_t damping_;
	real_t tension_;
	real_t spread_;
	Vector2 drag_;
	Ref<Texture> texture;

	bool size_changed_;
	void _update();

protected:
	static void _bind_methods();

public:
	void _notification(int p_what);

	// Size in x must be a multiple of resolution
	void set_size(const Rect2 &value);
	Rect2 get_size() const;

	// Size of the simulation grid
	void set_resolution(const uint32_t &value);
	uint32_t get_resolution() const;

	void set_color(const Color &value);
	Color get_color() const;

	// Physical parameters
	void set_tension(const real_t &value);
	real_t get_tension() const;

	void set_damping(const real_t &value);
	real_t get_damping() const;

	void set_spread(const real_t &value);
	real_t get_spread() const;

	void set_drag(const Vector2 &value);
	Vector2 get_drag() const;

	void set_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_texture() const;

	GdWaterSplash();
};

#endif // GD_WATER_SPLASH_H
