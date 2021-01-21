#ifndef DEBUG_CONSOLE_H
#define DEBUG_CONSOLE_H

#include "core/reference.h"
#include "scene/main/viewport.h"
#include "scene/2d/node_2d.h"

#include <stdint.h>

/* Windows code page 437 box drawing characters */
#define  BOX_DLR      "\315"  /* ═ */
#define  BOX_DUD      "\272"  /* ║ */
#define  BOX_DUL      "\274"  /* ╝ */
#define  BOX_DUR      "\310"  /* ╚ */
#define  BOX_DDL      "\273"  /* ╗ */
#define  BOX_DDR      "\311"  /* ╔ */
#define  BOX_DUDL     "\271"  /* ╣ */
#define  BOX_DUDR     "\314"  /* ╠ */
#define  BOX_DULR     "\312"  /* ╩ */
#define  BOX_DDLR     "\313"  /* ╦ */
#define  BOX_DUDLR    "\316"  /* ╬ */
#define  BOX_DU_SL    "\275"  /* ╜, not in CP850 */
#define  BOX_DU_SR    "\323"  /* ╙, not in CP850 */
#define  BOX_DD_SL    "\267"  /* ╖, not in CP850 */
#define  BOX_DD_SR    "\326"  /* ╓, not in CP850 */
#define  BOX_DL_SU    "\276"  /* ╛, not in CP850 */
#define  BOX_DL_SD    "\270"  /* ╕, not in CP850 */
#define  BOX_DR_SU    "\324"  /* ╘, not in CP850 */
#define  BOX_DR_SD    "\325"  /* ╒, not in CP850 */
#define  BOX_DU_SLR   "\320"  /* ╨, not in CP850 */
#define  BOX_DD_SLR   "\322"  /* ╥, not in CP850 */
#define  BOX_DL_SUD   "\265"  /* ╡, not in CP850 */
#define  BOX_DR_SUD   "\306"  /* ╞, not in CP850 */
#define  BOX_DLR_SU   "\317"  /* ╧, not in CP850 */
#define  BOX_DLR_SD   "\321"  /* ╤, not in CP850 */
#define  BOX_DLR_SUD  "\330"  /* ╪, not in CP850 */
#define  BOX_DUD_SL   "\266"  /* ╢, not in CP850 */
#define  BOX_DUD_SR   "\307"  /* ╟, not in CP850 */
#define  BOX_DUD_SLR  "\327"  /* ╫, not in CP850 */
#define  BOX_SLR      "\304"  /* ─ */
#define  BOX_SUD      "\263"  /* │ */
#define  BOX_SUL      "\331"  /* ┘ */
#define  BOX_SUR      "\300"  /* └ */
#define  BOX_SDL      "\277"  /* ┐ */
#define  BOX_SDR      "\332"  /* ┌ */
#define  BOX_SULR     "\301"  /* ┴ */
#define  BOX_SDLR     "\302"  /* ┬ */
#define  BOX_SUDL     "\264"  /* ┤ */
#define  BOX_SUDR     "\303"  /* ├ */
#define  BOX_SUDLR    "\305"  /* ┼ */

struct TextConsole : public Reference {
	enum FontSize {
		DOS_8x16,
		DOS_8x12,
		DOS_8x8,
		DOS_7x9,
		DOS_4x6,
		DOS_FONT_MAX,
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
		COLOR_MAX,
	};
	void _update_mesh();
	Point2i _write(const String &p_msg, Point2i pos, uint8_t foreground, uint8_t background);
	Point2i _put(const String &p_msg, Point2i pos, uint8_t foreground, uint8_t background);
	Point2i _putf(const String &p_msg, Point2i pos, uint8_t foreground, uint8_t background);
	void _scroll_up();

	void load_font(FontSize p_font);
	void resize(int p_width, int p_height);
	void resize(const Viewport &p_view);
	void draw(RID p_canvas_item, const Transform2D &p_xform);
	void set_pixel_ratio(real_t p_scale);
	void set_transparent_color_index(uint8_t p_transparent_color_index);

	void log(const String &p_msg, uint8_t foreground, uint8_t background = 0);
	void log(const String &p_msg);
	void logf(const String &p_msg, uint8_t foreground, uint8_t background = 0);
	void logf(const String &p_msg);

	TextConsole();
	~TextConsole();

	Ref<ArrayMesh> _mesh;
	bool _dirty_screen;
	struct cell {
		char character;
		uint8_t foreground:4;
		uint8_t background:4;
		bool inverted:1;
	};
	cell *_screen;
	Ref<ImageTexture> _font_texture;
	Size2i _font_size, _con_size;
	Point2i _cursor_pos;
	uint8_t _default_bg_color_index, _default_fg_color_index;
	// texture font info (2x256 chars) cache:
	struct _char_info_t {
		Point2 t[4];
	} _chars[512];
	real_t pixel_scale;
	int transparent_color_index;
};


class ConsoleInstance : public CanvasItem {

	GDCLASS(ConsoleInstance, CanvasItem);

private:
	Ref<TextConsole> console;

	Point2 _pos;
	Size2 _scale;
	Transform2D _xform;

protected:
	void _notification(int p_what);

	static void _bind_methods();

public:
	void _edit_set_position(const Point2 &p_position);
	Point2 _edit_get_position() const;
	void _edit_set_scale(const Size2 &p_scale);
	Size2 _edit_get_scale() const;

	Transform2D get_transform() const;

	void console_msg(const String &p_msg);

	Ref<TextConsole> get_console() const;
};

VARIANT_ENUM_CAST(TextConsole::ColorIndex);

#endif // DEBUG_CONSOLE_H
