/**************************************************************************/
/*  debug_sysinfo.cpp                                                     */
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

#include "debug_sysinfo.h"

#include "console_gauge.h"

#include "core/os/os.h"
#include "core/os/secondary_display.h"
#include "core/version.h"
#include "main/performance.h"
#include "servers/visual_server.h"

#include "textui/textUI.h"

#include <stdio.h>

#if defined(UNIX_ENABLED) && !defined(NO_STATVFS)
#include <sys/statvfs.h>
#endif
#if defined(__linux__) || defined(ANDROID_ENABLED)
#include <dirent.h>
#endif

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

// --- SYSINFO collectors -----------------------------------------------------------------
//
// Everything here is polled at `refresh_rate` (~5 Hz), so the cost budget per item is a
// syscall or two. Anything that cannot change while the process runs -- the SoC name, the
// GL strings, the thermal zone list -- is resolved once and cached in a function-local
// static instead.

namespace {

#if defined(__linux__) || defined(ANDROID_ENABLED)

// Lines of a /proc or /sys pseudo-file, empty when it cannot be read.
//
// These deliberately bypass FileAccess: on Android ACCESS_FILESYSTEM is served by
// FileAccessFilesystemJAndroid, which routes through the Java storage API and cannot see
// kernel pseudo-files at all. Their reported size is also 0, so only stdio streaming works.
Vector<String> console_read_sys_file(const String &p_path, int p_max_lines = 4096) {
	Vector<String> lines;
	FILE *f = fopen(p_path.utf8().get_data(), "rb");
	if (!f) {
		return lines;
	}
	char buf[512];
	while (lines.size() < p_max_lines && fgets(buf, sizeof(buf), f)) {
		lines.push_back(String::utf8(buf).strip_edges());
	}
	fclose(f);
	return lines;
}

String console_read_sys_line(const String &p_path) {
	const Vector<String> lines = console_read_sys_file(p_path, 1);
	return lines.empty() ? String() : lines[0];
}

#endif

// First line of /proc/cpuinfo that names the part. Android drops "model name" on arm, so
// "Hardware" and "Processor" are tried as well; empty when nothing matched.
String console_cpu_model() {
	static String cached;
	static bool resolved = false;
	if (resolved) {
		return cached;
	}
	resolved = true;
#if defined(__linux__) || defined(ANDROID_ENABLED)
	const Vector<String> lines = console_read_sys_file("/proc/cpuinfo");
	for (int i = 0; i < lines.size(); ++i) {
		const String &line = lines[i];
		const int colon = line.find(":");
		if (colon < 0) {
			continue;
		}
		const String key = line.substr(0, colon).strip_edges();
		if (key == "Hardware" || key == "model name" || key == "Processor") {
			cached = line.substr(colon + 1).strip_edges();
			if (key == "Hardware") {
				break; // the most specific of the three on Android
			}
		}
	}
#endif
	return cached;
}

// Highest CPU/SoC die temperature in degrees Celsius, or -1 when unavailable. Qualcomm
// parts expose a hundred zones, so the interesting ones are picked once by type.
float console_cpu_temperature() {
#if defined(__linux__) || defined(ANDROID_ENABLED)
	static Vector<String> zones;
	static bool resolved = false;
	if (!resolved) {
		resolved = true;
		DIR *d = opendir("/sys/class/thermal");
		if (d) {
			while (struct dirent *ent = readdir(d)) {
				const String name = String::utf8(ent->d_name);
				if (!name.begins_with("thermal_zone")) {
					continue;
				}
				const String base = "/sys/class/thermal/" + name;
				const String type = console_read_sys_line(base + "/type").to_lower();
				// "cpu*", "*-usr" (tsens), "soc" -- skip battery/charger/skin zones, which
				// say nothing about how hard the chip is being pushed.
				if (type.find("cpu") >= 0 || type.find("soc") >= 0) {
					zones.push_back(base + "/temp");
				}
				if (zones.size() >= 8) {
					break;
				}
			}
			closedir(d);
		}
	}

	float best = -1;
	for (int i = 0; i < zones.size(); ++i) {
		const String raw = console_read_sys_line(zones[i]);
		if (raw.is_valid_integer()) {
			// Millidegrees on every sane kernel, but a few report plain degrees.
			const int64_t v = raw.to_int64();
			const float c = v > 1000 ? v / 1000.0f : float(v);
			best = MAX(best, c);
		}
	}
	return best;
#else
	return -1;
#endif
}

// MemAvailable / MemTotal in bytes; both zero when /proc/meminfo is not readable.
void console_system_memory(uint64_t &r_avail, uint64_t &r_total) {
	r_avail = 0;
	r_total = 0;
#if defined(__linux__) || defined(ANDROID_ENABLED)
	const Vector<String> lines = console_read_sys_file("/proc/meminfo", 64);
	for (int i = 0; i < lines.size() && (!r_avail || !r_total); ++i) {
		const String &line = lines[i];
		const int colon = line.find(":");
		if (colon < 0) {
			continue;
		}
		const String key = line.substr(0, colon).strip_edges();
		if (key != "MemAvailable" && key != "MemTotal") {
			continue;
		}
		// "MemTotal:       11534336 kB"
		const String value = line.substr(colon + 1).strip_edges().split(" ")[0];
		if (!value.is_valid_integer()) {
			continue;
		}
		const uint64_t bytes = uint64_t(value.to_int64()) * 1024;
		if (key == "MemAvailable") {
			r_avail = bytes;
		} else {
			r_total = bytes;
		}
	}
#endif
}

// Free bytes on the volume holding the user data dir, or 0 when it cannot be determined.
uint64_t console_storage_free() {
#if defined(UNIX_ENABLED) && !defined(NO_STATVFS)
	const CharString path = OS::get_singleton()->get_user_data_dir().utf8();
	struct statvfs st;
	if (statvfs(path.get_data(), &st) == 0) {
		return uint64_t(st.f_bavail) * uint64_t(st.f_frsize);
	}
#endif
	return 0;
}

String console_power_line() {
	OS *os = OS::get_singleton();
	const int percent = os->get_power_percent_left();
	if (percent < 0) {
		return String();
	}
	String state;
	switch (os->get_power_state()) {
		case OS::POWERSTATE_CHARGING:
			state = " chg";
			break;
		case OS::POWERSTATE_CHARGED:
			state = " full";
			break;
		case OS::POWERSTATE_ON_BATTERY:
			state = " bat";
			break;
		default:
			break;
	}
	return itos(percent) + "%" + state;
}

// 85300 -> "85.3 K"; keeps wide counters inside one column on a 40-cell panel.
String console_format_count(double p_value) {
	if (p_value >= 1000000.0) {
		return vformat("%.1f M", p_value / 1000000.0);
	}
	if (p_value >= 1000.0) {
		return vformat("%.1f K", p_value / 1000.0);
	}
	return itos(int64_t(p_value));
}

double console_perf(Performance::Monitor p_monitor) {
	const Performance *perf = Performance::get_singleton();
	return perf ? perf->get_monitor(p_monitor) : 0;
}

} // namespace

Vector<SysInfoRow> debug_sysinfo_collect(TextConsole &p_console, Vector<real_t> &p_fps_history,
		int p_fps_history_capacity, int p_watch_count, const String &p_font_name) {
	Vector<SysInfoRow> rows;

	OS *os = OS::get_singleton();
	VisualServer *vs = VisualServer::get_singleton();
	const int screen = os->get_current_screen();
	const int width = p_console.get_console_size().width;

	// A helper for the "name  value" rows; the label column is fixed so the values line up.
	auto row = [&](const char *p_label, const String &p_value, TextConsole::ColorIndex p_color = TextConsole::COLOR_DEFAULT) {
		String label = String(" ") + p_label;
		while (label.length() < 10) {
			label += " ";
		}
		rows.push_back({ label + p_value, p_color });
	};

	String version = VERSION_FULL_NAME;
	if (String(VERSION_HASH).length() >= 7) {
		version += " [" + String(VERSION_HASH).substr(0, 7) + "]";
	}
	rows.push_back({ version, TextConsole::COLOR_WHITE });

	// --- device ---------------------------------------------------------------------
	String device = os->get_name();
	if (!os->get_model_name().empty() && os->get_model_name() != "GenericDevice") {
		device += " / " + os->get_model_name();
	}
	row("device", device);

	String cpu = itos(os->get_processor_count()) + " cores";
	const String cpu_model = console_cpu_model();
	if (!cpu_model.empty()) {
		cpu += "  " + cpu_model;
	}
	row("cpu", cpu);

	const float temp = console_cpu_temperature();
	String status;
	if (temp >= 0) {
		status = vformat("%.1f C", temp);
	}
	const String power = console_power_line();
	if (!power.empty()) {
		status += status.empty() ? power : "   batt " + power;
	}
	if (!status.empty()) {
		// Warm silicon means the frame times below are already being throttled.
		row("thermal", status, temp >= 80 ? TextConsole::COLOR_LIGHTRED : (temp >= 65 ? TextConsole::COLOR_YELLOW : TextConsole::COLOR_DEFAULT));
	}

	String gpu = vs ? vs->get_video_adapter_name() : String();
	if (gpu.empty()) {
		gpu = os->get_video_driver_name(os->get_current_video_driver());
	} else {
		gpu += " (" + String(os->get_video_driver_name(os->get_current_video_driver())) + ")";
	}
	row("gpu", gpu);
	row("locale", os->get_locale());

	// --- displays -------------------------------------------------------------------
	row("screen", vformat("%dx%d @ %d dpi", int(os->get_screen_size(screen).width), int(os->get_screen_size(screen).height), os->get_screen_dpi(screen)));
	if (SecondaryDisplay *sd = SecondaryDisplay::get_singleton()) {
		if (sd->is_available()) {
			// The panel this console is most likely being read on.
			row("panel", vformat("%dx%d @ %d dpi", sd->get_size().width, sd->get_size().height, sd->get_dpi()), TextConsole::COLOR_LIGHTCYAN);
		}
	}
	row("console", vformat("%dx%d cells, %s, scale %d", p_console.get_console_size().width, p_console.get_console_size().height, p_font_name, p_console.get_pixel_scale()));

	// --- frame ----------------------------------------------------------------------
	rows.push_back({ String(BOX_SLR).repeat(MAX(1, width)), TextConsole::COLOR_DARKGRAY });

	const float fps = console_perf(Performance::TIME_FPS);
	row("fps", vformat("%d   frame %.1f ms   phys %.1f ms", int(fps), console_perf(Performance::TIME_PROCESS) * 1000.0, console_perf(Performance::TIME_PHYSICS_PROCESS) * 1000.0));

	// Plot the fps history under the numbers: the instantaneous value says nothing about
	// whether a stutter just happened. The range is pinned to 0..60 so the plot means the
	// same thing frame to frame instead of auto-scaling to whatever the recent spread was.
	p_fps_history.push_back(fps);
	while (p_fps_history.size() > p_fps_history_capacity) {
		p_fps_history.remove(0);
	}
	const int plot_width = MAX(8, MIN(width - 12, p_fps_history_capacity));
	row("", console_graph(p_fps_history, plot_width, 0.0, 60.0), TextConsole::COLOR_LIGHTGREEN);
	row("draws", vformat("%d   verts %s   objs %d", int(console_perf(Performance::RENDER_DRAW_CALLS_IN_FRAME)), console_format_count(console_perf(Performance::RENDER_VERTICES_IN_FRAME)), int(console_perf(Performance::RENDER_OBJECTS_IN_FRAME))));
	row("changes", vformat("surf %d  mat %d  shader %d", int(console_perf(Performance::RENDER_SURFACE_CHANGES_IN_FRAME)), int(console_perf(Performance::RENDER_MATERIAL_CHANGES_IN_FRAME)), int(console_perf(Performance::RENDER_SHADER_CHANGES_IN_FRAME))));
	row("vram", vformat("%s  (tex %s, vtx %s)", String::humanize_size((uint64_t)console_perf(Performance::RENDER_VIDEO_MEM_USED)), String::humanize_size((uint64_t)console_perf(Performance::RENDER_TEXTURE_MEM_USED)), String::humanize_size((uint64_t)console_perf(Performance::RENDER_VERTEX_MEM_USED))));

	// --- objects & memory -----------------------------------------------------------
	const int orphans = int(console_perf(Performance::OBJECT_ORPHAN_NODE_COUNT));
	row("objects", vformat("%d  nodes %d  res %d", ObjectDB::get_object_count(), int(console_perf(Performance::OBJECT_NODE_COUNT)), int(console_perf(Performance::OBJECT_RESOURCE_COUNT))));
	// Orphans are nodes that were never freed after being removed from the tree: the
	// cheapest leak canary a running game has.
	row("orphans", itos(orphans), orphans > 0 ? TextConsole::COLOR_YELLOW : TextConsole::COLOR_DARKGRAY);
	row("mem", vformat("%s static (peak %s)", String::humanize_size(os->get_static_memory_usage()), String::humanize_size(os->get_static_memory_peak_usage())));
	row("dynamic", vformat("%s (peak %s)", String::humanize_size((uint64_t)console_perf(Performance::MEMORY_DYNAMIC)), String::humanize_size((uint64_t)console_perf(Performance::MEMORY_DYNAMIC_MAX))));

	uint64_t sys_avail = 0, sys_total = 0;
	console_system_memory(sys_avail, sys_total);
	if (sys_total) {
		row("sysmem", vformat("%s free of %s", String::humanize_size(sys_avail), String::humanize_size(sys_total)),
				sys_avail < (sys_total / 10) ? TextConsole::COLOR_LIGHTRED : TextConsole::COLOR_DEFAULT);
	}
	if (sys_total) {
		const real_t used = real_t(sys_total - sys_avail) / real_t(sys_total);
		row("", console_meter(used, MAX(8, MIN(width - 12, 24))),
				used > 0.9 ? TextConsole::COLOR_LIGHTRED : TextConsole::COLOR_CYAN);
	}
	if (const uint64_t storage = console_storage_free()) {
		row("storage", String::humanize_size(storage) + " free");
	}

	const double latency = console_perf(Performance::AUDIO_OUTPUT_LATENCY);
	if (latency > 0) {
		row("audio", vformat("%.1f ms latency", latency * 1000.0));
	}
	row("watches", itos(p_watch_count), TextConsole::COLOR_DARKGRAY);

	return rows;
}

void debug_sysinfo_render_classic(TextConsole &p_console, const Vector<SysInfoRow> &p_rows) {
	for (int i = 0; i < p_rows.size(); ++i) {
		p_console.logl(p_rows[i].line, p_rows[i].color);
	}
}

// --- textUI-backed "professional" rendering ---------------------------------------------
//
// A top-level textUI APPLICATION window is created once (cached, resized on demand) with a
// title bar and a status bar -- both painted automatically by textUI itself once the window
// has a non-empty title and HASSTATUSBAR set. No textUI event loop (dispatch_message()) is
// needed: CreateWindow()/SendMessage(..., PAINT/BORDER, ...) paint synchronously into a single
// global cell buffer, which is then read back cell by cell with GetVideoChar() and blitted into
// p_console's grid. TextConsole::ColorIndex is numerically identical to textUI's 0-15 DOS
// palette, and a cell there packs as character | (attribute << 8) with attribute = fg | (bg<<4),
// so decoding it is a plain bit-split.

namespace {

struct SysInfoWindow {
	WINDOW wnd = nullptr;
	Size2i size;
};

SysInfoWindow &sysinfo_window() {
	static SysInfoWindow w;
	return w;
}

// textUI's global cell buffer (video_address) is allocated lazily at whatever size
// init_console() was last given, but only grows once, on the very first allocation -- calling
// init_console() again with a *larger* size later does not reallocate it (see get_videomode(),
// textUI.c), which would silently turn a console resize into an out-of-bounds write/read. So
// init_console() is called exactly once, sized to textUI's own documented maximum
// (MAX_CONSOLE_WIDTH/MAX_CONSOLE_HEIGHT, textUI.c), and never again; only the WINDOW itself is
// resized (recreated) to track the console's actual, much smaller, live size.
enum { SYSINFO_SCREEN_WIDTH = 1024,
	SYSINFO_SCREEN_HEIGHT = 512 };

WINDOW ensure_sysinfo_window(const Size2i &p_size, const String &p_title) {
	SysInfoWindow &w = sysinfo_window();
	if (w.wnd && w.size == p_size) {
		return w.wnd;
	}
	if (w.wnd) {
		SendMessage(w.wnd, CLOSE_WINDOW, 0, 0);
		w.wnd = nullptr;
	} else {
		init_console(SYSINFO_SCREEN_WIDTH, SYSINFO_SCREEN_HEIGHT);
	}
	// A NULL wndproc makes CreateWindow fall back to the APPLICATION class's own ApplicationProc,
	// which already handles PAINT/BORDER (title bar, frame) -- no custom message handling needed
	// for a purely-painted, non-interactive panel.
	w.wnd = CreateWindow(APPLICATION, p_title.utf8().get_data(), 0, 0, p_size.height, p_size.width,
			NULL, NULL, NULL, HASTITLEBAR | HASSTATUSBAR | VISIBLE);
	CreateStatusBar(w.wnd);
	w.size = p_size;
	return w.wnd;
}

} // namespace

void debug_sysinfo_render_textui(TextConsole &p_console, const Vector<SysInfoRow> &p_rows,
		const String &p_title, const String &p_status) {
	const Size2i size = p_console.get_console_size();
	if (size.width <= 0 || size.height <= 0) {
		return;
	}

	WINDOW wnd = ensure_sysinfo_window(size, p_title);
	SendMessage(wnd, ADDSTATUS, (PARAM)p_status.utf8().get_data(), 0);
	SendMessage(wnd, BORDER, 0, 0);

	const int body_height = MAX(0, ClientHeight(wnd));
	for (int i = 0; i < p_rows.size() && i < body_height; ++i) {
		const TextConsole::ColorIndex color = p_rows[i].color;
		foreground = (color == TextConsole::COLOR_DEFAULT || color == TextConsole::COLOR_TRANSPARENT) ? TextConsole::COLOR_WHITE : color;
		background = TextConsole::COLOR_BLACK;
		PutWindowLine(wnd, p_rows[i].line.utf8().get_data(), 0, i);
	}

	for (int y = 0; y < size.height; ++y) {
		for (int x = 0; x < size.width; ++x) {
			const con_char_t cc = GetVideoChar(x, y);
			const uint8_t code = cc & 0xFF;
			const uint8_t attr = (cc >> 8) & 0xFF;
			p_console.put_cell(x, y, code, TextConsole::ColorIndex(attr & 0xF), TextConsole::ColorIndex((attr >> 4) & 0xF), TextConsole::BANK_CP437);
		}
	}
}

#ifdef DOCTEST

namespace {

Ref<TextConsole> sysinfo_test_console(int p_cols = 40, int p_rows = 16) {
	Ref<TextConsole> console = Ref<TextConsole>(memnew(TextConsole));
	console->load_font(TextConsole::DOS_8x16);
	console->resize(p_cols, p_rows);
	console->clear();
	return console;
}

// Reads one grid row back out as plain text, for asserting on rendered chrome content.
String row_text(const Ref<TextConsole> &p_console, int p_y) {
	String text;
	const int width = p_console->get_console_size().width;
	for (int x = 0; x < width; ++x) {
		TextConsole::cell c;
		text += p_console->get_cell(x, p_y, c) ? String::chr(uint8_t(c.character)) : String(" ");
	}
	return text;
}

} // namespace

TEST_CASE("Console sysinfo") {
	Ref<TextConsole> console = sysinfo_test_console();
	Vector<real_t> fps_history;

	Vector<SysInfoRow> rows = debug_sysinfo_collect(*console.ptr(), fps_history, 32, 3, "test_font");
	REQUIRE(rows.size() > 0);

	SUBCASE("classic backend logs every row") {
		debug_sysinfo_render_classic(*console.ptr(), rows);
	}

	SUBCASE("textui backend paints a title bar, a status line, and body rows") {
		debug_sysinfo_render_textui(*console.ptr(), rows, "System Info", "test status");
		const int height = console->get_console_size().height;
		REQUIRE(row_text(console, 0).find("System Info") >= 0);
		REQUIRE(row_text(console, height - 1).find("test status") >= 0);
		// Body rows sit between the title bar and the status line.
		REQUIRE(row_text(console, 1).strip_edges().length() > 0);
	}
}

#endif // DOCTEST
