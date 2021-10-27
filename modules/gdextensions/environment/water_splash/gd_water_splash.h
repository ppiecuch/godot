/*************************************************************************/
/*  gd_water_splash.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef _GD_WATER_SPLASH_H_
#define _GD_WATER_SPLASH_H_

#include "scene/2d/area_2d.h"
#include "scene/2d/canvas_modulate.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/rectangle_shape_2d.h"
#include "scene/resources/shape_2d.h"
#include "scene/resources/texture.h"

class GDWaterSplashColumn : public Area2D {
	GDCLASS(GDWaterSplashColumn, Area2D);

public:
	GDWaterSplashColumn(const Vector2 &, const Vector2 &delta, const Vector2 &drag);
	void update(float &tension, float &damping);
	void body_enter_shape(int body_id, Object *body, int body_shape, int area_shape);
	float target_height_;
	float height_;
	float speed_;
	Vector2 drag_;

protected:
	static void _bind_methods();
};

class GDWaterSplash : public Node2D {
	GDCLASS(GDWaterSplash, Node2D);

public:
	GDWaterSplash();

	void _notification(int p_what);

	// Size in x must be a multiple of resolution
	void set_size(const Rect2 &value);
	Rect2 get_size() const;

	// Size of the simulation grid
	void set_resolution(const uint32_t &value);
	uint32_t get_resolution() const;

	void set_color(const Color &value);
	Color get_color() const;

	/**
	 * Physical Parameters
	 */
	void set_tension(const float &value);
	float get_tension() const;

	void set_damping(const float &value);
	float get_damping() const;

	void set_spread(const float &value);
	float get_spread() const;

	void set_drag(const Vector2 &value);
	Vector2 get_drag() const;

	void set_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_texture() const;

protected:
	static void _bind_methods();

	Vector<GDWaterSplashColumn *> columns_;

	Rect2 rect_;
	uint32_t ncols_;
	uint32_t resolution_;
	Color color_;
	float damping_;
	float tension_;
	float spread_;
	Vector2 drag_;
	Ref<Texture> texture;

	bool size_changed_;
	void _update();
};

#endif // _GD_WATER_SPLASH_H_
