#ifndef PIXEL_TERRAIN_H
#define PIXEL_TERRAIN_H

#include "core/image.h"
#include "scene/2d/node_2d.h"

class PixelTerrain : public Node2D {
	GDCLASS(PixelTerrain, Node2D)

	Size2 view_size();
	Color clear_color;
	Ref<Image> dbl, map, cm;
	// player:
	int px, py;
	real_t pa;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void action_left(real_t p_step = 0.1);
	void action_right(real_t p_step = 0.1);
	void action_up();
	void action_down();

	PixelTerrain();
};

#endif // PIXEL_TERRAIN_H
