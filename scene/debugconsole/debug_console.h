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

#include "core/func_ref.h"
#include "core/os/mutex.h"
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
		FIG_CALVIN_S,
		FIG_ANSI_REGULAR,
		FIG_ANSI_SHADOW,
		FIG_DOS_REBEL,
		FIG_MAXIWI,
		FIG_MINIWI,
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

	// Atlas banks. The texture is BANK_COUNT half-atlases stacked vertically.
	enum Bank {
		BANK_CP437, // the 256 stock glyphs
		BANK_PATCH, // hand-drawn patches; codes without one are drawn inverted from BANK_CP437
		BANK_COUNT,
	};

	void _update_mesh();
	Point2i _write(const String &p_msg, Point2i pos, ColorIndex foreground, ColorIndex background);
	Point2i _put(const String &p_msg, Point2i pos, ColorIndex foreground, ColorIndex background);
	void _scroll_up(int p_scroll_lines = 1);

	// Direct cell-memory writes: the console's framebuffer poke, used by widgets (see
	// console_lcd.{h,cpp}) that draw at an absolute grid position rather than in the
	// line-oriented log flow. Unlike _write()/_put() these never scroll and never ERR_FAIL
	// on out-of-range coordinates -- a widget clips silently instead of indexing past the
	// grid. All of them mark the grid dirty so both render backends pick up the change.
	void put_cell(int p_x, int p_y, uint8_t p_code, ColorIndex p_fg, ColorIndex p_bg, Bank p_bank = BANK_CP437);
	void put_text(int p_x, int p_y, const String &p_text, ColorIndex p_fg, ColorIndex p_bg);
	void fill_rect(int p_x, int p_y, int p_w, int p_h, uint8_t p_code, ColorIndex p_fg, ColorIndex p_bg, Bank p_bank = BANK_CP437);

	void load_font(FontSize p_font);

	// Release the shared font atlases. The cache is a file-scope array of Ref<Texture>, so
	// without this it is torn down by the static destructors, which run after the servers are
	// gone -- ~ImageTexture() then frees a RID through a VisualServer that no longer exists.
	// Called from unregister_scene_types(); safe to call more than once, and safe to call when
	// nothing was ever loaded.
	static void cleanup_font_cache();
	void clear(); // blank the grid and home the cursor
	void print_banner(); // engine/version box, printed at the cursor
	Point2i get_cursor() const { return _cursor_pos; }
	void set_cursor(const Point2i &p_pos) { _cursor_pos = p_pos; }
	bool resize(int p_cols, int p_rows);
	bool resize(const Viewport *p_view);
	void draw(RID p_canvas_item, const Transform2D &p_xform);

	// Integer zoom: every glyph pixel is drawn as a p_scale x p_scale block. These are 1bpp
	// bitmap fonts, so fractional scaling only makes them blurry (CONSOLE.md section 6).
	void set_pixel_scale(int p_scale);
	int get_pixel_scale() const { return pixel_scale; }
	FontSize get_font() const { return _font_face; }
	Size2i get_font_size() const { return _font_size; }
	Size2i get_cell_size() const { return _font_size * pixel_scale; }
	Size2i get_console_size() const { return _con_size; }

	void logl(const String &p_msg);
	void logl(const String &p_msg, ColorIndex foreground, ColorIndex background = COLOR_DEFAULT);
	void logf(FigFontFace p_face, const String &p_msg);
	void logf(FigFontFace p_face, const String &p_msg, ColorIndex foreground, ColorIndex background = COLOR_DEFAULT);
	void logv(const Array &p_log);

	TextConsole();
	~TextConsole();

	Ref<ArrayMesh> _mesh;
	Ref<ArrayMesh> _mesh_bg; // cell backgrounds, drawn untextured (it carries no UVs)
	bool _dirty_screen;
	struct cell {
		char character; // glyph index within the bank, 0..255
		ColorIndex foreground : 5;
		ColorIndex background : 5;
		uint8_t bank : 2; // BANK_CP437 / BANK_PATCH
		bool inverted : 1; // render-time palette swap, independent of the atlas
	};
	cell *_screen;

	// Read a cell back for inspection (widgets and tests); false and r_cell untouched when
	// (p_x, p_y) is off-grid.
	bool get_cell(int p_x, int p_y, cell &r_cell) const;

	// A cell reduced to what any backend needs to draw it: the glyph, which bank it comes
	// from, and the final colours after inversion. Shared by the GPU mesh path and the
	// software blitter (console_raster.h) so the two cannot drift apart.
	struct resolved_cell {
		uint8_t code;
		uint8_t foreground, background;
		uint8_t bank; // which atlas bank the glyph comes from, after inversion is resolved
	};
	bool _resolve_cell(const cell &p_cell, resolved_cell &r_out) const;

	FontSize _font_face;
	bool _has_patch[256]; // bank 1 codes backed by a real glyph, see load_font()
	Size2i _font_size, _con_size;
	Point2i _cursor_pos;
	ColorIndex _default_bg_color_index, _default_fg_color_index;
	// texture font info (BANK_COUNT x 256 chars) cache:
	struct _char_info_t {
		Point2 t[4];
	} _chars[256 * BANK_COUNT];
	int pixel_scale; // integer zoom factor, 1..CONSOLE_MAX_PIXEL_SCALE
};

class ConsoleInstance : public CanvasItem {
	GDCLASS(ConsoleInstance, CanvasItem);

public:
	enum Page {
		PAGE_LOG,
		PAGE_SYSINFO,
		PAGE_PROPS,
		PAGE_MAX,
	};

private:
	// One rendered log line. A line holds several coloured runs, because inline markup
	// ("plain [fg=red]red[/fg] plain") must stay on a single row.
	struct LogSegment {
		String text;
		TextConsole::ColorIndex foreground, background;
	};
	struct LogLine {
		Vector<LogSegment> segments;
	};

	// A row on PAGE_PROPS. The object is held as an ObjectID, never as a raw pointer, so a
	// freed node is detected through ObjectDB instead of crashing the console.
	struct Watch {
		StringName name;
		ObjectID obj; // 0 => static value or FuncRef
		StringName property;
		Ref<FuncRef> getter;
		Variant cached;
		TextConsole::ColorIndex color;
	};

	Ref<TextConsole> console;
	TextConsole::FontSize font_face;
	int pixel_scale;
	bool auto_font; // re-run console_auto_font() on resize; any explicit set_font() pins it

	Page page;
	Vector<LogLine> log_lines; // ring buffer, the source of truth for PAGE_LOG
	Vector<LogSegment> pending; // segments of the line currently being assembled
	int log_capacity;
	Mutex log_mutex; // ConsoleLogger pushes from arbitrary threads (step 5)
	Vector<Watch> watches;

	// Rolling fps samples for the SYSINFO plot. Kept here rather than recomputed from
	// Performance, which only reports the instantaneous value.
	Vector<real_t> fps_history;
	int fps_history_capacity = 32;
	real_t refresh_rate; // Hz, for the polled pages
	real_t refresh_accum;
	bool dirty; // a re-render is pending

	// While an auxiliary screen is attached the grid follows *its* geometry and the game
	// viewport is left alone; see CONSOLE.md section 11.
	bool secondary_active;
	Size2i secondary_size;

public:
	enum SecondaryMode {
		SECONDARY_AUTO, // use the panel when one is attached, overlay otherwise
		SECONDARY_ALWAYS, // panel only; never draw over the game viewport
		SECONDARY_NEVER, // overlay only; ignore the panel entirely
	};

private:
	SecondaryMode secondary_mode;
	static SecondaryMode _secondary_mode_setting();

	// Second-screen gesture state: a short tap advances the page, a long press hides the
	// console again (there is no keyboard on the devices that have such a panel).
	bool touch_down;
	uint64_t touch_down_msec;
	// Taps only mean something when the game opts in (debug/console/panel_touch): an
	// always-live panel reacts to accidental hits and, on Android, its window would have to
	// be focusable to receive them, stealing the gamepad from the game.
	bool panel_touch;
	void _poll_secondary_touch();
	void _blank_secondary(); // paint the panel black while the console is hidden

	static ConsoleInstance *active; // the instance the DebugConsole singleton forwards to
	static Mutex active_mutex; // guards `active` and `backlog` against the logger threads

	// Lines logged before any instance existed; flushed by the first one to enter the tree.
	static Vector<LogSegment> backlog;
	enum { backlog_capacity = 512 };
	void _drain_backlog();

	Error _process_codes(const String &p_concodes);
	void _apply_font(); // reload the face/scale and reflow the grid
	void _viewport_size_changed();
	void _auto_select_font();

	void _append_segment(const String &p_text, TextConsole::ColorIndex p_fg, TextConsole::ColorIndex p_bg);
	void _flush_line(); // commits the pending segments as one log line
	void _commit_line(const Vector<LogSegment> &p_segments);
	void _wrap_line(const LogLine &p_line, int p_width, Vector<LogLine> &r_rows) const;
	void _push_line(const String &p_text, TextConsole::ColorIndex p_fg, TextConsole::ColorIndex p_bg);
	// Commits an already-rendered grid row verbatim, without splitting it on newlines.
	void _push_raw_line(const String &p_text, TextConsole::ColorIndex p_fg, TextConsole::ColorIndex p_bg);
	void _poll_watches();
	void _present_secondary(); // blit the grid onto the platform auxiliary screen, if any
	void _adopt_secondary_geometry(const Size2i &p_size, int p_dpi);

	void _render();
	void _render_header();
	void _render_log();
	void _render_sysinfo();

	void _render_props();
	int _find_watch(const StringName &p_name) const;
	String _format_value(const Variant &p_value) const;

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

	void set_font(TextConsole::FontSize p_font);
	TextConsole::FontSize get_font() const;
	void next_font();
	void set_pixel_scale(int p_scale);
	int get_pixel_scale() const;
	void set_auto_font(bool p_enabled);
	bool is_auto_font() const;

	static ConsoleInstance *get_active() { return active; }

	// Entry point for ConsoleLogger, callable from any thread. The line is dropped when no
	// console exists yet; it must never log or print anything itself.
	static void log_from_thread(const String &p_text, TextConsole::ColorIndex p_fg,
			TextConsole::ColorIndex p_bg = TextConsole::COLOR_DEFAULT);

	void switch_page(Page p_page);
	Page get_page() const;
	void next_page();
	void prev_page();

	void set_panel_touch_enabled(bool p_enabled);
	bool is_panel_touch_enabled() const;

	void set_refresh_rate(real_t p_hz);
	real_t get_refresh_rate() const;

	void log(const String &p_msg); // markup-aware, lands on PAGE_LOG
	void log_colored(const String &p_msg, TextConsole::ColorIndex p_fg, TextConsole::ColorIndex p_bg = TextConsole::COLOR_DEFAULT);
	void log_figlet(TextConsole::FigFontFace p_face, const String &p_msg, TextConsole::ColorIndex p_fg = TextConsole::COLOR_DEFAULT);
	void clear_log();

	// LCD "big font" readout drawn straight into the cell grid (console_lcd.{h,cpp}). Unlike
	// the log helpers this pokes absolute cells, so it is overwritten by the next page render;
	// callers redraw it each refresh. lcd_size() reports the cells it needs.
	Size2i lcd_size(const String &p_text) const;
	void lcd_draw(int p_x, int p_y, const String &p_text, TextConsole::ColorIndex p_lit, TextConsole::ColorIndex p_dim);

	// Bar/graph strings drawn from the fill glyphs at 0x80-0x8D (console_gauge.{h,cpp}).
	// These return markup-free console strings rather than drawing, so callers can embed them
	// in a log line or poke them at an absolute cell with put_text().
	String meter(real_t p_fraction, int p_cells) const;
	String graph(const Vector<real_t> &p_values, int p_cells, real_t p_min = 0.0, real_t p_max = 0.0) const;

	void set_value(const StringName &p_name, const Variant &p_value);
	void add_value(const StringName &p_name, const Ref<FuncRef> &p_getter);
	void add_var(const StringName &p_name, Object *p_object, const StringName &p_property);
	void remove_watch(const StringName &p_name);
	void clear_watches();
	Array get_watch_names() const;

	Ref<TextConsole> get_console() const;

	// Grid + logger claim + banner, independent of the scene tree. Called on ENTER_TREE, and
	// up front by SceneTree::console_show() when the node can only be attached next frame.
	void _bootstrap();

	// Builds the grid and renders the current page without ever entering a scene tree, for
	// headless capture (the `--test console` harness). Pass 0 to keep the current geometry.
	void render_offscreen(int p_cols = 0, int p_rows = 0);

	ConsoleInstance();
	~ConsoleInstance();
};

/* Picks a face and an integer pixel scale that stay readable on p_dpi while keeping the grid
 * usable, see CONSOLE.md section 6. p_dpi <= 0 falls back to 72. */
#define CONSOLE_MAX_PIXEL_SCALE 8

void console_banner_lines(Vector<String> &r_text, Vector<int> &r_color);

void console_auto_font(int p_dpi, const Size2i &p_viewport, TextConsole::FontSize &r_face, int &r_scale);

/* Engine singleton forwarding to the active ConsoleInstance, so game code never has to dig
 * the console node out of the scene tree. See CONSOLE.md section 5. */
class DebugConsole : public Object {
	GDCLASS(DebugConsole, Object);

	static DebugConsole *singleton;

	ConsoleInstance *_instance(bool p_create = false) const;

protected:
	static void _bind_methods();

public:
	static DebugConsole *get_singleton() { return singleton; }

	void show(bool p_visible);
	bool is_visible() const;

	void switch_page(int p_page);
	void next_page();
	void prev_page();
	int get_page() const;

	void set_panel_touch_enabled(bool p_enabled);
	bool is_panel_touch_enabled() const;

	void set_font(int p_font);
	int get_font() const;
	void next_font();
	void set_pixel_scale(int p_scale);
	int get_pixel_scale() const;
	void set_auto_font(bool p_enabled);
	bool is_auto_font() const;
	void set_refresh_rate(real_t p_hz);
	real_t get_refresh_rate() const;

	void log(const String &p_msg);
	void log_colored(const String &p_msg, int p_fg, int p_bg);
	void logf(int p_face, const String &p_msg);
	void clear_log();

	Vector2 lcd_size(const String &p_text) const;
	void lcd_draw(int p_x, int p_y, const String &p_text, int p_lit, int p_dim);

	String meter(real_t p_fraction, int p_cells);
	String graph(const PoolRealArray &p_values, int p_cells, real_t p_min = 0.0, real_t p_max = 0.0);

	void set_value(const StringName &p_name, const Variant &p_value);
	void add_value(const StringName &p_name, const Ref<FuncRef> &p_getter);
	void add_var(const StringName &p_name, Object *p_object, const StringName &p_property);
	void remove(const StringName &p_name);
	void clear();

	DebugConsole();
	~DebugConsole();
};

VARIANT_ENUM_CAST(TextConsole::ColorIndex);
VARIANT_ENUM_CAST(ConsoleInstance::Page);
VARIANT_ENUM_CAST(TextConsole::FigFontFace);
VARIANT_ENUM_CAST(TextConsole::FontSize);

#endif // DEBUG_CONSOLE_H
