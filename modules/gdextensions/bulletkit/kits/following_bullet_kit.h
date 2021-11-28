#ifndef FOLLOWING_BULLET_KIT_H
#define FOLLOWING_BULLET_KIT_H

#include "scene/resources/texture.h"
#include "scene/resources/packed_scene.h"
#include "scene/2d/node_2d.h"
#include "scene/main/scene_tree.h"

#include <cmath>

#include "../bullet_kit.h"


// Bullet definition.
class FollowingBullet : public Bullet {
	GDCLASS(FollowingBullet, Bullet)
public:
	Node2D* target_node = nullptr;

	void _init() {}

	void set_target_node(Node2D* node) {
		target_node = node;
	}

	Node2D* get_target_node() {
		return target_node;
	}

	static void _register_methods() {
		// Registering an Object reference property with PROPERTY_HINT_RESOURCE_TYPE and hint_string is just
		// a way to tell the editor plugin the type of the property, so that it can be viewed in the BulletKit inspector.
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target_node", PROPERTY_HINT_RESOURCE_TYPE, "Node2D"), "set_target_node", "get_target_node");
	}
};

// Bullet kit definition.
class FollowingBulletKit : public BulletKit {
	GDCLASS(FollowingBulletKit, BulletKit)
public:
	BULLET_KIT(FollowingBulletsPool)

	Ref<Texture> texture;
	float bullets_turning_speed = 1.0;

	virtual String get_bullet_class_name() const { return "FollowingBullet"; }

	static void _register_methods() {
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "bullets_turning_speed"), "set_bullets_turning_speed", "get_bullets_turning_speed");
	}
};

// Bullets pool definition.
class FollowingBulletsPool : public AbstractBulletsPool<FollowingBulletKit, FollowingBullet> {

	//void _init_bullet(FollowingBullet* bullet); Use default implementation.

	void _enable_bullet(FollowingBullet* bullet) {
		// Reset the bullet lifetime.
		bullet->lifetime = 0.0f;
		Rect2 texture_rect = Rect2(-kit->texture->get_size() / 2.0f, kit->texture->get_size());
		RID texture_rid = kit->texture->get_rid();
		
		// Configure the bullet to draw the kit texture each frame.
		VisualServer::get_singleton()->canvas_item_add_texture_rect(bullet->item_rid,
			texture_rect,
			texture_rid);
	}

	//void _disable_bullet(FollowingBullet* bullet); Use default implementation.

	bool _process_bullet(FollowingBullet* bullet, float delta) {
		if(bullet->target_node != nullptr) {
			// Find the rotation to the target node.
			Vector2 to_target = bullet->target_node->get_global_position() - bullet->transform.get_origin();
			float rotation_to_target = bullet->velocity.angle_to(to_target);
			float rotation_value = MIN(kit->bullets_turning_speed * delta, std::abs(rotation_to_target));

			// Apply the rotation, capped to the max turning speed.
			bullet->velocity = bullet->velocity.rotated(SGN(rotation_to_target) * rotation_value);
		}
		// Apply velocity.
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

BULLET_KIT_IMPLEMENTATION(FollowingBulletKit, FollowingBulletsPool)

#endif // FOLLOWING_BULLET_KIT_H
