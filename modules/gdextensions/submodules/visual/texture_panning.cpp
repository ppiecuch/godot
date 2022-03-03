/*************************************************************************/
/*  texture_panning.cpp                                                  */
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

#include "texture_panning.h"
#include "scene/main/viewport.h"

#ifdef TOOLS_ENABLED
Dictionary TexturePanning::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["view_size"] = get_view_size();

	return state;
}

void TexturePanning::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_view_size(p_state["view_size"]);
}

bool TexturePanning::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return _edit_get_rect().has_point(p_point);
};

Rect2 TexturePanning::_edit_get_rect() const {
	return Rect2(Point2(), get_view_size());
}

bool TexturePanning::_edit_use_rect() const {
	return true;
}
#endif

void TexturePanning::_refresh() {
	ERR_FAIL_COND(texture.is_null());

	_texture_offset = Vector2(0, 0) - texture->get_size() * texture_scale;
	update();
}

void TexturePanning::set_texture(Ref<Texture> p_texture) {
	if (texture != p_texture) {
		texture = p_texture;
		update();
	}
}

Ref<Texture> TexturePanning::get_texture() const {
	return texture;
}

void TexturePanning::set_texture_scale(Vector2 p_scale) {
	if (texture_scale != p_scale) {
		texture_scale = p_scale;
		update();
	}
}

Vector2 TexturePanning::get_texture_scale() const {
	return texture_scale;
}

void TexturePanning::set_scrolling_active(bool p_state) {
	if (p_state != scrolling_active) {
		scrolling_active = p_state;
	}
}

bool TexturePanning::is_scrolling_active() const {
	return scrolling_active;
}

void TexturePanning::set_scrolling_speed(const Vector2 &p_speed) {
	if (p_speed != scrolling_speed) {
		scrolling_speed = p_speed;
		emit_signal("scrolling_changed");
		update();
	}
}

Vector2 TexturePanning::get_scrolling_speed() const {
	return scrolling_speed;
}

void TexturePanning::set_view_size(const Size2 &p_size) {
	if (p_size != view_size) {
		view_size = p_size;
		item_rect_changed();
		emit_signal("view_size_changed");
		update();
	}
}

Size2 TexturePanning::get_view_size() const {
	return view_size;
}

void TexturePanning::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_PROCESS: {
		} break;
		case NOTIFICATION_DRAW: {
			if (texture.is_valid()) {
				RID ci = get_canvas_item();
				Rect2 dest_rect = Rect2(Point2(0, 0), view_size);
				Rect2 src_rect = Rect2(_texture_offset, (view_size + texture->get_size() * 2) / texture_scale);
				texture->draw_rect_region(ci, dest_rect, src_rect, get_modulate(), false, Ref<Texture>(), Ref<Texture>(), true);
			}
		} break;
	}
}

void TexturePanning::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture"), &TexturePanning::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &TexturePanning::get_texture);
	ClassDB::bind_method(D_METHOD("set_texture_scale"), &TexturePanning::set_texture_scale);
	ClassDB::bind_method(D_METHOD("get_texture_scale"), &TexturePanning::get_texture_scale);
	ClassDB::bind_method(D_METHOD("set_scrolling_speed"), &TexturePanning::set_scrolling_speed);
	ClassDB::bind_method(D_METHOD("get_scrolling_speed"), &TexturePanning::get_scrolling_speed);
	ClassDB::bind_method(D_METHOD("set_scrolling_active"), &TexturePanning::set_scrolling_active);
	ClassDB::bind_method(D_METHOD("is_scrolling_active"), &TexturePanning::is_scrolling_active);
	ClassDB::bind_method(D_METHOD("set_view_size"), &TexturePanning::set_view_size);
	ClassDB::bind_method(D_METHOD("get_view_size"), &TexturePanning::get_view_size);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "scrolling_active"), "set_scrolling_active", "is_scrolling_active");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "texture_scale"), "set_texture_scale", "get_texture_scale");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "scrolling_speed"), "set_scrolling_speed", "get_scrolling_speed");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_size"), "set_view_size", "get_view_size");

	ADD_SIGNAL(MethodInfo("view_size_changed"));
	ADD_SIGNAL(MethodInfo("movement_changed"));
}

TexturePanning::TexturePanning() {
	texture = Ref<Texture>(NULL);
	texture_scale = Vector2(2, 2);
	view_size = Vector2(100, 100);
	scrolling_speed = Vector2(0, 0);
	_texture_offset = Point2(0, 0);
}

// extends Node
//
// # Private properties
//
// var _texture = null setget _setter, _getter
// var _speed_x = -1 setget _setter, _getter
// var _speed_y = 0 setget _setter, _getter
// var _scale = 1 setget _setter, _getter
// var _screen_size setget _setter, _getter
// var _texture_size setget _setter, _getter
// var _modulate setget _setter, _getter
//
// func _setter(value = null):
//     print("Invalid access to private variable!")
//
//
// func _getter():
//     print("Invalid access to private variable!")
//     return null
//
// # Public properties
//
// var texture setget set_texture, get_texture
// var speed_x setget set_speed_x, get_speed_x
// var speed_y setget set_speed_y, get_speed_y
// var scale setget set_scale, get_scale
// var modulate setget set_modulate, get_modulate
//
// # Setters/Getters
//
// func set_texture(texture):
//     _texture = texture
//     if not has_node("Background"):
//         _refresh_child()
//     else:
//         _update_texture()
//
// func set_scale(scale):
//     _scale = scale
//     if not has_node("Background"):
//         _refresh_child()
//     else:
//         _update_texture()
//
// func get_scale(): return _scale
//
// func set_modulate(modulate):
//     _modulate = modulate
//     if not has_node("Background"):
//         _refresh_child()
//     else:
//         _update_modulate()
//
// func get_modulate(): return _modulate
//
// # Property descriptor for the editor
//
// func _get_property_list():
//     return [
//         {usage = PROPERTY_USAGE_CATEGORY, type = TYPE_NIL, name = "ScrollingBackground"},
//         {type = TYPE_OBJECT, name = "texture", hint = PROPERTY_HINT_RESOURCE_TYPE, hint_string = "ImageTexture"},
//         {type = TYPE_INT, name = "speed_x"},
//         {type = TYPE_INT, name = "speed_y"},
//         {type = TYPE_REAL, name = "scale"},
//         {type = TYPE_COLOR, name = "modulate"}
//     ]
//
// # Initialise node, once we're ready
//
// func _ready():
//     _refresh_child()
//
// # Update the Background node based on settings
//
// func _refresh_child():
//
//     if _texture == null:
//         # Texture not set, return early
//         return
//
//     if get_viewport() == null:
//         # We don't yet have a viewport.
//         return
//
//     if not has_node("Background"):
//         var spriteNode = Sprite.new()
//         spriteNode.set_name("Background")
//         add_child(spriteNode)
//
//     _screen_size = get_viewport().get_rect().size
//
//     _update_modulate()
//
//     _update_texture()
//
//     var spriteNode = get_node("Background")
//
//     var current_position = spriteNode.get_pos()
//     current_position.x = 0 - _texture_size.get_width() * _scale
//     current_position.y = 0 - _texture_size.get_height() * _scale
//     spriteNode.set_pos(current_position)
//
// func _update_modulate():
//     if _modulate != null:
//         var spriteNode = get_node("Background")
//         spriteNode.set_modulate(_modulate)
//
// func _update_texture():
//     var spriteNode = get_node("Background")
//
//     spriteNode.set_texture(_texture)
//     _texture_size = _texture.get_data()
//     _texture.flags = _texture.flags | ImageTexture.FLAG_REPEAT
//
//     var region_rect = Rect2(
//         0,
//         0,
//         (_screen_size.width + _texture_size.get_width() * 2) / _scale,
//         (_screen_size.height + _texture_size.get_height() * 2) / _scale
//     )
//
//     spriteNode.set_region_rect(
//         region_rect
//     )
//     spriteNode.set_region(true)
//     spriteNode.set_centered(false)
//     spriteNode.set_scale(Vector2(_scale, _scale))
//
// # Update the position according to speed and reset
// # accordingly, so it looks like as if the background is
// # continously scrolling
//
// func _physics_process(delta):
//
//     if _texture == null:
//         # Texture not set. Returning early
//         return
//
//     var spriteNode = get_node("Background")
//
//     var current_position = spriteNode.get_pos()
//
//     current_position.x = current_position.x + _speed_x
//     current_position.y = current_position.y + _speed_y
//
//     if (
//         current_position.x < 0 - _texture_size.get_width() * _scale * 2 ||
//         current_position.x > 0
//     ):
//         current_position.x = 0 - _texture_size.get_width() * _scale
//
//     if (
//         current_position.y < 0 - _texture_size.get_height() * _scale * 2||
//         current_position.y > 0
//     ):
//         current_position.y = 0 - _texture_size.get_height() * _scale
//
//     spriteNode.set_pos(current_position)
