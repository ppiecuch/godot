/**************************************************************************/
/*  tweak_bar.cpp                                                         */
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

#include "tweak_bar.h"

#include "thirdparty/anttweakbar/AntTweakBar.h"

// External functions defined in AntTweakBar.cpp
extern int TwEventGodot(const Ref<InputEvent> &ev);
extern void TwReplayDrawCommands(CanvasItem *ci);

// ---------------------------------------------------------------------------
// Static callbacks for AntTweakBar variable get/set
// ---------------------------------------------------------------------------

void TweakBar::_set_cb(const void *value, void *clientData) {
	VarCBInfo *info = (VarCBInfo *)clientData;
	String key = info->self->_var_key(info->bar_name, info->var_name);

	switch (info->type) {
		case VarData::FLOAT:
			info->self->m_vars[key].f = *(const float *)value;
			break;
		case VarData::INT:
			info->self->m_vars[key].i = *(const int *)value;
			break;
		case VarData::BOOL:
			info->self->m_vars[key].b = (*(const int *)value) != 0;
			break;
		case VarData::STRING: {
			const std::string &s = *(const std::string *)value;
			info->self->m_string_vars[key] = String::utf8(s.c_str());
			break;
		}
		case VarData::COLOR3:
			memcpy(info->self->m_vars[key].c3, value, sizeof(float) * 3);
			break;
		case VarData::COLOR4:
			memcpy(info->self->m_vars[key].c4, value, sizeof(float) * 4);
			break;
		case VarData::DIR3:
			memcpy(info->self->m_vars[key].d3, value, sizeof(float) * 3);
			break;
	}

	info->self->emit_signal("value_changed", info->bar_name, info->var_name);
}

void TweakBar::_get_cb(void *value, void *clientData) {
	VarCBInfo *info = (VarCBInfo *)clientData;
	String key = info->self->_var_key(info->bar_name, info->var_name);

	switch (info->type) {
		case VarData::FLOAT:
			if (info->self->m_vars.has(key))
				*(float *)value = info->self->m_vars[key].f;
			else
				*(float *)value = 0.0f;
			break;
		case VarData::INT:
			if (info->self->m_vars.has(key))
				*(int *)value = info->self->m_vars[key].i;
			else
				*(int *)value = 0;
			break;
		case VarData::BOOL:
			if (info->self->m_vars.has(key))
				*(int *)value = info->self->m_vars[key].b ? 1 : 0;
			else
				*(int *)value = 0;
			break;
		case VarData::STRING: {
			std::string &dest = *(std::string *)value;
			if (info->self->m_string_vars.has(key)) {
				CharString cs = info->self->m_string_vars[key].utf8();
				dest = cs.get_data();
			} else {
				dest.clear();
			}
			break;
		}
		case VarData::COLOR3:
			if (info->self->m_vars.has(key))
				memcpy(value, info->self->m_vars[key].c3, sizeof(float) * 3);
			else
				memset(value, 0, sizeof(float) * 3);
			break;
		case VarData::COLOR4:
			if (info->self->m_vars.has(key))
				memcpy(value, info->self->m_vars[key].c4, sizeof(float) * 4);
			else
				memset(value, 0, sizeof(float) * 4);
			break;
		case VarData::DIR3:
			if (info->self->m_vars.has(key))
				memcpy(value, info->self->m_vars[key].d3, sizeof(float) * 3);
			else
				memset(value, 0, sizeof(float) * 3);
			break;
	}
}

void TweakBar::_btn_cb(void *clientData) {
	VarCBInfo *info = (VarCBInfo *)clientData;
	info->self->emit_signal("button_pressed", info->bar_name, info->var_name);
}

// ---------------------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------------------

void TweakBar::_ensure_init() {
	if (!initialized) {
		TwInit(NULL);

		// Set up string copy functions
		static auto copy_std = [](std::string &dest, const std::string &src) { dest = src; };
		TwCopyStdStringToClientFunc(copy_std);

		static auto copy_cd = [](char **dest, const char *src) {
			size_t len = src ? strlen(src) : 0;
			if (*dest == NULL)
				*dest = (char *)malloc(len + 1);
			else if (strlen(*dest) < len)
				*dest = (char *)realloc(*dest, len + 1);
			if (len > 0)
				strncpy(*dest, src, len);
			(*dest)[len] = '\0';
		};
		TwCopyCDStringToClientFunc(copy_cd);

		initialized = true;
	}
}

String TweakBar::_var_key(const String &bar, const String &var) const {
	return bar + "/" + var;
}

// ---------------------------------------------------------------------------
// Lifecycle and notifications
// ---------------------------------------------------------------------------

TweakBar::TweakBar() {
	initialized = false;
}

TweakBar::~TweakBar() {
	for (int i = 0; i < m_cb_infos.size(); i++)
		memdelete(m_cb_infos[i]);
	for (int i = 0; i < m_btn_infos.size(); i++)
		memdelete(m_btn_infos[i]);

	if (initialized) {
		TwTerminate();
		initialized = false;
	}
}

void TweakBar::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_ensure_init();
			set_process(true);
			set_process_input(true);
			Size2 wnd = get_viewport_rect().size;
			TwWindowSize(wnd.x, wnd.y);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			set_process(false);
			set_process_input(false);
		} break;

		case NOTIFICATION_PROCESS: {
			// Check for viewport resize
			Size2 wnd = get_viewport_rect().size;
			TwWindowSize(wnd.x, wnd.y);

			// Render AntTweakBar (fills command buffer)
			TwDraw();

			// Trigger _draw
			update();
		} break;

		case NOTIFICATION_DRAW: {
			TwReplayDrawCommands(this);
		} break;
	}
}

// ---------------------------------------------------------------------------
// Input handling
// ---------------------------------------------------------------------------

void TweakBar::_handle_input(const Ref<InputEvent> &p_event) {
	if (initialized) {
		if (TwEventGodot(p_event)) {
			get_tree()->set_input_as_handled();
		}
	}
}

// ---------------------------------------------------------------------------
// Bar management
// ---------------------------------------------------------------------------

bool TweakBar::new_bar(const String &p_name) {
	_ensure_init();
	TwBar *bar = TwNewBar(p_name.utf8().get_data());
	return bar != NULL;
}

void TweakBar::delete_bar(const String &p_name) {
	if (!initialized)
		return;
	TwBar *bar = TwGetBarByName(p_name.utf8().get_data());
	if (bar) {
		TwDeleteBar(bar);
	}
}

int TweakBar::get_bar_count() const {
	if (!initialized)
		return 0;
	return TwGetBarCount();
}

void TweakBar::define(const String &p_def) {
	_ensure_init();
	TwDefine(p_def.utf8().get_data());
}

// ---------------------------------------------------------------------------
// Variable management
// ---------------------------------------------------------------------------

bool TweakBar::add_float(const String &p_bar, const String &p_name, const String &p_def) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	String key = _var_key(p_bar, p_name);
	VarData vd;
	memset(&vd, 0, sizeof(vd));
	vd.type = VarData::FLOAT;
	m_vars[key] = vd;

	VarCBInfo *info = memnew(VarCBInfo);
	info->self = this;
	info->bar_name = p_bar;
	info->var_name = p_name;
	info->type = VarData::FLOAT;
	m_cb_infos.push_back(info);

	return TwAddVarCB(bar, p_name.utf8().get_data(), TW_TYPE_FLOAT, _set_cb, _get_cb, info, p_def.utf8().get_data()) == 1;
}

bool TweakBar::add_int(const String &p_bar, const String &p_name, const String &p_def) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	String key = _var_key(p_bar, p_name);
	VarData vd;
	memset(&vd, 0, sizeof(vd));
	vd.type = VarData::INT;
	m_vars[key] = vd;

	VarCBInfo *info = memnew(VarCBInfo);
	info->self = this;
	info->bar_name = p_bar;
	info->var_name = p_name;
	info->type = VarData::INT;
	m_cb_infos.push_back(info);

	return TwAddVarCB(bar, p_name.utf8().get_data(), TW_TYPE_INT32, _set_cb, _get_cb, info, p_def.utf8().get_data()) == 1;
}

bool TweakBar::add_bool(const String &p_bar, const String &p_name, const String &p_def) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	String key = _var_key(p_bar, p_name);
	VarData vd;
	memset(&vd, 0, sizeof(vd));
	vd.type = VarData::BOOL;
	m_vars[key] = vd;

	VarCBInfo *info = memnew(VarCBInfo);
	info->self = this;
	info->bar_name = p_bar;
	info->var_name = p_name;
	info->type = VarData::BOOL;
	m_cb_infos.push_back(info);

	return TwAddVarCB(bar, p_name.utf8().get_data(), TW_TYPE_BOOL32, _set_cb, _get_cb, info, p_def.utf8().get_data()) == 1;
}

bool TweakBar::add_string(const String &p_bar, const String &p_name, const String &p_def) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	String key = _var_key(p_bar, p_name);
	m_string_vars[key] = "";

	VarCBInfo *info = memnew(VarCBInfo);
	info->self = this;
	info->bar_name = p_bar;
	info->var_name = p_name;
	info->type = VarData::STRING;
	m_cb_infos.push_back(info);

	return TwAddVarCB(bar, p_name.utf8().get_data(), TW_TYPE_STDSTRING, _set_cb, _get_cb, info, p_def.utf8().get_data()) == 1;
}

bool TweakBar::add_color3(const String &p_bar, const String &p_name, const String &p_def) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	String key = _var_key(p_bar, p_name);
	VarData vd;
	memset(&vd, 0, sizeof(vd));
	vd.type = VarData::COLOR3;
	vd.c3[0] = 1.0f;
	vd.c3[1] = 1.0f;
	vd.c3[2] = 1.0f;
	m_vars[key] = vd;

	VarCBInfo *info = memnew(VarCBInfo);
	info->self = this;
	info->bar_name = p_bar;
	info->var_name = p_name;
	info->type = VarData::COLOR3;
	m_cb_infos.push_back(info);

	return TwAddVarCB(bar, p_name.utf8().get_data(), TW_TYPE_COLOR3F, _set_cb, _get_cb, info, p_def.utf8().get_data()) == 1;
}

bool TweakBar::add_color4(const String &p_bar, const String &p_name, const String &p_def) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	String key = _var_key(p_bar, p_name);
	VarData vd;
	memset(&vd, 0, sizeof(vd));
	vd.type = VarData::COLOR4;
	vd.c4[0] = 1.0f;
	vd.c4[1] = 1.0f;
	vd.c4[2] = 1.0f;
	vd.c4[3] = 1.0f;
	m_vars[key] = vd;

	VarCBInfo *info = memnew(VarCBInfo);
	info->self = this;
	info->bar_name = p_bar;
	info->var_name = p_name;
	info->type = VarData::COLOR4;
	m_cb_infos.push_back(info);

	return TwAddVarCB(bar, p_name.utf8().get_data(), TW_TYPE_COLOR4F, _set_cb, _get_cb, info, p_def.utf8().get_data()) == 1;
}

bool TweakBar::add_direction(const String &p_bar, const String &p_name, const String &p_def) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	String key = _var_key(p_bar, p_name);
	VarData vd;
	memset(&vd, 0, sizeof(vd));
	vd.type = VarData::DIR3;
	vd.d3[1] = 1.0f; // default: up
	m_vars[key] = vd;

	VarCBInfo *info = memnew(VarCBInfo);
	info->self = this;
	info->bar_name = p_bar;
	info->var_name = p_name;
	info->type = VarData::DIR3;
	m_cb_infos.push_back(info);

	return TwAddVarCB(bar, p_name.utf8().get_data(), TW_TYPE_DIR3F, _set_cb, _get_cb, info, p_def.utf8().get_data()) == 1;
}

bool TweakBar::add_button(const String &p_bar, const String &p_name, const String &p_def) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	VarCBInfo *info = memnew(VarCBInfo);
	info->self = this;
	info->bar_name = p_bar;
	info->var_name = p_name;
	m_btn_infos.push_back(info);

	return TwAddButton(bar, p_name.utf8().get_data(), _btn_cb, info, p_def.utf8().get_data()) == 1;
}

bool TweakBar::add_separator(const String &p_bar, const String &p_name) {
	_ensure_init();
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	return TwAddSeparator(bar, p_name.utf8().get_data(), NULL) == 1;
}

bool TweakBar::remove_var(const String &p_bar, const String &p_name) {
	if (!initialized)
		return false;
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	String key = _var_key(p_bar, p_name);
	m_vars.erase(key);
	m_string_vars.erase(key);

	return TwRemoveVar(bar, p_name.utf8().get_data()) == 1;
}

bool TweakBar::remove_all_vars(const String &p_bar) {
	if (!initialized)
		return false;
	TwBar *bar = TwGetBarByName(p_bar.utf8().get_data());
	ERR_FAIL_COND_V(!bar, false);

	return TwRemoveAllVars(bar) == 1;
}

// ---------------------------------------------------------------------------
// Get/set variable values
// ---------------------------------------------------------------------------

Variant TweakBar::get_value(const String &p_bar, const String &p_name) const {
	String key = _var_key(p_bar, p_name);

	if (m_string_vars.has(key)) {
		return m_string_vars[key];
	}

	if (!m_vars.has(key))
		return Variant();

	const VarData &vd = m_vars[key];
	switch (vd.type) {
		case VarData::FLOAT:
			return vd.f;
		case VarData::INT:
			return vd.i;
		case VarData::BOOL:
			return vd.b;
		case VarData::COLOR3:
			return Color(vd.c3[0], vd.c3[1], vd.c3[2]);
		case VarData::COLOR4:
			return Color(vd.c4[0], vd.c4[1], vd.c4[2], vd.c4[3]);
		case VarData::DIR3:
			return Vector3(vd.d3[0], vd.d3[1], vd.d3[2]);
		default:
			return Variant();
	}
}

void TweakBar::set_value(const String &p_bar, const String &p_name, const Variant &p_value) {
	String key = _var_key(p_bar, p_name);

	if (m_string_vars.has(key)) {
		m_string_vars[key] = p_value;
		return;
	}

	if (!m_vars.has(key))
		return;

	VarData &vd = m_vars[key];
	switch (vd.type) {
		case VarData::FLOAT:
			vd.f = p_value;
			break;
		case VarData::INT:
			vd.i = p_value;
			break;
		case VarData::BOOL:
			vd.b = p_value;
			break;
		case VarData::COLOR3: {
			Color c = p_value;
			vd.c3[0] = c.r;
			vd.c3[1] = c.g;
			vd.c3[2] = c.b;
		} break;
		case VarData::COLOR4: {
			Color c = p_value;
			vd.c4[0] = c.r;
			vd.c4[1] = c.g;
			vd.c4[2] = c.b;
			vd.c4[3] = c.a;
		} break;
		case VarData::DIR3: {
			Vector3 v = p_value;
			vd.d3[0] = v.x;
			vd.d3[1] = v.y;
			vd.d3[2] = v.z;
		} break;
		case VarData::STRING:
			// handled above via m_string_vars
			break;
	}
}

// ---------------------------------------------------------------------------
// Info
// ---------------------------------------------------------------------------

String TweakBar::get_last_error() const {
	if (!initialized)
		return "";
	const char *err = TwGetLastError();
	return err ? String::utf8(err) : "";
}

void TweakBar::refresh() {
	update();
}

// ---------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------

void TweakBar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("new_bar", "name"), &TweakBar::new_bar);
	ClassDB::bind_method(D_METHOD("delete_bar", "name"), &TweakBar::delete_bar);
	ClassDB::bind_method(D_METHOD("get_bar_count"), &TweakBar::get_bar_count);
	ClassDB::bind_method(D_METHOD("define", "def"), &TweakBar::define);

	ClassDB::bind_method(D_METHOD("add_float", "bar", "name", "def"), &TweakBar::add_float, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_int", "bar", "name", "def"), &TweakBar::add_int, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_bool", "bar", "name", "def"), &TweakBar::add_bool, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_string", "bar", "name", "def"), &TweakBar::add_string, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_color3", "bar", "name", "def"), &TweakBar::add_color3, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_color4", "bar", "name", "def"), &TweakBar::add_color4, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_direction", "bar", "name", "def"), &TweakBar::add_direction, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_button", "bar", "name", "def"), &TweakBar::add_button, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_separator", "bar", "name"), &TweakBar::add_separator);
	ClassDB::bind_method(D_METHOD("remove_var", "bar", "name"), &TweakBar::remove_var);
	ClassDB::bind_method(D_METHOD("remove_all_vars", "bar"), &TweakBar::remove_all_vars);

	ClassDB::bind_method(D_METHOD("get_value", "bar", "name"), &TweakBar::get_value);
	ClassDB::bind_method(D_METHOD("set_value", "bar", "name", "value"), &TweakBar::set_value);

	ClassDB::bind_method(D_METHOD("get_last_error"), &TweakBar::get_last_error);
	ClassDB::bind_method(D_METHOD("refresh"), &TweakBar::refresh);

	// Input forwarding — bound as "_input" so the scene tree calls it
	ClassDB::bind_method(D_METHOD("_input", "event"), &TweakBar::_handle_input);

	ADD_SIGNAL(MethodInfo("value_changed",
			PropertyInfo(Variant::STRING, "bar_name"),
			PropertyInfo(Variant::STRING, "var_name")));
	ADD_SIGNAL(MethodInfo("button_pressed",
			PropertyInfo(Variant::STRING, "bar_name"),
			PropertyInfo(Variant::STRING, "button_name")));
}

// ---------------------------------------------------------------------------
// Test suite
// ---------------------------------------------------------------------------

#include "thirdparty/doctest/doctest.h"

TEST_SUITE("anttweakbar") {
	TEST_CASE("lifecycle") {
		int result = TwInit(NULL);
		CHECK(result == 1);

		result = TwWindowSize(800, 600);
		CHECK(result == 1);

		result = TwTerminate();
		CHECK(result == 1);
	}

	TEST_CASE("multiple_init_terminate_cycles") {
		CHECK(TwInit(NULL) == 1);
		CHECK(TwTerminate() == 1);
		CHECK(TwInit(NULL) == 1);
		CHECK(TwTerminate() == 1);
	}

	TEST_CASE("bar_create_delete") {
		TwInit(NULL);

		int base_count = TwGetBarCount(); // ATB may create internal help bar
		TwBar *bar = TwNewBar("TestBar");
		CHECK(bar != NULL);
		CHECK(TwGetBarCount() == base_count + 1);
		CHECK(TwGetBarByName("TestBar") == bar);

		CHECK(TwDeleteBar(bar) == 1);
		CHECK(TwGetBarCount() == base_count);

		TwTerminate();
	}

	TEST_CASE("bar_name") {
		TwInit(NULL);

		TwBar *bar = TwNewBar("NamedBar");
		CHECK(String(TwGetBarName(bar)) == "NamedBar");

		TwDeleteBar(bar);
		TwTerminate();
	}

	TEST_CASE("multiple_bars") {
		TwInit(NULL);

		int base_count = TwGetBarCount(); // ATB may create internal help bar
		TwBar *bar1 = TwNewBar("Bar1");
		TwBar *bar2 = TwNewBar("Bar2");
		CHECK(TwGetBarCount() == base_count + 2);
		TwBar *b0 = TwGetBarByIndex(0);
		TwBar *b1 = TwGetBarByIndex(1);
		CHECK(b0 != nullptr);
		CHECK(b1 != nullptr);
		CHECK(bar1 != bar2);

		TwDeleteBar(bar1);
		TwDeleteBar(bar2);
		CHECK(TwGetBarCount() == base_count);

		TwTerminate();
	}

	TEST_CASE("top_bottom_bar") {
		TwInit(NULL);

		TwBar *bar1 = TwNewBar("Top");
		TwBar *bar2 = TwNewBar("Bottom");

		CHECK(TwSetTopBar(bar1) == 1);
		CHECK(TwGetTopBar() == bar1);
		CHECK(TwSetBottomBar(bar2) == 1);
		CHECK(TwGetBottomBar() == bar2);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("float_var_rw") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("VarBar");

		float val = 3.14f;
		CHECK(TwAddVarRW(bar, "speed", TW_TYPE_FLOAT, &val, "min=0 max=10") == 1);
		CHECK(val == doctest::Approx(3.14f));

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("int_var_rw") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("IntBar");

		int val = 42;
		CHECK(TwAddVarRW(bar, "count", TW_TYPE_INT32, &val, "min=0 max=100") == 1);
		CHECK(val == 42);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("bool_var_rw") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("BoolBar");

		int val = 1;
		CHECK(TwAddVarRW(bar, "active", TW_TYPE_BOOL32, &val, "") == 1);
		CHECK(val == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("color3_var") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("ColBar");

		float col[3] = { 1.0f, 0.5f, 0.0f };
		CHECK(TwAddVarRW(bar, "color", TW_TYPE_COLOR3F, col, "") == 1);
		CHECK(col[0] == doctest::Approx(1.0f));
		CHECK(col[1] == doctest::Approx(0.5f));

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("color4_var") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("Col4Bar");

		float col[4] = { 1.0f, 0.5f, 0.0f, 0.8f };
		CHECK(TwAddVarRW(bar, "color4", TW_TYPE_COLOR4F, col, "") == 1);
		CHECK(col[3] == doctest::Approx(0.8f));

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("direction_var") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("DirBar");

		float dir[3] = { 0.0f, 1.0f, 0.0f };
		CHECK(TwAddVarRW(bar, "dir", TW_TYPE_DIR3F, dir, "") == 1);
		CHECK(dir[1] == doctest::Approx(1.0f));

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("read_only_var") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("ROBar");

		float val = 1.0f;
		CHECK(TwAddVarRO(bar, "ro_val", TW_TYPE_FLOAT, &val, "") == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("callback_var") {
		static float cb_val = 5.0f;

		static auto set_cb = [](const void *value, void *data) {
			cb_val = *(const float *)value;
		};
		static auto get_cb = [](void *value, void *data) {
			*(float *)value = cb_val;
		};

		TwInit(NULL);
		TwBar *bar = TwNewBar("CBBar");

		CHECK(TwAddVarCB(bar, "cb_val", TW_TYPE_FLOAT,
					  (TwSetVarCallback)set_cb,
					  (TwGetVarCallback)get_cb,
					  NULL, "min=0 max=10") == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("button") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("BtnBar");

		CHECK(TwAddButton(bar, "reset", NULL, NULL, "label='Reset'") == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("separator") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("SepBar");

		CHECK(TwAddSeparator(bar, "sep1", NULL) == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("remove_var") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("RemBar");

		float val = 0;
		TwAddVarRW(bar, "temp", TW_TYPE_FLOAT, &val, "");
		CHECK(TwRemoveVar(bar, "temp") == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("remove_all_vars") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("RemAllBar");

		float v1 = 0, v2 = 0;
		TwAddVarRW(bar, "a", TW_TYPE_FLOAT, &v1, "");
		TwAddVarRW(bar, "b", TW_TYPE_FLOAT, &v2, "");
		CHECK(TwRemoveAllVars(bar) == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("define_bar_properties") {
		TwInit(NULL);
		TwNewBar("DefBar");

		CHECK(TwDefine("DefBar label='Test Bar'") == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("define_enum") {
		TwInit(NULL);

		TwEnumVal enumVals[] = {
			{ 0, "Low" },
			{ 1, "Medium" },
			{ 2, "High" }
		};
		TwType enumType = TwDefineEnum("Quality", enumVals, 3);
		CHECK(enumType != TW_TYPE_UNDEF);

		TwTerminate();
	}

	TEST_CASE("define_enum_from_string") {
		TwInit(NULL);

		TwType enumType = TwDefineEnumFromString("Mode", "Auto,Manual,Custom");
		CHECK(enumType != TW_TYPE_UNDEF);

		TwTerminate();
	}

	TEST_CASE("param_set_get") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("ParamBar");

		const char *label = "My Bar";
		CHECK(TwSetParam(bar, NULL, "label", TW_PARAM_CSTRING, 1, label) == 1);

		char buf[128] = {};
		int r = TwGetParam(bar, NULL, "label", TW_PARAM_CSTRING, sizeof(buf), buf);
		CHECK(r >= 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("refresh_bar") {
		TwInit(NULL);
		TwBar *bar = TwNewBar("RefreshBar");

		CHECK(TwRefreshBar(bar) == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("draw_cycle") {
		TwInit(NULL);
		TwWindowSize(800, 600);
		TwBar *bar = TwNewBar("DrawBar");
		float val = 1.0f;
		TwAddVarRW(bar, "val", TW_TYPE_FLOAT, &val, "");

		// TwDraw should not crash
		CHECK(TwDraw() == 1);

		TwDeleteAllBars();
		TwTerminate();
	}

	TEST_CASE("window_operations") {
		TwInit(NULL);
		TwWindowSize(1024, 768);

		CHECK(TwSetCurrentWindow(0) == 1);
		CHECK(TwGetCurrentWindow() == 0);
		CHECK(TwWindowExists(0) == 1);

		TwTerminate();
	}

	TEST_CASE("error_handling") {
		TwInit(NULL);

		CHECK(TwDeleteBar(NULL) == 0);
		const char *err = TwGetLastError();
		CHECK(err != NULL);

		TwTerminate();
	}

	TEST_CASE("wrapper_default_state") {
		TweakBar tw;
		// Before initialization, bar count is 0
		CHECK(tw.get_bar_count() == 0);
		CHECK(tw.get_last_error() == "");
	}

	TEST_CASE("wrapper_bar_management") {
		TweakBar tw;

		// Create first bar to trigger _ensure_init, then capture base count
		CHECK(tw.new_bar("WrapBar") == true);
		int count_with_bar = tw.get_bar_count();
		CHECK(count_with_bar >= 1);

		tw.delete_bar("WrapBar");
		CHECK(tw.get_bar_count() == count_with_bar - 1);

		// Cleanup — TweakBar destructor calls TwTerminate
	}

	TEST_CASE("wrapper_float_var") {
		TweakBar tw;
		tw.new_bar("FloatBar");
		CHECK(tw.add_float("FloatBar", "speed", "min=0 max=10 step=0.1") == true);

		// Default value is 0
		Variant val = tw.get_value("FloatBar", "speed");
		CHECK(float(val) == doctest::Approx(0.0f));

		// Set and get
		tw.set_value("FloatBar", "speed", 5.5f);
		val = tw.get_value("FloatBar", "speed");
		CHECK(float(val) == doctest::Approx(5.5f));
	}

	TEST_CASE("wrapper_int_var") {
		TweakBar tw;
		tw.new_bar("IntBar");
		CHECK(tw.add_int("IntBar", "count", "min=0 max=100") == true);

		tw.set_value("IntBar", "count", 42);
		Variant val = tw.get_value("IntBar", "count");
		CHECK(int(val) == 42);
	}

	TEST_CASE("wrapper_bool_var") {
		TweakBar tw;
		tw.new_bar("BoolBar");
		CHECK(tw.add_bool("BoolBar", "active", "") == true);

		// Default false
		Variant val = tw.get_value("BoolBar", "active");
		CHECK(bool(val) == false);

		tw.set_value("BoolBar", "active", true);
		val = tw.get_value("BoolBar", "active");
		CHECK(bool(val) == true);
	}

	TEST_CASE("wrapper_color3_var") {
		TweakBar tw;
		tw.new_bar("ColBar");
		CHECK(tw.add_color3("ColBar", "tint", "") == true);

		// Default white
		Color c = tw.get_value("ColBar", "tint");
		CHECK(c.r == doctest::Approx(1.0f));
		CHECK(c.g == doctest::Approx(1.0f));
		CHECK(c.b == doctest::Approx(1.0f));

		tw.set_value("ColBar", "tint", Color(0.5f, 0.3f, 0.1f));
		c = tw.get_value("ColBar", "tint");
		CHECK(c.r == doctest::Approx(0.5f));
		CHECK(c.g == doctest::Approx(0.3f));
		CHECK(c.b == doctest::Approx(0.1f));
	}

	TEST_CASE("wrapper_color4_var") {
		TweakBar tw;
		tw.new_bar("Col4Bar");
		CHECK(tw.add_color4("Col4Bar", "overlay", "") == true);

		tw.set_value("Col4Bar", "overlay", Color(1, 0, 0, 0.5f));
		Color c = tw.get_value("Col4Bar", "overlay");
		CHECK(c.r == doctest::Approx(1.0f));
		CHECK(c.a == doctest::Approx(0.5f));
	}

	TEST_CASE("wrapper_direction_var") {
		TweakBar tw;
		tw.new_bar("DirBar");
		CHECK(tw.add_direction("DirBar", "wind", "") == true);

		// Default direction is up (0, 1, 0)
		Vector3 d = tw.get_value("DirBar", "wind");
		CHECK(d.y == doctest::Approx(1.0f));

		tw.set_value("DirBar", "wind", Vector3(1, 0, 0));
		d = tw.get_value("DirBar", "wind");
		CHECK(d.x == doctest::Approx(1.0f));
	}

	TEST_CASE("wrapper_button") {
		TweakBar tw;
		tw.new_bar("BtnBar");
		CHECK(tw.add_button("BtnBar", "reset", "label='Reset All'") == true);
	}

	TEST_CASE("wrapper_separator") {
		TweakBar tw;
		tw.new_bar("SepBar");
		CHECK(tw.add_separator("SepBar", "sep1") == true);
	}

	TEST_CASE("wrapper_remove_var") {
		TweakBar tw;
		tw.new_bar("RemBar");
		tw.add_float("RemBar", "temp", "");
		CHECK(tw.remove_var("RemBar", "temp") == true);

		// Value should be gone
		Variant v = tw.get_value("RemBar", "temp");
		CHECK(v.get_type() == Variant::NIL);
	}

	TEST_CASE("wrapper_remove_all_vars") {
		TweakBar tw;
		tw.new_bar("RemAllBar");
		tw.add_float("RemAllBar", "a", "");
		tw.add_int("RemAllBar", "b", "");
		CHECK(tw.remove_all_vars("RemAllBar") == true);
	}

	TEST_CASE("wrapper_define") {
		TweakBar tw;
		tw.new_bar("DefBar");
		tw.define("DefBar label='My Tweaks' size='300 400'");
		// Should not crash; properties applied to bar
	}

	TEST_CASE("wrapper_nonexistent_bar") {
		TweakBar tw;
		// Adding to nonexistent bar should fail gracefully
		CHECK(tw.add_float("NoBar", "val", "") == false);
	}

	TEST_CASE("wrapper_get_nonexistent_var") {
		TweakBar tw;
		tw.new_bar("GetBar");
		Variant v = tw.get_value("GetBar", "missing");
		CHECK(v.get_type() == Variant::NIL);
	}

	TEST_CASE("wrapper_multiple_bars") {
		TweakBar tw;
		tw.new_bar("Settings"); // triggers _ensure_init
		int count_after_first = tw.get_bar_count();
		tw.new_bar("Debug");
		CHECK(tw.get_bar_count() == count_after_first + 1);

		tw.add_float("Settings", "speed", "");
		tw.add_float("Debug", "fps", "");

		tw.set_value("Settings", "speed", 10.0f);
		tw.set_value("Debug", "fps", 60.0f);

		CHECK(float(tw.get_value("Settings", "speed")) == doctest::Approx(10.0f));
		CHECK(float(tw.get_value("Debug", "fps")) == doctest::Approx(60.0f));

		tw.delete_bar("Settings");
		CHECK(tw.get_bar_count() == count_after_first);
	}

	TEST_CASE("wrapper_mixed_var_types") {
		TweakBar tw;
		tw.new_bar("Mix");

		tw.add_float("Mix", "speed", "");
		tw.add_int("Mix", "count", "");
		tw.add_bool("Mix", "active", "");
		tw.add_color3("Mix", "color", "");
		tw.add_direction("Mix", "dir", "");
		tw.add_button("Mix", "reset", "");
		tw.add_separator("Mix", "sep");

		tw.set_value("Mix", "speed", 3.14f);
		tw.set_value("Mix", "count", 7);
		tw.set_value("Mix", "active", true);

		CHECK(float(tw.get_value("Mix", "speed")) == doctest::Approx(3.14f));
		CHECK(int(tw.get_value("Mix", "count")) == 7);
		CHECK(bool(tw.get_value("Mix", "active")) == true);
	}
}
