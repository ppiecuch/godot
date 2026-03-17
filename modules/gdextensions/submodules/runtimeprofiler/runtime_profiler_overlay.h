/**************************************************************************/
/*  runtime_profiler_overlay.h                                            */
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

#ifndef RUNTIME_PROFILER_OVERLAY_H
#define RUNTIME_PROFILER_OVERLAY_H

#include "core/object.h"

class CanvasLayer;
class PanelContainer;
class RuntimeProfiler;

class RuntimeProfilerOverlay : public Object {
	GDCLASS(RuntimeProfilerOverlay, Object);

	static RuntimeProfilerOverlay *singleton;

	CanvasLayer *canvas_layer;
	PanelContainer *panel;
	RuntimeProfiler *profiler;

	bool overlay_visible;
	bool initialized;
	bool enabled;

	// Keyboard combo state (polled each frame)
	bool action_was_pressed;

	// Gamepad combo state (L1 + R1 + Select)
	bool pad_combo_was_active;

	bool _init_overlay();
	void _idle_frame();
	void _toggle_overlay();
	void _setup_input_action();

protected:
	static void _bind_methods();

public:
	static RuntimeProfilerOverlay *get_singleton();

	void show_overlay();
	void hide_overlay();
	void toggle_overlay();
	bool is_overlay_visible() const;

	void set_enabled(bool p_enabled);
	bool is_enabled() const;

	RuntimeProfiler *get_profiler() const;

	RuntimeProfilerOverlay();
	~RuntimeProfilerOverlay();
};

#endif // RUNTIME_PROFILER_OVERLAY_H
