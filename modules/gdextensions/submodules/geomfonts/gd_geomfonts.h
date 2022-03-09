#ifndef GD_GEOMFONTS_H
#define GD_GEOMFONTS_H

#include "core/color.h"
#include "core/reference.h"
#include "core/math/transform_2d.h"
#include "scene/resources/mesh.h"

class GdGeomFonts : public Reference {
	GDCLASS(GdGeomFonts, Reference);

	RID canvas, canvas_item;
	Transform2D xform;
	Color modulate;

	Ref<ArrayMesh> _mesh;

	bool _is_ready();

protected:
	static void _bind_methods();

private:
	void set_transform(const Transform2D &p_xform);
	Transform2D get_transform() const;
	void set_color(const Color &p_color);
	Color get_color() const;

	void clear();
	void finish();

	void bob_font_add_text(const String &p_text, const Point3 &p_pos = Point3(), real_t p_size = 1, bool p_wire = true);

	void easy_font_add_text(const String &p_text, const Point2 &p_pos = Point2(), real_t p_spacing = 0);
	Size2 easy_font_text_size(const String &p_text);

	GdGeomFonts();
	~GdGeomFonts();
};

#endif // GD_GEOMFONTS_H
