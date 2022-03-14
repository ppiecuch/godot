#ifndef GD_DISCORD_H
#define GD_DISCORD_H

#include "core/object.h"
#include "scene/main/http_request.h"


#define GODOTDISCORD_MAJOR 0
#define GODOTDISCORD_MINOR 0
#define GODOTDISCORD_PATCH 1

#define GD_STR(x) #x
#define GODOTDISCORD_VERSION GD_STR(GODOTDISCORD_MAJOR) "." GD_STR(GODOTDISCORD_MINOR) "." GD_STR(GODOTDISCORD_PATCH)
#define GODOTDISCORD_AGENT vformat("godot-discord (%s)", GODOTDISCORD_VERSION)

class GdDiscordClient : public Object {
	GDCLASS(GdDiscordClient, Object);

	HTTPRequest *http;
	void _request_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	GdDiscordClient();
};

#endif // GD_DISCORD_H
