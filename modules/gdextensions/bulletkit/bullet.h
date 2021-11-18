#ifndef BULLET_H
#define BULLET_H

#include "core/object.h"
#include "core/math/transform_2d.h"

struct BulletID {
	int32_t index;
	int32_t cycle;
	int32_t set;

	BulletID(int32_t index, int32_t cycle, int32_t set): 
		index(index), cycle(cycle), set(set) {}
};

class Bullet : public Object {
	GDCLASS(Bullet, Object)

public:
	RID item_rid;
	int32_t cycle = 0;
	int32_t shape_index = -1;
	Transform2D transform;
	Vector2 velocity;
	float lifetime = 0;
	Variant data;

	void _init() {}

	RID get_item_rid() { return item_rid; }
	void set_item_rid(RID value) { ERR_PRINT("Can't edit the item rid of bullets!"); }

	int32_t get_cycle() { return cycle; }
	void set_cycle(int32_t value) { ERR_PRINT("Can't edit the cycle of bullets!"); }

	int32_t get_shape_index() { return shape_index; }
	void set_shape_index(int32_t value) { ERR_PRINT("Can't edit the shape index of bullets!"); }

	bool _get(const StringName &p_name, Variant &r_ret) const {
		if (p_name == "data") { r_ret = data; return true; }
		return false;
	}

	static void _register_methods() {
		ClassDB::bind_method(D_METHOD("get_item_rid"), &Bullet::get_item_rid);
		ClassDB::bind_method(D_METHOD("set_item_rid", "rid"), &Bullet::set_item_rid);
		ClassDB::bind_method(D_METHOD("get_cycle", "rid"), &Bullet::get_cycle);
		ClassDB::bind_method(D_METHOD("set_cycle", "cycle"), &Bullet::set_cycle);
		ClassDB::bind_method(D_METHOD("get_shape_index", "rid"), &Bullet::get_shape_index);
		ClassDB::bind_method(D_METHOD("set_shape_index", "index"), &Bullet::set_shape_index);

		ADD_PROPERTY(PropertyInfo(Variant::_RID, "item_rid"), "set_stage_manager", "get_stage_manager");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "cycle"), "set_stage_manager", "get_stage_manager");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "shape_index"), "set_stage_manager", "get_stage_manager");

		ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "transform"), "set_transform", "get_transform");
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "velocity"), "set_velocity", "get_velocity");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime"), "set_lifetime", "get_lifetime");
	}
};

#endif // BULLET_H
