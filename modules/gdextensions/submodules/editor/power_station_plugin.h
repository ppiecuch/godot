/**************************************************************************/
/*  power_station_plugin.h                                                */
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

// Power Station Glib PhyMod Library
// Copyright (c) 2000 David A. Bartold

#ifndef _POWER_STATION_PLUGIN_H_
#define _POWER_STATION_PLUGIN_H_

#include "core/int_types.h"
#include "core/local_vector.h"
#include "editor/editor_plugin.h"
#include "scene/gui/dialogs.h"

typedef struct _PSState {
	int size;
	int height, circum, length;
	int plane_length, plane_width;
	double tenseness, speed, damping;
	int actuation;
	double velocity;
	double sample_length;
	LocalVector<int16_t> samples;
	struct _PSMetalObj *object;
	int obj_type;
	float x_angle, y_angle;
	bool decay_is_used;
	double decay_value;
	float progress;
} PSState;

/// Godot editor plugin

class PowerStationEditorPlugin : public EditorPlugin {
	GDCLASS(PowerStationEditorPlugin, EditorPlugin)

	EditorNode *editor;

	void add_icons_menu_item(const String &p_name, const String &p_callback);
	void remove_icons_menu_item(const String &p_name);

	void _on_show_ps_editor_pressed(Variant p_null);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void generate();

	PowerStationEditorPlugin(EditorNode *p_node);
};

#endif // _POWER_STATION_PLUGIN_H_
