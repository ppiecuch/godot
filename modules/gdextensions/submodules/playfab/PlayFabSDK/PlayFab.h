/*************************************************************************/
/*  PlayFab.h                                                            */
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

/* Auto-generated with SDKGenerator (don't manually edit) */

#ifndef PLAYFAB_H
#define PLAYFAB_H

#include "core/variant.h"
#include "core/reference.h"
#include "core/error_macros.h"
#include "core/bind/core_bind.h"
#include "core/io/json.h"
#include "core/io/http_client.h"
#include "common/gd_core.h"

namespace PlayFab {

// PlayFab enums

enum E_PRO {
	CHK_SESSION_TICKET,
	CHK_ENTITY_TOKEN,
	CHK_SECRET_KEY,
	USE_TITLE_ID,
	USE_AUTH_AUTHORIZATION,
	USE_AUTH_ENTITY_TOKEN,
	USE_AUTH_SECRET_KEY,
};

enum E_EPI {
	UPD_SESSION_TICKET,
	UPD_ENTITY_TOKEN,
	UPD_ATTRIBUTE,
	REQ_MULTI_STEP_CLIENT_LOGIN,
};

#include "PlayFabSettings.h"
#include "PlayFabHttpClient.h"

Ref<PlayFabHTTPClient> _http_cli;


#include "PlayFabAPI_Admin.h"
#include "PlayFabAPI_Client.h"
#include "PlayFabAPI_Matchmaker.h"
#include "PlayFabAPI_Server.h"
#include "PlayFabAPI_Authentication.h"
#include "PlayFabAPI_CloudScript.h"
#include "PlayFabAPI_Data.h"
#include "PlayFabAPI_Economy.h"
#include "PlayFabAPI_Events.h"
#include "PlayFabAPI_Experimentation.h"
#include "PlayFabAPI_Insights.h"
#include "PlayFabAPI_Groups.h"
#include "PlayFabAPI_Leaderboards.h"
#include "PlayFabAPI_Localization.h"
#include "PlayFabAPI_Multiplayer.h"
#include "PlayFabAPI_Profiles.h"

_FORCE_INLINE_ int ClientAttributeInstall(Dictionary dict_request) {
	return Client::AttributeInstall(dict_request);
}

String status_ntoa(int n) {
	switch (n) {
		case HTTPClient::STATUS_DISCONNECTED: return "STATUS_DISCONNECTED";
		case HTTPClient::STATUS_RESOLVING: return "STATUS_RESOLVING";
		case HTTPClient::STATUS_CANT_RESOLVE: return "STATUS_CANT_RESOLVE";
		case HTTPClient::STATUS_CONNECTING: return "STATUS_CONNECTING";
		case HTTPClient::STATUS_CANT_CONNECT: return "STATUS_CANT_CONNECT";
		case HTTPClient::STATUS_CONNECTED: return "STATUS_CONNECTED";
		case HTTPClient::STATUS_REQUESTING: return "STATUS_REQUESTING";
		case HTTPClient::STATUS_BODY: return "STATUS_BODY";
		case HTTPClient::STATUS_CONNECTION_ERROR: return "STATUS_CONNECTION_ERROR";
		case HTTPClient::STATUS_SSL_HANDSHAKE_ERROR: return "STATUS_SSL_HANDSHAKE_ERROR";
		default: return vformat("STATUS_[UNKNOWN %d]", n);
	}
}

bool is_valid() {
	return _http_cli.is_valid();
}

void reset() {
	if (is_valid()) {
		_http_cli->reset();
		PlayFabSettings().reset();
	}
}

int get_status() {
	if (!is_valid()) {
		return HTTPClient::STATUS_DISCONNECTED;
	} else {
		return _http_cli->status_curr;
	}
}

int request_queue_size() {
	if (!is_valid()) {
		return 0;
	} else {
		int queue_size = _http_cli->_request_buffers.size();
		if (_http_cli->_current_request) {
			queue_size += 1;
		}
		return queue_size;
	}
}

void _ready() {
	_http_cli = newref(PlayFabHTTPClient);
}

void _process(real_t delta) {
	_http_cli->update(delta);
}

} // namespace PlayFab

#endif // PLAYFAB_H
