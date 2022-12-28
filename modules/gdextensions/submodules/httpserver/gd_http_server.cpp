// Reference:
// ----------
// 1. https://github.com/xaltaq/GDHTTPServer/blob/master/httpserver.gd
// 2. https://github.com/Faless/netgame-godot
// 3. https://github.com/deep-entertainment/godottpd/blob/main/addons/godottpd/http_server.gd

#include "gdhttpserver.h"

#include "common/gd_core.h"

void GdHttpServer::_process_connection(Ref<StreamPeerTCP> connection) {
	print_debug("Got peer: ", connection.get_connected_host(), ":", connection.get_connected_port());
	connection->set_no_delay(true);
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

		self->wait_mutex.lock();
		while (self->to_wait.size()) {
			Thread *w = self->to_wait.front()->get();
			self->to_wait.erase(w);
			self->wait_mutex.unlock();
			w->wait_to_finish();
			self->wait_mutex.lock();
		}
		self->wait_mutex.unlock();

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
