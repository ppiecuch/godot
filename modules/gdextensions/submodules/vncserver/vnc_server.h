/**************************************************************************/
/*  vnc_server.h                                                          */
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

#ifndef VNC_SERVER_H
#define VNC_SERVER_H

#include "core/io/tcp_server.h"
#include "core/object.h"
#include "core/os/mutex.h"
#include "core/os/thread.h"
#include "core/reference.h"

#include "rfb_encoder.h"
#include "vnc_overlay.h"

class VNCServer : public Object {
	GDCLASS(VNCServer, Object);

	enum Command {
		CMD_NONE,
		CMD_START,
		CMD_STOP,
	};

	struct ClientState {
		Ref<StreamPeerTCP> peer;
		bool handshake_done;
		bool update_requested;
		bool incremental;
	};

	Ref<TCP_Server> tcp_server;
	Thread thread;
	Mutex mutex;
	volatile bool quit;
	volatile Command cmd;
	volatile bool active;

	int port;
	int max_fps;
	String desktop_name;

	Vector<ClientState> clients;

	// Framebuffer double buffer
	Vector<uint8_t> fb_front; // written by main thread
	Vector<uint8_t> fb_back; // read by server thread
	int fb_width;
	int fb_height;
	volatile bool fb_new_frame;

	// Overlay
	VNCOverlay overlay;
	Mutex overlay_mutex;

	// Encoder
	RFBEncoder encoder;

	static void _thread_func(void *p_self);
	void _server_loop();
	bool _do_handshake(ClientState &client);
	void _poll_client(ClientState &client);
	void _send_update_to_clients();

	void _capture_viewport();

protected:
	static void _bind_methods();

public:
	static VNCServer *get_singleton();

	void start();
	void stop();
	bool is_active() const;
	int get_client_count() const;

	void set_port(int p_port);
	int get_port() const;

	void set_max_fps(int p_fps);
	int get_max_fps() const;

	void set_desktop_name(const String &p_name);
	String get_desktop_name() const;

	// Overlay API
	void overlay_set_text(int p_id, int p_x, int p_y, const String &p_text, const Color &p_color = Color(1, 1, 1), bool p_bold = false);
	void overlay_set_background(int p_id, const Color &p_bg_color);
	void overlay_remove_text(int p_id);
	void overlay_clear();
	void overlay_set_margin(int p_margin);

	// Called from main loop to capture framebuffer
	void process();

	VNCServer();
	~VNCServer();
};

#endif // VNC_SERVER_H
