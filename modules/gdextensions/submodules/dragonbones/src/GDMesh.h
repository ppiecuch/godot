/**************************************************************************/
/*  GDMesh.h                                                              */
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

#ifndef GDMESH_H
#define GDMESH_H

#include "GDDisplay.h"

class GDMesh : public GDDisplay {
	GDCLASS(GDMesh, GDDisplay);

private:
	GDMesh(const GDMesh &);

public:
	Vector<int> indices;
	Vector<Color> verticesColor;
	Vector<Point2> verticesUV;
	Vector<Point2> verticesPos;

	Color col_debug;

public:
	GDMesh() { col_debug = Color(Math::random(0.5f, 1.f), Math::random(0.3f, 1.f), Math::random(0.3f, 1.f), 1); }
	virtual ~GDMesh() {}

	static GDMesh *create() {
		return memnew(GDMesh);
	}

	void _render() {
		if (indices.empty())
			return;

		if (texture.is_valid()) {
			VS::get_singleton()->canvas_item_add_triangle_array(
					get_canvas_item(),
					indices,
					verticesPos,
					verticesColor,
					verticesUV,
					Vector<int>(),
					Vector<float>(),
					texture.is_valid() ? texture->get_rid() : RID(),
					-1);
		}

		if (b_debug || !texture.is_valid()) {
			for (int idx = 0; idx < indices.size(); idx += 3) {
				VS::get_singleton()->canvas_item_add_line(get_canvas_item(), verticesPos[indices[idx]], verticesPos[indices[idx + 1]], col_debug, 1.0);
				VS::get_singleton()->canvas_item_add_line(get_canvas_item(), verticesPos[indices[idx + 1]], verticesPos[indices[idx + 2]], col_debug, 1.0);
				VS::get_singleton()->canvas_item_add_line(get_canvas_item(), verticesPos[indices[idx + 2]], verticesPos[indices[idx]], col_debug, 1.0);
			}
		}
	}

	virtual void set_modulate(const Color &col) {
		GDDisplay::set_modulate(col);
		col_debug.a = modulate.a;
		for (long i = 0; i < verticesColor.size(); ++i) {
			verticesColor.write[i] = modulate;
		}
	}

	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_DRAW:
				_render();
				break;
		}
	}
};

#endif // GDMESH_H
