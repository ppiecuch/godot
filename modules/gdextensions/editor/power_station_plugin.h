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
#include "scene/resources/audio_stream_sample.h"

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

/// Power Station sound generator

class PowerStationGenerator : public Reference {
	GDCLASS(PowerStationGenerator, Reference)

	PSState state;
	int sample_format; // 0=8bit, 1=16bit
	String output_file;
	bool auto_generate;

	Ref<Script> dlg_script;
	AcceptDialog *dlg;
	Timer *cleanup;

#ifdef TOOLS_ENABLED
	void _cleanup_ui();
	void _on_generate_pressed();
	void _on_save_pressed();
	void _on_sound_progress(real_t p_progress);
	void _on_sound_ready(Ref<AudioStreamSample> sound);
	void _on_value_changed(float value, String node);
	void _on_objtype_selected(int p_index);
	void _on_window_about_to_show();
	void _on_window_popup_hide();
	void _on_window_resized();
	void _on_window_visibility_changed();
#endif

protected:
	static void _bind_methods();

public:
	// Property accessors for PSState fields
	void set_obj_type(int p_type);
	int get_obj_type() const;
	void set_height(int p_height);
	int get_height() const;
	void set_circumference(int p_circum);
	int get_circumference() const;
	void set_length(int p_length);
	int get_length() const;
	void set_plane_length(int p_length);
	int get_plane_length() const;
	void set_plane_width(int p_width);
	int get_plane_width() const;
	void set_tenseness(real_t p_val);
	real_t get_tenseness() const;
	void set_speed(real_t p_val);
	real_t get_speed() const;
	void set_damping(real_t p_val);
	real_t get_damping() const;
	void set_actuation(int p_val);
	int get_actuation() const;
	void set_velocity(real_t p_val);
	real_t get_velocity() const;
	void set_sample_length(real_t p_val);
	real_t get_sample_length() const;
	void set_decay_enabled(bool p_val);
	bool is_decay_enabled() const;
	void set_decay_value(real_t p_val);
	real_t get_decay_value() const;
	void set_sound_quality(int p_quality);
	int get_sound_quality() const;
	void set_output_file(const String &p_path);
	String get_output_file() const;

	Ref<AudioStreamSample> get_samples() const;
#ifdef TOOLS_ENABLED
	AcceptDialog *load_ui();
	void open_ui();
#endif
	void generate();

	PowerStationGenerator();
};

/// Godot editor plugin

class PowerStationEditorPlugin : public EditorPlugin {
	GDCLASS(PowerStationEditorPlugin, EditorPlugin)

	EditorNode *editor;

	Ref<PowerStationGenerator> gen;

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
