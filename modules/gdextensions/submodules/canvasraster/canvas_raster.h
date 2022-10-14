#include "scene/2d/node_2d.h"

class CanvasIty : public Node2D {
	GDCLASS(CanvasIty, Node2D);

	Size2 canvas_size;

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

	CanvasIty();
};
