/**************************************************************************/
/*  tweak_bar.h                                                           */
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

#ifndef TWEAK_BAR_H
#define TWEAK_BAR_H

#include "core/reference.h"
#include "scene/2d/node_2d.h"

class TweakBar : public Node2D {
	GDCLASS(TweakBar, Node2D);

	bool initialized;

	struct VarData {
		enum Type { FLOAT,
			INT,
			BOOL,
			STRING,
			COLOR3,
			COLOR4,
			DIR3 } type;
		float f;
		int i;
		bool b;
		float c3[3];
		float c4[4];
		float d3[3];
	};

	struct VarCBInfo {
		TweakBar *self;
		String bar_name;
		String var_name;
		VarData::Type type;
	};

	Map<String, VarData> m_vars;
	Map<String, String> m_string_vars;
	Vector<VarCBInfo *> m_cb_infos;
	Vector<VarCBInfo *> m_btn_infos;

	static void _set_cb(const void *value, void *clientData);
	static void _get_cb(void *value, void *clientData);
	static void _btn_cb(void *clientData);

	void _ensure_init();
	String _var_key(const String &bar, const String &var) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Bar management
	bool new_bar(const String &p_name);
	void delete_bar(const String &p_name);
	int get_bar_count() const;
	void define(const String &p_def);

	// Variables — bar name, variable name, AntTweakBar definition string
	bool add_float(const String &p_bar, const String &p_name, const String &p_def);
	bool add_int(const String &p_bar, const String &p_name, const String &p_def);
	bool add_bool(const String &p_bar, const String &p_name, const String &p_def);
	bool add_string(const String &p_bar, const String &p_name, const String &p_def);
	bool add_color3(const String &p_bar, const String &p_name, const String &p_def);
	bool add_color4(const String &p_bar, const String &p_name, const String &p_def);
	bool add_direction(const String &p_bar, const String &p_name, const String &p_def);
	bool add_button(const String &p_bar, const String &p_name, const String &p_def);
	bool add_separator(const String &p_bar, const String &p_name);
	bool remove_var(const String &p_bar, const String &p_name);
	bool remove_all_vars(const String &p_bar);

	// Get/set variable values
	Variant get_value(const String &p_bar, const String &p_name) const;
	void set_value(const String &p_bar, const String &p_name, const Variant &p_value);

	// Input forwarding
	void _handle_input(const Ref<InputEvent> &p_event);

	// Info
	String get_last_error() const;
	void refresh();

	TweakBar();
	~TweakBar();
};

#endif // TWEAK_BAR_H
