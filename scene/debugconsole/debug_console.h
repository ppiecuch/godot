/**************************************************************************/
/*  debug_console.h                                                       */
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

#ifndef DEBUG_CONSOLE_H
#define DEBUG_CONSOLE_H

#include "core/reference.h"
#include "scene/2d/node_2d.h"
#include "scene/main/viewport.h"

#include <stdint.h>

/* Windows code page 437 box drawing characters */
#define BOX_DLR "\315" /* ═ */
#define BOX_DUD "\272" /* ║ */
#define BOX_DUL "\274" /* ╝ */
#define BOX_DUR "\310" /* ╚ */
#define BOX_DDL "\273" /* ╗ */
#define BOX_DDR "\311" /* ╔ */
#define BOX_DUDL "\271" /* ╣ */
#define BOX_DUDR "\314" /* ╠ */
#define BOX_DULR "\312" /* ╩ */
#define BOX_DDLR "\313" /* ╦ */
#define BOX_DUDLR "\316" /* ╬ */
#define BOX_DU_SL "\275" /* ╜, not in CP850 */
#define BOX_DU_SR "\323" /* ╙, not in CP850 */
#define BOX_DD_SL "\267" /* ╖, not in CP850 */
#define BOX_DD_SR "\326" /* ╓, not in CP850 */
#define BOX_DL_SU "\276" /* ╛, not in CP850 */
#define BOX_DL_SD "\270" /* ╕, not in CP850 */
#define BOX_DR_SU "\324" /* ╘, not in CP850 */
#define BOX_DR_SD "\325" /* ╒, not in CP850 */
#define BOX_DU_SLR "\320" /* ╨, not in CP850 */
#define BOX_DD_SLR "\322" /* ╥, not in CP850 */
#define BOX_DL_SUD "\265" /* ╡, not in CP850 */
#define BOX_DR_SUD "\306" /* ╞, not in CP850 */
#define BOX_DLR_SU "\317" /* ╧, not in CP850 */
#define BOX_DLR_SD "\321" /* ╤, not in CP850 */
#define BOX_DLR_SUD "\330" /* ╪, not in CP850 */
#define BOX_DUD_SL "\266" /* ╢, not in CP850 */
#define BOX_DUD_SR "\307" /* ╟, not in CP850 */
#define BOX_DUD_SLR "\327" /* ╫, not in CP850 */
#define BOX_SLR "\304" /* ─ */
#define BOX_SUD "\263" /* │ */
#define BOX_SUL "\331" /* ┘ */
#define BOX_SUR "\300" /* └ */
#define BOX_SDL "\277" /* ┐ */
#define BOX_SDR "\332" /* ┌ */
#define BOX_SULR "\301" /* ┴ */
#define BOX_SDLR "\302" /* ┬ */
#define BOX_SUDL "\264" /* ┤ */
#define BOX_SUDR "\303" /* ├ */
#define BOX_SUDLR "\305" /* ┼ */

struct TextConsole : public Reference {
	enum FontSize {
		DOS_8x16,
		DOS_8x12,
		DOS_8x8,
		DOS_7x9,
		DOS_4x6,
		DosFontCount,
	};
	enum FigFontFace {
		FIG_FUTURE,
		FIG_CALVIS_S,
		FIG_ANSI_REGULAR,
		FIG_DOS_REBEL,
		FIG_MAXIWI,
		FIG_MAXII,
		FigFontFaceCount,
	};
	enum ColorIndex {
		COLOR_BLACK,
		COLOR_BLUE,
		COLOR_GREEN,
		COLOR_CYAN,
		COLOR_RED,
		COLOR_MAGENTA,
		COLOR_BROWN,
		COLOR_LIGHTGRAY,
		COLOR_DARKGRAY,
		COLOR_LIGHTBLUE,
		COLOR_LIGHTGREEN,
		COLOR_LIGHTCYAN,
		COLOR_LIGHTRED,
		COLOR_LIGHTMAGENTA,
		COLOR_YELLOW,
		COLOR_WHITE,
		COLOR_TRANSPARENT,
		COLOR_DEFAULT,
		COLOR_MAX_VALID = COLOR_TRANSPARENT,
		COLOR_COUNT = COLOR_DEFAULT
	};

	void _update_mesh();
	Point2i _write(const String &p_msg, Point2i pos, ColorIndex foreground, ColorIndex background);
	Point2i _put(const String &p_msg, Point2i pos, ColorIndex foreground, ColorIndex background);
	void _scroll_up(int p_scroll_lines = 1);

	void load_font(FontSize p_font);
	bool resize(int p_cols, int p_rows);
	bool resize(const Viewport *p_view);
	void draw(RID p_canvas_item, const Transform2D &p_xform);
	void set_pixel_ratio(real_t p_scale);

	void logl(const String &p_msg);
	void logl(const String &p_msg, ColorIndex foreground, ColorIndex background = COLOR_DEFAULT);
	void logf(FigFontFace p_face, const String &p_msg);
	void logf(FigFontFace p_face, const String &p_msg, ColorIndex foreground, ColorIndex background = COLOR_DEFAULT);
	void logv(const Array &p_log);

	TextConsole();
	~TextConsole();

	Ref<ArrayMesh> _mesh;
	bool _dirty_screen;
	struct cell {
		char character;
		ColorIndex foreground : 5;
		ColorIndex background : 5;
		bool inverted : 1;
	};
	cell *_screen;
	FontSize _font_face;
	Size2i _font_size, _con_size;
	Point2i _cursor_pos;
	ColorIndex _default_bg_color_index, _default_fg_color_index;
	// texture font info (2x256 chars) cache:
	struct _char_info_t {
		Point2 t[4];
	} _chars[512];
	real_t pixel_scale;
};

class ConsoleInstance : public CanvasItem {
	GDCLASS(ConsoleInstance, CanvasItem);

private:
	Ref<TextConsole> console;

	Error _process_codes(const String &p_concodes);

protected:
	void _notification(int p_what);

	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	void _edit_set_position(const Point2 &p_position);
	Point2 _edit_get_position() const;
	void _edit_set_scale(const Size2 &p_scale);
	Size2 _edit_get_scale() const;
#endif

	Transform2D get_transform() const;

	void console_msg(const String &p_msg);
	void console_resize(const Viewport *p_view);

	Ref<TextConsole> get_console() const;

	ConsoleInstance();
};

VARIANT_ENUM_CAST(TextConsole::ColorIndex);
VARIANT_ENUM_CAST(TextConsole::FigFontFace);

#endif // DEBUG_CONSOLE_H
