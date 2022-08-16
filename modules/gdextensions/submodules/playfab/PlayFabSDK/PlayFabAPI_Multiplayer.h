/*************************************************************************/
/*  PlayFabAPI_Multiplayer.h                                             */
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

/* Auto-generated at Tue Aug 16 2022 20:53:57 GMT+0000 (Coordinated Universal Time) */

namespace Multiplayer {


int CancelAllMatchmakingTicketsForPlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Cancel all active tickets the player is a member of in a given queue.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/cancelallmatchmakingticketsforplayer
	

	return _http_cli->request_append(
		"/Match/CancelAllMatchmakingTicketsForPlayer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CancelAllServerBackfillTicketsForPlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Cancel all active backfill tickets the player is a member of in a given queue.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/cancelallserverbackfillticketsforplayer
	

	return _http_cli->request_append(
		"/Match/CancelAllServerBackfillTicketsForPlayer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CancelMatchmakingTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Cancel a matchmaking ticket.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/cancelmatchmakingticket
	

	return _http_cli->request_append(
		"/Match/CancelMatchmakingTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CancelServerBackfillTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Cancel a server backfill ticket.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/cancelserverbackfillticket
	

	return _http_cli->request_append(
		"/Match/CancelServerBackfillTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateBuildAlias(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a multiplayer server build alias.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/createbuildalias
	

	return _http_cli->request_append(
		"/MultiplayerServer/CreateBuildAlias",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateBuildWithCustomContainer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a multiplayer server build with a custom container.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/createbuildwithcustomcontainer
	

	return _http_cli->request_append(
		"/MultiplayerServer/CreateBuildWithCustomContainer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateBuildWithManagedContainer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a multiplayer server build with a managed container.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/createbuildwithmanagedcontainer
	

	return _http_cli->request_append(
		"/MultiplayerServer/CreateBuildWithManagedContainer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateBuildWithProcessBasedServer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a multiplayer server build with the server running as a process.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/createbuildwithprocessbasedserver
	

	return _http_cli->request_append(
		"/MultiplayerServer/CreateBuildWithProcessBasedServer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateLobby(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create a lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/createlobby
	

	return _http_cli->request_append(
		"/Lobby/CreateLobby",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateMatchmakingTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create a matchmaking ticket as a client.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/creatematchmakingticket
	

	return _http_cli->request_append(
		"/Match/CreateMatchmakingTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateRemoteUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a remote user to log on to a VM for a multiplayer server build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/createremoteuser
	

	return _http_cli->request_append(
		"/MultiplayerServer/CreateRemoteUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateServerBackfillTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create a backfill matchmaking ticket as a server. A backfill ticket represents an ongoing game. The matchmaking service
	// automatically starts matching the backfill ticket against other matchmaking tickets. Backfill tickets cannot match with
	// other backfill tickets.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/createserverbackfillticket
	

	return _http_cli->request_append(
		"/Match/CreateServerBackfillTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateServerMatchmakingTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create a matchmaking ticket as a server. The matchmaking service automatically starts matching the ticket against other
	// matchmaking tickets.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/createservermatchmakingticket
	

	return _http_cli->request_append(
		"/Match/CreateServerMatchmakingTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateTitleMultiplayerServersQuotaChange(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a request to change a title's multiplayer server quotas.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/createtitlemultiplayerserversquotachange
	

	return _http_cli->request_append(
		"/MultiplayerServer/CreateTitleMultiplayerServersQuotaChange",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteAsset(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a multiplayer server game asset for a title.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/deleteasset
	

	return _http_cli->request_append(
		"/MultiplayerServer/DeleteAsset",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteBuild(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a multiplayer server build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/deletebuild
	

	return _http_cli->request_append(
		"/MultiplayerServer/DeleteBuild",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteBuildAlias(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a multiplayer server build alias.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/deletebuildalias
	

	return _http_cli->request_append(
		"/MultiplayerServer/DeleteBuildAlias",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteBuildRegion(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes a multiplayer server build's region.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/deletebuildregion
	

	return _http_cli->request_append(
		"/MultiplayerServer/DeleteBuildRegion",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteCertificate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a multiplayer server game certificate.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/deletecertificate
	

	return _http_cli->request_append(
		"/MultiplayerServer/DeleteCertificate",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteContainerImageRepository(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a container image repository.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/deletecontainerimagerepository
	

	return _http_cli->request_append(
		"/MultiplayerServer/DeleteContainerImageRepository",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteLobby(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Delete a lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/deletelobby
	

	return _http_cli->request_append(
		"/Lobby/DeleteLobby",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteRemoteUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a remote user to log on to a VM for a multiplayer server build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/deleteremoteuser
	

	return _http_cli->request_append(
		"/MultiplayerServer/DeleteRemoteUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int EnableMultiplayerServersForTitle(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Enables the multiplayer server feature for a title.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/enablemultiplayerserversfortitle
	

	return _http_cli->request_append(
		"/MultiplayerServer/EnableMultiplayerServersForTitle",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int EnablePartiesForTitle(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Enables the parties feature for a title.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/enablepartiesfortitle
	

	return _http_cli->request_append(
		"/Party/EnablePartiesForTitle",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int FindFriendLobbies(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Find lobbies which match certain criteria, and which friends are in.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/findfriendlobbies
	

	return _http_cli->request_append(
		"/Lobby/FindFriendLobbies",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int FindLobbies(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Find all the lobbies that match certain criteria.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/findlobbies
	

	return _http_cli->request_append(
		"/Lobby/FindLobbies",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetAssetDownloadUrl(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets a URL that can be used to download the specified asset. A sample pre-authenticated url -
	// https://sampleStorageAccount.blob.core.windows.net/gameassets/gameserver.zip?sv=2015-04-05&ss=b&srt=sco&sp=rw&st=startDate&se=endDate&spr=https&sig=sampleSig&api-version=2017-07-29
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getassetdownloadurl
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetAssetDownloadUrl",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetAssetUploadUrl(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the URL to upload assets to. A sample pre-authenticated url -
	// https://sampleStorageAccount.blob.core.windows.net/gameassets/gameserver.zip?sv=2015-04-05&ss=b&srt=sco&sp=rw&st=startDate&se=endDate&spr=https&sig=sampleSig&api-version=2017-07-29
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getassetuploadurl
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetAssetUploadUrl",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetBuild(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets a multiplayer server build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getbuild
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetBuild",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetBuildAlias(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets a multiplayer server build alias.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getbuildalias
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetBuildAlias",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetCognitiveServicesLocales(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets supported locales for the cognitive services based on the specified service type.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getcognitiveserviceslocales
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetCognitiveServicesLocales",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetCognitiveServicesToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets a token for the cognitive services based on the specified service type.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getcognitiveservicestoken
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetCognitiveServicesToken",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetContainerRegistryCredentials(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the credentials to the container registry.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getcontainerregistrycredentials
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetContainerRegistryCredentials",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetLobby(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get a lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/getlobby
	

	return _http_cli->request_append(
		"/Lobby/GetLobby",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetMatch(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get a match.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/getmatch
	

	return _http_cli->request_append(
		"/Match/GetMatch",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetMatchmakingTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get a matchmaking ticket by ticket Id.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/getmatchmakingticket
	

	return _http_cli->request_append(
		"/Match/GetMatchmakingTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetMultiplayerServerAndToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets multiplayer server session details and associated user connection tokens for a build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getmultiplayerserverandtoken
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetMultiplayerServerAndToken",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetMultiplayerServerDetails(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets multiplayer server session details for a build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getmultiplayerserverdetails
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetMultiplayerServerDetails",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetMultiplayerServerLogs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets multiplayer server logs after a server has terminated.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getmultiplayerserverlogs
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetMultiplayerServerLogs",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetMultiplayerSessionLogsBySessionId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets multiplayer server logs after a server has terminated.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getmultiplayersessionlogsbysessionid
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetMultiplayerSessionLogsBySessionId",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetQueueStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get the statistics for a queue.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/getqueuestatistics
	

	return _http_cli->request_append(
		"/Match/GetQueueStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetRemoteLoginEndpoint(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets a remote login endpoint to a VM that is hosting a multiplayer server build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/getremoteloginendpoint
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetRemoteLoginEndpoint",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetServerBackfillTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get a matchmaking backfill ticket by ticket Id.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/getserverbackfillticket
	

	return _http_cli->request_append(
		"/Match/GetServerBackfillTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetTitleEnabledForMultiplayerServersStatus(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the status of whether a title is enabled for the multiplayer server feature.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/gettitleenabledformultiplayerserversstatus
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetTitleEnabledForMultiplayerServersStatus",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetTitleMultiplayerServersQuotaChange(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets a title's server quota change request.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/gettitlemultiplayerserversquotachange
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetTitleMultiplayerServersQuotaChange",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetTitleMultiplayerServersQuotas(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the quotas for a title in relation to multiplayer servers.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/gettitlemultiplayerserversquotas
	

	return _http_cli->request_append(
		"/MultiplayerServer/GetTitleMultiplayerServersQuotas",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int InviteToLobby(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Send a notification to invite a player to a lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/invitetolobby
	

	return _http_cli->request_append(
		"/Lobby/InviteToLobby",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int JoinArrangedLobby(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Join an Arranged lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/joinarrangedlobby
	

	return _http_cli->request_append(
		"/Lobby/JoinArrangedLobby",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int JoinLobby(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Join a lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/joinlobby
	

	return _http_cli->request_append(
		"/Lobby/JoinLobby",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int JoinMatchmakingTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Join a matchmaking ticket.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/joinmatchmakingticket
	

	return _http_cli->request_append(
		"/Match/JoinMatchmakingTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int LeaveLobby(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Leave a lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/leavelobby
	

	return _http_cli->request_append(
		"/Lobby/LeaveLobby",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListArchivedMultiplayerServers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists archived multiplayer server sessions for a build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listarchivedmultiplayerservers
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListArchivedMultiplayerServers",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListAssetSummaries(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists multiplayer server game assets for a title.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listassetsummaries
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListAssetSummaries",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListBuildAliases(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists details of all build aliases for a title. Accepts tokens for title and if game client access is enabled, allows
	// game client to request list of builds with player entity token.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listbuildaliases
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListBuildAliases",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListBuildSummariesV2(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists summarized details of all multiplayer server builds for a title. Accepts tokens for title and if game client
	// access is enabled, allows game client to request list of builds with player entity token.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listbuildsummariesv2
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListBuildSummariesV2",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListCertificateSummaries(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists multiplayer server game certificates for a title.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listcertificatesummaries
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListCertificateSummaries",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListContainerImages(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists custom container images for a title.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listcontainerimages
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListContainerImages",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListContainerImageTags(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists the tags for a custom container image.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listcontainerimagetags
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListContainerImageTags",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListMatchmakingTicketsForPlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// List all matchmaking ticket Ids the user is a member of.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/listmatchmakingticketsforplayer
	

	return _http_cli->request_append(
		"/Match/ListMatchmakingTicketsForPlayer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListMultiplayerServers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists multiplayer server sessions for a build.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listmultiplayerservers
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListMultiplayerServers",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListPartyQosServers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists quality of service servers for party.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listpartyqosservers
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListPartyQosServers",
		dict_request,
		user_callback,
		dict_header_extra,
		Array(),
		Array()
	);
}

int ListQosServersForTitle(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists quality of service servers for the title. By default, servers are only returned for regions where a Multiplayer
	// Servers build has been deployed.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listqosserversfortitle
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListQosServersForTitle",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListServerBackfillTicketsForPlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// List all server backfill ticket Ids the user is a member of.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/matchmaking/listserverbackfillticketsforplayer
	

	return _http_cli->request_append(
		"/Match/ListServerBackfillTicketsForPlayer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListTitleMultiplayerServersQuotaChanges(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// List all server quota change requests for a title.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listtitlemultiplayerserversquotachanges
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListTitleMultiplayerServersQuotaChanges",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListVirtualMachineSummaries(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists virtual machines for a title.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/listvirtualmachinesummaries
	

	return _http_cli->request_append(
		"/MultiplayerServer/ListVirtualMachineSummaries",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RemoveMember(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Remove a member from a lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/removemember
	

	return _http_cli->request_append(
		"/Lobby/RemoveMember",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RequestMultiplayerServer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Request a multiplayer server session. Accepts tokens for title and if game client access is enabled, allows game client
	// to request a server with player entity token.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/requestmultiplayerserver
	

	return _http_cli->request_append(
		"/MultiplayerServer/RequestMultiplayerServer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RequestMultiplayerServerAndToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Request a multiplayer server session and associated user connection tokens. Accepts tokens for title and if game client
	// access is enabled, allows game client to request a server with player entity token.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/requestmultiplayerserverandtoken
	

	return _http_cli->request_append(
		"/MultiplayerServer/RequestMultiplayerServerAndToken",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RequestParty(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Request a party session.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/requestparty
	

	return _http_cli->request_append(
		"/Party/RequestParty",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RolloverContainerRegistryCredentials(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Rolls over the credentials to the container registry.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/rollovercontainerregistrycredentials
	

	return _http_cli->request_append(
		"/MultiplayerServer/RolloverContainerRegistryCredentials",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ShutdownMultiplayerServer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Shuts down a multiplayer server session.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/shutdownmultiplayerserver
	

	return _http_cli->request_append(
		"/MultiplayerServer/ShutdownMultiplayerServer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SubscribeToLobbyResource(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Subscribe to lobby resource notifications.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/subscribetolobbyresource
	

	return _http_cli->request_append(
		"/Lobby/SubscribeToLobbyResource",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UnsubscribeFromLobbyResource(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Unsubscribe from lobby notifications.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/unsubscribefromlobbyresource
	

	return _http_cli->request_append(
		"/Lobby/UnsubscribeFromLobbyResource",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UntagContainerImage(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Untags a container image.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/untagcontainerimage
	

	return _http_cli->request_append(
		"/MultiplayerServer/UntagContainerImage",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateBuildAlias(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a multiplayer server build alias.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/updatebuildalias
	

	return _http_cli->request_append(
		"/MultiplayerServer/UpdateBuildAlias",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateBuildName(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates a multiplayer server build's name.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/updatebuildname
	

	return _http_cli->request_append(
		"/MultiplayerServer/UpdateBuildName",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateBuildRegion(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates a multiplayer server build's region. If the region is not yet created, it will be created
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/updatebuildregion
	

	return _http_cli->request_append(
		"/MultiplayerServer/UpdateBuildRegion",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateBuildRegions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates a multiplayer server build's regions.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/updatebuildregions
	

	return _http_cli->request_append(
		"/MultiplayerServer/UpdateBuildRegions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateLobby(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update a lobby.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/lobby/updatelobby
	

	return _http_cli->request_append(
		"/Lobby/UpdateLobby",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UploadCertificate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Uploads a multiplayer server game certificate.
	// https://docs.microsoft.com/rest/api/playfab/multiplayer/multiplayerserver/uploadcertificate
	

	return _http_cli->request_append(
		"/MultiplayerServer/UploadCertificate",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

} // namespace Multiplayer
