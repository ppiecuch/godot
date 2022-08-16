/*************************************************************************/
/*  PlayFabAPI_Profiles.h                                                */
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

namespace Profiles {


int DeleteMasterPlayerAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes a master player account entirely from all titles and deletes all associated data
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/deletemasterplayeraccount
	

	return _http_cli->request_append(
		"/MasterPlayer/DeleteMasterPlayerAccount",
		dict_request,
		user_callback,
		dict_header_extra,
		Array(),
		Array()
	);
}

int ExportMasterPlayerAccountData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Exports all associated data of a master player account
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/exportmasterplayeraccountdata
	

	return _http_cli->request_append(
		"/MasterPlayer/ExportMasterPlayerAccountData",
		dict_request,
		user_callback,
		dict_header_extra,
		Array(),
		Array()
	);
}

int GetGlobalPolicy(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the global title access policy
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/getglobalpolicy
	

	return _http_cli->request_append(
		"/Profile/GetGlobalPolicy",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetMasterPlayerTitleList(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get the list of titles that the player has played
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/getmasterplayertitlelist
	

	return _http_cli->request_append(
		"/MasterPlayer/GetMasterPlayerTitleList",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetProfile(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/getprofile
	

	return _http_cli->request_append(
		"/Profile/GetProfile",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetProfiles(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/getprofiles
	

	return _http_cli->request_append(
		"/Profile/GetProfiles",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetTitlePlayersFromMasterPlayerAccountIds(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title player accounts associated with the given master player account.
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/gettitleplayersfrommasterplayeraccountids
	

	return _http_cli->request_append(
		"/Profile/GetTitlePlayersFromMasterPlayerAccountIds",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetAvatarUrl(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update the avatar url of the entity
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/setavatarurl
	

	return _http_cli->request_append(
		"/Profile/SetAvatarUrl",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetDisplayName(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update the display name of the entity
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/setdisplayname
	

	return _http_cli->request_append(
		"/Profile/SetDisplayName",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetGlobalPolicy(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the global title access policy
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/setglobalpolicy
	

	return _http_cli->request_append(
		"/Profile/SetGlobalPolicy",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetProfileLanguage(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the entity's language. The precedence hierarchy for communication to the player is Title Player Account
	// language, Master Player Account language, and then title default language if the first two aren't set or supported.
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/setprofilelanguage
	

	return _http_cli->request_append(
		"/Profile/SetProfileLanguage",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetProfilePolicy(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the profiles access policy
	// https://docs.microsoft.com/rest/api/playfab/profiles/account-management/setprofilepolicy
	

	return _http_cli->request_append(
		"/Profile/SetProfilePolicy",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

} // namespace Profiles
