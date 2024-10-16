/**************************************************************************/
/*  PlayFabAPI_Matchmaker.h                                               */
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

/* Auto-generated with SDKGenerator (don't manually edit) */

namespace Matchmaker {

int AuthUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Validates a user with the PlayFab service
	// https://docs.microsoft.com/rest/api/playfab/matchmaker/matchmaking/authuser

	return _http_cli->request_append(
			"/Matchmaker/AuthUser",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
			Array());
}

int PlayerJoined(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Informs the PlayFab game server hosting service that the indicated user has joined the Game Server Instance specified
	// https://docs.microsoft.com/rest/api/playfab/matchmaker/matchmaking/playerjoined

	return _http_cli->request_append(
			"/Matchmaker/PlayerJoined",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
			Array());
}

int PlayerLeft(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Informs the PlayFab game server hosting service that the indicated user has left the Game Server Instance specified
	// https://docs.microsoft.com/rest/api/playfab/matchmaker/matchmaking/playerleft

	return _http_cli->request_append(
			"/Matchmaker/PlayerLeft",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
			Array());
}

int UserInfo(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the relevant details for a specified user, which the external match-making service can then use to compute
	// effective matches
	// https://docs.microsoft.com/rest/api/playfab/matchmaker/matchmaking/userinfo

	return _http_cli->request_append(
			"/Matchmaker/UserInfo",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
			Array());
}

} // namespace Matchmaker
