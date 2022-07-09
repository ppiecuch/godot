#ifndef GODOT_SPACEMOUSE_H
#define GODOT_SPACEMOUSE_H

#include "core/object.h"
#include "core/math/vector3.h"
#include "scene/main/node.h"

typedef struct hid_device_ hid_device;

class SpaceMouse : public Object {
	GDCLASS(SpaceMouse, Object)

	typedef struct SpaceData {
		int px, py, pz, rx, ry, rz;
	} SpaceData;

	typedef struct SpaceMotion {
		Vector3 translation;
		Vector3 rotation;
	} SpaceMotion;

	hid_device *space_device;
	unsigned char space_buffer[256];
	SpaceMotion motion_data;
	SpaceData space_data = {0, 0, 0, 0, 0, 0};
	bool space_connected = false;
	int current_model = -1;

protected:
	static void _bind_methods();

public:
	static SpaceMouse *get_singleton();

	bool connect();
	bool poll();
	Vector3 translation();
	Vector3 rotation();

	SpaceMouse();
	~SpaceMouse();
};

inline Vector3 SpaceMouse::translation() { return motion_data.translation; }
inline Vector3 SpaceMouse::rotation() { return motion_data.rotation; }


class SpaceMouseNode : public Node {
	GDCLASS(SpaceMouseNode, Node)

	SpaceMouse *space_mouse;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	SpaceMouseNode();
	~SpaceMouseNode();
};

#endif // GODOT_SPACEMOUSE_H
