/*************************************************************************/
/*  debug_console.cpp                                                    */
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

#include "debug_console.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "core/version.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

#include "inc/gd_core.h"

typedef struct {
	const char *image;
	const unsigned char *pixels;
	int char_w, char_h, size, width, height, channels;
} EmbedImageItem;

#define FORMAT_1CHANNEL 1
#define FORMAT_3CHANNEL 3
#define FORMAT_4CHANNEL 4

extern const unsigned char font_8x16[], font_8x12[], font_8x8[], font_7x9[], font_4x6[];
constexpr EmbedImageItem embed_debug_font[] = {
	{ "dos-8x16", font_8x16, 8, 16, 196608 / 3, 128, 512, FORMAT_1CHANNEL },
	{ "dos-8x12", font_8x12, 8, 12, 147456 / 3, 128, 384, FORMAT_1CHANNEL },
	{ "dos-8x8", font_8x8, 8, 8, 98304 / 3, 128, 256, FORMAT_1CHANNEL },
	{ "dos-7x9", font_7x9, 7, 9, 96768 / 3, 112, 288, FORMAT_1CHANNEL },
	{ "dos-4x6", font_4x6, 4, 6, 36864 / 3, 64, 192, FORMAT_1CHANNEL },
	{ NULL, NULL, 0, 0, 0, 0, 0, 0 }
};

// LCD text with big letters.
// --------------------------
const int BoxFSize = 3;
extern int boxf_offsets[];
extern char *boxf[];

static PoolByteArray _poolbytearray_from_data(const uint8_t *bytes, size_t bytes_size, int bytes_channels, Image::Format dest_format) {
	PoolByteArray data;
	if (bytes_channels == FORMAT_1CHANNEL && dest_format == Image::Format::FORMAT_LA8) {
		data.resize(2 * bytes_size);
		PoolByteArray::Write wr = data.write();
		for (size_t b = 0; b < 2 * bytes_size; b += 2) {
			wr[b] = wr[b + 1] = bytes[b >> 1];
		}
	} else if (bytes_channels == FORMAT_1CHANNEL && dest_format == Image::Format::FORMAT_RGB8) {
		data.resize(3 * bytes_size);
		PoolByteArray::Write wr = data.write();
		for (size_t b = 0; b < 3 * bytes_size; b += 3) {
			wr[b] = wr[b + 1] = wr[b + 3] = bytes[b / 3];
		}
	} else if (bytes_channels == FORMAT_1CHANNEL && dest_format == Image::Format::FORMAT_RGBA8) {
		data.resize(4 * bytes_size);
		PoolByteArray::Write wr = data.write();
		for (size_t b = 0; b < 4 * bytes_size; b += 4) {
			wr[b] = wr[b + 1] = wr[b + 3] = wr[b + 4] = bytes[b >> 2];
		}
	} else {
		data.resize(bytes_size);
		memcpy(data.write().ptr(), bytes, bytes_size);
	}
	return data;
}

static Ref<Texture> _font_texture_cache[TextConsole::DOS_FONT_MAX];

void TextConsole::load_font(FontSize p_font) {
	ERR_FAIL_INDEX(p_font, DOS_FONT_MAX);

	const EmbedImageItem &data = embed_debug_font[p_font];

	if (!_font_texture_cache[p_font].is_valid()) {
		constexpr Image::Format _image_format[] = {
			Image::Format::FORMAT_MAX,
			Image::Format::FORMAT_LA8,
			Image::Format::FORMAT_LA8,
			Image::Format::FORMAT_RGBA8,
			Image::Format::FORMAT_RGBA8,
		};

		Ref<Image> image = Ref<Image>(memnew(Image()));
		image->create(data.width, data.height, false, _image_format[data.channels], _poolbytearray_from_data(data.pixels, data.size, data.channels, _image_format[data.channels]));
		Ref<ImageTexture> font_texture = Ref<ImageTexture>(memnew(ImageTexture()));
		font_texture->create_from_image(image, 0);
		_font_texture_cache[p_font] = font_texture;
	}

	_font_texture = _font_texture_cache[p_font];
	_font_size = Size2i(data.char_w, data.char_h);

	/* 16x32 chars */ for (int loop = 0; loop < 512; ++loop) // loop through all 512 chars
	{
		real_t cx = (real_t)(loop % 16) / 16.0f; // X position of current character
		real_t cy = (real_t)(loop / 16) / 32.0f; // Y position of current character

		const int col = loop % 16, row = loop / 16, ch = row * 16 + col;

		_chars[ch].t[0] = Point2(cx, cy); /* 0, 0 */
		_chars[ch].t[1] = Point2(cx, cy + 0.03125f); /* 0, 1 */
		_chars[ch].t[2] = Point2(cx + 0.0625f, cy); /* 1, 0 */
		_chars[ch].t[3] = Point2(cx + 0.0625f, cy + 0.03125f); /* 1, 1 */
	}
}

void TextConsole::resize(const Viewport *p_view) {
	ERR_FAIL_COND(p_view == nullptr);

	const Size2i size = p_view->get_size();
	// screen size rounded to font size
	const int screen_width = size.width - size.width % int(_font_size.width);
	const int screen_height = size.height - size.height % int(_font_size.height);
	// console size:
	resize(screen_width / _font_size.width, screen_height / _font_size.height);
}

void TextConsole::resize(int p_width, int p_height) {
	ERR_FAIL_COND(p_width < 1);
	ERR_FAIL_COND(p_height < 1);

	_con_size.width = p_width;
	_con_size.height = p_height;

	if (_screen)
		memdelete_arr(_screen);
	_screen = memnew_arr(cell, _con_size.width * _con_size.height);
	memset(_screen, 0, sizeof(cell) * _con_size.width * _con_size.height);
}

static Color palette[16] = {
	/*  0. BLACK        */ Color::from_abgr(0xFF000000),
	/*  1. BLUE         */ Color::from_abgr(0xFFFF0000),
	/*  2. GREEN        */ Color::from_abgr(0xFF00FF00),
	/*  3. CYAN         */ Color::from_abgr(0xFFFFFF00),
	/*  4. RED          */ Color::from_abgr(0xFF0000FF),
	/*  5. MAGENTA      */ Color::from_abgr(0xFFFF00FF),
	/*  6. BROWN        */ Color::from_abgr(0xFF2A2AA5),
	/*  7. LIGHTGRAY    */ Color::from_abgr(0xFFD3D3D3),
	/*  8. DARKGRAY     */ Color::from_abgr(0xFFA9A9A9),
	/*  9. LIGHTBLUE    */ Color::from_abgr(0xFFE6D8AD),
	/* 10. LIGHTGREEN   */ Color::from_abgr(0xFF90EE90),
	/* 11. LIGHTCYAN    */ Color::from_abgr(0xFFFFFFE0),
	/* 12. LIGHTRED     */ Color::from_abgr(0xFFCBCCFF),
	/* 13. LIGHTMAGENTA */ Color::from_abgr(0xFFF942FF),
	/* 14. YELLOW       */ Color::from_abgr(0xFF0FFEFF),
	/* 15. WHITE        */ Color::from_abgr(0xFFFFFFFF)
};

Point2i TextConsole::_write(const String &p_msg, Point2i pos, ColorIndex foreground, ColorIndex background) {
	ERR_FAIL_COND_V(_screen == 0, pos);
	ERR_FAIL_COND_V(pos.x >= _con_size.width, pos);
	ERR_FAIL_COND_V(pos.y >= _con_size.height, pos);

	CharString ascii = p_msg.ascii();
	for (int i = 0; i < ascii.size(); ++i) {
		const cell c = { ascii[i], foreground, background, 0 };
		_screen[pos.y * _con_size.width + pos.x] = c;
		pos.x++;
		if (pos.x == _con_size.width)
			pos = Point2i(0, pos.y + 1);
		if (pos.y == _con_size.height) {
			_scroll_up();
			pos = Point2i(pos.x, pos.y - 1);
		}
	}
	_dirty_screen = true;
	return pos;
}

Point2i TextConsole::_putl(const String &p_msg, Point2i pos, ColorIndex foreground, ColorIndex background) {
	ERR_FAIL_COND_V(_screen == 0, pos);
	ERR_FAIL_COND_V(pos.x >= _con_size.width, pos);
	ERR_FAIL_COND_V(pos.y >= _con_size.height, pos);

	CharString ascii = p_msg.ascii();
	for (int i = 0; i < ascii.length(); ++i) {
		const cell c = { ascii[i], foreground, background, 0 };
		if (pos.x < _con_size.width) {
			_screen[pos.y * _con_size.width + pos.x] = c;
			pos.x++;
		}
	}
	_dirty_screen = true;
	return pos;
}

Point2i TextConsole::_putf(const String &p_msg, Point2i pos, ColorIndex foreground, ColorIndex background) {
	ERR_FAIL_COND_V(_screen == 0, pos);
	ERR_FAIL_COND_V(pos.x >= _con_size.width, pos);
	ERR_FAIL_COND_V(pos.y >= _con_size.height, pos);

	static Point2i v1 = Point2i(0, 1);

	CharString ascii = p_msg.ascii();
	for (int r = 0; r < BoxFSize; ++r) {
		for (int c = 0; c < ascii.length(); ++c) {
			const uint8_t ch = ascii[c];
			if (ch == ' ') {
				_putl("  ", pos + Point2i(c * BoxFSize, 0), foreground, background);
			} else {
				_putl(boxf[boxf_offsets[ch] + r], pos + Point2i(c * BoxFSize, 0), foreground, background);
			}
		}
		if (pos.y + 1 == _con_size.height) {
			_scroll_up();
		} else {
			pos += v1;
		}
	}
	return pos;
}

void TextConsole::_scroll_up(int p_scroll_lines) {
	ERR_FAIL_COND(p_scroll_lines <= 0);
	ERR_FAIL_COND(_screen == 0);

	const int cell_size = sizeof(cell);
	memmove(_screen, _screen + _con_size.width * p_scroll_lines, (_con_size.height - p_scroll_lines) * _con_size.width * cell_size * p_scroll_lines);
	memset(_screen + (_con_size.height - p_scroll_lines) * _con_size.width, 0, _con_size.width * cell_size * p_scroll_lines);
}

void TextConsole::_update_mesh() {
	ERR_FAIL_COND(_screen == 0);

	// transparent color:
	static Color transparent(0, 0, 0, 0);
	// char corners:
	static const Point2i v0(0, 0); /* 0, 0 */
	static const Point2i v1(0, _font_size.height); /* 0, 1 */
	static const Point2i v2(_font_size.width, 0); /* 1, 0 */
	static const Point2i v3(_font_size.width, _font_size.height); /* 1, 1 */

	PoolVector2Array vertices;
	PoolColorArray bg_colors, fg_colors;
	PoolVector2Array textures;

	Point2i xx;
	cell *p = _screen;
	for (int row = 0; row < _con_size.height; ++row) {
		for (int col = 0; col < _con_size.width; ++col) {
			if (p->character) {
				const uint8_t fg = p->foreground;
				const uint8_t bg = p->background;

				const int tex_info = uint8_t(p->character) + (p->inverted ? 0 : 256);
				vertices.push_back(xx + v0);
				vertices.push_back(xx + v1);
				vertices.push_back(xx + v2);
				vertices.push_back(xx + v1);
				vertices.push_back(xx + v2);
				vertices.push_back(xx + v3);
				// bg:
				if (bg == transparent_color_index)
					bg_colors.push_multi(6, transparent);
				else
					bg_colors.push_multi(6, palette[bg]);
				// fg:
				textures.push_back(_chars[tex_info].t[0]);
				textures.push_back(_chars[tex_info].t[1]);
				textures.push_back(_chars[tex_info].t[2]);
				textures.push_back(_chars[tex_info].t[1]);
				textures.push_back(_chars[tex_info].t[2]);
				textures.push_back(_chars[tex_info].t[3]);
				fg_colors.push_multi(6, palette[fg]);
			}
			xx += v2;
			p++;
		}
		xx = Point2i(0, xx.y + _font_size.height);
	}

	Array background, foreground;
	background.resize(VS::ARRAY_MAX);
	background[VS::ARRAY_VERTEX] = vertices;
	background[VS::ARRAY_COLOR] = bg_colors;
	foreground.resize(VS::ARRAY_MAX);
	foreground[VS::ARRAY_VERTEX] = vertices;
	foreground[VS::ARRAY_TEX_UV] = textures;
	foreground[VS::ARRAY_COLOR] = fg_colors;

	if (vertices.size() == 0) {
		_mesh = Ref<ArrayMesh>();
	} else {
		_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, background, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
		_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, foreground, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
	}
}

void TextConsole::draw(RID p_canvas_item, const Transform2D &p_xform) {
	ERR_FAIL_COND(!_font_texture.is_valid());

	if (_dirty_screen) {
		_update_mesh();
		_dirty_screen = false;
	}

	if (_mesh.is_valid()) {
		RID texture_rid = _font_texture->get_rid();
		VS::get_singleton()->canvas_item_add_mesh(p_canvas_item, _mesh->get_rid(), p_xform, Color(1, 1, 1, 1), texture_rid, RID());
	}
}

void TextConsole::set_pixel_ratio(real_t p_scale) {
	pixel_scale = p_scale;
}

void TextConsole::logl(const String &p_msg) {

	logl(p_msg, _default_fg_color_index, _default_bg_color_index);
}

void TextConsole::logl(const String &p_msg, ColorIndex foreground, ColorIndex background) {

	// next line
	_cursor_pos = Point2i(0, _putl(p_msg, _cursor_pos, _default_fg_color_index, _default_bg_color_index).y + 1);
	if (_cursor_pos.y == _con_size.height) {
		_scroll_up();
		_cursor_pos = Point2i(0, _cursor_pos.y - 1);
	}
}

void TextConsole::logf(const String &p_msg) {

	logf(p_msg, _default_fg_color_index, _default_bg_color_index);
}

void TextConsole::logf(const String &p_msg, ColorIndex foreground, ColorIndex background) {

	// next line
	_cursor_pos = Point2i(0, _putf(p_msg, _cursor_pos, _default_fg_color_index, _default_bg_color_index).y + 1);
	if (_cursor_pos.y == _con_size.height) {
		_scroll_up();
		_cursor_pos = Point2i(0, _cursor_pos.y - 1);
	}
}

void TextConsole::logv(const Array &p_log) {
	Array log_msg = p_log;
	Point2i pos = _cursor_pos;
	while (!log_msg.empty()) {
		String msg = log_msg.pop_front();
		const int foreground = int(log_msg.pop_front());
		const int background = int(log_msg.pop_front());
		pos = _putl(msg, pos,
				foreground == COLOR_DEFAULT ? _default_fg_color_index : ColorIndex(foreground),
				background == COLOR_DEFAULT ? _default_bg_color_index : ColorIndex(background)
		);
	}
	_cursor_pos = Point2i(0, pos.y + 1);
	if (_cursor_pos.y == _con_size.height) {
		_scroll_up();
		_cursor_pos = Point2i(0, _cursor_pos.y - 1);
	}
}

TextConsole::TextConsole() {

	_dirty_screen = false;
	_cursor_pos = Point2i(0, 0);
	_default_bg_color_index = COLOR_BLACK, _default_fg_color_index = COLOR_WHITE;
	_screen = 0;

	transparent_color_index = 0;

	resize(80, 25);
	logl(BOX_DDR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DDL, COLOR_LIGHTGRAY);
	logl(BOX_DUD "   Godot Engine debug console   " BOX_DUD, COLOR_LIGHTGRAY);
	logl(BOX_DUD "     KomSoft Oprogramowanie     " BOX_DUD, COLOR_LIGHTGRAY);
	logl(BOX_DUR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DLR BOX_DUL, COLOR_LIGHTGRAY);
	logl("\020 " VERSION_FULL_NAME);
	logl("\020 Hello!");
	logf("2021");
}

TextConsole::~TextConsole() {

	memdelete_arr(_screen);
}

#ifdef TOOLS_ENABLED
void ConsoleInstance::_edit_set_position(const Point2 &p_position) {
	_pos = p_position;
	update();
}

Point2 ConsoleInstance::_edit_get_position() const {
	return _pos;
}

void ConsoleInstance::_edit_set_scale(const Size2 &p_scale) {
	_scale = p_scale;
	update();
}

Size2 ConsoleInstance::_edit_get_scale() const {
	return _scale;
}
#endif

Transform2D ConsoleInstance::get_transform() const {
	return _xform;
}

void ConsoleInstance::_notification(int p_notification) {

	switch (p_notification) {

		case NOTIFICATION_READY: {

			if (!console.is_valid()) {
				console = Ref<TextConsole>(memnew(TextConsole()));
				console->load_font(TextConsole::DOS_8x12);
			}
		} break;

		case NOTIFICATION_DRAW: {

			if (console.is_valid()) {
				console->draw(get_canvas_item(), _xform);
			}
		} break;
	}
}

void ConsoleInstance::console_msg(const String &p_msg) {
	ERR_FAIL_COND(!console.is_valid());

	// parse control characters
	_process_codes(p_msg);
	update();
}

void ConsoleInstance::console_resize(const Viewport *p_view) {
	ERR_FAIL_COND(!console.is_valid());

	console->resize(p_view);
	update();
}

Error ConsoleInstance::_process_codes(const String &p_concodes) {

	static const String colors_value[] = {
		"black",
		"blue",
		"green",
		"cyan",
		"red",
		"magenta",
		"brown",
		"lightgray",
		"darkgray",
		"lightblue",
		"lightgreen",
		"lightcyan",
		"lightred",
		"lightmagenta",
		"yellow",
		"white",
		"default",
	};

	Array log_msg;
	List<String> tag_stack;
	int pos = 0;
	TextConsole::ColorIndex fg = TextConsole::COLOR_DEFAULT, bg = TextConsole::COLOR_DEFAULT;

#define _append_log_msg(MSG, FG, BG) \
	{                                \
		log_msg.push_back(MSG);      \
		log_msg.push_back(FG);       \
		log_msg.push_back(BG);       \
	}

	while (pos < p_concodes.length()) {

		int brk_pos = p_concodes.find("[", pos);

		if (brk_pos < 0)
			brk_pos = p_concodes.length();
		if (brk_pos > pos)
			_append_log_msg(p_concodes.substr(pos, brk_pos - pos), fg, bg);
		if (brk_pos == p_concodes.length())
			break; //nothing else to add

		const int brk_end = p_concodes.find("]", brk_pos + 1);

		if (brk_end == -1) {
			//no close, add the rest
			_append_log_msg(p_concodes.substr(brk_pos, p_concodes.length() - brk_pos), fg, bg);
			break;
		}

		String tag = p_concodes.substr(brk_pos + 1, brk_end - brk_pos - 1);
		Vector<String> split_tag_block = tag.split(" ", false);

		if (tag.begins_with("/") && tag_stack.size()) {

			bool tag_ok = tag_stack.size() && tag_stack.front()->get() == tag.substr(1, tag.length());

			if (tag_stack.front()->get() == "fg")
				fg = TextConsole::COLOR_DEFAULT;
			if (tag_stack.front()->get() == "bg")
				bg = TextConsole::COLOR_DEFAULT;

			if (!tag_ok) {
				_append_log_msg("[" + tag, fg, bg);
				pos = brk_end;
				continue;
			}

			pos = brk_end + 1;
			tag_stack.pop_front();

		} else if (tag.begins_with("fg=") || tag.begins_with("bg=")) {

			String col = tag.substr(3, tag.length());
			int color_index = -1;

			for (int c = 0; c < TextConsole::COLOR_MAX; ++c) {
				if (col == colors_value[c]) {
					color_index = c;
					break;
				}
			}

			if (color_index == -1) {
				WARN_PRINT("Unknown color name: " + col);
			} else {
				if (tag.begins_with("fg="))
					fg = TextConsole::ColorIndex(color_index);
				else if (tag.begins_with("bg="))
					bg = TextConsole::ColorIndex(color_index);
			}

			pos = brk_end + 1;
			tag_stack.push_front(tag.substr(0, 2));
		}
	}
	if (!log_msg.empty()) {
		console->logv(log_msg);
	}
	return OK;
}

Ref<TextConsole> ConsoleInstance::get_console() const {
	return console;
}

void ConsoleInstance::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_console"), &ConsoleInstance::get_console);

	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_BLACK, "COLOR_BLACK");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_BLUE, "COLOR_BLUE");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_GREEN, "COLOR_GREEN");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_RED, "COLOR_RED");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_MAGENTA, "COLOR_MAGENTA");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_BROWN, "COLOR_BROWN");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_LIGHTGRAY, "COLOR_LIGHTGRAY");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_LIGHTBLUE, "COLOR_LIGHTBLUE");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_LIGHTGREEN, "COLOR_LIGHTGREEN");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_LIGHTCYAN, "COLOR_LIGHTCYAN");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_LIGHTRED, "COLOR_LIGHTRED");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_LIGHTMAGENTA, "COLOR_LIGHTMAGENTA");
	BIND_ENUM_CONSTANT_CUSTOM(TextConsole::COLOR_WHITE, "COLOR_WHITE");
}
