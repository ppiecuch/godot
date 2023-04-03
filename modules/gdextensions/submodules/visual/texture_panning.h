/**************************************************************************/
/*  texture_panning.h                                                     */
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

#ifndef GD_TEXTURE_PANNING_H
#define GD_TEXTURE_PANNING_H

#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

class TexturePanning : public Node2D {
	GDCLASS(TexturePanning, Node2D);

private:
	Ref<Texture> texture;
	Size2 texture_scale;
	Vector2 view_size;
	Vector2 scrolling_speed;
	bool scrolling_active;

	Vector2 _texture_offset;
	void _refresh();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	bool _edit_use_rect() const;
#endif

	void set_texture(Ref<Texture> p_texture);
	Ref<Texture> get_texture() const;

	void set_texture_scale(Vector2 p_scale);
	Vector2 get_texture_scale() const;

	void set_scrolling_speed(const Vector2 &p_speed);
	Vector2 get_scrolling_speed() const;

	void set_scrolling_active(bool p_state);
	bool is_scrolling_active() const;

	void set_view_size(const Size2 &p_size);
	Size2 get_view_size() const;

	TexturePanning();
};

#endif // GD_TEXTURE_PANNING_H
