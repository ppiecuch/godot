/**************************************************************************/
/*  debug_sysinfo.h                                                       */
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

#ifndef DEBUG_SYSINFO_H
#define DEBUG_SYSINFO_H

#include "debug_console.h"

// One line of the SYSINFO page: a fully formatted string plus the single color it's drawn in,
// matching the granularity of the old per-logl()-call rendering. Both render backends below
// consume the same rows, so they always show the same data -- only the chrome differs.
struct SysInfoRow {
	String line;
	TextConsole::ColorIndex color;
};

// Gathers every SYSINFO row (device, cpu, gpu, fps, memory, ...). Mutates p_fps_history (pushes
// the current fps sample, trims to p_fps_history_capacity) exactly as the page's own refresh did
// before this was a free function -- the sample history is render-independent state, not a
// rendering detail.
Vector<SysInfoRow> debug_sysinfo_collect(TextConsole &p_console, Vector<real_t> &p_fps_history,
		int p_fps_history_capacity, int p_watch_count, const String &p_font_name);

// Today's plain rendering: one console->logl() per row.
void debug_sysinfo_render_classic(TextConsole &p_console, const Vector<SysInfoRow> &p_rows);

// Renders the same rows through thirdparty/textui as a top-level window with a title bar and a
// status line, then blits its cell buffer into p_console's grid. See debug_sysinfo.cpp for why
// this needs no textUI event loop.
void debug_sysinfo_render_textui(TextConsole &p_console, const Vector<SysInfoRow> &p_rows,
		const String &p_title, const String &p_status);

#endif // DEBUG_SYSINFO_H
