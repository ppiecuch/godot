#ifndef 3ds_gfx_hw_h
#define 3ds_gfx_hw_h

#include "scene/2d/node_2d.h"

class NdsSprite : public Node2D {
	GDCLASS(NdsSprite, Node2D);
public:
	NdsSprite();
	~NdsSprite();
};

class NdsBackground : public Node2D {
	GDCLASS(NdsSprite, Node2D);
public:
	NdsBackground();
	~NdsBackground();
};

#endif // 3ds_gfx_hw_h
