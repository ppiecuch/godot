#include "register_types.h"
#include "timer2.h"
#include "tween2.h"
#include "inputstorage.h"
#include "trail_2d.h"
#include "phantom.h"

#include "core/engine.h"


void register_gd_extends_types() {
	ClassDB::register_class<Timer2>();
    ClassDB::register_class<TimerObject>();
    ClassDB::register_class<Tween2>();
    ClassDB::register_class<TweenAction>();
    ClassDB::register_class<InputStorage>();
    ClassDB::register_class<InputStorageNode>();
    ClassDB::register_class<TrailPoint2D>();
    ClassDB::register_class<TrailLine2D>();
    ClassDB::register_class<Phantom>();

    Engine::get_singleton()->add_singleton(Engine::Singleton("Timer2", Timer2::get_singleton()));
    Engine::get_singleton()->add_singleton(Engine::Singleton("Tween2", Tween2::get_singleton()));
    Engine::get_singleton()->add_singleton(Engine::Singleton("InputStorage", InputStorage::get_singleton()));
}

void unregister_gd_extends_types() {
}
