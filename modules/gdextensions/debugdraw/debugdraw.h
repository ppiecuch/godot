/*************************************************************************/
/*  debugdraw.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H

#include "core/list.h"
#include "core/object.h"
#include "scene/resources/font.h"

class DebugDraw : public Object {
	GDCLASS(DebugDraw, Object);

public:
	/** Lifecycle */
	DebugDraw();
	~DebugDraw();

	/** Singleton */
	static DebugDraw *get_singleton();

	/** Methods */
	void circle(const Vector2 &position, float radius, const Color &color, float duration = .0f);
	void line(const Vector2 &a, const Vector2 &b, const Color &color, float width = 1, float duration = .0f);
	void rect(const Rect2 &rect, const Color &color, float width = 1, float duration = .0f);
	void area(const Rect2 &rect, const Color &color, float duration = .0f);
	void print(const String &text, const Color &color, float duration = .0f);

	void clear();
	void _idle_frame();

protected:
	/** Godot bindings */
	static void _bind_methods();

	/** Singleton */
	static DebugDraw *singleton;

	/** Current drawings */
	struct Drawing {
		RID canvas_item;
		float time_left;
	};

	List<Drawing> drawings, prints;
	Ref<Font> default_font;
	RID canvas;

	/** State */
	bool init();
	bool ready;
};

#endif // DEBUGDRAW_H
