// -*- C++ -*-
//


#ifndef _STARFIELD_2D_H_
#define _STARFIELD_2D_H_

#include "core/reference.h"
#include "scene/2d/node_2d.h"

class Starfield;

class Starfield2D : public Node2D {
	GDCLASS(Starfield2D, Node2D);

private:
	Size2 virtual_size;
	Vector2 movement;

	Ref<Starfield> _starfield;
	Dictionary _image_atlas;

public:
	Starfield2D();
	~Starfield2D();

	void _notification(int p_what);

protected:
	static void _bind_methods();

	void _update();
};


#endif /* _STARFIELD_2D_H_ */
