#ifndef AREAPROBER_H
#define AREAPROBER_H

#include "core/reference.h"
#include "core/os/os.h"
#include "core/object.h"
#include "core/resource.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/shape_2d.h"

#include <vector>

class AreaProber : public Node2D {
	GDCLASS(AreaProber, Node2D);
protected:
	static void _bind_methods();
public:
	Array probe(Vector2 position);
  	void set_collision_shape(const Ref<Shape2D> &p_shape);
	Ref<Shape2D> get_collision_shape() const;

	void set_collision_mask(int p_mask);
	int get_collision_mask() const;

	void set_collision_detect_bodies(bool p_enabled);
	bool get_collision_detect_bodies() const;

	void set_collision_detect_areas(bool p_enabled);
	bool get_collision_detect_areas() const;
	AreaProber();
private:
	Ref<Shape2D> collision_shape;
	int collision_mask;
	bool collision_detect_bodies;
	bool collision_detect_areas; 

	
};

#endif // AREAPROBER_H
