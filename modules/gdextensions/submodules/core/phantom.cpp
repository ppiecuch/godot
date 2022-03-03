/*************************************************************************/
/*  phantom.cpp                                                          */
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

//
// Created by Gen on 16/1/21.
//

#include "phantom.h"
#include "servers/visual_server.h"

void Phantom::_update_targets() {
	if (is_inside_tree()) {
		targets.clear();
		for (int i = 0, t = target_paths.size(); i < t; ++i) {
			NodePath path = target_paths[i];
			if (has_node(path)) {
				Sprite *sprite = cast_to<Sprite>(get_node(path));
				if (sprite) {
					targets.push_back(sprite);
				}
			}
		}
	} else {
	}
}

void Phantom::make_item() {
	DrawSprite sprite;
	sprite.life = life_frame;
	for (int i = 0, t = targets.size(); i < t; ++i) {
		Sprite *target = cast_to<Sprite>((Object *)targets[i]);
		Item item;
		item.position = target->get_global_position();
		item.offset = target->get_offset();
		Transform2D m = target->get_global_transform();
		item.scale = m.get_scale();
		item.rotation = m.get_rotation();
		if (m[0][0] * m[1][1] < 0) {
			item.scale.x = -item.scale.x;
			item.rotation = Math_PI * 2 - item.rotation;
		}
		item.texture = target->get_texture();
		if (target->is_region()) {
			item.src_rect = target->get_region_rect();
		} else {
			Size2 size = item.texture->get_size();
			size.width /= target->get_hframes();
			size.height /= target->get_vframes();
			float left = (target->get_frame() % target->get_hframes()) * size.width;
			float top = (target->get_frame() / target->get_hframes()) * size.height;
			item.src_rect = Rect2(left, top, size.width, size.height);
		}
		if (target->is_centered())
			item.offset -= item.src_rect.size / 2;
		sprite.items.push_back(item);
	}
	sprites.push(sprite);
}

void Phantom::_update_size() {
	sprites.alloc(life_frame / (frame_interval + 1) + 1);
}

void Phantom::_update_fixed_frame() {
	for (int i = sprites.size() - 1; i >= 0; --i) {
		DrawSprite &s = sprites[i];
		if (s.life > 0) {
			s.life -= 1;
		} else
			break;
	}
	if (phantom_enable) {
		if (frame_count <= 0) {
			frame_count = frame_interval;
			make_item();
		} else {
			frame_count -= 1;
		}
	}
}

void Phantom::_update_and_draw() {
	int t = sprites.size();
	if (t == 0)
		return;
	RID ci = get_canvas_item();
	Transform2D m = get_global_transform().affine_inverse();

	for (int i = t - 1; i >= 0; --i) {
		DrawSprite &sprite = sprites[i];
		if (sprite.life > 0) {
			for (int j = 0, _t = sprite.items.size(); j < _t; ++j) {
				const Item *item = &sprite.items[j];
				Transform2D xform;
				xform.set_rotation(item->rotation);
				xform.elements[2] += item->position;
				xform = m * xform;
				xform.scale_basis(item->scale);
				VisualServer::get_singleton()->canvas_item_add_set_transform(ci, xform);
				float p = sprite.life / (float)life_frame;
				Color color(1, 1, 1, p);
				if (!color_ramp.is_null())
					color = color_ramp->get_color_at_offset(1 - p);
				item->texture->draw_rect_region(ci, Rect2(item->offset, item->src_rect.size), item->src_rect, color);
			}
		} else
			break;
	}
}

void Phantom::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_ENTER_TREE: {
			_update_targets();
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			_update_fixed_frame();
		} break;
		case NOTIFICATION_PROCESS: {
			update();
		} break;
		case NOTIFICATION_DRAW: {
			_update_and_draw();
		} break;
	}
}

void Phantom::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_paths", "target_paths"), &Phantom::set_target_paths);
	ClassDB::bind_method(D_METHOD("get_target_paths"), &Phantom::get_target_paths);
	ClassDB::bind_method(D_METHOD("get_targets"), &Phantom::get_targets);

	ClassDB::bind_method(D_METHOD("set_gradient", "gradient"), &Phantom::set_gradient);
	ClassDB::bind_method(D_METHOD("get_gradient"), &Phantom::get_gradient);

	ClassDB::bind_method(D_METHOD("set_life_frame", "life_frame"), &Phantom::set_life_frame);
	ClassDB::bind_method(D_METHOD("get_life_frame"), &Phantom::get_life_frame);

	ClassDB::bind_method(D_METHOD("set_frame_interval", "frame_interval"), &Phantom::set_frame_interval);
	ClassDB::bind_method(D_METHOD("get_frame_interval"), &Phantom::get_frame_interval);

	ClassDB::bind_method(D_METHOD("set_phantom_enable", "enable"), &Phantom::set_phantom_enable);
	ClassDB::bind_method(D_METHOD("get_phantom_enable"), &Phantom::get_phantom_enable);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "target_paths"), "set_target_paths", "get_target_paths");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "life_time"), "set_life_frame", "get_life_frame");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_interval"), "set_frame_interval", "get_frame_interval");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "phantom_enable"), "set_phantom_enable", "get_phantom_enable");
}
