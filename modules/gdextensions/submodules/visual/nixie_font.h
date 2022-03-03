/*************************************************************************/
/*  nixie_font.h                                                         */
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

#ifndef GD_NIXIE_FONT_H
#define GD_NIXIE_FONT_H

#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

class NixieFont : public Node2D {
	GDCLASS(NixieFont, Node2D);

public:
	enum Align {

		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
		AlignCount
	};

private:
	String draw_text;
	Align align;
	bool enable_animation;
	int animation_speed;
	int broken_tube_effect;

	Ref<ImageTexture> _texture;
	bool _dirty;
	Size2 _char_size;
	Ref<ArrayMesh> _mesh;
	struct CharControl {
		int age; // displaing age
		uint8_t phase; // current step
		uint8_t alt; // alternate char overlay
		int alt_age; // next alternate char
	};
	PoolVector<CharControl> _control;
	PoolVector<Rect2> _frames;
	Size2 _text_rect_size;
	int _visible_chars;
	int _animation_delay;

	void _push_texture(PoolVector2Array &array, const Rect2 &quad);
	void _push_quad(PoolVector2Array &array, const Point2 &origin, const Size2 &size);
	Size2 _calculate_text_rect(const String &s, int *visible_chars = 0) const;
	void _update_animation();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	bool _edit_use_rect() const;
#endif

	String get_text();
	void set_text(String p_text);

	void set_align(Align p_align);
	Align get_align() const;

	bool get_enable_animation();
	void set_enable_animation(bool p_state);

	int get_animation_speed();
	void set_animation_speed(int p_speed);

	int get_broken_tube_effect();
	void set_broken_tube_effect(int p_duration);

	NixieFont();
	~NixieFont();
};

VARIANT_ENUM_CAST(NixieFont::Align);

#endif // GD_NIXIE_FONT_H
