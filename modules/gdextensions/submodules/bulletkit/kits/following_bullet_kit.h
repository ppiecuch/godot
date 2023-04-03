/**************************************************************************/
/*  following_bullet_kit.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef FOLLOWING_BULLET_KIT_H
#define FOLLOWING_BULLET_KIT_H

#include "scene/2d/node_2d.h"
#include "scene/main/scene_tree.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/texture.h"

#include <cmath>

#include "../bullet_kit.h"

// Bullet definition.
class FollowingBullet : public Bullet {
	GDCLASS(FollowingBullet, Bullet)
public:
	Node2D *target_node = nullptr;

	void _init() {}

	void set_target_node(Node2D *node) {
		target_node = node;
	}

	Node2D *get_target_node() {
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

	void _enable_bullet(FollowingBullet *bullet) {
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

	bool _process_bullet(FollowingBullet *bullet, float delta) {
		if (bullet->target_node != nullptr) {
			// Find the rotation to the target node.
			Vector2 to_target = bullet->target_node->get_global_position() - bullet->transform.get_origin();
			float rotation_to_target = bullet->velocity.angle_to(to_target);
			float rotation_value = MIN(kit->bullets_turning_speed * delta, std::abs(rotation_to_target));

			// Apply the rotation, capped to the max turning speed.
			bullet->velocity = bullet->velocity.rotated(SGN(rotation_to_target) * rotation_value);
		}
		// Apply velocity.
		bullet->transform.set_origin(bullet->transform.get_origin() + bullet->velocity * delta);

		if (!active_rect.has_point(bullet->transform.get_origin())) {
			// Return true if the bullet should be deleted.
			return true;
		}
		// Rotate the bullet based on its velocity "rotate" is enabled.
		if (kit->rotate) {
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
