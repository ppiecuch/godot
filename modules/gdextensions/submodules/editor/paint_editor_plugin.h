
#include "core/image.h"
#include "editor/editor_plugin.h"

class MPBrush;
class MPTile;
class MPSurface;

class PaintEditorPlugin : public EditorPlugin {
	GDCLASS(PaintEditorPlugin, EditorPlugin)

	static bool instance_flag;

	MPBrush *brush;
	MPSurface *surface;

protected:
	static void _bind_methods();

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

	PaintEditorPlugin();
	~PaintEditorPlugin();
};
