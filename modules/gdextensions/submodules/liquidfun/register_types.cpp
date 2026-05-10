/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "core/engine.h"

#include "godot_box2d.h"

#include <Box2D/Box2D.h>

void register_liquidfun_types() {
	b2Log("LiquidFun version: %d.%d.%d (Box2D version: %d.%d.%d)\n",
			b2_liquidFunVersion.major, b2_liquidFunVersion.minor, b2_liquidFunVersion.revision,
			b2_version.major, b2_version.minor, b2_version.revision);

	ClassDB::register_virtual_class<WorldB2>();
	ClassDB::register_virtual_class<ShapeB2>();

	ClassDB::register_virtual_class<BodyB2>();
	ClassDB::register_class<BodyDefB2>();

	ClassDB::register_virtual_class<FixtureB2>();
	ClassDB::register_class<FixtureDefB2>();

	ClassDB::register_virtual_class<JointB2>();
	ClassDB::register_virtual_class<RevoluteJointB2>();
	ClassDB::register_virtual_class<MouseJointB2>();
	ClassDB::register_virtual_class<DistanceJointB2>();
	ClassDB::register_virtual_class<WeldJointB2>();
	ClassDB::register_virtual_class<PrismaticJointB2>();
	ClassDB::register_virtual_class<PulleyJointB2>();
	ClassDB::register_virtual_class<GearJointB2>();
	ClassDB::register_virtual_class<WheelJointB2>();
	ClassDB::register_virtual_class<FrictionJointB2>();
	ClassDB::register_virtual_class<RopeJointB2>();
	ClassDB::register_virtual_class<MotorJointB2>();

	ClassDB::register_virtual_class<JointDefB2>();
	ClassDB::register_class<RevoluteJointDefB2>();
	ClassDB::register_class<MouseJointDefB2>();
	ClassDB::register_class<DistanceJointDefB2>();
	ClassDB::register_class<WeldJointDefB2>();
	ClassDB::register_class<PrismaticJointDefB2>();
	ClassDB::register_class<PulleyJointDefB2>();
	ClassDB::register_class<GearJointDefB2>();
	ClassDB::register_class<WheelJointDefB2>();
	ClassDB::register_class<FrictionJointDefB2>();
	ClassDB::register_class<RopeJointDefB2>();
	ClassDB::register_class<MotorJointDefB2>();

	Engine::get_singleton()->add_singleton(Engine::Singleton("Box2D", memnew(Box2D)));
}

void unregister_liquidfun_types() {
	if (Box2D *instance = Box2D::get()) {
		memdelete(instance);
	}
}
