/**************************************************************************/
/*  gd_http_server.cpp                                                    */
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

static std::string _html_page(const std::string &title, const std::string &body);
static bool _remote_monitor_handler(const http::HTTPMessage *message, http::HTTPMessage *response);
static bool _monitor_value_handler(const http::HTTPMessage *message, http::HTTPMessage *response);
static bool _index_handler(const http::HTTPMessage *message, http::HTTPMessage *response);
#ifdef GDEXT_HWINFO_ENABLED
static bool _hwinfo_handler(const http::HTTPMessage *message, http::HTTPMessage *response);
#endif

#define _print_debug(...) DEBUG_PRINT(vconcat("[Http] ", __VA_ARGS__))
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
					Dictionary resp = get_script_instance()->call("_http_handler", make_dict("path", http_message.get_path().c_str(), "query", http_message.get_query_string().c_str()));
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
				response.set_header("Content-Type", "text/html; charset=utf-8");
				std::string err_body = "<h1>404 &mdash; Not Found</h1>"
									   "<p>The requested path <code>" +
						http_message.get_path() + "</code> was not found.</p>"
												  "<nav><a href=\"/\">&larr; Back to index</a></nav>";
				response.set_message_body(_html_page("404 Not Found", err_body));
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
	register_handler(_index_handler);
	register_handler(_monitor_value_handler);
	register_handler(_remote_monitor_handler);
#ifdef GDEXT_HWINFO_ENABLED
	register_handler(_hwinfo_handler);
#endif
	thread.start(_thread_start, this);

	GLOBAL_DEF("network/http_server/port", 8081);
#if defined(DEBUG_ENABLED) || defined(TOOLS_ENABLED)
	GLOBAL_DEF("network/http_server/autostart", true);
#else
	GLOBAL_DEF("network/http_server/autostart", false);
#endif

	if (instance) {
		WARN_PRINT("Http server instance already created.");
	}
	instance = this;

#ifndef TOOLS_ENABLED
	if (GLOBAL_GET("network/http_server/autostart")) {
		print_verbose("Auto-start http server on default port " + itos(GLOBAL_GET("network/http_server/port")));
		start();
	}
#endif
}

GdHttpServer::~GdHttpServer() {
	quit = true;
	thread.wait_to_finish();
	instance = nullptr;
}

/// Default handlers

#include "core/version.h"
#include "main/performance.h"

struct MonitorEntry {
	const char *name;
	const char *label;
	const char *category;
	Performance::Monitor monitor;
};

static const MonitorEntry _monitors[] = {
	{ "TIME_FPS", "FPS", "Time", Performance::TIME_FPS },
	{ "TIME_PROCESS", "Process", "Time", Performance::TIME_PROCESS },
	{ "TIME_PHYSICS_PROCESS", "Physics Process", "Time", Performance::TIME_PHYSICS_PROCESS },
	{ "MEMORY_STATIC", "Static", "Memory", Performance::MEMORY_STATIC },
	{ "MEMORY_DYNAMIC", "Dynamic", "Memory", Performance::MEMORY_DYNAMIC },
	{ "MEMORY_STATIC_MAX", "Static Max", "Memory", Performance::MEMORY_STATIC_MAX },
	{ "MEMORY_DYNAMIC_MAX", "Dynamic Max", "Memory", Performance::MEMORY_DYNAMIC_MAX },
	{ "MEMORY_MESSAGE_BUFFER_MAX", "Message Buffer Max", "Memory", Performance::MEMORY_MESSAGE_BUFFER_MAX },
	{ "OBJECT_COUNT", "Count", "Objects", Performance::OBJECT_COUNT },
	{ "OBJECT_RESOURCE_COUNT", "Resources", "Objects", Performance::OBJECT_RESOURCE_COUNT },
	{ "OBJECT_NODE_COUNT", "Nodes", "Objects", Performance::OBJECT_NODE_COUNT },
	{ "OBJECT_ORPHAN_NODE_COUNT", "Orphan Nodes", "Objects", Performance::OBJECT_ORPHAN_NODE_COUNT },
	{ "RENDER_OBJECTS_IN_FRAME", "Objects", "Render", Performance::RENDER_OBJECTS_IN_FRAME },
	{ "RENDER_VERTICES_IN_FRAME", "Vertices", "Render", Performance::RENDER_VERTICES_IN_FRAME },
	{ "RENDER_MATERIAL_CHANGES_IN_FRAME", "Material Changes", "Render", Performance::RENDER_MATERIAL_CHANGES_IN_FRAME },
	{ "RENDER_SHADER_CHANGES_IN_FRAME", "Shader Changes", "Render", Performance::RENDER_SHADER_CHANGES_IN_FRAME },
	{ "RENDER_SURFACE_CHANGES_IN_FRAME", "Surface Changes", "Render", Performance::RENDER_SURFACE_CHANGES_IN_FRAME },
	{ "RENDER_DRAW_CALLS_IN_FRAME", "Draw Calls", "Render", Performance::RENDER_DRAW_CALLS_IN_FRAME },
	{ "RENDER_2D_ITEMS_IN_FRAME", "2D Items", "Render", Performance::RENDER_2D_ITEMS_IN_FRAME },
	{ "RENDER_2D_DRAW_CALLS_IN_FRAME", "2D Draw Calls", "Render", Performance::RENDER_2D_DRAW_CALLS_IN_FRAME },
	{ "RENDER_VIDEO_MEM_USED", "Video Mem", "Render", Performance::RENDER_VIDEO_MEM_USED },
	{ "RENDER_TEXTURE_MEM_USED", "Texture Mem", "Render", Performance::RENDER_TEXTURE_MEM_USED },
	{ "RENDER_VERTEX_MEM_USED", "Vertex Mem", "Render", Performance::RENDER_VERTEX_MEM_USED },
	{ "RENDER_USAGE_VIDEO_MEM_TOTAL", "Video Mem Total", "Render", Performance::RENDER_USAGE_VIDEO_MEM_TOTAL },
	{ "PHYSICS_2D_ACTIVE_OBJECTS", "2D Active Objects", "Physics", Performance::PHYSICS_2D_ACTIVE_OBJECTS },
	{ "PHYSICS_2D_COLLISION_PAIRS", "2D Collision Pairs", "Physics", Performance::PHYSICS_2D_COLLISION_PAIRS },
	{ "PHYSICS_2D_ISLAND_COUNT", "2D Islands", "Physics", Performance::PHYSICS_2D_ISLAND_COUNT },
	{ "PHYSICS_3D_ACTIVE_OBJECTS", "3D Active Objects", "Physics", Performance::PHYSICS_3D_ACTIVE_OBJECTS },
	{ "PHYSICS_3D_COLLISION_PAIRS", "3D Collision Pairs", "Physics", Performance::PHYSICS_3D_COLLISION_PAIRS },
	{ "PHYSICS_3D_ISLAND_COUNT", "3D Islands", "Physics", Performance::PHYSICS_3D_ISLAND_COUNT },
	{ "AUDIO_OUTPUT_LATENCY", "Output Latency", "Audio", Performance::AUDIO_OUTPUT_LATENCY },
	{ nullptr, nullptr, nullptr, Performance::MONITOR_MAX },
};

// Shared HTML page shell using htmx for live updates.
static std::string _html_page(const std::string &title, const std::string &body) {
	return "<!DOCTYPE html>"
		   "<html lang=\"en\"><head><meta charset=\"utf-8\">"
		   "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
		   "<title>" +
			title +
			"</title>"
			"<script src=\"https://unpkg.com/htmx.org@2.0.4\"></script>"
			"<style>"
			"*{box-sizing:border-box;margin:0;padding:0}"
			"body{font-family:system-ui,-apple-system,sans-serif;background:#1a1a2e;color:#e0e0e0;padding:1rem}"
			"h1{color:#7fdbca;margin-bottom:.5rem;font-size:1.4rem}"
			"h2{color:#c792ea;margin:1rem 0 .5rem;font-size:1.1rem;border-bottom:1px solid #333}"
			".subtitle{color:#666;font-size:.8rem;margin-bottom:1rem}"
			"table{width:100%;border-collapse:collapse;margin-bottom:1rem}"
			"th{text-align:left;padding:.3rem .5rem;color:#82aaff;font-weight:500;font-size:.8rem}"
			"td{padding:.3rem .5rem;border-top:1px solid #2a2a4a;font-size:.85rem}"
			"td.val{text-align:right;font-variant-numeric:tabular-nums;font-family:ui-monospace,monospace;color:#c3e88d}"
			"a{color:#82aaff;text-decoration:none}a:hover{text-decoration:underline}"
			"nav{margin-bottom:1rem}nav a{margin-right:1rem}"
			".live{opacity:.7;font-size:.7rem;color:#7fdbca}"
			"</style></head><body>" +
			body + "</body></html>";
}

// Returns a single monitor value as plain text (for htmx polling).
static bool _monitor_value_handler(const http::HTTPMessage *message, http::HTTPMessage *response) {
	ERR_FAIL_NULL_V(message, false);
	ERR_FAIL_NULL_V(response, false);
	if (message->get_path() != "/gdmon/value") {
		return false;
	}
	std::string query = message->get_query_string();
	if (query.empty()) {
		return false;
	}
	for (int i = 0; _monitors[i].name; i++) {
		if (query == _monitors[i].name) {
			response->set_status_code(200);
			response->set_header("Content-Type", "text/plain");
			response->set_message_body(rtos(Performance::get_singleton()->get_monitor(_monitors[i].monitor)).utf8().c_str());
			return true;
		}
	}
	return false;
}

static bool _remote_monitor_handler(const http::HTTPMessage *message, http::HTTPMessage *response) {
	ERR_FAIL_NULL_V(message, false);
	ERR_FAIL_NULL_V(response, false);

	if (message->get_path() == "/gdmon") {
		std::string query = message->get_query_string();

		// Plain text API: GET /gdmon?TIME_FPS returns raw value.
		if (!query.empty()) {
			for (int i = 0; _monitors[i].name; i++) {
				if (query == _monitors[i].name) {
					response->set_status_code(200);
					response->set_header("Content-Type", "text/plain");
					response->set_message_body(rtos(Performance::get_singleton()->get_monitor(_monitors[i].monitor)).utf8().c_str());
					return true;
				}
			}
			return false;
		}

		// HTML dashboard: GET /gdmon serves interactive page with htmx polling.
		std::string body;
		body += "<h1>Godot Performance Monitor</h1>";
		body += "<p class=\"subtitle\">Godot " VERSION_FULL_CONFIG " &mdash; values update every 2s via htmx</p>";
		body += "<nav><a href=\"/\">&larr; Index</a></nav>";

		std::string current_category;
		for (int i = 0; _monitors[i].name; i++) {
			if (current_category != _monitors[i].category) {
				if (!current_category.empty()) {
					body += "</table>";
				}
				current_category = _monitors[i].category;
				body += "<h2>";
				body += current_category;
				body += "</h2><table><tr><th>Metric</th><th style=\"text-align:right\">Value <span class=\"live\">live</span></th></tr>";
			}
			body += "<tr><td>";
			body += _monitors[i].label;
			body += "</td><td class=\"val\" hx-get=\"/gdmon/value?";
			body += _monitors[i].name;
			body += "\" hx-trigger=\"load,every 2s\" hx-swap=\"innerHTML\">";
			body += rtos(Performance::get_singleton()->get_monitor(_monitors[i].monitor)).utf8().c_str();
			body += "</td></tr>";
		}
		body += "</table>";

		response->set_status_code(200);
		response->set_header("Content-Type", "text/html; charset=utf-8");
		response->set_message_body(_html_page("Godot Monitor", body));
		return true;
	}
	return false;
}

static bool _index_handler(const http::HTTPMessage *message, http::HTTPMessage *response) {
	ERR_FAIL_NULL_V(message, false);
	ERR_FAIL_NULL_V(response, false);
	if (message->get_path() != "/") {
		return false;
	}

	std::string body;
	body += "<h1>Godot HTTP Server</h1>";
	body += "<p class=\"subtitle\">Godot " VERSION_FULL_CONFIG "</p>";
	body += "<h2>Endpoints</h2>";
	body += "<table>";
	body += "<tr><th>Path</th><th>Description</th></tr>";
	body += "<tr><td><a href=\"/gdmon\">/gdmon</a></td><td>Performance monitor dashboard (live)</td></tr>";
	body += "<tr><td>/gdmon?METRIC</td><td>Single metric value as plain text (e.g. <a href=\"/gdmon?TIME_FPS\">/gdmon?TIME_FPS</a>)</td></tr>";
#ifdef GDEXT_HWINFO_ENABLED
	body += "<tr><td><a href=\"/hwinfo\">/hwinfo</a></td><td>Hardware information dashboard</td></tr>";
	body += "<tr><td><a href=\"/hwinfo?json\">/hwinfo?json</a></td><td>Hardware info as JSON</td></tr>";
#endif
	body += "</table>";

	response->set_status_code(200);
	response->set_header("Content-Type", "text/html; charset=utf-8");
	response->set_message_body(_html_page("Godot HTTP Server", body));
	return true;
}

// --- Hardware Info endpoint ---

#ifdef GDEXT_HWINFO_ENABLED
#include <hwinfo/battery.h>
#include <hwinfo/cpu.h>
#include <hwinfo/disk.h>
#include <hwinfo/gpu.h>
#include <hwinfo/mainboard.h>
#include <hwinfo/os.h>
#include <hwinfo/ram.h>
#include <hwinfo/system.h>

#include "core/io/json.h"

static bool _hwinfo_handler(const http::HTTPMessage *message, http::HTTPMessage *response) {
	ERR_FAIL_NULL_V(message, false);
	ERR_FAIL_NULL_V(response, false);
	if (message->get_path() != "/hwinfo") {
		return false;
	}

	// Helper lambda
	auto s = [](const std::string &str) -> String { return String::utf8(str.c_str()); };

	// Collect all hardware info into a Godot Dictionary for JSON serialization
	Dictionary hw;

	// CPU
	{
		Array cpus;
		auto sockets = hwinfo::getAllSockets();
		for (const auto &sock : sockets) {
			const auto &cpu = sock.cpu();
			Dictionary d;
			d["socket_id"] = sock.id();
			d["model_name"] = s(cpu.modelName());
			d["vendor"] = s(cpu.vendor());
			d["physical_cores"] = cpu.numPhysicalCores();
			d["logical_cores"] = cpu.numLogicalCores();
			d["max_clock_mhz"] = (int64_t)cpu.maxClockSpeed_MHz();
			d["regular_clock_mhz"] = (int64_t)cpu.regularClockSpeed_MHz();
			d["cache_bytes"] = (int64_t)cpu.cacheSize_Bytes();
			cpus.push_back(d);
		}
		hw["cpu"] = cpus;
	}

	// GPU
	{
		Array gpus;
		auto all_gpus = hwinfo::getAllGPUs();
		for (const auto &gpu : all_gpus) {
			Dictionary d;
			d["name"] = s(gpu.name());
			d["vendor"] = s(gpu.vendor());
			d["driver_version"] = s(gpu.driverVersion());
			d["memory_mb"] = (int64_t)gpu.totalMemoryMBytes();
			gpus.push_back(d);
		}
		hw["gpu"] = gpus;
	}

	// RAM
	{
		hwinfo::RAM ram;
		Dictionary d;
		d["total_mb"] = (int64_t)(ram.total_Bytes() / (1024 * 1024));
		d["available_mb"] = (int64_t)(ram.available_Bytes() / (1024 * 1024));
		d["free_mb"] = (int64_t)(ram.free_Bytes() / (1024 * 1024));
		hw["ram"] = d;
	}

	// OS
	{
		hwinfo::OS os;
		Dictionary d;
		d["full_name"] = s(os.fullName());
		d["name"] = s(os.name());
		d["version"] = s(os.version());
		d["kernel"] = s(os.kernel());
		d["is_64bit"] = os.is64bit();
		hw["os"] = d;
	}

	// System
	{
		hwinfo::System sys;
		Dictionary d;
		d["machine_id"] = s(sys.getMachineUniqueId());
		d["uptime_seconds"] = (int64_t)sys.getUptimeSeconds();
		d["num_processes"] = (int64_t)sys.getNumProcesses();
		hw["system"] = d;
	}

	// MainBoard
	{
		hwinfo::MainBoard mb;
		Dictionary d;
		d["vendor"] = s(mb.vendor());
		d["name"] = s(mb.name());
		d["version"] = s(mb.version());
		hw["mainboard"] = d;
	}

	// Disks
	{
		Array disks;
		auto all_disks = hwinfo::getAllDisks();
		for (const auto &disk : all_disks) {
			Dictionary d;
			d["model"] = s(disk.model());
			d["vendor"] = s(disk.vendor());
			d["size_gb"] = (int64_t)(disk.size_Bytes() / (1024 * 1024 * 1024));
			disks.push_back(d);
		}
		hw["disks"] = disks;
	}

	// Battery
	{
		Array batteries;
		auto all_bats = hwinfo::getAllBatteries();
		for (auto &bat : all_bats) {
			Dictionary d;
			d["vendor"] = s(bat.vendor());
			d["model"] = s(bat.model());
			d["capacity"] = bat.capacity();
			d["charging"] = bat.charging();
			batteries.push_back(d);
		}
		hw["batteries"] = batteries;
	}

	std::string query = message->get_query_string();

	if (query == "json") {
		// JSON API
		String json_str = JSON::print(hw, "", false);
		response->set_status_code(200);
		response->set_header("Content-Type", "application/json; charset=utf-8");
		response->set_header("Access-Control-Allow-Origin", "*");
		response->set_message_body(json_str.utf8().c_str());
		return true;
	}

	// HTML dashboard
	std::string body;
	body += "<h1>Hardware Information</h1>";
	body += "<p class=\"subtitle\">via <a href=\"/hwinfo?json\">JSON API</a></p>";
	body += "<nav><a href=\"/\">&larr; Index</a></nav>";

	// Helper: emit a table row
	auto row = [&body](const char *label, const String &value) {
		body += std::string("<tr><td>") + label + "</td><td class=\"val\">" + value.utf8().get_data() + "</td></tr>";
	};
	auto tbl_start = [&body](const char *title) {
		body += std::string("<h2>") + title + "</h2><table><tr><th>Property</th><th style=\"text-align:right\">Value</th></tr>";
	};
	auto tbl_end = [&body]() { body += "</table>"; };

	// CPU
	{
		Array cpus = hw["cpu"];
		tbl_start("CPU");
		for (int i = 0; i < cpus.size(); i++) {
			Dictionary cpu = cpus[i];
			row("Model", cpu["model_name"]);
			row("Vendor", cpu["vendor"]);
			row("Physical Cores", itos((int)cpu["physical_cores"]));
			row("Logical Cores", itos((int)cpu["logical_cores"]));
			row("Max Clock (MHz)", itos((int64_t)cpu["max_clock_mhz"]));
			row("Cache (bytes)", itos((int64_t)cpu["cache_bytes"]));
		}
		tbl_end();
	}

	// GPU
	{
		Array gpus = hw["gpu"];
		if (gpus.size() > 0) {
			tbl_start("GPU");
			for (int i = 0; i < gpus.size(); i++) {
				Dictionary gpu = gpus[i];
				row("Name", gpu["name"]);
				row("Vendor", gpu["vendor"]);
				row("Driver", gpu["driver_version"]);
				row("Memory (MB)", itos((int64_t)gpu["memory_mb"]));
			}
			tbl_end();
		}
	}

	// RAM
	{
		Dictionary ram = hw["ram"];
		tbl_start("RAM");
		row("Total (MB)", itos((int64_t)ram["total_mb"]));
		row("Available (MB)", itos((int64_t)ram["available_mb"]));
		row("Free (MB)", itos((int64_t)ram["free_mb"]));
		tbl_end();
	}

	// OS
	{
		Dictionary os = hw["os"];
		tbl_start("Operating System");
		row("Full Name", os["full_name"]);
		row("Kernel", os["kernel"]);
		row("64-bit", (bool)os["is_64bit"] ? "Yes" : "No");
		tbl_end();
	}

	// System
	{
		Dictionary sys = hw["system"];
		tbl_start("System");
		row("Machine ID", sys["machine_id"]);
		row("Uptime (s)", itos((int64_t)sys["uptime_seconds"]));
		row("Processes", itos((int64_t)sys["num_processes"]));
		tbl_end();
	}

	// MainBoard
	{
		Dictionary mb = hw["mainboard"];
		String mb_name = mb["name"];
		if (!mb_name.empty() && mb_name != "<unknown>") {
			tbl_start("MainBoard");
			row("Name", mb_name);
			row("Vendor", mb["vendor"]);
			tbl_end();
		}
	}

	// Disks
	{
		Array disks = hw["disks"];
		if (disks.size() > 0) {
			body += "<h2>Storage</h2><table><tr><th>Model</th><th>Vendor</th><th style=\"text-align:right\">Size (GB)</th></tr>";
			for (int i = 0; i < disks.size(); i++) {
				Dictionary disk = disks[i];
				body += std::string("<tr><td>") + String(disk["model"]).utf8().get_data() + "</td>";
				body += std::string("<td>") + String(disk["vendor"]).utf8().get_data() + "</td>";
				body += std::string("<td class=\"val\">") + itos((int64_t)disk["size_gb"]).utf8().get_data() + "</td></tr>";
			}
			tbl_end();
		}
	}

	// Battery
	{
		Array bats = hw["batteries"];
		if (bats.size() > 0) {
			tbl_start("Battery");
			for (int i = 0; i < bats.size(); i++) {
				Dictionary bat = bats[i];
				row("Capacity", itos((int)(double(bat["capacity"]) * 100)) + "%");
				row("Charging", (bool)bat["charging"] ? "Yes" : "No");
			}
			tbl_end();
		}
	}

	response->set_status_code(200);
	response->set_header("Content-Type", "text/html; charset=utf-8");
	response->set_message_body(_html_page("Hardware Information", body));
	return true;
}
#endif // GDEXT_HWINFO_ENABLED
