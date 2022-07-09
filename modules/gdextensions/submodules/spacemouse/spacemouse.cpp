/*
Copyright (c) 2022 Andres Hernandez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "core/variant.h"
#include "spacemouse/spacemouse.h"

#include <string.h>
#include <hidapi.h>

#ifndef HID_API_MAKE_VERSION
#define HID_API_MAKE_VERSION(mj, mn, p) (((mj) << 24) | ((mn) << 8) | (p))
#endif
#ifndef HID_API_VERSION
#define HID_API_VERSION HID_API_MAKE_VERSION(HID_API_VERSION_MAJOR, HID_API_VERSION_MINOR, HID_API_VERSION_PATCH)
#endif

#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <mac/hidapi_darwin.h>
#endif

#if defined(_WIN32) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <windows/hidapi_winapi.h>
#endif

#if defined(USING_HIDAPI_LIBUSB) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <linux/hidapi_libusb.h>
#endif

#define MODELS 7
#define IDS 3

int model_ids[MODELS][IDS] = {
	{ 0x046d, 0xc626, 0x00 }, // space navigator
	{ 0x256f, 0xc635, 0x00 }, // space mouse compact
	{ 0x256f, 0xc632, 0x01 }, // space mouse pro wireless
	{ 0x046d, 0xc62b, 0x00 }, // space mouse pro
	{ 0x256f, 0xc62e, 0x01 }, // space mouse wireless
	{ 0x256f, 0xc652, 0x01 }, // universal receiver
	{ 0x046d, 0xc629, 0x00 }, // space pilot pro
};

enum Format {
	Original = 0,
	Current = 1
};

inline static int to_int(unsigned char* buffer) {
	int val = (buffer[1] << 8) + buffer[0];
	if (val >= 32768) {
		val = -(65536 - val);
	}
	return val;
}

void SpaceMouse::_bind_methods() {
	ClassDB::bind_method(D_METHOD("connect"), &SpaceMouse::connect);
	ClassDB::bind_method(D_METHOD("poll"), &SpaceMouse::poll);
	ClassDB::bind_method(D_METHOD("translation"), &SpaceMouse::translation);
	ClassDB::bind_method(D_METHOD("rotation"), &SpaceMouse::rotation);
}

static SpaceMouse *instance = nullptr;

SpaceMouse *SpaceMouse::get_singleton() { return instance; }

SpaceMouse::SpaceMouse() {
	instance = this;
}

SpaceMouse::~SpaceMouse() {
	hid_close(space_device);
	hid_exit();

	instance = nullptr;
}

bool SpaceMouse::connect() {
	if (space_connected) {
		return true;
	}

	if (hid_init()) {
		space_connected = 0;
		return false;
	}

#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
	hid_darwin_set_open_exclusive(1);
#endif

	struct hid_device_info *hid_devices, *current_hid;
	hid_devices = hid_enumerate(0x0, 0x0);
	current_hid = hid_devices;
	int not_found = 1;

	while (current_hid && not_found) {
		for (int model = 0; model < MODELS; model++) {
			if (model_ids[model][0] == current_hid->vendor_id &&
					model_ids[model][1] == current_hid->product_id) {
				current_model = model;
				not_found = 0;
			}
		}
		current_hid = current_hid->next;
	}

	hid_free_enumeration(hid_devices);

	space_device = hid_open(model_ids[current_model][0], model_ids[current_model][1], nullptr);

	if (!space_device) {
		space_connected = false;
		return false;
	}

	hid_set_nonblocking(space_device, 1);

	memset(space_buffer, 0x00, sizeof(space_buffer));
	space_buffer[0] = 0x01;
	space_buffer[1] = 0x81;

	space_connected = true;

	return true;
}

bool SpaceMouse::poll() {
	int poll_count = 2;

	if (model_ids[current_model][2] == 0) {
		poll_count *= 2;
	}

	space_data.px = 0;
	space_data.py = 0;
	space_data.pz = 0;
	space_data.rx = 0;
	space_data.ry = 0;
	space_data.rz = 0;

	while(space_connected && poll_count-- > 0) {
		int read_len = hid_read(space_device, space_buffer, sizeof(space_buffer));
		if (read_len > 0) {
			switch(model_ids[current_model][2]) {
				case Original:
					if (space_buffer[0] == 1) {
						space_data.px += to_int(&space_buffer[1]);
						space_data.py += to_int(&space_buffer[5]);
						space_data.pz += to_int(&space_buffer[3]);
					} else if (space_buffer[0] == 2) {
						space_data.rx += to_int(&space_buffer[1]);
						space_data.ry += to_int(&space_buffer[5]);
						space_data.rz += to_int(&space_buffer[3]);
					}
					break;
				case Current:
					if (space_buffer[0] == 1) {
						space_data.px += to_int(&space_buffer[1]);
						space_data.py += to_int(&space_buffer[5]);
						space_data.pz += to_int(&space_buffer[3]);
						space_data.rx += to_int(&space_buffer[7]);
						space_data.ry += to_int(&space_buffer[11]);
						space_data.rz += to_int(&space_buffer[9]);
					}
					break;
			}
		}
	}

	if (space_connected) {
		motion_data.translation += Vector3(space_data.px, space_data.py, space_data.pz);
		motion_data.rotation += Vector3(space_data.rx, space_data.ry, space_data.rz);

		return true;
	}

	return false;
}


/// SpaceMouseNode

void SpaceMouseNode::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_EXIT_TREE: {
		} break;
		case NOTIFICATION_PROCESS: {
			space_mouse->poll();
		} break;
	}
}

void SpaceMouseNode::_bind_methods() {
}

SpaceMouseNode::SpaceMouseNode() {
	space_mouse = SpaceMouse::get_singleton();
}

SpaceMouseNode::~SpaceMouseNode() {
}
