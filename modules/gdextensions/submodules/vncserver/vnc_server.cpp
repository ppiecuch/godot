/**************************************************************************/
/*  vnc_server.cpp                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "vnc_server.h"

#include "core/image.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"

static VNCServer *singleton = nullptr;

VNCServer *VNCServer::get_singleton() {
	return singleton;
}

void VNCServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start"), &VNCServer::start);
	ClassDB::bind_method(D_METHOD("stop"), &VNCServer::stop);
	ClassDB::bind_method(D_METHOD("is_active"), &VNCServer::is_active);
	ClassDB::bind_method(D_METHOD("get_client_count"), &VNCServer::get_client_count);

	ClassDB::bind_method(D_METHOD("set_port", "port"), &VNCServer::set_port);
	ClassDB::bind_method(D_METHOD("get_port"), &VNCServer::get_port);
	ClassDB::bind_method(D_METHOD("set_max_fps", "fps"), &VNCServer::set_max_fps);
	ClassDB::bind_method(D_METHOD("get_max_fps"), &VNCServer::get_max_fps);

	ClassDB::bind_method(D_METHOD("set_desktop_name", "name"), &VNCServer::set_desktop_name);
	ClassDB::bind_method(D_METHOD("get_desktop_name"), &VNCServer::get_desktop_name);

	ClassDB::bind_method(D_METHOD("overlay_set_text", "id", "x", "y", "text", "color", "bold"), &VNCServer::overlay_set_text, DEFVAL(Color(1, 1, 1)), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("overlay_set_background", "id", "bg_color"), &VNCServer::overlay_set_background);
	ClassDB::bind_method(D_METHOD("overlay_remove_text", "id"), &VNCServer::overlay_remove_text);
	ClassDB::bind_method(D_METHOD("overlay_clear"), &VNCServer::overlay_clear);
	ClassDB::bind_method(D_METHOD("overlay_set_margin", "margin"), &VNCServer::overlay_set_margin);

	ClassDB::bind_method(D_METHOD("process"), &VNCServer::process);

	ADD_SIGNAL(MethodInfo("client_connected", PropertyInfo(Variant::STRING, "address")));
	ADD_SIGNAL(MethodInfo("client_disconnected", PropertyInfo(Variant::STRING, "address")));
}

// --- Overlay API ---

void VNCServer::overlay_set_text(int p_id, int p_x, int p_y, const String &p_text, const Color &p_color, bool p_bold) {
	overlay_mutex.lock();
	overlay.set_text(p_id, p_x, p_y, p_text, p_color, p_bold);
	overlay_mutex.unlock();
}

void VNCServer::overlay_set_background(int p_id, const Color &p_bg_color) {
	overlay_mutex.lock();
	overlay.set_background(p_id, p_bg_color);
	overlay_mutex.unlock();
}

void VNCServer::overlay_remove_text(int p_id) {
	overlay_mutex.lock();
	overlay.remove_text(p_id);
	overlay_mutex.unlock();
}

void VNCServer::overlay_clear() {
	overlay_mutex.lock();
	overlay.clear();
	overlay_mutex.unlock();
}

void VNCServer::overlay_set_margin(int p_margin) {
	overlay_mutex.lock();
	overlay.set_margin(p_margin);
	overlay_mutex.unlock();
}

// --- Framebuffer capture (called on main thread via deferred call) ---

void VNCServer::process() {
	if (!active)
		return;

	_capture_viewport();
}

void VNCServer::_capture_viewport() {
	SceneTree *tree = SceneTree::get_singleton();
	if (!tree)
		return;

	Viewport *vp = tree->get_root();
	if (!vp)
		return;

	Ref<ViewportTexture> vt = vp->get_texture();
	if (vt.is_null())
		return;

	Ref<Image> img = vt->get_data();
	if (img.is_null())
		return;

	if (img->get_format() != Image::FORMAT_RGB8) {
		img->convert(Image::FORMAT_RGB8);
	}

	int w = img->get_width();
	int h = img->get_height();
	PoolVector<uint8_t> data = img->get_data();
	int byte_size = w * h * 3;

	if (data.size() != byte_size)
		return;

	// Copy pixels and apply overlay
	Vector<uint8_t> frame;
	frame.resize(byte_size);
	{
		PoolVector<uint8_t>::Read r = data.read();
		memcpy(frame.ptrw(), r.ptr(), byte_size);
	}

	overlay_mutex.lock();
	if (!overlay.is_empty()) {
		overlay.render(frame.ptrw(), w, h);
	}
	overlay_mutex.unlock();

	// Swap into front buffer
	mutex.lock();
	fb_front = frame;
	fb_width = w;
	fb_height = h;
	fb_new_frame = true;
	mutex.unlock();
}

// --- Server thread ---

void VNCServer::_thread_func(void *p_self) {
	VNCServer *self = (VNCServer *)p_self;
	self->_server_loop();
}

void VNCServer::_server_loop() {
	while (!quit) {
		if (cmd == CMD_START) {
			if (tcp_server->listen(port) == OK) {
				active = true;
				print_verbose("VNCServer: listening on port " + itos(port));
			} else {
				active = false;
				WARN_PRINT("VNCServer: failed to listen on port " + itos(port));
			}
			cmd = CMD_NONE;
		} else if (cmd == CMD_STOP) {
			// Disconnect all clients
			for (int i = 0; i < clients.size(); i++) {
				clients.write[i].peer->disconnect_from_host();
			}
			clients.clear();
			tcp_server->stop();
			active = false;
			cmd = CMD_NONE;
		}

		if (!active) {
			OS::get_singleton()->delay_usec(100000);
			continue;
		}

		// Accept new connections
		while (tcp_server->is_connection_available()) {
			Ref<StreamPeerTCP> peer = tcp_server->take_connection();
			peer->set_no_delay(true);

			ClientState cs;
			cs.peer = peer;
			cs.handshake_done = false;
			cs.update_requested = false;
			cs.incremental = false;
			clients.push_back(cs);

			String addr = peer->get_connected_host().operator String() + ":" + itos(peer->get_connected_port());
			print_verbose("VNCServer: client connected from " + addr);
			call_deferred("emit_signal", "client_connected", addr);
		}

		// Process clients
		for (int i = clients.size() - 1; i >= 0; i--) {
			ClientState &cs = clients.write[i];

			if (!cs.peer->is_connected_to_host()) {
				String addr = cs.peer->get_connected_host().operator String() + ":" + itos(cs.peer->get_connected_port());
				call_deferred("emit_signal", "client_disconnected", addr);
				clients.remove(i);
				continue;
			}

			if (!cs.handshake_done) {
				if (!_do_handshake(cs)) {
					cs.peer->disconnect_from_host();
					clients.remove(i);
				}
				continue;
			}

			_poll_client(cs);
		}

		// Request a framebuffer capture on the main thread
		if (clients.size() > 0) {
			call_deferred("process");
		}

		// Send framebuffer updates
		_send_update_to_clients();

		// Rate limit
		int delay_us = max_fps > 0 ? (1000000 / max_fps) : 100000;
		OS::get_singleton()->delay_usec(delay_us);
	}
}

bool VNCServer::_do_handshake(ClientState &client) {
	Ref<StreamPeerTCP> &peer = client.peer;

	// Send protocol version
	const char *version_str = RFB_PROTOCOL_VERSION_STRING;
	if (peer->put_data((const uint8_t *)version_str, RFB_PROTOCOL_VERSION_LENGTH) != OK)
		return false;

	// Wait for client version response
	int avail = peer->get_available_bytes();
	if (avail < RFB_PROTOCOL_VERSION_LENGTH) {
		// Not ready yet — we'll retry next iteration
		// But for simplicity, block briefly
		for (int i = 0; i < 50 && peer->get_available_bytes() < RFB_PROTOCOL_VERSION_LENGTH; i++) {
			OS::get_singleton()->delay_usec(10000);
			if (!peer->is_connected_to_host())
				return false;
		}
		if (peer->get_available_bytes() < RFB_PROTOCOL_VERSION_LENGTH)
			return false;
	}

	uint8_t client_version[RFB_PROTOCOL_VERSION_LENGTH];
	if (peer->get_data(client_version, RFB_PROTOCOL_VERSION_LENGTH) != OK)
		return false;

	// Send security type: no auth
	uint32_t security = rfb_htonl(RFB_SECURITY_NONE);
	if (peer->put_data((const uint8_t *)&security, 4) != OK)
		return false;

	// Wait for ClientInit
	for (int i = 0; i < 50 && peer->get_available_bytes() < 1; i++) {
		OS::get_singleton()->delay_usec(10000);
		if (!peer->is_connected_to_host())
			return false;
	}
	if (peer->get_available_bytes() < 1)
		return false;

	uint8_t shared_flag;
	if (peer->get_data(&shared_flag, 1) != OK)
		return false;

	// Send ServerInit
	mutex.lock();
	int w = fb_width > 0 ? fb_width : 640;
	int h = fb_height > 0 ? fb_height : 480;
	mutex.unlock();

	CharString name_utf8 = desktop_name.utf8();
	Vector<uint8_t> server_init = RFBEncoder::build_server_init(w, h, name_utf8.get_data());
	if (peer->put_data(server_init.ptr(), server_init.size()) != OK)
		return false;

	client.handshake_done = true;
	return true;
}

void VNCServer::_poll_client(ClientState &client) {
	Ref<StreamPeerTCP> &peer = client.peer;
	int avail = peer->get_available_bytes();
	if (avail <= 0)
		return;

	uint8_t msg_type;
	if (peer->get_data(&msg_type, 1) != OK)
		return;

	switch (msg_type) {
		case RFB_MSG_SET_PIXEL_FORMAT: {
			uint8_t buf[sizeof(RFBSetPixelFormat) - 1];
			peer->get_data(buf, sizeof(buf));
			// We ignore client pixel format — always serve our default.
		} break;
		case RFB_MSG_SET_ENCODINGS: {
			uint8_t hdr[3];
			peer->get_data(hdr, 3);
			uint16_t num_enc = (hdr[1] << 8) | hdr[2];
			for (int i = 0; i < num_enc; i++) {
				uint8_t enc[4];
				peer->get_data(enc, 4);
			}
			// We only support Raw — ignore encoding preferences.
		} break;
		case RFB_MSG_FB_UPDATE_REQUEST: {
			uint8_t buf[sizeof(RFBFramebufferUpdateRequest) - 1];
			peer->get_data(buf, sizeof(buf));
			client.incremental = buf[0];
			client.update_requested = true;
		} break;
		case RFB_MSG_KEY_EVENT: {
			uint8_t buf[sizeof(RFBKeyEvent) - 1];
			peer->get_data(buf, sizeof(buf));
			// View-only — discard.
		} break;
		case RFB_MSG_POINTER_EVENT: {
			uint8_t buf[sizeof(RFBPointerEvent) - 1];
			peer->get_data(buf, sizeof(buf));
			// View-only — discard.
		} break;
		case RFB_MSG_CLIENT_CUT_TEXT: {
			uint8_t hdr[7];
			peer->get_data(hdr, 7);
			uint32_t length = ((uint32_t)hdr[3] << 24) | ((uint32_t)hdr[4] << 16) |
					((uint32_t)hdr[5] << 8) | (uint32_t)hdr[6];
			// Discard text
			for (uint32_t i = 0; i < length && peer->is_connected_to_host(); i++) {
				uint8_t c;
				peer->get_data(&c, 1);
			}
		} break;
	}
}

void VNCServer::_send_update_to_clients() {
	bool any_requesting = false;
	for (int i = 0; i < clients.size(); i++) {
		if (clients[i].update_requested) {
			any_requesting = true;
			break;
		}
	}
	if (!any_requesting)
		return;

	// Grab the current frame
	mutex.lock();
	if (!fb_new_frame || fb_front.size() == 0) {
		mutex.unlock();
		return;
	}
	fb_back = fb_front;
	int w = fb_width;
	int h = fb_height;
	fb_new_frame = false;
	mutex.unlock();

	const uint8_t *pixels = fb_back.ptr();

	// Find dirty rects
	Vector<RFBDirtyRect> dirty = encoder.find_dirty_rects(pixels, w, h);
	if (dirty.size() == 0)
		return;

	const Vector<uint8_t> &encoded = encoder.encode_update(pixels, w, dirty);

	// Send to all requesting clients
	for (int i = 0; i < clients.size(); i++) {
		ClientState &cs = clients.write[i];
		if (!cs.update_requested || !cs.handshake_done)
			continue;

		cs.peer->put_data(encoded.ptr(), encoded.size());
		cs.update_requested = false;
	}
}

// --- Public API ---

void VNCServer::start() {
	cmd = CMD_START;
}

void VNCServer::stop() {
	cmd = CMD_STOP;
}

bool VNCServer::is_active() const {
	return active;
}

int VNCServer::get_client_count() const {
	return clients.size();
}

void VNCServer::set_port(int p_port) {
	port = p_port;
}

int VNCServer::get_port() const {
	return port;
}

void VNCServer::set_max_fps(int p_fps) {
	max_fps = CLAMP(p_fps, 1, 60);
}

int VNCServer::get_max_fps() const {
	return max_fps;
}

void VNCServer::set_desktop_name(const String &p_name) {
	desktop_name = p_name;
}

String VNCServer::get_desktop_name() const {
	return desktop_name;
}

// --- Lifecycle ---

VNCServer::VNCServer() {
	tcp_server.instance();
	quit = false;
	cmd = CMD_NONE;
	active = false;
	fb_width = 0;
	fb_height = 0;
	fb_new_frame = false;
	max_fps = 10;
	desktop_name = "Godot VNC";

	GLOBAL_DEF("network/vnc_server/port", 5900);
	GLOBAL_DEF("network/vnc_server/max_fps", 10);
#if defined(DEBUG_ENABLED) || defined(TOOLS_ENABLED)
	GLOBAL_DEF("network/vnc_server/autostart", true);
#else
	GLOBAL_DEF("network/vnc_server/autostart", false);
#endif

	port = GLOBAL_GET("network/vnc_server/port");
	max_fps = GLOBAL_GET("network/vnc_server/max_fps");

	if (singleton) {
		WARN_PRINT("VNCServer instance already exists.");
	}
	singleton = this;

	thread.start(_thread_func, this);
}

VNCServer::~VNCServer() {
	quit = true;
	thread.wait_to_finish();
	singleton = nullptr;
}
