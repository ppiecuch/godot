#ifndef BULLET_KIT_H
#define BULLET_KIT_H

#include "core/resource.h"
#include "scene/resources/shape_2d.h"
#include "scene/resources/material.h"
#include "scene/resources/texture.h"
#include "scene/resources/packed_scene.h"

#include <memory>

#include "bullet.h"

#define BULLET_KIT(BulletsPoolType) \
std::unique_ptr<BulletsPool> _create_pool() override;

#define BULLET_KIT_IMPLEMENTATION(BulletKitType, BulletsPoolType) \
std::unique_ptr<BulletsPool> BulletKitType::_create_pool() { return std::unique_ptr<BulletsPool>(new BulletsPoolType()); }


class BulletsPool;

class BulletKit : public Resource {
	GDCLASS(BulletKit, Resource)

public:
	// The material used to render each bullet.
	Ref<Material> material;
	// Controls whether collisions with other objects are enabled. Turning it off increases performance.
	bool collisions_enabled = true;
	// Collisions related properties.
	int32_t collision_layer = 0;
	int32_t collision_mask = 0;
	Ref<Shape2D> collision_shape;
	// Controls whether the active rect is automatically set as the viewport visible rect.
	bool use_viewport_as_active_rect = true;
	// Controls where the bullets can live, if a bullet exits this rect, it will be removed.
	Rect2 active_rect;
	// If enabled, bullets will auto-rotate based on their direction of travel.
	bool rotate = false;
	// Allows the ability to have a unique-ish value in each instance of the bullet material.
	// Can be used to offset the bullets animation by a unique amount to avoid having them animate in sync.
	int32_t unique_modulate_component = 0;
	// Additional data the user can set via the editor.
	Variant data;

	virtual String get_bullet_class_name() const { return ""; }
	virtual Dictionary get_bullet_properties() const { return Dictionary(); }

	bool _get(const StringName &p_name, Variant &r_ret) const {
		if (p_name == "data") { r_ret = data; return true; }
		else if (p_name == "bullet_class_name") { r_ret = get_bullet_class_name(); return true; }
		else if (p_name == "bullet_properties") { r_ret = get_bullet_properties(); return true; }
		return false;
	}

	static void _register_methods() {
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material", "get_material");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collisions_enabled"), "set_collisions_enabled", "get_collisions_enabled");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer"), "set_collision_layer", "get_collision_layer");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask"), "set_collision_mask", "get_collision_mask");
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collision_shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape2D"), "set_collision_shape", "get_collision_shape");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_viewport_as_active_rect"), "set_use_viewport_as_active_rect", "get_use_viewport_as_active_rect");
		ADD_PROPERTY(PropertyInfo(Variant::RECT2, "active_rect"), "set_active_rect", "get_active_rect");
		ADD_PROPERTY(PropertyInfo(Variant::RECT2, "rotate"), "set_rotate", "get_rotate");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "unique_modulate_component", PROPERTY_HINT_ENUM, "None,Red,Green,Blue,Alpha"), "set_unique_modulate_component", "get_unique_modulate_component");
	}

	virtual std::unique_ptr<BulletsPool> _create_pool() { return std::unique_ptr<BulletsPool>(); }
};

#include "bullets_pool.h"

#endif // BULLET_KIT_H
