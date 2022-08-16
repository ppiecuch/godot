/*************************************************************************/
/*  PlayFabAPI_Admin.h                                                   */
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

/* Auto-generated at Tue Aug 16 2022 20:53:55 GMT+0000 (Coordinated Universal Time) */

namespace Admin {


int AbortTaskInstance(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Abort an ongoing task instance.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/aborttaskinstance
	

	return _http_cli->request_append(
		"/Admin/AbortTaskInstance",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddLocalizedNews(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update news item to include localized version
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/addlocalizednews
	

	return _http_cli->request_append(
		"/Admin/AddLocalizedNews",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddNews(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds a new news item to the title's news feed
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/addnews
	

	return _http_cli->request_append(
		"/Admin/AddNews",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddPlayerTag(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds a given tag to a player profile. The tag's namespace is automatically generated based on the source of the tag.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/addplayertag
	

	return _http_cli->request_append(
		"/Admin/AddPlayerTag",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddUserVirtualCurrency(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Increments the specified virtual currency by the stated amount
	// https://docs.microsoft.com/rest/api/playfab/admin/player-item-management/adduservirtualcurrency
	

	return _http_cli->request_append(
		"/Admin/AddUserVirtualCurrency",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddVirtualCurrencyTypes(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds one or more virtual currencies to the set defined for the title. Virtual Currencies have a maximum value of
	// 2,147,483,647 when granted to a player. Any value over that will be discarded.
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/addvirtualcurrencytypes
	

	return _http_cli->request_append(
		"/Admin/AddVirtualCurrencyTypes",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int BanUsers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Bans users by PlayFab ID with optional IP address, or MAC address for the provided game.
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/banusers
	

	return _http_cli->request_append(
		"/Admin/BanUsers",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CheckLimitedEditionItemAvailability(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Checks the global count for the limited edition item.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-item-management/checklimitededitionitemavailability
	

	return _http_cli->request_append(
		"/Admin/CheckLimitedEditionItemAvailability",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreateActionsOnPlayersInSegmentTask(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create an ActionsOnPlayersInSegment task, which iterates through all players in a segment to execute action.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/createactionsonplayersinsegmenttask
	

	return _http_cli->request_append(
		"/Admin/CreateActionsOnPlayersInSegmentTask",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreateCloudScriptAzureFunctionsTask(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create a CloudScript task, which can run a CloudScript on a schedule.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/createcloudscriptazurefunctionstask
	

	return _http_cli->request_append(
		"/Admin/CreateCloudScriptAzureFunctionsTask",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreateCloudScriptTask(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create a CloudScript task, which can run a CloudScript on a schedule.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/createcloudscripttask
	

	return _http_cli->request_append(
		"/Admin/CreateCloudScriptTask",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreateInsightsScheduledScalingTask(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create a Insights Scheduled Scaling task, which can scale Insights Performance Units on a schedule
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/createinsightsscheduledscalingtask
	

	return _http_cli->request_append(
		"/Admin/CreateInsightsScheduledScalingTask",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreateOpenIdConnection(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Registers a relationship between a title and an Open ID Connect provider.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/createopenidconnection
	

	return _http_cli->request_append(
		"/Admin/CreateOpenIdConnection",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreatePlayerSharedSecret(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a new Player Shared Secret Key. It may take up to 5 minutes for this key to become generally available after
	// this API returns.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/createplayersharedsecret
	

	return _http_cli->request_append(
		"/Admin/CreatePlayerSharedSecret",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreatePlayerStatisticDefinition(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds a new player statistic configuration to the title, optionally allowing the developer to specify a reset interval
	// and an aggregation method.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/createplayerstatisticdefinition
	

	return _http_cli->request_append(
		"/Admin/CreatePlayerStatisticDefinition",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreateSegment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a new player segment by defining the conditions on player properties. Also, create actions to target the player
	// segments for a title.
	// https://docs.microsoft.com/rest/api/playfab/admin/segments/createsegment
	

	return _http_cli->request_append(
		"/Admin/CreateSegment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteContent(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Delete a content file from the title. When deleting a file that does not exist, it returns success.
	// https://docs.microsoft.com/rest/api/playfab/admin/content/deletecontent
	

	return _http_cli->request_append(
		"/Admin/DeleteContent",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteEventSink(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a sink to stop sending PlayStream and Telemetry event data
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/deleteeventsink
	

	return _http_cli->request_append(
		"/Admin/DeleteEventSink",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteMasterPlayerAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes a master player account entirely from all titles and deletes all associated data
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/deletemasterplayeraccount
	

	return _http_cli->request_append(
		"/Admin/DeleteMasterPlayerAccount",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteMembershipSubscription(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a player's subscription
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/deletemembershipsubscription
	

	return _http_cli->request_append(
		"/Admin/DeleteMembershipSubscription",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteOpenIdConnection(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes a relationship between a title and an OpenID Connect provider.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/deleteopenidconnection
	

	return _http_cli->request_append(
		"/Admin/DeleteOpenIdConnection",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeletePlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes a user's player account from a title and deletes all associated data
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/deleteplayer
	

	return _http_cli->request_append(
		"/Admin/DeletePlayer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeletePlayerSharedSecret(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes an existing Player Shared Secret Key. It may take up to 5 minutes for this delete to be reflected after this API
	// returns.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/deleteplayersharedsecret
	

	return _http_cli->request_append(
		"/Admin/DeletePlayerSharedSecret",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteSegment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes an existing player segment and its associated action(s) for a title.
	// https://docs.microsoft.com/rest/api/playfab/admin/segments/deletesegment
	

	return _http_cli->request_append(
		"/Admin/DeleteSegment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteStore(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes an existing virtual item store
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/deletestore
	

	return _http_cli->request_append(
		"/Admin/DeleteStore",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteTask(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Delete a task.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/deletetask
	

	return _http_cli->request_append(
		"/Admin/DeleteTask",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteTitle(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Permanently deletes a title and all associated configuration
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/deletetitle
	

	return _http_cli->request_append(
		"/Admin/DeleteTitle",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteTitleDataOverride(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a specified set of title data overrides.
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/deletetitledataoverride
	

	return _http_cli->request_append(
		"/Admin/DeleteTitleDataOverride",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ExportMasterPlayerData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Exports all associated data of a master player account
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/exportmasterplayerdata
	

	return _http_cli->request_append(
		"/Admin/ExportMasterPlayerData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ExportPlayersInSegment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Starts an export for the player profiles in a segment. This API creates a snapshot of all the player profiles which
	// match the segment definition at the time of the API call. Profiles which change while an export is in progress will not
	// be reflected in the results.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/exportplayersinsegment
	

	return _http_cli->request_append(
		"/Admin/ExportPlayersInSegment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetActionsOnPlayersInSegmentTaskInstance(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get information about a ActionsOnPlayersInSegment task instance.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/getactionsonplayersinsegmenttaskinstance
	

	return _http_cli->request_append(
		"/Admin/GetActionsOnPlayersInSegmentTaskInstance",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetAllSegments(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves an array of player segment definitions. Results from this can be used in subsequent API calls such as
	// GetPlayersInSegment which requires a Segment ID. While segment names can change the ID for that segment will not change.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/getallsegments
	

	return _http_cli->request_append(
		"/Admin/GetAllSegments",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCatalogItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the specified version of the title's catalog of virtual goods, including all defined properties
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/getcatalogitems
	

	return _http_cli->request_append(
		"/Admin/GetCatalogItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCloudScriptAzureFunctionsTaskInstance(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get detail information about a CloudScript Azure Functions task instance.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/getcloudscriptazurefunctionstaskinstance
	

	return _http_cli->request_append(
		"/Admin/GetCloudScriptAzureFunctionsTaskInstance",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCloudScriptRevision(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the contents and information of a specific Cloud Script revision.
	// https://docs.microsoft.com/rest/api/playfab/admin/server-side-cloud-script/getcloudscriptrevision
	

	return _http_cli->request_append(
		"/Admin/GetCloudScriptRevision",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCloudScriptTaskInstance(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get detail information about a CloudScript task instance.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/getcloudscripttaskinstance
	

	return _http_cli->request_append(
		"/Admin/GetCloudScriptTaskInstance",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCloudScriptVersions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists all the current cloud script versions. For each version, information about the current published and latest
	// revisions is also listed.
	// https://docs.microsoft.com/rest/api/playfab/admin/server-side-cloud-script/getcloudscriptversions
	

	return _http_cli->request_append(
		"/Admin/GetCloudScriptVersions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetContentList(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// List all contents of the title and get statistics such as size
	// https://docs.microsoft.com/rest/api/playfab/admin/content/getcontentlist
	

	return _http_cli->request_append(
		"/Admin/GetContentList",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetContentUploadUrl(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the pre-signed URL for uploading a content file. A subsequent HTTP PUT to the returned URL uploads the
	// content. Also, please be aware that the Content service is specifically PlayFab's CDN offering, for which standard CDN
	// rates apply.
	// https://docs.microsoft.com/rest/api/playfab/admin/content/getcontentuploadurl
	

	return _http_cli->request_append(
		"/Admin/GetContentUploadUrl",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetDataReport(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a download URL for the requested report
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getdatareport
	

	return _http_cli->request_append(
		"/Admin/GetDataReport",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetEventSinks(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the set of sinks to which to route PlayStream and Telemetry event data.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/geteventsinks
	

	return _http_cli->request_append(
		"/Admin/GetEventSinks",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetMatchmakerGameInfo(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the details for a specific completed session, including links to standard out and standard error logs
	// https://docs.microsoft.com/rest/api/playfab/admin/matchmaking/getmatchmakergameinfo
	

	return _http_cli->request_append(
		"/Admin/GetMatchmakerGameInfo",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetMatchmakerGameModes(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the details of defined game modes for the specified game server executable
	// https://docs.microsoft.com/rest/api/playfab/admin/matchmaking/getmatchmakergamemodes
	

	return _http_cli->request_append(
		"/Admin/GetMatchmakerGameModes",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayedTitleList(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get the list of titles that the player has played
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/getplayedtitlelist
	

	return _http_cli->request_append(
		"/Admin/GetPlayedTitleList",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerIdFromAuthToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets a player's ID from an auth token.
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/getplayeridfromauthtoken
	

	return _http_cli->request_append(
		"/Admin/GetPlayerIdFromAuthToken",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerProfile(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the player's profile
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/getplayerprofile
	

	return _http_cli->request_append(
		"/Admin/GetPlayerProfile",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerSegments(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// List all segments that a player currently belongs to at this moment in time.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/getplayersegments
	

	return _http_cli->request_append(
		"/Admin/GetPlayerSegments",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerSharedSecrets(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Returns all Player Shared Secret Keys including disabled and expired.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/getplayersharedsecrets
	

	return _http_cli->request_append(
		"/Admin/GetPlayerSharedSecrets",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayersInSegment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Allows for paging through all players in a given segment. This API creates a snapshot of all player profiles that match
	// the segment definition at the time of its creation and lives through the Total Seconds to Live, refreshing its life span
	// on each subsequent use of the Continuation Token. Profiles that change during the course of paging will not be reflected
	// in the results. AB Test segments are currently not supported by this operation. NOTE: This API is limited to being
	// called 30 times in one minute. You will be returned an error if you exceed this threshold.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/getplayersinsegment
	

	return _http_cli->request_append(
		"/Admin/GetPlayersInSegment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerStatisticDefinitions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the configuration information for all player statistics defined in the title, regardless of whether they have
	// a reset interval.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getplayerstatisticdefinitions
	

	return _http_cli->request_append(
		"/Admin/GetPlayerStatisticDefinitions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerStatisticVersions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the information on the available versions of the specified statistic.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getplayerstatisticversions
	

	return _http_cli->request_append(
		"/Admin/GetPlayerStatisticVersions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerTags(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get all tags with a given Namespace (optional) from a player profile.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/getplayertags
	

	return _http_cli->request_append(
		"/Admin/GetPlayerTags",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPolicy(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the requested policy.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/getpolicy
	

	return _http_cli->request_append(
		"/Admin/GetPolicy",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the key-value store of custom publisher settings
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/getpublisherdata
	

	return _http_cli->request_append(
		"/Admin/GetPublisherData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetRandomResultTables(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the random drop table configuration for the title
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/getrandomresulttables
	

	return _http_cli->request_append(
		"/Admin/GetRandomResultTables",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetSegmentExport(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the result of an export started by ExportPlayersInSegment API. If the ExportPlayersInSegment is successful and
	// complete, this API returns the IndexUrl from which the index file can be downloaded. The index file has a list of urls
	// from which the files containing the player profile data can be downloaded. Otherwise, it returns the current 'State' of
	// the export
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/getsegmentexport
	

	return _http_cli->request_append(
		"/Admin/GetSegmentExport",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetSegments(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get detail information of a segment and its associated definition(s) and action(s) for a title.
	// https://docs.microsoft.com/rest/api/playfab/admin/segments/getsegments
	

	return _http_cli->request_append(
		"/Admin/GetSegments",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetStoreItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the set of items defined for the specified store, including all prices defined
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/getstoreitems
	

	return _http_cli->request_append(
		"/Admin/GetStoreItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetTaskInstances(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Query for task instances by task, status, or time range.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/gettaskinstances
	

	return _http_cli->request_append(
		"/Admin/GetTaskInstances",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetTasks(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get definition information on a specified task or all tasks within a title.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/gettasks
	

	return _http_cli->request_append(
		"/Admin/GetTasks",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetTitleData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the key-value store of custom title settings which can be read by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/gettitledata
	

	return _http_cli->request_append(
		"/Admin/GetTitleData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetTitleInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the key-value store of custom title settings which cannot be read by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/gettitleinternaldata
	

	return _http_cli->request_append(
		"/Admin/GetTitleInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserAccountInfo(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the relevant details for a specified user, based upon a match against a supplied unique identifier
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/getuseraccountinfo
	

	return _http_cli->request_append(
		"/Admin/GetUserAccountInfo",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserBans(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets all bans for a user.
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/getuserbans
	

	return _http_cli->request_append(
		"/Admin/GetUserBans",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getuserdata
	

	return _http_cli->request_append(
		"/Admin/GetUserData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getuserinternaldata
	

	return _http_cli->request_append(
		"/Admin/GetUserInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserInventory(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the specified user's current inventory of virtual goods
	// https://docs.microsoft.com/rest/api/playfab/admin/player-item-management/getuserinventory
	

	return _http_cli->request_append(
		"/Admin/GetUserInventory",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the publisher-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getuserpublisherdata
	

	return _http_cli->request_append(
		"/Admin/GetUserPublisherData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserPublisherInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the publisher-specific custom data for the user which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getuserpublisherinternaldata
	

	return _http_cli->request_append(
		"/Admin/GetUserPublisherInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserPublisherReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the publisher-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getuserpublisherreadonlydata
	

	return _http_cli->request_append(
		"/Admin/GetUserPublisherReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/getuserreadonlydata
	

	return _http_cli->request_append(
		"/Admin/GetUserReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GrantItemsToUsers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds the specified items to the specified user inventories
	// https://docs.microsoft.com/rest/api/playfab/admin/player-item-management/grantitemstousers
	

	return _http_cli->request_append(
		"/Admin/GrantItemsToUsers",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int IncrementLimitedEditionItemAvailability(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Increases the global count for the given scarce resource.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-item-management/incrementlimitededitionitemavailability
	

	return _http_cli->request_append(
		"/Admin/IncrementLimitedEditionItemAvailability",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int IncrementPlayerStatisticVersion(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Resets the indicated statistic, removing all player entries for it and backing up the old values.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/incrementplayerstatisticversion
	

	return _http_cli->request_append(
		"/Admin/IncrementPlayerStatisticVersion",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ListOpenIdConnection(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a list of all Open ID Connect providers registered to a title.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/listopenidconnection
	

	return _http_cli->request_append(
		"/Admin/ListOpenIdConnection",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ListVirtualCurrencyTypes(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retuns the list of all defined virtual currencies for the title
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/listvirtualcurrencytypes
	

	return _http_cli->request_append(
		"/Admin/ListVirtualCurrencyTypes",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ModifyServerBuild(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the build details for the specified game server executable
	// https://docs.microsoft.com/rest/api/playfab/admin/custom-server-management/modifyserverbuild
	

	return _http_cli->request_append(
		"/Admin/ModifyServerBuild",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RefundPurchase(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Attempts to process an order refund through the original real money payment provider.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/refundpurchase
	

	return _http_cli->request_append(
		"/Admin/RefundPurchase",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RemovePlayerTag(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Remove a given tag from a player profile. The tag's namespace is automatically generated based on the source of the tag.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/removeplayertag
	

	return _http_cli->request_append(
		"/Admin/RemovePlayerTag",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RemoveVirtualCurrencyTypes(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes one or more virtual currencies from the set defined for the title.
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/removevirtualcurrencytypes
	

	return _http_cli->request_append(
		"/Admin/RemoveVirtualCurrencyTypes",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ResetCharacterStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Completely removes all statistics for the specified character, for the current game
	// https://docs.microsoft.com/rest/api/playfab/admin/characters/resetcharacterstatistics
	

	return _http_cli->request_append(
		"/Admin/ResetCharacterStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ResetPassword(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Reset a player's password for a given title.
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/resetpassword
	

	return _http_cli->request_append(
		"/Admin/ResetPassword",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ResetUserStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Completely removes all statistics for the specified user, for the current game
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/resetuserstatistics
	

	return _http_cli->request_append(
		"/Admin/ResetUserStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ResolvePurchaseDispute(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Attempts to resolve a dispute with the original order's payment provider.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/resolvepurchasedispute
	

	return _http_cli->request_append(
		"/Admin/ResolvePurchaseDispute",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RevokeAllBansForUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Revoke all active bans for a user.
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/revokeallbansforuser
	

	return _http_cli->request_append(
		"/Admin/RevokeAllBansForUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RevokeBans(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Revoke all active bans specified with BanId.
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/revokebans
	

	return _http_cli->request_append(
		"/Admin/RevokeBans",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RevokeInventoryItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Revokes access to an item in a user's inventory
	// https://docs.microsoft.com/rest/api/playfab/admin/player-item-management/revokeinventoryitem
	

	return _http_cli->request_append(
		"/Admin/RevokeInventoryItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RevokeInventoryItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Revokes access for up to 25 items across multiple users and characters.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-item-management/revokeinventoryitems
	

	return _http_cli->request_append(
		"/Admin/RevokeInventoryItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RunTask(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Run a task immediately regardless of its schedule.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/runtask
	

	return _http_cli->request_append(
		"/Admin/RunTask",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SendAccountRecoveryEmail(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Forces an email to be sent to the registered email address for the user's account, with a link allowing the user to
	// change the password.If an account recovery email template ID is provided, an email using the custom email template will
	// be used.
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/sendaccountrecoveryemail
	

	return _http_cli->request_append(
		"/Admin/SendAccountRecoveryEmail",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetCatalogItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates the catalog configuration of all virtual goods for the specified catalog version
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/setcatalogitems
	

	return _http_cli->request_append(
		"/Admin/SetCatalogItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetEventSink(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the sink to which to route PlayStream and Telemetry event data.
	// https://docs.microsoft.com/rest/api/playfab/admin/playstream/seteventsink
	

	return _http_cli->request_append(
		"/Admin/SetEventSink",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetMembershipOverride(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the override expiration for a membership subscription
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/setmembershipoverride
	

	return _http_cli->request_append(
		"/Admin/SetMembershipOverride",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetPlayerSecret(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets or resets the player's secret. Player secrets are used to sign API requests.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/setplayersecret
	

	return _http_cli->request_append(
		"/Admin/SetPlayerSecret",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetPublishedRevision(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the currently published revision of a title Cloud Script
	// https://docs.microsoft.com/rest/api/playfab/admin/server-side-cloud-script/setpublishedrevision
	

	return _http_cli->request_append(
		"/Admin/SetPublishedRevision",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the key-value store of custom publisher settings
	// https://docs.microsoft.com/rest/api/playfab/admin/shared-group-data/setpublisherdata
	

	return _http_cli->request_append(
		"/Admin/SetPublisherData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetStoreItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets all the items in one virtual store
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/setstoreitems
	

	return _http_cli->request_append(
		"/Admin/SetStoreItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetTitleData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates and updates the key-value store of custom title settings which can be read by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/settitledata
	

	return _http_cli->request_append(
		"/Admin/SetTitleData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetTitleDataAndOverrides(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Set and delete key-value pairs in a title data override instance.
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/settitledataandoverrides
	

	return _http_cli->request_append(
		"/Admin/SetTitleDataAndOverrides",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetTitleInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the key-value store of custom title settings which cannot be read by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/settitleinternaldata
	

	return _http_cli->request_append(
		"/Admin/SetTitleInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetupPushNotification(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the Amazon Resource Name (ARN) for iOS and Android push notifications. Documentation on the exact restrictions can
	// be found at: http://docs.aws.amazon.com/sns/latest/api/API_CreatePlatformApplication.html. Currently, Amazon device
	// Messaging is not supported.
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/setuppushnotification
	

	return _http_cli->request_append(
		"/Admin/SetupPushNotification",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SubtractUserVirtualCurrency(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Decrements the specified virtual currency by the stated amount
	// https://docs.microsoft.com/rest/api/playfab/admin/player-item-management/subtractuservirtualcurrency
	

	return _http_cli->request_append(
		"/Admin/SubtractUserVirtualCurrency",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateBans(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates information of a list of existing bans specified with Ban Ids.
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/updatebans
	

	return _http_cli->request_append(
		"/Admin/UpdateBans",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateCatalogItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the catalog configuration for virtual goods in the specified catalog version
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/updatecatalogitems
	

	return _http_cli->request_append(
		"/Admin/UpdateCatalogItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateCloudScript(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a new Cloud Script revision and uploads source code to it. Note that at this time, only one file should be
	// submitted in the revision.
	// https://docs.microsoft.com/rest/api/playfab/admin/server-side-cloud-script/updatecloudscript
	

	return _http_cli->request_append(
		"/Admin/UpdateCloudScript",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateOpenIdConnection(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Modifies data and credentials for an existing relationship between a title and an Open ID Connect provider
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/updateopenidconnection
	

	return _http_cli->request_append(
		"/Admin/UpdateOpenIdConnection",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdatePlayerSharedSecret(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates a existing Player Shared Secret Key. It may take up to 5 minutes for this update to become generally available
	// after this API returns.
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/updateplayersharedsecret
	

	return _http_cli->request_append(
		"/Admin/UpdatePlayerSharedSecret",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdatePlayerStatisticDefinition(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates a player statistic configuration for the title, optionally allowing the developer to specify a reset interval.
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/updateplayerstatisticdefinition
	

	return _http_cli->request_append(
		"/Admin/UpdatePlayerStatisticDefinition",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdatePolicy(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Changes a policy for a title
	// https://docs.microsoft.com/rest/api/playfab/admin/authentication/updatepolicy
	

	return _http_cli->request_append(
		"/Admin/UpdatePolicy",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateRandomResultTables(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the random drop table configuration for the title
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/updaterandomresulttables
	

	return _http_cli->request_append(
		"/Admin/UpdateRandomResultTables",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateSegment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates an existing player segment and its associated definition(s) and action(s) for a title.
	// https://docs.microsoft.com/rest/api/playfab/admin/segments/updatesegment
	

	return _http_cli->request_append(
		"/Admin/UpdateSegment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateStoreItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates an existing virtual item store with new or modified items
	// https://docs.microsoft.com/rest/api/playfab/admin/title-wide-data-management/updatestoreitems
	

	return _http_cli->request_append(
		"/Admin/UpdateStoreItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateTask(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update an existing task.
	// https://docs.microsoft.com/rest/api/playfab/admin/scheduledtask/updatetask
	

	return _http_cli->request_append(
		"/Admin/UpdateTask",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/updateuserdata
	

	return _http_cli->request_append(
		"/Admin/UpdateUserData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/updateuserinternaldata
	

	return _http_cli->request_append(
		"/Admin/UpdateUserInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the publisher-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/updateuserpublisherdata
	

	return _http_cli->request_append(
		"/Admin/UpdateUserPublisherData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserPublisherInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the publisher-specific custom data for the user which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/updateuserpublisherinternaldata
	

	return _http_cli->request_append(
		"/Admin/UpdateUserPublisherInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserPublisherReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the publisher-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/updateuserpublisherreadonlydata
	

	return _http_cli->request_append(
		"/Admin/UpdateUserPublisherReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/admin/player-data-management/updateuserreadonlydata
	

	return _http_cli->request_append(
		"/Admin/UpdateUserReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserTitleDisplayName(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title specific display name for a user
	// https://docs.microsoft.com/rest/api/playfab/admin/account-management/updateusertitledisplayname
	

	return _http_cli->request_append(
		"/Admin/UpdateUserTitleDisplayName",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

} // namespace Admin
