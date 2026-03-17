/**************************************************************************/
/*  gd_softrender_display.h                                               */
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

#ifndef GD_SOFTRENDER_DISPLAY_H
#define GD_SOFTRENDER_DISPLAY_H

// Node2D that displays a SoftRender framebuffer as a texture.
// Add as a child of any Node2D, then use get_soft_render() to draw into it.
//
// Example: Add SoftRenderDisplay node in editor, then in GDScript:
//
//   func _ready():
//       var sr = $SoftRenderDisplay.get_soft_render()
//       sr.clear_color(Color(0.2, 0.2, 0.3, 1))
//       sr.clear(SoftRender.COLOR_BUFFER_BIT)
//       sr.matrix_mode(SoftRender.MATRIX_PROJECTION)
//       sr.load_identity()
//       sr.perspective(60, 320.0/240.0, 0.1, 100.0)
//       sr.matrix_mode(SoftRender.MATRIX_MODELVIEW)
//       sr.load_identity()
//       sr.translate(Vector3(0, 0, -3))
//       sr.begin_mesh(SoftRender.PRIM_TRIANGLES)
//       sr.color4(Color.yellow)
//       sr.vertex3(Vector3(0, 1, 0))
//       sr.vertex3(Vector3(-1, -1, 0))
//       sr.vertex3(Vector3(1, -1, 0))
//       sr.end_mesh()
//       $SoftRenderDisplay.refresh()  # triggers redraw
//
// Inspector properties:
//   canvas_size: framebuffer resolution (default 256x256)
//   centered: offset drawing so node position is at center
//   backend: PortableGL (default) or Fusion2X

#include "gd_softrender.h"
#include "scene/2d/node_2d.h"

class SoftRenderDisplay : public Node2D {
	GDCLASS(SoftRenderDisplay, Node2D);

	Size2 canvas_size;
	bool centered;
	int backend;
	Ref<SoftRender> soft_render;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;
#endif

	void set_canvas_size(const Size2 &p_size);
	Size2 get_canvas_size() const;
	void set_centered(bool p_centered);
	bool is_centered() const;
	void set_backend(int p_backend);
	int get_backend() const;
	Ref<SoftRender> get_soft_render();
	void refresh();

	SoftRenderDisplay();
};

#endif // GD_SOFTRENDER_DISPLAY_H
