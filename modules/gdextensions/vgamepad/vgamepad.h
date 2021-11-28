#ifndef GD_VGAMEPAD_H
#define GD_VGAMEPAD_H

#include "scene/2d/node_2d.h"

class VGamePad : public Node2D {
	GDCLASS(VGamePad, Node2D);

public:
	enum VGamePadDesignHint {
		VGAMEPAD_RED_ACCENT,
	};

	enum VGamePadControls {
		VGAMEPAD_BUTTON_ROTATE_RIGHT,
		VGAMEPAD_BUTTON_ROTATE_LEFT,
		VGAMEPAD_BUTTON_ROTATE_A,
		VGAMEPAD_BUTTON_ROTATE_B,
		VGAMEPAD_BUTTON_ROTATE_X,
		VGAMEPAD_BUTTON_ROTATE_Y,
		VGAMEPAD_KEYPAD,
		VGAMEPAD_DPAD,
		VGAMEPAD_DPAD_DECOR,
		VGAMEPAD_ANALOG,
	};

private:
	bool _dirty;
	VGamePadControls controls;

protected:
	static void _bind_methods();

	void _notification(int p_what);
	void _input(const Ref<InputEvent> &p_event);

public:
	VGamePad();
	~VGamePad();
};

#endif // GD_VGAMEPAD_H
