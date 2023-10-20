/**************************************************************************/
/*  paint_editor_plugin.h                                                 */
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

#include "core/image.h"
#include "editor/editor_plugin.h"

class MPBrush;
class MPTile;
class MPSurface;

class PaintEditorPlugin : public EditorPlugin {
	GDCLASS(PaintEditorPlugin, EditorPlugin)

	EditorNode *editor;

	static bool instance_flag;

	MPBrush *brush;
	MPSurface *surface;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	typedef void (*MPOnUpdateFunction)(PaintEditorPlugin *handler, MPSurface *surface, MPTile *tile);

	void start_stroke();
	void stroke_to(real_t x, real_t y, real_t pressure, real_t xtilt, real_t ytilt);
	void stroke_to(real_t x, real_t y);
	void end_stroke();

	real_t get_brush_value(MyPaintBrushSetting setting);

	void set_brush_color(Color newColor);
	void set_brush_value(MyPaintBrushSetting setting, real_t value);

	void request_update_tile(MPSurface *surface, MPTile *tile);
	void has_new_tile(MPSurface *surface, MPTile *tile);
	void has_cleared_surface(MPSurface *surface);

	void set_surface_size(const Size2 &size);
	Size2 get_surface_size() const;

	void clear_surface();
	Ref<Image> render_image();

	void load_image(const Ref<Image> &image);
	void load_brush(const String &content);

	PaintEditorPlugin(EditorNode *p_node);
	~PaintEditorPlugin();
};
