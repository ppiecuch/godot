// -*- C++ -*-
//


#ifndef _DESTRUCTIBLE_2D_H_
#define _DESTRUCTIBLE_2D_H_

#include "core/reference.h"
#include "scene/2d/sprite.h"

class DestructibleSprite : public Sprite {
	GDCLASS(DestructibleSprite, Sprite);

private:
	Vector2 force_impulse_direction;
	real_t force_impulse_duration;
	bool fade_debris;

public:
	DestructibleSprite();
	~DestructibleSprite();

	void _notification(int p_what);

protected:

	static void _bind_methods();
};


#endif /* _DESTRUCTIBLE_2D_H_ */
