#ifndef BOX2D_SHAPE_H
#define BOX2D_SHAPE_H

#include "core/reference.h"

class ShapeB2 : public Reference {
	GDCLASS(ShapeB2, Reference);

protected:
	static void _bind_methods();

	/** Box2D entity */
	class b2Shape *entity;
	bool exclusive;

public:
	/** Box2D methods */
	bool test_point(const Transform2D &xf, const Vector2 &point);

	/** Box2D accessor */
	const class b2Shape *get_b2() const;

	ShapeB2(class b2Shape *, bool);
	~ShapeB2();
};

#endif // BOX2D_SHAPE_H
