#ifndef FOLLOWING_DYNAMIC_BULLET_KIT_H
#define FOLLOWING_DYNAMIC_BULLET_KIT_H

#include "scene/resources/texture.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/curve.h"
#include "scene/2d/node_2d.h"

#include "../bullet_kit.h"


// Bullet definition.
class FollowingDynamicBullet : public Bullet {
	GDCLASS(FollowingDynamicBullet, Bullet)
public:
	Node2D* target_node = nullptr;
	float starting_speed = 0;

	void set_target_node(Node2D* node) {
		target_node = node;
	}

	Node2D* get_target_node() {
		return target_node;
	}

	void set_velocity(Vector2 velocity) {
		starting_speed = velocity.length();
		this->velocity = velocity;
	}

	Vector2 get_velocity() {
		return velocity;
	}

	void _init() {}

	static void _register_methods() {
		// Registering an Object reference property with PROPERTY_HINT_RESOURCE_TYPE and hint_string is just
		// a way to tell the editor plugin the type of the property, so that it can be viewed in the BulletKit inspector.
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target_node", PROPERTY_HINT_RESOURCE_TYPE, "Node2D"), "set_target_node", "get_target_node");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "velocity"), "set_velocity", "get_velocity");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "starting_speed"), "set_starting_speed", "get_starting_speed");
	}
};

// Bullet kit definition.
class FollowingDynamicBulletKit : public BulletKit {
	GDCLASS(FollowingDynamicBulletKit, BulletKit)
public:
	BULLET_KIT(FollowingDynamicBulletsPool)

	Ref<Texture> texture;
	float lifetime_curves_span = 1.0;
	float distance_curves_span = 128.0;
	bool lifetime_curves_loop = true;
	int32_t speed_control_mode = 0;
	Ref<Curve> speed_multiplier;
	int32_t turning_speed_control_mode = 0;
	Ref<Curve> turning_speed;

	virtual String get_bullet_class_name() const { return "FollowingDynamicBullet"; }

	static void _register_methods() {
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime_curves_span", PROPERTY_HINT_RANGE, "0.001,256.0"), "set_lifetime_curves_span", "get_lifetime_curves_span");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "distance_curves_span", PROPERTY_HINT_RANGE, "0.001,65536.0"), "set_distance_curves_span", "get_distance_curves_span");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lifetime_curves_loop"), "set_lifetime_curves_loop", "get_lifetime_curves_loop");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "speed_control_mode", PROPERTY_HINT_ENUM,
			"Based On Lifetime,Based On Target Distance,Based On Angle To Target"), "set_speed_control_mode", "get_speed_control_mode");
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "speed_multiplier", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_speed_multiplier", "get_speed_multiplier");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "turning_speed_control_mode", PROPERTY_HINT_ENUM,
			"Based On Lifetime,Based On Target Distance,Based On Angle To Target"), "set_turning_speed_control_mode", "get_turning_speed_control_mode");
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "turning_speed", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_turning_speed", "get_turning_speed");
	}
};

// Bullets pool definition.
class FollowingDynamicBulletsPool : public AbstractBulletsPool<FollowingDynamicBulletKit, FollowingDynamicBullet> {

	// void _init_bullet(FollowingDynamicBullet* bullet); Use default implementation.

	void _enable_bullet(FollowingDynamicBullet* bullet) {
		// Reset the bullet lifetime.
		bullet->lifetime = 0.0;
		Rect2 texture_rect = Rect2(-kit->texture->get_size() / 2.0, kit->texture->get_size());
		RID texture_rid = kit->texture->get_rid();
		
		// Configure the bullet to draw the kit texture each frame.
		VisualServer::get_singleton()->canvas_item_add_texture_rect(bullet->item_rid,
			texture_rect,
			texture_rid);
	}

	// void _disable_bullet(FollowingDynamicBullet* bullet); Use default implementation.

	bool _process_bullet(FollowingDynamicBullet* bullet, float delta) {
		float adjusted_lifetime = bullet->lifetime / kit->lifetime_curves_span;
		if(kit->lifetime_curves_loop) {
			adjusted_lifetime = fmod(adjusted_lifetime, 1.0f);
		}
		float bullet_turning_speed = 0.0;
		float speed_multiplier = 1.0;
		
		if(kit->turning_speed.is_valid() && bullet->target_node != nullptr) {
			Vector2 to_target = bullet->target_node->get_global_position() - bullet->transform.get_origin();
			// If based on lifetime.
			if(kit->turning_speed_control_mode == 0) {
				bullet_turning_speed = kit->turning_speed->interpolate(adjusted_lifetime);
			}
			// If based on distance to target.
			else if(kit->turning_speed_control_mode == 1) {
				float distance_to_target = to_target.length();
				bullet_turning_speed = kit->turning_speed->interpolate(distance_to_target / kit->distance_curves_span);
			}
			// If based on angle to target.
			else if(kit->turning_speed_control_mode == 2) {
				float angle_to_target = bullet->velocity.angle_to(to_target);
				bullet_turning_speed = kit->turning_speed->interpolate(std::abs(angle_to_target) / Math_PI);
			}
		}
		if(kit->speed_multiplier.is_valid()) {
			// If based on lifetime.
			if(kit->speed_control_mode <= 0) {
				speed_multiplier = kit->speed_multiplier->interpolate(adjusted_lifetime);
			}
			// If based on target node: 1 or 2.
			else if(kit->speed_control_mode < 3 && bullet->target_node != nullptr) {
				Vector2 to_target = bullet->target_node->get_global_position() - bullet->transform.get_origin();
				// If based on distance to target.
				if(kit->speed_control_mode == 1) {
					float distance_to_target = to_target.length();
					speed_multiplier = kit->speed_multiplier->interpolate(distance_to_target / kit->distance_curves_span);
				}
				// If based on angle to target.
				else if(kit->speed_control_mode == 2) {
					float angle_to_target = bullet->velocity.angle_to(to_target);
					speed_multiplier = kit->speed_multiplier->interpolate(std::abs(angle_to_target) / Math_PI);
				}
			}
		}

		if(speed_multiplier != 1.0f) {
			bullet->velocity = bullet->velocity.normalized() * bullet->starting_speed * speed_multiplier;
		}
		if(bullet_turning_speed != 0.0 && bullet->target_node != nullptr) {
			// Find the rotation to the target node.
			Vector2 to_target = bullet->target_node->get_global_position() - bullet->transform.get_origin();
			float rotation_to_target = bullet->velocity.angle_to(to_target);
			float rotation_value = MIN(bullet_turning_speed * delta, std::abs(rotation_to_target));
			// Apply the rotation, capped to the max turning speed.
			bullet->velocity = bullet->velocity.rotated(SGN(rotation_to_target) * rotation_value);
		}

		bullet->transform.set_origin(bullet->transform.get_origin() + bullet->velocity * delta);

		if(!active_rect.has_point(bullet->transform.get_origin())) {
			// Return true if the bullet should be deleted.
			return true;
		}
		// Rotate the bullet based on its velocity "rotate" is enabled.
		if(kit->rotate) {
			bullet->transform.set_rotation(bullet->velocity.angle());
		}
		// Bullet is still alive, increase its lifetime.
		bullet->lifetime += delta;
		// Return false if the bullet should not be deleted yet.
		return false;
	}
};

BULLET_KIT_IMPLEMENTATION(FollowingDynamicBulletKit, FollowingDynamicBulletsPool)

#endif // FOLLOWING_DYNAMIC_BULLET_KIT_H
