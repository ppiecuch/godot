/**************************************************************************/
/*  goost_engine.h                                                        */
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

#pragma once

#include "core/object.h"
#include "scene/main/scene_tree.h"

#include "func_buffer.h"
#include "invoke_state.h"

class GoostEngine : public Object {
	GDCLASS(GoostEngine, Object);

private:
	static GoostEngine *singleton;
	FuncBuffer deferred_calls;
	Vector<Ref<InvokeState>> invokes;

protected:
	static void _bind_methods();

	Variant _defer_call_bind(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
	Variant _defer_call_unique_bind(const Variant **p_args, int p_argcount, Variant::CallError &r_error);

	Ref<InvokeState> _invoke(Object *p_obj, StringName p_method, real_t p_delay, real_t p_repeat_rate, bool p_pause_mode_process, bool p_deferred);
	void _on_invoke_timeout(Ref<InvokeState> p_state, bool p_pause_mode, bool p_deferred);

public:
	static GoostEngine *get_singleton() { return singleton; }

	Dictionary get_version_info() const;
	Dictionary get_author_info() const;
	Array get_copyright_info() const;
	Dictionary get_license_info() const;
	String get_license_text() const;

	Dictionary get_color_constants() const;

	void defer_call(Object *p_obj, StringName p_method, VARIANT_ARG_LIST);
	void defer_call_unique(Object *p_obj, StringName p_method, VARIANT_ARG_LIST);

	Ref<InvokeState> invoke(Object *p_obj, StringName p_method, real_t p_delay, real_t p_repeat_rate, bool p_pause_mode_process = true);
	Ref<InvokeState> invoke_deferred(Object *p_obj, StringName p_method, real_t p_delay, real_t p_repeat_rate, bool p_pause_mode_process = true);
	Array get_invokes() const;

	static void flush_calls();

	GoostEngine() {
		ERR_FAIL_COND_MSG(singleton != nullptr, "Singleton already exists");
		singleton = this;
	}
};
