/*************************************************************************/
/*  dynamic_bullet_kit.h                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef DYNAMIC_BULLET_KIT_H
#define DYNAMIC_BULLET_KIT_H

#include "scene/resources/curve.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/texture.h"

#include "../bullet_kit.h"

// Bullet definition.
class DynamicBullet : public Bullet {
	GDCLASS(DynamicBullet, Bullet)
public:
	Transform2D starting_trasform;
	float starting_speed = 0;

	void set_transform(Transform2D transform) {
		starting_trasform = transform;
		this->transform = transform;
	}

	Transform2D get_transform() {
		return transform;
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
		ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "transform"), "set_transform", "get_transform");
		ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "starting_trasform"), "set_starting_trasform", "get_starting_trasform");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "velocity"), "set_velocity", "get_velocity");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "starting_speed"), "set_starting_speed", "get_starting_speed");
	}
};

// Bullet kit definition.
class DynamicBulletKit : public BulletKit {
	GDCLASS(DynamicBulletKit, BulletKit)
public:
	BULLET_KIT(DynamicBulletsPool)

	Ref<Texture> texture;
	float lifetime_curves_span = 1.0;
	bool lifetime_curves_loop = true;
	Ref<Curve> speed_multiplier_over_lifetime;
	Ref<Curve> rotation_offset_over_lifetime;

	virtual String get_bullet_class_name() const { return "DynamicBullet"; }

	static void _register_methods() {
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime_curves_span"), "set_lifetime_curves_span", "get_lifetime_curves_span");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lifetime_curves_loop"), "set_lifetime_curves_loop", "get_lifetime_curves_loop");
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "speed_multiplier_over_lifetime", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_speed_multiplier_over_lifetime", "get_speed_multiplier_over_lifetime");
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "rotation_offset_over_lifetime", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_rotation_offset_over_lifetime", "get_rotation_offset_over_lifetime");
	}
};

// Bullets pool definition.
class DynamicBulletsPool : public AbstractBulletsPool<DynamicBulletKit, DynamicBullet> {
	// void _init_bullet(Bullet* bullet); Use default implementation.

	void _enable_bullet(DynamicBullet *bullet) {
		// Reset the bullet lifetime.
		bullet->lifetime = 0.0f;
		Rect2 texture_rect = Rect2(-kit->texture->get_size() / 2.0f, kit->texture->get_size());
		RID texture_rid = kit->texture->get_rid();

		// Configure the bullet to draw the kit texture each frame.
		VisualServer::get_singleton()->canvas_item_add_texture_rect(bullet->item_rid,
				texture_rect,
				texture_rid);
	}

	// void _disable_bullet(Bullet* bullet); Use default implementation.

	bool _process_bullet(DynamicBullet *bullet, float delta) {
		float adjusted_lifetime = bullet->lifetime / kit->lifetime_curves_span;
		if (kit->lifetime_curves_loop) {
			adjusted_lifetime = fmod(adjusted_lifetime, 1.0f);
		}

		if (kit->speed_multiplier_over_lifetime.is_valid()) {
			float speed_multiplier = kit->speed_multiplier_over_lifetime->interpolate(adjusted_lifetime);
			bullet->velocity = bullet->velocity.normalized() * bullet->starting_speed * speed_multiplier;
		}
		if (kit->rotation_offset_over_lifetime.is_valid()) {
			float rotation_offset = kit->rotation_offset_over_lifetime->interpolate(adjusted_lifetime);
			float absolute_rotation = bullet->starting_trasform.get_rotation() + rotation_offset;

			bullet->velocity = bullet->velocity.rotated(absolute_rotation - bullet->transform.get_rotation());
		}

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

BULLET_KIT_IMPLEMENTATION(DynamicBulletKit, DynamicBulletsPool)

#endif // DYNAMIC_BULLET_KIT_H
