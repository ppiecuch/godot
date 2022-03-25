#ifndef WIDGET_CONTROLS_H
#define WIDGET_CONTROLS_H

#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"
#include "scene/resources/theme.h"

enum WidgetType {
	WIDGET_TRANSLATION_XY,
	WIDGET_TRANSLATION_Z,
	WIDGET_ROTATION,
};

constexpr int WIDGET_TYPES = WIDGET_ROTATION + 1;

class ControlWidget : public Node2D {
	GDCLASS(ControlWidget, Node2D);

	WidgetType control_type;
	Ref<Texture> ball_texture;
	Ref<ArrayMesh> mesh;
	Rect2 control_rect;
	bool control_themed;

	Ref<StyleBoxFlat> _style;
	struct StyleInfo {
		real_t width;
		real_t radius;
		Color bg_color;
		real_t shadow_size;
		Color shadow_color;
		Vector2 shadow_offset;
	} _style_info;
	bool _enabled, _press;
	char _locked;

	Rect2 _get_global_rect() const;
	bool _is_point_inside(const Vector2 &vec);

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _input(const Ref<InputEvent> &p_event);

public:
#ifdef TOOLS_ENABLED
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
#endif

	void set_control_type(WidgetType p_type);
	WidgetType get_control_type() const;

	void set_control_themed(bool p_themed);
	bool get_control_themed() const;

	ControlWidget();
};

VARIANT_ENUM_CAST(WidgetType);

#endif // WIDGET_CONTROLS_H
