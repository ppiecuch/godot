/*************************************************************************/
/*  phantom.h                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

//
// Created by Gen on 16/1/21.
//

#ifndef GODOT_MAIN_PHANTOM_H
#define GODOT_MAIN_PHANTOM_H

#include "core/math/vector2.h"
#include "core/object.h"
#include "core/reference.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/sprite.h"
#include "scene/resources/texture.h"

#include "queue.hpp"

using namespace GdExtends;

class Phantom : public Node2D {
	GDCLASS(Phantom, Node2D);

protected:
	struct Item {
		Vector2 position;
		Ref<Texture> texture;
		Rect2 src_rect;
		Vector2 scale;
		float rotation;
		Vector2 offset;
	};
	struct DrawSprite {
		Vector<Item> items;
		int life;
	};

private:
	Array target_paths;
	Array targets;
	Queue<DrawSprite> sprites;

	void _update_targets();

	int life_frame;
	int frame_interval;
	void _update_size();

	Ref<Gradient> color_ramp;

	void _update_fixed_frame();
	void _update_and_draw();

	void make_item();

	int frame_count;

	bool phantom_enable;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	_FORCE_INLINE_ void set_target_paths(const Array &p_paths) {
		target_paths = p_paths;
		_update_targets();
	}
	_FORCE_INLINE_ const Array &get_target_paths() { return target_paths; }
	_FORCE_INLINE_ const Array &get_targets() { return targets; }

	_FORCE_INLINE_ void set_gradient(const Ref<Gradient> &p_color_ramp) { color_ramp = p_color_ramp; }
	_FORCE_INLINE_ Ref<Gradient> get_gradient() { return color_ramp; }

	_FORCE_INLINE_ void set_life_frame(int p_frame) {
		life_frame = p_frame;
		_update_size();
	}
	_FORCE_INLINE_ int get_life_frame() { return life_frame; }

	_FORCE_INLINE_ void set_frame_interval(int p_frame) {
		frame_interval = p_frame;
		_update_size();
	}
	_FORCE_INLINE_ int get_frame_interval() { return frame_interval; }

	_FORCE_INLINE_ void set_phantom_enable(bool p_enable) { phantom_enable = p_enable; }
	_FORCE_INLINE_ bool get_phantom_enable() { return phantom_enable; }

	_FORCE_INLINE_ Phantom() {
		frame_count = 0;
		sprites.alloc(30);
		frame_interval = 1;
		life_frame = 60;
		phantom_enable = false;
		set_physics_process(true);
		set_process(true);
	}
};

#endif //GODOT_MAIN_PHANTOM_H