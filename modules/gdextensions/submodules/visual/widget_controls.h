#ifndef WIDGET_CONTROLS_H
#define WIDGET_CONTROLS_H

#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"

enum WidgetType {
	WIDGET_TRANSLATION_XY,
	WIDGET_TRANSLATION_Z,
	WIDGET_ROTATION,
};

class ControlWidget : public Node2D {
	GDCLASS(ControlWidget, Node2D);

	WidgetType control;
	Ref<ArrayMesh> mesh;
	int radius;

	bool _dirty;
	bool _enabled, _press;
	char _locked;

protected:
	static void _bind_methods();
	void notifications(int p_what);

	void _input(const Ref<InputEvent> &p_event);

public:
	void set_control_type(WidgetType p_type);
	WidgetType get_control_type() const;

	ControlWidget();
	~ControlWidget();
};

#endif // WIDGET_CONTROLS_H
