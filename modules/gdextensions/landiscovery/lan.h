#include "core/map.h"
#include "core/object.h"
#include "core/io/packet_peer_udp.h"
#include "scene/main/node.h"
#include "scene/main/timer.h"

class GdLanAdvertiser : public Node {
	GDCLASS(GdLanAdvertiser, Object);

	real_t broadcast_interval;
	int broadcast_port;
	Dictionary server_info;

	Ref<PacketPeerUDP> _udp_socket;
	Timer *_broadcast_timer;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _broadcast();

public:
	GdLanAdvertiser();
};

class GdLanListener : public Node {
	GDCLASS(GdLanListener, Object);

	int listen_port;
	int server_cleanup_timeout; // Number of seconds to wait when a server
	                            // hasn't been heard from before remove

	Ref<PacketPeerUDP> _udp_socket;
	Timer *_cleanup_timer;
	Map<String, Dictionary> _known_servers;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _cleanup();

public:
	GdLanListener();
};
