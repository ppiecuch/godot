#ifndef BASIC_BULLET_KIT_H
#define BASIC_BULLET_KIT_H

#include "scene/resources/texture.h"
#include "scene/resources/packed_scene.h"

#include "../bullet_kit.h"


// Bullet kit definition.
class BasicBulletKit : public BulletKit {
	GDCLASS(BasicBulletKit, BulletKit)
public:
	BULLET_KIT(BasicBulletsPool)

	Ref<Texture> texture;

	virtual String get_bullet_class_name() const { return "Bullet"; }

	static void _register_methods() {
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	}
};

// Bullets pool definition.
class BasicBulletsPool : public AbstractBulletsPool<BasicBulletKit, Bullet> {

	// void _init_bullet(Bullet* bullet); Use default implementation.

	void _enable_bullet(Bullet* bullet) {
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

	bool _process_bullet(Bullet* bullet, float delta) {
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

BULLET_KIT_IMPLEMENTATION(BasicBulletKit, BasicBulletsPool)

#endif // BASIC_BULLET_KIT_H
