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
// 2. https://github.com/deep-entertainment/godottpd/blob/main/addons/godottpd/http_server.gd
// 3. https://github.com/sayotte/martin/blob/master/route.c
// 4. https://github.com/Accessory/FlowHttp/blob/main/routes/Router.h
// 5. https://github.com/malpower/evhttp-sample/blob/master/src/router.cc
// 6. https://github.com/Faless/netgame-godot
// 7. https://github.com/yhirose/cpp-httplib/blob/master/httplib.h

#include "gd_http_server.h"

#include "common/gd_core.h"
#include "core/project_settings.h"

#include <map>

static bool _remote_monitor_handler(const http::HTTPMessage *message, http::HTTPMessage *response);

#define _print_debug(...) DEBUG_PRINT(string_format(array("[Http] ", __VA_ARGS__)))
#define _print_fmt_debug(fmt, ...) DEBUG_PRINT(String("[Http] " fmt).sprintf(array(__VA_ARGS__)))

static GdHttpServer *instance = nullptr;

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
				for (Handler &h : routes) {
					http::HTTPMessage response;
					if (h(&http_message, &response)) {
						const std::string out = response.to_string();
						connection->put_data(reinterpret_cast<const uint8_t *>(out.c_str()), out.size());
						return true;
					}
				}
				if (get_script_instance() && get_script_instance()->has_method("_http_handler")) {
					Dictionary resp = get_script_instance()->call("_http_handler", helper::dict("path", http_message.get_path().c_str(), "query", http_message.get_query_string().c_str()));
					if (!resp.empty()) {
						http::HTTPMessage response;
						response.set_message_body(String(resp["body"]).utf8().c_str());
						const std::string out = response.to_string();
						connection->put_data(reinterpret_cast<const uint8_t *>(out.c_str()), out.size());
						return true;
					}
				}
				// 404
				http::HTTPMessage response;
				response.set_status_code(404);
				response.set_status_message("Handler not found");
				const std::string out = response.to_string();
				connection->put_data(reinterpret_cast<const uint8_t *>(out.c_str()), out.size());
				return false;
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
			if (self->server->listen(self->port) == OK) {
				self->active = true;
			} else {
				self->active = false;
				self->quit = true; // finish thread if we cannot bind server
			}
			self->cmd = CMD_NONE;
		} else if (self->cmd == CMD_STOP) {
			self->server->stop();
			self->active = false;
			self->cmd = CMD_NONE;
		}

		if (self->active) {
			if (self->server->is_connection_available()) {
				self->_process_connection(self->server->take_connection());
			}
		}

		OS::get_singleton()->delay_usec(100000);
	}
}

void GdHttpServer::start() {
	stop();
	port = GLOBAL_GET("network/http_server/port");
	cmd = CMD_ACTIVATE;
}

bool GdHttpServer::is_active() const {
	return active;
}

void GdHttpServer::stop() {
	cmd = CMD_STOP;
}

void GdHttpServer::register_handler(RouteHandler p_handler) {
	routes.push_back([p_handler](const http::HTTPMessage *message, http::HTTPMessage *response) {
		return p_handler(message, response);
	});
}

void GdHttpServer::register_handler(Handler p_handler) {
	routes.push_back(p_handler);
}

GdHttpServer *GdHttpServer::get_singleton() { return instance; }

GdHttpServer::GdHttpServer() {
	server.instance();
	quit = false;
	active = false;
	cmd = CMD_NONE;
	register_handler(_remote_monitor_handler);
	thread.start(_thread_start, this);

	GLOBAL_DEF("network/http_server/port", 8081);
#if defined(DEBUG_ENABLED) || defined(TOOLS_ENABLED)
	GLOBAL_DEF("network/http_server/autostart", true);
#else
	GLOBAL_DEF("network/http_server/autostart", false);
#endif

	if (instance) {
		WARN_PRINT("Http server instance is already create.");
	}
	instance = this;

	if (GLOBAL_GET("network/http_server/autostart")) {
		print_verbose("Auto-start http server on default port " + itos(GLOBAL_GET("network/http_server/port")));
		start();
	}
}

GdHttpServer::~GdHttpServer() {
	quit = true;
	thread.wait_to_finish();
	instance = nullptr;
}

/// Default monitor handler

#include "main/performance.h"

std::map<std::string, Performance::Monitor> _monitors{
	{ "TIME_FPS", Performance::TIME_FPS },
	{ "TIME_PROCESS", Performance::TIME_PROCESS },
	{ "TIME_PHYSICS_PROCESS", Performance::TIME_PHYSICS_PROCESS },
	{ "MEMORY_STATIC", Performance::MEMORY_STATIC },
	{ "MEMORY_DYNAMIC", Performance::MEMORY_DYNAMIC },
	{ "MEMORY_STATIC_MAX", Performance::MEMORY_STATIC_MAX },
	{ "MEMORY_DYNAMIC_MAX", Performance::MEMORY_DYNAMIC_MAX },
	{ "MEMORY_MESSAGE_BUFFER_MAX", Performance::MEMORY_MESSAGE_BUFFER_MAX },
	{ "OBJECT_COUNT", Performance::OBJECT_COUNT },
	{ "OBJECT_RESOURCE_COUNT", Performance::OBJECT_RESOURCE_COUNT },
	{ "OBJECT_NODE_COUNT", Performance::OBJECT_NODE_COUNT },
	{ "OBJECT_ORPHAN_NODE_COUNT", Performance::OBJECT_ORPHAN_NODE_COUNT },
	{ "RENDER_OBJECTS_IN_FRAME", Performance::RENDER_OBJECTS_IN_FRAME },
	{ "RENDER_VERTICES_IN_FRAME", Performance::RENDER_VERTICES_IN_FRAME },
	{ "RENDER_MATERIAL_CHANGES_IN_FRAME", Performance::RENDER_MATERIAL_CHANGES_IN_FRAME },
	{ "RENDER_SHADER_CHANGES_IN_FRAME", Performance::RENDER_SHADER_CHANGES_IN_FRAME },
	{ "RENDER_SURFACE_CHANGES_IN_FRAME", Performance::RENDER_SURFACE_CHANGES_IN_FRAME },
	{ "RENDER_DRAW_CALLS_IN_FRAME", Performance::RENDER_DRAW_CALLS_IN_FRAME },
	{ "RENDER_2D_ITEMS_IN_FRAME", Performance::RENDER_2D_ITEMS_IN_FRAME },
	{ "RENDER_2D_DRAW_CALLS_IN_FRAME", Performance::RENDER_2D_DRAW_CALLS_IN_FRAME },
	{ "RENDER_VIDEO_MEM_USED", Performance::RENDER_VIDEO_MEM_USED },
	{ "RENDER_TEXTURE_MEM_USED", Performance::RENDER_TEXTURE_MEM_USED },
	{ "RENDER_VERTEX_MEM_USED", Performance::RENDER_VERTEX_MEM_USED },
	{ "RENDER_USAGE_VIDEO_MEM_TOTAL", Performance::RENDER_USAGE_VIDEO_MEM_TOTAL },
	{ "PHYSICS_2D_ACTIVE_OBJECTS", Performance::PHYSICS_2D_ACTIVE_OBJECTS },
	{ "PHYSICS_2D_COLLISION_PAIRS", Performance::PHYSICS_2D_COLLISION_PAIRS },
	{ "PHYSICS_2D_ISLAND_COUNT", Performance::PHYSICS_2D_ISLAND_COUNT },
	{ "PHYSICS_3D_ACTIVE_OBJECTS", Performance::PHYSICS_3D_ACTIVE_OBJECTS },
	{ "PHYSICS_3D_COLLISION_PAIRS", Performance::PHYSICS_3D_COLLISION_PAIRS },
	{ "PHYSICS_3D_ISLAND_COUNT", Performance::PHYSICS_3D_ISLAND_COUNT },
	{ "AUDIO_OUTPUT_LATENCY", Performance::AUDIO_OUTPUT_LATENCY },
};

static bool _remote_monitor_handler(const http::HTTPMessage *message, http::HTTPMessage *response) {
	ERR_FAIL_NULL_V(message, false);
	ERR_FAIL_NULL_V(response, false);
	if (message->get_path() == "/gdmon") {
		std::string query = message->get_query_string();
		if (!query.empty()) {
			if (_monitors.count(query)) {
				response->set_message_body(rtos(Performance::get_singleton()->get_monitor(_monitors[query])).utf8().c_str());
				return true;
			}
		}
	}
	return false;
}
