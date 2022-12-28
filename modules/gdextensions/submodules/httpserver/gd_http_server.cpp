/*************************************************************************/
/*  gd_http_server.cpp                                                   */
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

// Reference:
// ----------
// 1. https://github.com/xaltaq/GDHTTPServer/blob/master/httpserver.gd
// 2. https://github.com/Faless/netgame-godot
// 3. https://github.com/deep-entertainment/godottpd/blob/main/addons/godottpd/http_server.gd
// 4. https://github.com/sayotte/martin/blob/master/route.c
// 5. https://github.com/Accessory/FlowHttp/blob/main/routes/Router.h
// 6. https://github.com/malpower/evhttp-sample/blob/master/src/router.cc

#include "gd_http_server.h"
#include "http_protocol.h"

#include "editor/editor_settings.h"
#include "common/gd_core.h"

#define _print_debug(fmt, ...) print_debug("[Http] " fmt, ##__VA_ARGS__)

bool GdHttpServer::_process_connection(Ref<StreamPeerTCP> connection) {
	_print_debug("Got peer: ", connection->get_connected_host(), ":", connection->get_connected_port());
	connection->set_no_delay(true);
	while (true) {
		if (!connection->is_connected_to_host()) {
			_print_debug("Connection lost");
			return false;
		}
		const int bytes = connection->get_available_bytes();
		if (bytes > 0) {
			std::vector<uint8_t> data(bytes);
			if (connection->get_data(&data[0], bytes) == OK) {
				http::HTTPMessage http_message;
				http::HTTPMessageParser http_parser;
				http_parser.parse(&http_message, data);
			} else {
				_print_debug("Receiving data failed");
				return false;
			}
		}
	}
}

void GdHttpServer::_thread_start(void *s) {
	GdHttpServer *self = (GdHttpServer *)s;
	while (!self->quit) {
		if (self->cmd == CMD_ACTIVATE) {
			self->server->listen(self->port);
			self->active = true;
			self->cmd = CMD_NONE;
		} else if (self->cmd == CMD_STOP) {
			self->server->stop();
			self->active = false;
			self->cmd = CMD_NONE;
		}

		if (self->active) {
			if (self->server->is_connection_available()) {
				_process_connection(self->server->take_connection());
			}
		}

		OS::get_singleton()->delay_usec(100000);
	}
}

void GdHttpServer::start() {
	stop();
	port = EDITOR_GET("network/http_server/port");
	cmd = CMD_ACTIVATE;
}

bool GdHttpServer::is_active() const {
	return active;
}

void GdHttpServer::stop() {
	cmd = CMD_STOP;
}

GdHttpServer::GdHttpServer() {
	server.instance();
	quit = false;
	active = false;
	cmd = CMD_NONE;
	thread.start(_thread_start, this);

	EDITOR_DEF("network/http_server/port", 8081);
}

GdHttpServer::~GdHttpServer() {
	quit = true;
	thread.wait_to_finish();
}
