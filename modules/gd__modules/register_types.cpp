#include "register_types.h"
#include "core/engine.h"

#include "timer2.h"
#include "tween2.h"
#include "inputstorage.h"
#include "trail_2d.h"
#include "phantom.h"

#include "line_builder_2d.h"

#include "gd2c/bytecode_exporter.h"
#include "gd2c/gd2c.h"

#include "statemachine.h"
#include "state.h"

#include "debugdraw.h"


void register_gd_extends_types() {
	ClassDB::register_class<Timer2>();
	ClassDB::register_class<TimerObject>();
	ClassDB::register_class<Tween2>();
	ClassDB::register_class<TweenAction>();
	ClassDB::register_class<InputStorage>();
	ClassDB::register_class<InputStorageNode>();
	ClassDB::register_class<TrailPoint2D>();
	ClassDB::register_class<TrailLine2D>();
	ClassDB::register_class<LineBuilder2D>();
	ClassDB::register_class<Phantom>();

	ClassDB::register_class<GDScriptBytecodeExporter>();
	ClassDB::register_class<GD2CApi>();

	Engine::get_singleton()->add_singleton(Engine::Singleton("Timer2", memnew(Timer2));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Tween2", memnew(Tween2));
	Engine::get_singleton()->add_singleton(Engine::Singleton("InputStorage", memnew(InputStorage));

	ObjectTypeDB::register_type<StateMachine>();
	ObjectTypeDB::register_type<State>();

	Globals::get_singleton()->add_singleton(Globals::Singleton("DebugDraw", memnew(DebugDraw));
}

void unregister_gd_extends_types() {

}
