/*************************************************************************/
/*  nixie_font.cpp                                                       */
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

#include "nixie_font.h"
#include "nixie_font_res.h"

#include <cstring> // std::memcpy

#define MAKE_ABGR(r, g, b, a) ((uint32_t)(((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(r)))
#define MAKE_RGBA(r, g, b, a) ((uint32_t)(((uint32_t)(r) << 24) | ((uint32_t)(g) << 16) | ((uint32_t)(b) << 8) | (uint32_t)(a)))

static PoolVector<Rect2> _build_tiles(Size2 grid_size, int total_frames, Point2 tex_origin = Point2(0, 0), Size2 tex_size = Size2(1, 1)) {
	PoolVector<Rect2> frames;
	const real_t cw = tex_size.width / grid_size.width, ch = tex_size.height / grid_size.height;
	for (int fr = 0; fr < grid_size.height /* rows */; fr++) {
		for (int fc = 0; fc < grid_size.width /* cols */; fc++) {
			frames.push_back(Rect2(tex_origin.x + (grid_size.width - fc - 1) * cw, tex_origin.y + fr * ch, cw, ch));
			if (frames.size() == total_frames)
				break;
		}
	}
	return frames;
}

enum LightColorMask {

	Mask00 = 00,
	Mask05 = 12,
	Mask10 = 24,
	Mask15 = 36,
	Mask20 = 48,
	Mask25 = 61,
	Mask30 = 73,
	Mask35 = 85,
	Mask40 = 97,
	Mask45 = 110,
	Mask50 = 122,
	Mask55 = 134,
	Mask60 = 146,
	Mask65 = 158,
	Mask70 = 170,
	Mask75 = 182,
	Mask80 = 194,
	Mask85 = 207,
	Mask90 = 219,
	Mask95 = 233,
	Mask100 = 255,
};

const int NixieFontCols = 8; // rows and cols
const int NixieFontRows = 12; // in font sheet
const int NixieFontChars = NixieFontCols * NixieFontRows; // might be less than Rows*Cols

const struct {
	int t; // timing
	uint8_t alpha; // alpha
} LightBlinkingPattern[] = {

	{ 99, Mask10 }, // 10%
	{ 1, Mask50 }, // 50%
	{ 8, Mask80 }, // 80%
	{ 2, Mask70 }, // 70%
	{ 2, Mask20 }, // 20%
	{ 1, Mask15 }, // 15%
	{ 1, Mask70 }, // 70%
	{ 2, Mask80 }, // 80%
	{ 1, Mask75 }, // 75%
	{ 1, Mask80 }, // 80%
	{ 1, Mask15 }, // 15%
	{ 1, Mask60 }, // 60%
	{ 1, Mask65 }, // 65%
	{ 1, Mask50 }, // 50%
	{ 2, Mask55 }, // 121-122
	{ 2, Mask60 }, // 123-124
	{ 2, Mask55 }, // 125-126
	{ 2, Mask60 }, // 127-128
	{ 1, Mask40 }, // 129
	{ 2, Mask45 }, // 130-131
	{ 1, Mask20 }, // 132
	{ 11, Mask30 }, // 133-143
	{ 1, Mask20 }, // 144
	{ 2, Mask05 }, // 145-146
	{ 4, Mask30 }, // 147-150
	{ 2, Mask20 }, // 151-152
	{ 3, Mask25 }, // 153-155
	{ 1, Mask15 }, // 156
	{ 4, Mask25 }, // 157-160
	{ 1, Mask15 }, // 161
	{ 12, Mask25 }, // 162-173
	{ 1, Mask15 }, // 174
	{ 2, Mask05 }, // 175-176
	{ 2, Mask20 }, // 177-178
	{ 13, Mask35 }, // 179-191
	{ 1, Mask25 }, // 192
	{ 2, Mask15 }, // 193-194
	{ 1, Mask45 }, // 195
	{ 7, Mask65 }, // 196-202
	{ 1, Mask25 }, // 203
	{ 1, Mask10 }, // 204
	{ 5, Mask75 }, // 205-210
	{ 2, Mask15 }, // 211-212
	{ 9, Mask65 }, // 213-221
	{ 1, Mask15 }, // 222
	{ 5, Mask25 }, // 223-227
	{ 1, Mask15 }, // 228
	{ 2, Mask20 }, // 229-230
	{ 10, Mask65 }, // 231-240
	{ 20, Mask75 }, // 241-260
	{ 1, Mask25 }, // 261
	{ 5, Mask75 }, // 262-266
	{ 4, Mask85 }, // 267-269
	{ 1, Mask25 }, // 270
	{ 16, Mask75 }, // 271-284
	{ 1, Mask25 }, // 285
	{ 1, Mask35 }, // 286
	{ 1, Mask75 }, // 287
	{ 12, Mask85 }, // 288-298
	{ 1, Mask35 }, // 299
	{ 1, Mask45 }, // 300
	{ 3, Mask75 }, // 301-303
	{ 1, Mask55 }, // 304
	{ 8, Mask85 }, // 305-311
	{ 3, Mask25 }, // 312-314
	{ 3, Mask85 }, // 315
	{ 3, Mask85 }, // 315-317
	{ 600, Mask100 }, // 318-417
	{ 3, Mask25 }, // 418-420
	{ -1, Mask100 }
};

#ifdef TOOLS_ENABLED
bool NixieFont::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return _edit_get_rect().has_point(p_point);
};

Rect2 NixieFont::_edit_get_rect() const {
	return Rect2(Point2(0, 0), _text_rect_size);
}

bool NixieFont::_edit_use_rect() const {
	return true;
}
#endif

Size2 NixieFont::_calculate_text_rect(const String &s, int *visible_chars) const {
	real_t max_line_width = 0;
	real_t line_width = 0;
	real_t lines_count = 0;
	int all_chars = 0;

	for (int i = 0; i < s.size(); i++) {
		CharType current = s[i];

		if (current < 32) {
			if (current == '\n') {
				if (line_width > max_line_width)
					max_line_width = line_width;
				line_width = 0;
				lines_count++;
			}
		} else {
			line_width++;
			if (current > 32)
				all_chars++;
		}
	}

	if (line_width > max_line_width)
		max_line_width = line_width;
	if (visible_chars)
		*visible_chars = all_chars;
	return Size2(max_line_width, lines_count + 1);
}

void NixieFont::_push_texture(PoolVector2Array &array, const Rect2 &quad) {
	static const Vector2 sideu(1, 0);
	static const Vector2 sidev(0, 1);

	const Point2 &uv_origin = quad.position;
	const Size2 &uv_size = quad.size;

	// 0--1,2
	// | // |
	// 2,4--3
	array.push_back(uv_origin);
	array.push_back(uv_origin + uv_size * sideu);
	array.push_back(uv_origin + uv_size * sidev);
	array.push_back(uv_origin + uv_size * sideu);
	array.push_back(uv_origin + uv_size);
	array.push_back(uv_origin + uv_size * sidev);
}

void NixieFont::_push_quad(PoolVector2Array &array, const Point2 &origin, const Size2 &size) {
	const Vector2 sidex(size.width, 0);
	const Vector2 sidey(0, size.height);

	// 0--1,2
	// | // |
	// 2,4--3
	array.push_back(origin);
	array.push_back(origin + sidex);
	array.push_back(origin + sidey);

	array.push_back(origin + sidex);
	array.push_back(origin + sidex + sidey);
	array.push_back(origin + sidey);
}

#define _push_quad_color(arr, c) arr.push_multi(6, Color(1, 1, 1, c / 255.0));

void NixieFont::_update_animation() {
	if (_dirty) {
		_text_rect_size = _calculate_text_rect(draw_text, &_visible_chars) * _char_size;

		_control.resize(_visible_chars);
		auto w = _control.write();
		int index = 0;
		for (int i = 0; i < _control.size(); i++) {
			w[index].phase = 0;
			w[index].age = 0 - (Math::rand() & 0x7f); // random initial delay
			w[index].alt = 0;
			w[index].alt_age = Math::rand() & 0x7f;
			index++;
		}
		_dirty = false;
		item_rect_changed();
	}

	PoolVector2Array v, uv;
	PoolColorArray c;

	auto wr = _control.write();

	const int char_w = _char_size.width;
	const int char_h = _char_size.height;
	Point2 pos(0, 0);
	int control = 0;
	for (int p = 0; p < draw_text.size(); p++) {
		const CharType &chr = draw_text[p];

		if (chr == ' ') {
			pos.x += char_w;
			continue; // space
		}
		if (chr == '\n') {
			pos.x = 0;
			pos.y += char_h;
			continue; // new line
		}
		if (chr < 32) {
			continue; // unknown
		}

		ERR_FAIL_INDEX(control, _control.size());

		CharControl &ctl = wr[control];
		const Rect2 &tex_coords = _frames[uint8_t(chr) - 32];
		_push_quad(v, pos, _char_size);
		_push_texture(uv, tex_coords);
		_push_quad_color(c, LightBlinkingPattern[ctl.phase].alpha);
		if (ctl.alt_age > 0) {
			ctl.alt_age--;
		} else {
			if (broken_tube_effect) {
				// display "broken" tube effect for some frames
				const char alt_chr = ctl.alt ? ctl.alt : 0x10 + (rand() % 10);
				const Rect2 &alt_tex_coords = _frames[alt_chr];
				const uint8_t alpha = LightBlinkingPattern[ctl.phase].alpha >> 2;
				_push_quad(v, pos, _char_size);
				_push_texture(uv, alt_tex_coords);
				_push_quad_color(c, alpha);
				switch (ctl.alt_age--) {
					case 0:
						ctl.alt = alt_chr;
						break;
					default:
						if (ctl.alt_age == -broken_tube_effect) {
							ctl.alt = 0;
							ctl.alt_age = 0x80 + ((rand() & 0xff) << 1); // schedule next event
						}
						break;
				}
			}
		}
		pos.x += char_w;

		ctl.age++;
		if (ctl.age > LightBlinkingPattern[ctl.phase].t) {
			ctl.phase++;
			if (LightBlinkingPattern[ctl.phase].t == -1)
				ctl.phase = 0;
			ctl.age = 0;
		}

		control++;
	}

	// build final mesh
	if (v.size() > 0) {
		Array mesh_array;
		mesh_array.resize(VS::ARRAY_MAX);
		mesh_array[VS::ARRAY_VERTEX] = v;
		if (c.size() > 0)
			mesh_array[VS::ARRAY_COLOR] = c;
		if (uv.size() > 0)
			mesh_array[VS::ARRAY_TEX_UV] = uv;
		if (_mesh.is_valid())
			_mesh->clear_mesh();
		else
			_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
	}
}

String NixieFont::get_text() {
	return draw_text;
}

void NixieFont::set_text(String p_text) {
	if (draw_text != p_text) {
		draw_text = p_text;
		if (draw_text.empty()) {
			_visible_chars = 0;
			_text_rect_size = Size2(0, 0);
			_mesh = Ref<ArrayMesh>(nullptr);
			_dirty = false;
		} else {
			_dirty = true;
		}
		update();
	}
}

void NixieFont::set_align(Align p_align) {
	ERR_FAIL_INDEX((int)p_align, AlignCount);

	align = p_align;
	update();
}

NixieFont::Align NixieFont::get_align() const {
	return align;
}

bool NixieFont::get_enable_animation() {
	return enable_animation;
}

void NixieFont::set_enable_animation(bool p_state) {
	if (p_state != enable_animation) {
		enable_animation = p_state;
		set_process(enable_animation);
		update();
	}
}

int NixieFont::get_animation_speed() {
	return animation_speed;
}

void NixieFont::set_animation_speed(int p_speed) {
	ERR_FAIL_COND(p_speed < 0 || p_speed > 10);

	if (animation_speed != p_speed) {
		animation_speed = _animation_delay = p_speed;
		update();
	}
}

int NixieFont::get_broken_tube_effect() {
	return broken_tube_effect;
}

void NixieFont::set_broken_tube_effect(int p_duration) {
	ERR_FAIL_COND(p_duration < 0 || p_duration > 10);

	if (p_duration != broken_tube_effect) {
		broken_tube_effect = p_duration;
	}
}

void NixieFont::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			const EmbedImageItem &e = embed_nixie_font[0];

			ERR_FAIL_COND(e.pixels == NULL);
			ERR_FAIL_COND(e.channels < 3);

			Ref<Image> font_image = memnew(Image);
			PoolByteArray data;
			data.resize(e.size);
			std::memcpy(data.write().ptr(), e.pixels, e.size);
			font_image->create(e.width, e.height, false, e.channels == 4 ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8, data);
			_texture = Ref<ImageTexture>(memnew(ImageTexture()));
			_texture->create_from_image(font_image);
			_char_size = Size2((e.width / NixieFontCols) - 1, e.height / NixieFontRows);
			_frames = _build_tiles(Size2(NixieFontCols, NixieFontRows), NixieFontChars);

			item_rect_changed();
		} break;
		case NOTIFICATION_PROCESS: {
			if (enable_animation) {
				if (_visible_chars || _dirty) {
					if (_animation_delay)
						_animation_delay--;
					else {
						_update_animation();
						_animation_delay = animation_speed;
						update();
					}
				}
			}
		} break;
		case NOTIFICATION_DRAW: {
			if (_dirty)
				_update_animation();
			if (_mesh.is_valid() && _visible_chars) {
				draw_mesh(_mesh, _texture, Ref<Texture>());
			}
		} break;
	}
}

void NixieFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_text"), &NixieFont::get_text);
	ClassDB::bind_method(D_METHOD("set_text"), &NixieFont::set_text);
	ClassDB::bind_method(D_METHOD("set_align", "align"), &NixieFont::set_align);
	ClassDB::bind_method(D_METHOD("get_align"), &NixieFont::get_align);
	ClassDB::bind_method(D_METHOD("set_enable_animation"), &NixieFont::set_enable_animation);
	ClassDB::bind_method(D_METHOD("get_enable_animation"), &NixieFont::get_enable_animation);
	ClassDB::bind_method(D_METHOD("set_animation_speed"), &NixieFont::set_animation_speed);
	ClassDB::bind_method(D_METHOD("get_animation_speed"), &NixieFont::get_animation_speed);
	ClassDB::bind_method(D_METHOD("set_broken_tube_effect"), &NixieFont::set_broken_tube_effect);
	ClassDB::bind_method(D_METHOD("get_broken_tube_effect"), &NixieFont::get_broken_tube_effect);

	BIND_ENUM_CONSTANT(ALIGN_LEFT);
	BIND_ENUM_CONSTANT(ALIGN_CENTER);
	BIND_ENUM_CONSTANT(ALIGN_RIGHT);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "align", PROPERTY_HINT_ENUM, "Left,Center,Right"), "set_align", "get_align");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_animation"), "set_enable_animation", "get_enable_animation");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "animation_speed", PROPERTY_HINT_RANGE, "0,10,1,or_lesser,or_greater"), "set_animation_speed", "get_animation_speed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "broken_tube_effect", PROPERTY_HINT_RANGE, "0,10,1,or_lesser,or_greater"), "set_broken_tube_effect", "get_broken_tube_effect");
}

NixieFont::NixieFont() {
	draw_text = "";
	animation_speed = 4;
	enable_animation = false;
	broken_tube_effect = 5;
	_texture = Ref<Texture>(NULL);
	_animation_delay = animation_speed;
	_dirty = false;
	_char_size = Size2(1, 1);
	_control = PoolVector<CharControl>();
	_char_size = Size2(1, 1);
	_frames = PoolVector<Rect2>();
	_text_rect_size = Size2(1, 1);
}

NixieFont::~NixieFont() {
}
