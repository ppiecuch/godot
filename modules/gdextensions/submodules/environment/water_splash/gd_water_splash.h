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
	real_t _target_height;
	real_t _height;
	real_t _speed;
	Vector2 _drag;

	void update(const real_t &p_tension, const real_t &p_damping);
	void body_enter_shape(int p_body_id, Object *p_body, int p_body_shape, int p_area_shape);

	GdWaterSplashColumn(const Vector2 &p_pos, const Vector2 &p_delta, const Vector2 &p_drag);
	~GdWaterSplashColumn();
};

class GdWaterSplash : public Node2D {
	GDCLASS(GdWaterSplash, Node2D);

	Vector<GdWaterSplashColumn *> _columns;

	Rect2 _rect;
	uint32_t _ncols;
	uint32_t _resolution;
	Color _color;
	real_t _damping;
	real_t _tension;
	real_t _spread;
	Vector2 _drag;
	Ref<Texture> texture;

	bool _size_changed;
	void _update();

protected:
	static void _bind_methods();

public:
	void _notification(int p_what);

	// Size in x must be a multiple of resolution
	void set_size(const Rect2 &p_value);
	Rect2 get_size() const;

	// Size of the simulation grid
	void set_resolution(const uint32_t &p_value);
	uint32_t get_resolution() const;

	void set_color(const Color &p_value);
	Color get_color() const;

	// Physical parameters
	void set_tension(const real_t &p_value);
	real_t get_tension() const;

	void set_damping(const real_t &p_value);
	real_t get_damping() const;

	void set_spread(const real_t &p_value);
	real_t get_spread() const;

	void set_drag(const Vector2 &p_value);
	Vector2 get_drag() const;

	void set_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_texture() const;

	GdWaterSplash();
};

#endif // GD_WATER_SPLASH_H
