/**************************************************************************/
/*  test_console.cpp                                                      */
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

#include "test_console.h"

#include "core/os/os.h"

#ifdef GD_INAPP_CONSOLE

#include "core/image.h"
#include "core/print_string.h"
#include "scene/debugconsole/console_gauge.h"
#include "scene/debugconsole/console_lcd.h"
#include "scene/debugconsole/console_raster.h"
#include "scene/debugconsole/debug_console.h"

namespace TestConsole {

namespace {

// Grid -> RGBA8 image through the same blitter the second screen uses.
Ref<Image> console_snapshot(TextConsole &p_console) {
	const Size2i size = console_raster_size(p_console);
	PoolByteArray buffer;
	buffer.resize(size.width * size.height * 4);
	{
		PoolByteArray::Write wr = buffer.write();
		uint32_t *pixels = (uint32_t *)wr.ptr();
		// The console leaves COLOR_TRANSPARENT cells untouched, so start from opaque black
		// the way the second screen will.
		for (int i = 0; i < size.width * size.height; ++i) {
			pixels[i] = 0xff000000;
		}
		console_blit(p_console, pixels, size.width, size.width, size.height, CONSOLE_PIXEL_ABGR32);
	}
	// CONSOLE_PIXEL_ABGR32 is byte order R,G,B,A on little-endian, which is exactly
	// Image::FORMAT_RGBA8 -- so the Android buffer layout is what gets validated here.
	Ref<Image> image = Ref<Image>(memnew(Image));
	image->create(size.width, size.height, false, Image::FORMAT_RGBA8, buffer);
	return image;
}

// Stacks the panels with a visible gutter, so one file shows both the raw grid and the
// page render at the same font size.
Ref<Image> stack_panels(const Ref<Image> &p_top, const Ref<Image> &p_bottom, int p_gap) {
	const int width = MAX(p_top->get_width(), p_bottom->get_width());
	const int height = p_top->get_height() + p_gap + p_bottom->get_height();

	Ref<Image> out = Ref<Image>(memnew(Image));
	out->create(width, height, false, Image::FORMAT_RGBA8);
	out->fill(Color(0.15, 0.15, 0.18));
	out->blit_rect(p_top, Rect2(0, 0, p_top->get_width(), p_top->get_height()), Point2(0, 0));
	out->blit_rect(p_bottom, Rect2(0, 0, p_bottom->get_width(), p_bottom->get_height()),
			Point2(0, p_top->get_height() + p_gap));
	return out;
}

// Drives a real ConsoleInstance far enough to render PAGE_LOG without a SceneTree, so the
// header bar, the wrapping and the logger integration are exercised rather than mimicked.
Ref<Image> render_log_page(int p_cols, int p_rows) {
	ConsoleInstance *inst = memnew(ConsoleInstance);
	// Match the panel above: same face, and no zoom, whatever this screen's dpi suggests.
	inst->set_font(TextConsole::DOS_8x16);
	inst->set_pixel_scale(1);
	inst->render_offscreen(p_cols, p_rows); // builds the grid and claims the logger

	// ConsoleLogger is registered by Main, and `active` now points at this instance, so the
	// ordinary engine logging macros are the ones under test here.
	print_line("engine print_line() lands on the LOG page");
	WARN_PRINT("warnings are coloured by severity");
	ERR_PRINT("so are errors, with the source location");

	inst->log("plain text with [fg=lightgreen]inline[/fg] [fg=lightred]markup[/fg] runs");
	inst->log("[bg=blue][fg=white] background tags work too [/fg][/bg]");
	inst->log_colored("this line was pushed with log_colored()", TextConsole::COLOR_LIGHTMAGENTA);
	inst->log_colored("cpu  " + inst->meter(0.62, 24) + "  62%", TextConsole::COLOR_LIGHTGREEN);
	inst->log_colored("heap " + inst->meter(0.31, 24) + "  31%", TextConsole::COLOR_YELLOW);
	{
		Vector<real_t> fps;
		for (int i = 0; i < 40; ++i) {
			fps.push_back(38.0 + 22.0 * Math::sin(real_t(i) * 0.45));
		}
		inst->log_colored("fps  " + inst->graph(fps, 40, 0, 60), TextConsole::COLOR_LIGHTCYAN);
	}
	inst->log_figlet(TextConsole::FIG_MINIWI, "log view", TextConsole::COLOR_LIGHTBLUE);
	inst->log_colored("a line long enough to prove that wrapping keeps the tail of the message on "
					  "screen instead of pushing the rest of the page away",
			TextConsole::COLOR_DARKGRAY);

	inst->switch_page(ConsoleInstance::PAGE_LOG); // forces the header + log render

	Ref<Image> image = console_snapshot(**inst->get_console());
	memdelete(inst); // also clears ConsoleInstance::active, before the logger sees it again
	return image;
}

} // namespace

// Renders a TextConsole with the software blitter and writes the result to a PNG, so the
// second-screen render path can be eyeballed without any Android involvement.
// Run with: godot --test console [output.png]
MainLoop *test() {
	String out_path = "console_raster.png";
	const List<String> &args = OS::get_singleton()->get_cmdline_args();
	for (const List<String>::Element *E = args.front(); E; E = E->next()) {
		if (E->get().ends_with(".png")) {
			out_path = E->get();
		}
	}

	Ref<TextConsole> console = Ref<TextConsole>(memnew(TextConsole));
	console->load_font(TextConsole::DOS_8x16);
	console->resize(72, 112); // prints the startup banner

	console->logl("");
	console->logl("software blitter test", TextConsole::COLOR_YELLOW);
	console->logl("colours:", TextConsole::COLOR_LIGHTGRAY);
	for (int i = 0; i < TextConsole::COLOR_TRANSPARENT; ++i) {
		console->logl(vformat("  %2d \xdb\xdb\xdb\xdb sample", i), TextConsole::ColorIndex(i));
	}
	console->logl("bank toggle: \x01inverted\x01 plain", TextConsole::COLOR_WHITE);

	// LCD "big font" readout poked straight into the grid (console_lcd.{h,cpp}), ported from
	// the text_ui LCDBOX widget. Drawn at absolute cells, so reserve the three rows it needs.
	console->logl("LCD readout (ported LCDBOX):", TextConsole::COLOR_LIGHTGRAY);
	{
		const Point2i base = console->get_cursor();
		console_lcd_draw(*console.ptr(), 1, base.y, "01:23", TextConsole::COLOR_LIGHTGREEN, TextConsole::COLOR_BLACK);
		console_lcd_draw(*console.ptr(), 20, base.y, "GODOT", TextConsole::COLOR_YELLOW, TextConsole::COLOR_BLACK);
		console->set_cursor(Point2i(0, base.y + console_lcd_size("0").height));
	}
	console->logl("");

	// Bars and graphs built from the 0x80-0x8D fill glyphs (console_gauge.{h,cpp}). These are
	// ordinary font patches, so they must render at every font size without special casing.
	console->logl("gauges (0x80-0x8d fill glyphs):", TextConsole::COLOR_LIGHTGRAY);
	for (int i = 0; i <= 8; ++i) {
		console->logl(vformat("  %3d%% ", i * 100 / 8) + console_meter(real_t(i) / 8.0, 24),
				TextConsole::COLOR_CYAN);
	}
	Vector<real_t> wave;
	for (int i = 0; i < 48; ++i) {
		wave.push_back(Math::sin(real_t(i) * 0.32));
	}
	console->logl("  wave " + console_graph(wave, 48), TextConsole::COLOR_LIGHTGREEN);
	Vector<real_t> ramp;
	for (int i = 0; i < 24; ++i) {
		ramp.push_back(real_t(i));
	}
	console->logl("  ramp " + console_graph(ramp, 24), TextConsole::COLOR_YELLOW);
	console->logl("  flat " + console_graph(ramp, 24, 0.0, 0.0) + " (auto range)", TextConsole::COLOR_DARKGRAY);
	console->logl("");

	console->logl("figlet faces:", TextConsole::COLOR_LIGHTGRAY);
	static const char *const fig_names[TextConsole::FigFontFaceCount] = {
		"future", "calvins", "ansiregular", "ansishadow",
		"dosrebel", "maxiwi", "miniwi", "maxii"
	};
	for (int i = 0; i < TextConsole::FigFontFaceCount; ++i) {
		console->logl(vformat("  %s:", fig_names[i]), TextConsole::COLOR_DARKGRAY);
		console->logf(TextConsole::FigFontFace(i), "Godot 42", TextConsole::COLOR_LIGHTCYAN);
	}

	// Second panel: the real PAGE_LOG render, header bar and all, driven through
	// ConsoleInstance so the logger integration is exercised rather than mimicked.
	Ref<Image> image = stack_panels(console_snapshot(*console.ptr()), render_log_page(72, 40), 12);

	const Error err = image->save_png(out_path);
	if (err != OK) {
		OS::get_singleton()->print("console: failed to write %s (error %d)\n", out_path.utf8().get_data(), int(err));
	} else {
		OS::get_singleton()->print("console: wrote %s (%dx%d)\n", out_path.utf8().get_data(),
				image->get_width(), image->get_height());
	}

	return nullptr;
}
} // namespace TestConsole

#else

namespace TestConsole {

MainLoop *test() {
	OS::get_singleton()->print("console: built without the in-app console (inapp_console=no)\n");
	return nullptr;
}
} // namespace TestConsole

#endif // GD_INAPP_CONSOLE
