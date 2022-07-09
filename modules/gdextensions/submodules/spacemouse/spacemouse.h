/*************************************************************************/
/*  spacemouse.h                                                         */
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

#ifndef GODOT_SPACEMOUSE_H
#define GODOT_SPACEMOUSE_H

#include "core/math/vector3.h"
#include "core/object.h"
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
	SpaceData space_data = { 0, 0, 0, 0, 0, 0 };
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

inline Vector3 SpaceMouse::translation() {
	return motion_data.translation;
}
inline Vector3 SpaceMouse::rotation() {
	return motion_data.rotation;
}

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
