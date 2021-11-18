#include "bullets.h"
#include "kits/basic_bullet_kit.h"
#include "kits/following_bullet_kit.h"
#include "kits/dynamic_bullet_kit.h"
#include "kits/following_dynamic_bullet_kit.h"

void register_bullet_kit() {
	ClassDB::register_class<Bullet>();
	ClassDB::register_class<BulletKit>();
	ClassDB::register_class<Bullets>();

	// Default Bullet Kits.
	ClassDB::register_class<BasicBulletKit>();

	ClassDB::register_class<FollowingBullet>();
	ClassDB::register_class<FollowingBulletKit>();

	ClassDB::register_class<DynamicBullet>();
	ClassDB::register_class<DynamicBulletKit>();

	ClassDB::register_class<FollowingDynamicBullet>();
	ClassDB::register_class<FollowingDynamicBulletKit>();
	
	// Custom Bullet Kits.
	//register_class<CustomBulletKit>();
}
