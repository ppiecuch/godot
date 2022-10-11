/*************************************************************************/
/*  starfield_2d.h                                                       */
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

#ifndef GD_STARFIELD_2D_H
#define GD_STARFIELD_2D_H

#include "core/reference.h"
#include "scene/2d/node_2d.h"

#include "starfield.h"

class Starfield;

class Starfield2D : public Node2D {
	GDCLASS(Starfield2D, Node2D);

private:
	Vector2 movement_vector;
	bool movement_active;
	Vector2 view_size;

	Ref<Starfield> _starfield;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;
#endif

	void set_movement_vector(const Vector2 &p_movement);
	Vector2 get_movement_vector() const;

	void set_movement_active(bool p_state);
	bool is_movement_active() const;

	void set_view_size(const Size2 &p_size);
	Size2 get_view_size() const;

	int add_stars_layer(int p_num_stars, Vector2 p_expanse_size, real_t p_star_size, Color p_star_color = Color::solid(0.6));
	int add_point_stars_layer(int p_num_stars, Vector2 p_expanse_size, Color p_star_color = Color::solid(0.6));
	int add_texture_stars_layer(int p_num_stars, Vector2 p_expanse_size, real_t p_star_size, Starfield::StarTexture p_texture_id, Color p_star_color = Color::solid(0.6));

	void set_layer_movement_opt(int p_layer, Vector2 p_movement_scale, bool p_with_alpha);
	void set_layer_color_opt(int p_layer, Color p_base_color, real_t p_alpha_pulsation);

	Starfield2D();
	~Starfield2D();
};

#endif /* GD_STARFIELD_2D_H */
