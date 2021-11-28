#include "vgamepad.h"

#include "core/input_map.h"

void VGamePad::_input(const Ref<InputEvent> &p_event) {
	if (const InputEventMouseMotion *m = cast_to<InputEventMouseMotion>(*p_event)) {
	} else if (const InputEventKey *k = cast_to<InputEventKey>(*p_event)) {
	}
}

void VGamePad::_notification(int p_what) {
	switch(p_what) {
		case NOTIFICATION_READY: {
		} break;
	}
}

void VGamePad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_input"), &VGamePad::_input);
}

VGamePad::VGamePad() {
	_dirty = false;
}

VGamePad::~VGamePad() {
}
