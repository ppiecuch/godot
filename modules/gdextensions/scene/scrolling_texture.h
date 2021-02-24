#ifndef GD_SCROLLING_TEXTURE_H
#define GD_SCROLLING_TEXTURE_H

#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

class ScrollingTexture : public Node2D {

	GDCLASS(ScrollingTexture, Node2D);

private:
	Ref<Texture> texture;
	Size2 texture_scale;
	Point2 texture_offset;
	Vector2 view_size;
	Vector2 scrolling_speed;
	bool scrolling_active;
	Color modulate;

	void _refresh();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	bool _edit_use_rect() const;
#endif

	void set_scrolling_speed(const Vector2 &p_speed);
	Vector2 get_scrolling_speed() const;

	void set_scrolling_active(bool p_state);
	bool is_scrolling_active() const;

	void set_view_size(const Size2 &p_size);
	Size2 get_view_size() const;

	ScrollingTexture();
	~ScrollingTexture();
};

#endif // GD_SCROLLING_TEXTURE_H
