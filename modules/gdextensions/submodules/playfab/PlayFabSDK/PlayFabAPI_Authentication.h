/*************************************************************************/
/*  PlayFabAPI_Authentication.h                                          */
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

namespace Authentication {

int ActivateKey(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Activates the given API key. Active keys may be used for authentication.
	// https://docs.microsoft.com/rest/api/playfab/authentication/api-keys/activatekey

	return _http_cli->request_append(
			"/APIKey/ActivateKey",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int CreateKey(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Creates an API key for the given entity.
	// https://docs.microsoft.com/rest/api/playfab/authentication/api-keys/createkey

	return _http_cli->request_append(
			"/APIKey/CreateKey",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int DeactivateKey(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Deactivates the given API key, causing subsequent authentication attempts with it to fail.Deactivating a key is a way to
	// verify that the key is not in use before deleting it.
	// https://docs.microsoft.com/rest/api/playfab/authentication/api-keys/deactivatekey

	return _http_cli->request_append(
			"/APIKey/DeactivateKey",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int Delete(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Delete a game_server entity.
	// https://docs.microsoft.com/rest/api/playfab/authentication/authentication/delete

	return _http_cli->request_append(
			"/GameServerIdentity/Delete",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int DeleteKey(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Deletes the given API key.
	// https://docs.microsoft.com/rest/api/playfab/authentication/api-keys/deletekey

	return _http_cli->request_append(
			"/APIKey/DeleteKey",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int GetEntityToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Method to exchange a legacy AuthenticationTicket or title SecretKey for an Entity Token or to refresh a still valid
	// Entity Token.
	// https://docs.microsoft.com/rest/api/playfab/authentication/authentication/getentitytoken

	Array list_prologue_work;

	if (!PlayFabSettings()._internalSettings.EntityToken.empty()) {
		list_prologue_work = array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN);
	} else if (!PlayFabSettings().DeveloperSecretKey.empty()) {
		list_prologue_work = array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY);
	} else if (!PlayFabSettings()._internalSettings.ClientSessionTicket.empty()) {
		list_prologue_work = array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION);
	}

	return _http_cli->request_append(
			"/Authentication/GetEntityToken",
			dict_request,
			user_callback,
			dict_header_extra,
			list_prologue_work,
			array(UPD_ENTITY_TOKEN));
}

int GetKeys(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Gets the API keys associated with the given entity.
	// https://docs.microsoft.com/rest/api/playfab/authentication/api-keys/getkeys

	return _http_cli->request_append(
			"/APIKey/GetKeys",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ValidateEntityToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Method for a server to validate a client provided EntityToken. Only callable by the title entity.
	// https://docs.microsoft.com/rest/api/playfab/authentication/authentication/validateentitytoken

	return _http_cli->request_append(
			"/Authentication/ValidateEntityToken",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

} // namespace Authentication
