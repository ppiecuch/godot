/**************************************************************************/
/*  trackergen_studio_plugin.h                                           */
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

#ifndef TRACKERGEN_STUDIO_PLUGIN_H
#define TRACKERGEN_STUDIO_PLUGIN_H

#ifdef TOOLS_ENABLED

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"

#include "trackergen_song.h"

// Minimal "Tracker Studio" seed: adds a Tools menu item that renders a built-in
// demo song to user://trackergen_studio.wav via AudioStreamTracker::save_to_wav.
// A fuller studio (song picker, EQ bands, live tempo/tension, record) builds on
// this + the offline render + Godot's AudioEffectEQ10 bus.
class TrackerStudioEditorPlugin : public EditorPlugin {
	GDCLASS(TrackerStudioEditorPlugin, EditorPlugin);

	EditorNode *editor;

	Ref<TrackerSong> _build_demo_song();
	void _render_demo(Variant p_ud);

protected:
	static void _bind_methods();

public:
	virtual String get_name() const { return "TrackerStudio"; }

	TrackerStudioEditorPlugin(EditorNode *p_node);
	~TrackerStudioEditorPlugin();
};

#endif // TOOLS_ENABLED

#endif // TRACKERGEN_STUDIO_PLUGIN_H
