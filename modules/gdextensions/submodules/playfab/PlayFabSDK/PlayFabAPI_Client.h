/**************************************************************************/
/*  PlayFabAPI_Client.h                                                   */
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

namespace Client {

bool IsClientLoggedIn() {
	return !PlayFabSettings()._internalSettings.ClientSessionTicket.empty();
}

int AcceptTrade(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Accepts an open trade (one that has not yet been accepted or cancelled), if the locally signed-in player is in the
	// allowed player list for the trade, or it is open to all players. If the call is successful, the offered and accepted
	// items will be swapped between the two players' inventories.
	// https://docs.microsoft.com/rest/api/playfab/client/trading/accepttrade

	return _http_cli->request_append(
			"/Client/AcceptTrade",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int AddFriend(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Adds the PlayFab user, based upon a match against a supplied unique identifier, to the friend list of the local user. At
	// least one of FriendPlayFabId,FriendUsername,FriendEmail, or FriendTitleDisplayName should be initialized.
	// https://docs.microsoft.com/rest/api/playfab/client/friend-list-management/addfriend

	return _http_cli->request_append(
			"/Client/AddFriend",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int AddGenericID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Adds the specified generic service identifier to the player's PlayFab account. This is designed to allow for a PlayFab
	// ID lookup of any arbitrary service identifier a title wants to add. This identifier should never be used as
	// authentication credentials, as the intent is that it is easily accessible by other players.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/addgenericid

	return _http_cli->request_append(
			"/Client/AddGenericID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int AddOrUpdateContactEmail(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Adds or updates a contact email to the player's profile.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/addorupdatecontactemail

	return _http_cli->request_append(
			"/Client/AddOrUpdateContactEmail",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int AddSharedGroupMembers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Adds users to the set of those able to update both the shared data, as well as the set of users in the group. Only users
	// in the group can add new members. Shared Groups are designed for sharing data between a very small number of players,
	// please see our guide: https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/client/shared-group-data/addsharedgroupmembers

	return _http_cli->request_append(
			"/Client/AddSharedGroupMembers",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int AddUsernamePassword(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Adds playfab username/password auth to an existing account created via an anonymous auth method, e.g. automatic device
	// ID login.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/addusernamepassword

	return _http_cli->request_append(
			"/Client/AddUsernamePassword",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int AddUserVirtualCurrency(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Increments the user's balance of the specified virtual currency by the stated amount
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/adduservirtualcurrency

	return _http_cli->request_append(
			"/Client/AddUserVirtualCurrency",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int AndroidDevicePushNotificationRegistration(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Registers the Android device to receive push notifications
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/androiddevicepushnotificationregistration

	return _http_cli->request_append(
			"/Client/AndroidDevicePushNotificationRegistration",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int AttributeInstall(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Attributes an install for advertisment.
	// https://docs.microsoft.com/rest/api/playfab/client/advertising/attributeinstall

	return _http_cli->request_append(
			"/Client/AttributeInstall",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			array(UPD_ATTRIBUTE));
}

int CancelTrade(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Cancels an open trade (one that has not yet been accepted or cancelled). Note that only the player who created the trade
	// can cancel it via this API call, to prevent griefing of the trade system (cancelling trades in order to prevent other
	// players from accepting them, for trades that can be claimed by more than one player).
	// https://docs.microsoft.com/rest/api/playfab/client/trading/canceltrade

	return _http_cli->request_append(
			"/Client/CancelTrade",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ConfirmPurchase(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Confirms with the payment provider that the purchase was approved (if applicable) and adjusts inventory and virtual
	// currency balances as appropriate
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/confirmpurchase

	return _http_cli->request_append(
			"/Client/ConfirmPurchase",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ConsumeItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Consume uses of a consumable item. When all uses are consumed, it will be removed from the player's inventory.
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/consumeitem

	return _http_cli->request_append(
			"/Client/ConsumeItem",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ConsumeMicrosoftStoreEntitlements(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Grants the player's current entitlements from Microsoft Store's Collection API
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/consumemicrosoftstoreentitlements

	return _http_cli->request_append(
			"/Client/ConsumeMicrosoftStoreEntitlements",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ConsumePS5Entitlements(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Checks for any new consumable entitlements. If any are found, they are consumed (if they're consumables) and added as
	// PlayFab items
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/consumeps5entitlements

	return _http_cli->request_append(
			"/Client/ConsumePS5Entitlements",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ConsumePSNEntitlements(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Checks for any new consumable entitlements. If any are found, they are consumed and added as PlayFab items
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/consumepsnentitlements

	return _http_cli->request_append(
			"/Client/ConsumePSNEntitlements",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ConsumeXboxEntitlements(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Grants the player's current entitlements from Xbox Live, consuming all availble items in Xbox and granting them to the
	// player's PlayFab inventory. This call is idempotent and will not grant previously granted items to the player.
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/consumexboxentitlements

	return _http_cli->request_append(
			"/Client/ConsumeXboxEntitlements",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int CreateSharedGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Requests the creation of a shared group object, containing key/value pairs which may be updated by all members of the
	// group. Upon creation, the current user will be the only member of the group. Shared Groups are designed for sharing data
	// between a very small number of players, please see our guide:
	// https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/client/shared-group-data/createsharedgroup

	return _http_cli->request_append(
			"/Client/CreateSharedGroup",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ExecuteCloudScript(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Executes a CloudScript function, with the 'currentPlayerId' set to the PlayFab ID of the authenticated player. The
	// PlayFab ID is the entity ID of the player's master_player_account entity.
	// https://docs.microsoft.com/rest/api/playfab/client/server-side-cloud-script/executecloudscript

	return _http_cli->request_append(
			"/Client/ExecuteCloudScript",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetAccountInfo(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the user's PlayFab account details
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getaccountinfo

	return _http_cli->request_append(
			"/Client/GetAccountInfo",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetAdPlacements(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Returns a list of ad placements and a reward for each
	// https://docs.microsoft.com/rest/api/playfab/client/advertising/getadplacements

	return _http_cli->request_append(
			"/Client/GetAdPlacements",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetAllUsersCharacters(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Lists all of the characters that belong to a specific user. CharacterIds are not globally unique; characterId must be
	// evaluated with the parent PlayFabId to guarantee uniqueness.
	// https://docs.microsoft.com/rest/api/playfab/client/characters/getalluserscharacters

	return _http_cli->request_append(
			"/Client/GetAllUsersCharacters",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetCatalogItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the specified version of the title's catalog of virtual goods, including all defined properties
	// https://docs.microsoft.com/rest/api/playfab/client/title-wide-data-management/getcatalogitems

	return _http_cli->request_append(
			"/Client/GetCatalogItems",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetCharacterData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the title-specific custom data for the character which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/client/character-data/getcharacterdata

	return _http_cli->request_append(
			"/Client/GetCharacterData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetCharacterInventory(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the specified character's current inventory of virtual goods
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/getcharacterinventory

	return _http_cli->request_append(
			"/Client/GetCharacterInventory",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetCharacterLeaderboard(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves a list of ranked characters for the given statistic, starting from the indicated point in the leaderboard
	// https://docs.microsoft.com/rest/api/playfab/client/characters/getcharacterleaderboard

	return _http_cli->request_append(
			"/Client/GetCharacterLeaderboard",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetCharacterReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the title-specific custom data for the character which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/client/character-data/getcharacterreadonlydata

	return _http_cli->request_append(
			"/Client/GetCharacterReadOnlyData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetCharacterStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the details of all title-specific statistics for the user
	// https://docs.microsoft.com/rest/api/playfab/client/characters/getcharacterstatistics

	return _http_cli->request_append(
			"/Client/GetCharacterStatistics",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetContentDownloadUrl(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// This API retrieves a pre-signed URL for accessing a content file for the title. A subsequent HTTP GET to the returned
	// URL will attempt to download the content. A HEAD query to the returned URL will attempt to retrieve the metadata of the
	// content. Note that a successful result does not guarantee the existence of this content - if it has not been uploaded,
	// the query to retrieve the data will fail. See this post for more information:
	// https://community.playfab.com/hc/community/posts/205469488-How-to-upload-files-to-PlayFab-s-Content-Service. Also,
	// please be aware that the Content service is specifically PlayFab's CDN offering, for which standard CDN rates apply.
	// https://docs.microsoft.com/rest/api/playfab/client/content/getcontentdownloadurl

	return _http_cli->request_append(
			"/Client/GetContentDownloadUrl",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetCurrentGames(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Get details about all current running game servers matching the given parameters.
	// https://docs.microsoft.com/rest/api/playfab/client/matchmaking/getcurrentgames

	return _http_cli->request_append(
			"/Client/GetCurrentGames",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetFriendLeaderboard(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves a list of ranked friends of the current player for the given statistic, starting from the indicated point in
	// the leaderboard
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getfriendleaderboard

	return _http_cli->request_append(
			"/Client/GetFriendLeaderboard",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetFriendLeaderboardAroundPlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves a list of ranked friends of the current player for the given statistic, centered on the requested PlayFab
	// user. If PlayFabId is empty or null will return currently logged in user.
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getfriendleaderboardaroundplayer

	return _http_cli->request_append(
			"/Client/GetFriendLeaderboardAroundPlayer",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetFriendsList(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the current friend list for the local user, constrained to users who have PlayFab accounts. Friends from
	// linked accounts (Facebook, Steam) are also included. You may optionally exclude some linked services' friends.
	// https://docs.microsoft.com/rest/api/playfab/client/friend-list-management/getfriendslist

	return _http_cli->request_append(
			"/Client/GetFriendsList",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetGameServerRegions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Get details about the regions hosting game servers matching the given parameters.
	// https://docs.microsoft.com/rest/api/playfab/client/matchmaking/getgameserverregions

	return _http_cli->request_append(
			"/Client/GetGameServerRegions",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetLeaderboard(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves a list of ranked users for the given statistic, starting from the indicated point in the leaderboard
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getleaderboard

	return _http_cli->request_append(
			"/Client/GetLeaderboard",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetLeaderboardAroundCharacter(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves a list of ranked characters for the given statistic, centered on the requested Character ID
	// https://docs.microsoft.com/rest/api/playfab/client/characters/getleaderboardaroundcharacter

	return _http_cli->request_append(
			"/Client/GetLeaderboardAroundCharacter",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetLeaderboardAroundPlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves a list of ranked users for the given statistic, centered on the requested player. If PlayFabId is empty or
	// null will return currently logged in user.
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getleaderboardaroundplayer

	return _http_cli->request_append(
			"/Client/GetLeaderboardAroundPlayer",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetLeaderboardForUserCharacters(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves a list of all of the user's characters for the given statistic.
	// https://docs.microsoft.com/rest/api/playfab/client/characters/getleaderboardforusercharacters

	return _http_cli->request_append(
			"/Client/GetLeaderboardForUserCharacters",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPaymentToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// For payments flows where the provider requires playfab (the fulfiller) to initiate the transaction, but the client
	// completes the rest of the flow. In the Xsolla case, the token returned here will be passed to Xsolla by the client to
	// create a cart. Poll GetPurchase using the returned OrderId once you've completed the payment.
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/getpaymenttoken

	return _http_cli->request_append(
			"/Client/GetPaymentToken",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPhotonAuthenticationToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Gets a Photon custom authentication token that can be used to securely join the player into a Photon room. See
	// https://docs.microsoft.com/gaming/playfab/features/multiplayer/photon/quickstart for more details.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/getphotonauthenticationtoken

	return _http_cli->request_append(
			"/Client/GetPhotonAuthenticationToken",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayerCombinedInfo(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves all of the user's different kinds of info.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayercombinedinfo

	return _http_cli->request_append(
			"/Client/GetPlayerCombinedInfo",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayerProfile(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the player's profile
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayerprofile

	return _http_cli->request_append(
			"/Client/GetPlayerProfile",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayerSegments(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// List all segments that a player currently belongs to at this moment in time.
	// https://docs.microsoft.com/rest/api/playfab/client/playstream/getplayersegments

	return _http_cli->request_append(
			"/Client/GetPlayerSegments",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayerStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the indicated statistics (current version and values for all statistics, if none are specified), for the local
	// player.
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getplayerstatistics

	return _http_cli->request_append(
			"/Client/GetPlayerStatistics",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayerStatisticVersions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the information on the available versions of the specified statistic.
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getplayerstatisticversions

	return _http_cli->request_append(
			"/Client/GetPlayerStatisticVersions",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayerTags(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Get all tags with a given Namespace (optional) from a player profile.
	// https://docs.microsoft.com/rest/api/playfab/client/playstream/getplayertags

	return _http_cli->request_append(
			"/Client/GetPlayerTags",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayerTrades(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Gets all trades the player has either opened or accepted, optionally filtered by trade status.
	// https://docs.microsoft.com/rest/api/playfab/client/trading/getplayertrades

	return _http_cli->request_append(
			"/Client/GetPlayerTrades",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromFacebookIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Facebook identifiers.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromfacebookids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromFacebookIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromFacebookInstantGamesIds(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Facebook Instant Game identifiers.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromfacebookinstantgamesids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromFacebookInstantGamesIds",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromGameCenterIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Game Center identifiers (referenced in the Game Center
	// Programming Guide as the Player Identifier).
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromgamecenterids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromGameCenterIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromGenericIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of generic service identifiers. A generic identifier is the
	// service name plus the service-specific ID for the player, as specified by the title when the generic identifier was
	// added to the player account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromgenericids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromGenericIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromGoogleIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Google identifiers. The Google identifiers are the IDs for
	// the user accounts, available as "id" in the Google+ People API calls.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromgoogleids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromGoogleIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromGooglePlayGamesPlayerIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Google Play Games identifiers. The Google Play Games
	// identifiers are the IDs for the user accounts, available as "playerId" in the Google Play Games Services - Players API
	// calls.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromgoogleplaygamesplayerids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromGooglePlayGamesPlayerIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromKongregateIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Kongregate identifiers. The Kongregate identifiers are the
	// IDs for the user accounts, available as "user_id" from the Kongregate API methods(ex:
	// http://developers.kongregate.com/docs/client/getUserId).
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromkongregateids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromKongregateIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromNintendoServiceAccountIds(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Nintendo Service Account identifiers.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromnintendoserviceaccountids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromNintendoServiceAccountIds",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromNintendoSwitchDeviceIds(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Nintendo Switch Device identifiers.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromnintendoswitchdeviceids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromNintendoSwitchDeviceIds",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromPSNAccountIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of PlayStation :tm: Network identifiers.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfrompsnaccountids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromPSNAccountIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromSteamIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Steam identifiers. The Steam identifiers are the profile
	// IDs for the user accounts, available as SteamId in the Steamworks Community API calls.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromsteamids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromSteamIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromTwitchIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of Twitch identifiers. The Twitch identifiers are the IDs for
	// the user accounts, available as "_id" from the Twitch API methods (ex:
	// https://github.com/justintv/Twitch-API/blob/master/v3_resources/users.md#get-usersuser).
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromtwitchids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromTwitchIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPlayFabIDsFromXboxLiveIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the unique PlayFab identifiers for the given set of XboxLive identifiers.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/getplayfabidsfromxboxliveids

	return _http_cli->request_append(
			"/Client/GetPlayFabIDsFromXboxLiveIDs",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the key-value store of custom publisher settings
	// https://docs.microsoft.com/rest/api/playfab/client/title-wide-data-management/getpublisherdata

	return _http_cli->request_append(
			"/Client/GetPublisherData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetPurchase(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves a purchase along with its current PlayFab status. Returns inventory items from the purchase that are still
	// active.
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/getpurchase

	return _http_cli->request_append(
			"/Client/GetPurchase",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetSharedGroupData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves data stored in a shared group object, as well as the list of members in the group. Non-members of the group
	// may use this to retrieve group data, including membership, but they will not receive data for keys marked as private.
	// Shared Groups are designed for sharing data between a very small number of players, please see our guide:
	// https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/client/shared-group-data/getsharedgroupdata

	return _http_cli->request_append(
			"/Client/GetSharedGroupData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetStoreItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the set of items defined for the specified store, including all prices defined
	// https://docs.microsoft.com/rest/api/playfab/client/title-wide-data-management/getstoreitems

	return _http_cli->request_append(
			"/Client/GetStoreItems",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetTime(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the current server time
	// https://docs.microsoft.com/rest/api/playfab/client/title-wide-data-management/gettime

	return _http_cli->request_append(
			"/Client/GetTime",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetTitleData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the key-value store of custom title settings
	// https://docs.microsoft.com/rest/api/playfab/client/title-wide-data-management/gettitledata

	return _http_cli->request_append(
			"/Client/GetTitleData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetTitleNews(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the title news feed, as configured in the developer portal
	// https://docs.microsoft.com/rest/api/playfab/client/title-wide-data-management/gettitlenews

	return _http_cli->request_append(
			"/Client/GetTitleNews",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetTitlePublicKey(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Returns the title's base 64 encoded RSA CSP blob.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/gettitlepublickey

	return _http_cli->request_append(
			"/Client/GetTitlePublicKey",
			dict_request,
			user_callback,
			dict_header_extra,
			Array(),
			Array());
}

int GetTradeStatus(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Gets the current status of an existing trade.
	// https://docs.microsoft.com/rest/api/playfab/client/trading/gettradestatus

	return _http_cli->request_append(
			"/Client/GetTradeStatus",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetUserData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the title-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getuserdata

	return _http_cli->request_append(
			"/Client/GetUserData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetUserInventory(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the user's current inventory of virtual goods
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/getuserinventory

	return _http_cli->request_append(
			"/Client/GetUserInventory",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetUserPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the publisher-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getuserpublisherdata

	return _http_cli->request_append(
			"/Client/GetUserPublisherData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetUserPublisherReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the publisher-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getuserpublisherreadonlydata

	return _http_cli->request_append(
			"/Client/GetUserPublisherReadOnlyData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GetUserReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves the title-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/getuserreadonlydata

	return _http_cli->request_append(
			"/Client/GetUserReadOnlyData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int GrantCharacterToUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Grants the specified character type to the user. CharacterIds are not globally unique; characterId must be evaluated
	// with the parent PlayFabId to guarantee uniqueness.
	// https://docs.microsoft.com/rest/api/playfab/client/characters/grantcharactertouser

	return _http_cli->request_append(
			"/Client/GrantCharacterToUser",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkAndroidDeviceID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Android device identifier to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkandroiddeviceid

	return _http_cli->request_append(
			"/Client/LinkAndroidDeviceID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkApple(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Apple account associated with the token to the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkapple

	return _http_cli->request_append(
			"/Client/LinkApple",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkCustomID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the custom identifier, generated by the title, to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkcustomid

	return _http_cli->request_append(
			"/Client/LinkCustomID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkFacebookAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Facebook account associated with the provided Facebook access token to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkfacebookaccount

	return _http_cli->request_append(
			"/Client/LinkFacebookAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkFacebookInstantGamesId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Facebook Instant Games Id to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkfacebookinstantgamesid

	return _http_cli->request_append(
			"/Client/LinkFacebookInstantGamesId",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkGameCenterAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Game Center account associated with the provided Game Center ID to the user's PlayFab account. Logging in with
	// a Game Center ID is insecure if you do not include the optional PublicKeyUrl, Salt, Signature, and Timestamp parameters
	// in this request. It is recommended you require these parameters on all Game Center calls by going to the Apple Add-ons
	// page in the PlayFab Game Manager and enabling the 'Require secure authentication only for this app' option.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkgamecenteraccount

	return _http_cli->request_append(
			"/Client/LinkGameCenterAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkGoogleAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the currently signed-in user account to their Google account, using their Google account credentials
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkgoogleaccount

	return _http_cli->request_append(
			"/Client/LinkGoogleAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkGooglePlayGamesServicesAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the currently signed-in user account to their Google Play Games account, using their Google Play Games account
	// credentials
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkgoogleplaygamesservicesaccount

	return _http_cli->request_append(
			"/Client/LinkGooglePlayGamesServicesAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkIOSDeviceID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the vendor-specific iOS device identifier to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkiosdeviceid

	return _http_cli->request_append(
			"/Client/LinkIOSDeviceID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkKongregate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Kongregate identifier to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkkongregate

	return _http_cli->request_append(
			"/Client/LinkKongregate",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkNintendoServiceAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Nintendo account associated with the token to the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linknintendoserviceaccount

	return _http_cli->request_append(
			"/Client/LinkNintendoServiceAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkNintendoSwitchDeviceId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the NintendoSwitchDeviceId to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linknintendoswitchdeviceid

	return _http_cli->request_append(
			"/Client/LinkNintendoSwitchDeviceId",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkOpenIdConnect(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links an OpenID Connect account to a user's PlayFab account, based on an existing relationship between a title and an
	// Open ID Connect provider and the OpenId Connect JWT from that provider.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkopenidconnect

	return _http_cli->request_append(
			"/Client/LinkOpenIdConnect",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkPSNAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the PlayStation :tm: Network account associated with the provided access code to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkpsnaccount

	return _http_cli->request_append(
			"/Client/LinkPSNAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkSteamAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Steam account associated with the provided Steam authentication ticket to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linksteamaccount

	return _http_cli->request_append(
			"/Client/LinkSteamAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkTwitch(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Twitch account associated with the token to the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linktwitch

	return _http_cli->request_append(
			"/Client/LinkTwitch",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LinkXboxAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Links the Xbox Live account associated with the provided access code to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/linkxboxaccount

	return _http_cli->request_append(
			"/Client/LinkXboxAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int LoginWithAndroidDeviceID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using the Android device identifier, returning a session identifier that can subsequently be used for
	// API calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithandroiddeviceid

	return _http_cli->request_append(
			"/Client/LoginWithAndroidDeviceID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithApple(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs in the user with a Sign in with Apple identity token.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithapple

	return _http_cli->request_append(
			"/Client/LoginWithApple",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithCustomID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a custom unique identifier generated by the title, returning a session identifier that can
	// subsequently be used for API calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithcustomid

	return _http_cli->request_append(
			"/Client/LoginWithCustomID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithEmailAddress(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user into the PlayFab account, returning a session identifier that can subsequently be used for API calls
	// which require an authenticated user. Unlike most other login API calls, LoginWithEmailAddress does not permit the
	// creation of new accounts via the CreateAccountFlag. Email addresses may be used to create accounts via
	// RegisterPlayFabUser.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithemailaddress

	return _http_cli->request_append(
			"/Client/LoginWithEmailAddress",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithFacebook(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a Facebook access token, returning a session identifier that can subsequently be used for API
	// calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithfacebook

	return _http_cli->request_append(
			"/Client/LoginWithFacebook",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithFacebookInstantGamesId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a Facebook Instant Games ID, returning a session identifier that can subsequently be used for
	// API calls which require an authenticated user. Requires Facebook Instant Games to be configured.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithfacebookinstantgamesid

	return _http_cli->request_append(
			"/Client/LoginWithFacebookInstantGamesId",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithGameCenter(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using an iOS Game Center player identifier, returning a session identifier that can subsequently be
	// used for API calls which require an authenticated user. Logging in with a Game Center ID is insecure if you do not
	// include the optional PublicKeyUrl, Salt, Signature, and Timestamp parameters in this request. It is recommended you
	// require these parameters on all Game Center calls by going to the Apple Add-ons page in the PlayFab Game Manager and
	// enabling the 'Require secure authentication only for this app' option.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithgamecenter

	return _http_cli->request_append(
			"/Client/LoginWithGameCenter",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithGoogleAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using their Google account credentials
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithgoogleaccount

	return _http_cli->request_append(
			"/Client/LoginWithGoogleAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithGooglePlayGamesServices(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using their Google Play Games account credentials
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithgoogleplaygamesservices

	return _http_cli->request_append(
			"/Client/LoginWithGooglePlayGamesServices",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithIOSDeviceID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using the vendor-specific iOS device identifier, returning a session identifier that can subsequently
	// be used for API calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithiosdeviceid

	return _http_cli->request_append(
			"/Client/LoginWithIOSDeviceID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithKongregate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a Kongregate player account.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithkongregate

	return _http_cli->request_append(
			"/Client/LoginWithKongregate",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithNintendoServiceAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs in the user with a Nintendo service account token.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithnintendoserviceaccount

	return _http_cli->request_append(
			"/Client/LoginWithNintendoServiceAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithNintendoSwitchDeviceId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a Nintendo Switch Device ID, returning a session identifier that can subsequently be used for
	// API calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithnintendoswitchdeviceid

	return _http_cli->request_append(
			"/Client/LoginWithNintendoSwitchDeviceId",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithOpenIdConnect(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Logs in a user with an Open ID Connect JWT created by an existing relationship between a title and an Open ID Connect
	// provider.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithopenidconnect

	return _http_cli->request_append(
			"/Client/LoginWithOpenIdConnect",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithPlayFab(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user into the PlayFab account, returning a session identifier that can subsequently be used for API calls
	// which require an authenticated user. Unlike most other login API calls, LoginWithPlayFab does not permit the creation of
	// new accounts via the CreateAccountFlag. Username/Password credentials may be used to create accounts via
	// RegisterPlayFabUser, or added to existing accounts using AddUsernamePassword.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithplayfab

	return _http_cli->request_append(
			"/Client/LoginWithPlayFab",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithPSN(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a PlayStation :tm: Network authentication code, returning a session identifier that can
	// subsequently be used for API calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithpsn

	return _http_cli->request_append(
			"/Client/LoginWithPSN",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithSteam(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a Steam authentication ticket, returning a session identifier that can subsequently be used for
	// API calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithsteam

	return _http_cli->request_append(
			"/Client/LoginWithSteam",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithTwitch(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a Twitch access token.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithtwitch

	return _http_cli->request_append(
			"/Client/LoginWithTwitch",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int LoginWithXbox(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Signs the user in using a Xbox Live Token, returning a session identifier that can subsequently be used for API calls
	// which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/loginwithxbox

	return _http_cli->request_append(
			"/Client/LoginWithXbox",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, UPD_ENTITY_TOKEN, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int Matchmake(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Attempts to locate a game session matching the given parameters. If the goal is to match the player into a specific
	// active session, only the LobbyId is required. Otherwise, the BuildVersion, GameMode, and Region are all required
	// parameters. Note that parameters specified in the search are required (they are not weighting factors). If a slot is
	// found in a server instance matching the parameters, the slot will be assigned to that player, removing it from the
	// availabe set. In that case, the information on the game session will be returned, otherwise the Status returned will be
	// GameNotFound.
	// https://docs.microsoft.com/rest/api/playfab/client/matchmaking/matchmake

	return _http_cli->request_append(
			"/Client/Matchmake",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int OpenTrade(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Opens a new outstanding trade. Note that a given item instance may only be in one open trade at a time.
	// https://docs.microsoft.com/rest/api/playfab/client/trading/opentrade

	return _http_cli->request_append(
			"/Client/OpenTrade",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int PayForPurchase(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Selects a payment option for purchase order created via StartPurchase
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/payforpurchase

	return _http_cli->request_append(
			"/Client/PayForPurchase",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int PurchaseItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Buys a single item with virtual currency. You must specify both the virtual currency to use to purchase, as well as what
	// the client believes the price to be. This lets the server fail the purchase if the price has changed.
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/purchaseitem

	return _http_cli->request_append(
			"/Client/PurchaseItem",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RedeemCoupon(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Adds the virtual goods associated with the coupon to the user's inventory. Coupons can be generated via the
	// Economy->Catalogs tab in the PlayFab Game Manager.
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/redeemcoupon

	return _http_cli->request_append(
			"/Client/RedeemCoupon",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RefreshPSNAuthToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Uses the supplied OAuth code to refresh the internally cached player PSN :tm: auth token
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/refreshpsnauthtoken

	return _http_cli->request_append(
			"/Client/RefreshPSNAuthToken",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RegisterForIOSPushNotification(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Registers the iOS device to receive push notifications
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/registerforiospushnotification

	return _http_cli->request_append(
			"/Client/RegisterForIOSPushNotification",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RegisterPlayFabUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Registers a new Playfab user account, returning a session identifier that can subsequently be used for API calls which
	// require an authenticated user. You must supply either a username or an email address.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/registerplayfabuser

	return _http_cli->request_append(
			"/Client/RegisterPlayFabUser",
			dict_request,
			user_callback,
			dict_header_extra,
			array(USE_TITLE_ID),
			array(UPD_SESSION_TICKET, REQ_MULTI_STEP_CLIENT_LOGIN));
}

int RemoveContactEmail(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Removes a contact email from the player's profile.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/removecontactemail

	return _http_cli->request_append(
			"/Client/RemoveContactEmail",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RemoveFriend(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Removes a specified user from the friend list of the local user
	// https://docs.microsoft.com/rest/api/playfab/client/friend-list-management/removefriend

	return _http_cli->request_append(
			"/Client/RemoveFriend",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RemoveGenericID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Removes the specified generic service identifier from the player's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/removegenericid

	return _http_cli->request_append(
			"/Client/RemoveGenericID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RemoveSharedGroupMembers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Removes users from the set of those able to update the shared data and the set of users in the group. Only users in the
	// group can remove members. If as a result of the call, zero users remain with access, the group and its associated data
	// will be deleted. Shared Groups are designed for sharing data between a very small number of players, please see our
	// guide: https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/client/shared-group-data/removesharedgroupmembers

	return _http_cli->request_append(
			"/Client/RemoveSharedGroupMembers",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ReportAdActivity(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Report player's ad activity
	// https://docs.microsoft.com/rest/api/playfab/client/advertising/reportadactivity

	return _http_cli->request_append(
			"/Client/ReportAdActivity",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ReportDeviceInfo(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Write a PlayStream event to describe the provided player device information. This API method is not designed to be
	// called directly by developers. Each PlayFab client SDK will eventually report this information automatically.
	// https://docs.microsoft.com/rest/api/playfab/client/analytics/reportdeviceinfo

	return _http_cli->request_append(
			"/Client/ReportDeviceInfo",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ReportPlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Submit a report for another player (due to bad bahavior, etc.), so that customer service representatives for the title
	// can take action concerning potentially toxic players.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/reportplayer

	return _http_cli->request_append(
			"/Client/ReportPlayer",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RestoreIOSPurchases(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Restores all in-app purchases based on the given restore receipt
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/restoreiospurchases

	return _http_cli->request_append(
			"/Client/RestoreIOSPurchases",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int RewardAdActivity(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Reward player's ad activity
	// https://docs.microsoft.com/rest/api/playfab/client/advertising/rewardadactivity

	return _http_cli->request_append(
			"/Client/RewardAdActivity",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int SendAccountRecoveryEmail(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Forces an email to be sent to the registered email address for the user's account, with a link allowing the user to
	// change the password.If an account recovery email template ID is provided, an email using the custom email template will
	// be used.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/sendaccountrecoveryemail

	return _http_cli->request_append(
			"/Client/SendAccountRecoveryEmail",
			dict_request,
			user_callback,
			dict_header_extra,
			Array(),
			Array());
}

int SetFriendTags(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Updates the tag list for a specified user in the friend list of the local user
	// https://docs.microsoft.com/rest/api/playfab/client/friend-list-management/setfriendtags

	return _http_cli->request_append(
			"/Client/SetFriendTags",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int SetPlayerSecret(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Sets the player's secret if it is not already set. Player secrets are used to sign API requests. To reset a player's
	// secret use the Admin or Server API method SetPlayerSecret.
	// https://docs.microsoft.com/rest/api/playfab/client/authentication/setplayersecret

	return _http_cli->request_append(
			"/Client/SetPlayerSecret",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int StartPurchase(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Creates an order for a list of items from the title catalog
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/startpurchase

	return _http_cli->request_append(
			"/Client/StartPurchase",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int SubtractUserVirtualCurrency(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Decrements the user's balance of the specified virtual currency by the stated amount. It is possible to make a VC
	// balance negative with this API.
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/subtractuservirtualcurrency

	return _http_cli->request_append(
			"/Client/SubtractUserVirtualCurrency",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkAndroidDeviceID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Android device identifier from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkandroiddeviceid

	return _http_cli->request_append(
			"/Client/UnlinkAndroidDeviceID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkApple(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Apple account from the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkapple

	return _http_cli->request_append(
			"/Client/UnlinkApple",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkCustomID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related custom identifier from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkcustomid

	return _http_cli->request_append(
			"/Client/UnlinkCustomID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkFacebookAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Facebook account from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkfacebookaccount

	return _http_cli->request_append(
			"/Client/UnlinkFacebookAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkFacebookInstantGamesId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Facebook Instant Game Ids from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkfacebookinstantgamesid

	return _http_cli->request_append(
			"/Client/UnlinkFacebookInstantGamesId",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkGameCenterAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Game Center account from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkgamecenteraccount

	return _http_cli->request_append(
			"/Client/UnlinkGameCenterAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkGoogleAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Google account from the user's PlayFab account
	// (https://developers.google.com/android/reference/com/google/android/gms/auth/GoogleAuthUtil#public-methods).
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkgoogleaccount

	return _http_cli->request_append(
			"/Client/UnlinkGoogleAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkGooglePlayGamesServicesAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Google Play Games account from the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkgoogleplaygamesservicesaccount

	return _http_cli->request_append(
			"/Client/UnlinkGooglePlayGamesServicesAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkIOSDeviceID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related iOS device identifier from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkiosdeviceid

	return _http_cli->request_append(
			"/Client/UnlinkIOSDeviceID",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkKongregate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Kongregate identifier from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkkongregate

	return _http_cli->request_append(
			"/Client/UnlinkKongregate",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkNintendoServiceAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Nintendo account from the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinknintendoserviceaccount

	return _http_cli->request_append(
			"/Client/UnlinkNintendoServiceAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkNintendoSwitchDeviceId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related NintendoSwitchDeviceId from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinknintendoswitchdeviceid

	return _http_cli->request_append(
			"/Client/UnlinkNintendoSwitchDeviceId",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkOpenIdConnect(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks an OpenID Connect account from a user's PlayFab account, based on the connection ID of an existing relationship
	// between a title and an Open ID Connect provider.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkopenidconnect

	return _http_cli->request_append(
			"/Client/UnlinkOpenIdConnect",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkPSNAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related PSN :tm: account from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkpsnaccount

	return _http_cli->request_append(
			"/Client/UnlinkPSNAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkSteamAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Steam account from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinksteamaccount

	return _http_cli->request_append(
			"/Client/UnlinkSteamAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkTwitch(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Twitch account from the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinktwitch

	return _http_cli->request_append(
			"/Client/UnlinkTwitch",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlinkXboxAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unlinks the related Xbox Live account from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/unlinkxboxaccount

	return _http_cli->request_append(
			"/Client/UnlinkXboxAccount",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlockContainerInstance(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Opens the specified container, with the specified key (when required), and returns the contents of the opened container.
	// If the container (and key when relevant) are consumable (RemainingUses > 0), their RemainingUses will be decremented,
	// consistent with the operation of ConsumeItem.
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/unlockcontainerinstance

	return _http_cli->request_append(
			"/Client/UnlockContainerInstance",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UnlockContainerItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Searches target inventory for an ItemInstance matching the given CatalogItemId, if necessary unlocks it using an
	// appropriate key, and returns the contents of the opened container. If the container (and key when relevant) are
	// consumable (RemainingUses > 0), their RemainingUses will be decremented, consistent with the operation of ConsumeItem.
	// https://docs.microsoft.com/rest/api/playfab/client/player-item-management/unlockcontaineritem

	return _http_cli->request_append(
			"/Client/UnlockContainerItem",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UpdateAvatarUrl(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Update the avatar URL of the player
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/updateavatarurl

	return _http_cli->request_append(
			"/Client/UpdateAvatarUrl",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UpdateCharacterData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Creates and updates the title-specific custom data for the user's character which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/client/character-data/updatecharacterdata

	return _http_cli->request_append(
			"/Client/UpdateCharacterData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UpdateCharacterStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Updates the values of the specified title-specific statistics for the specific character. By default, clients are not
	// permitted to update statistics. Developers may override this setting in the Game Manager > Settings > API Features.
	// https://docs.microsoft.com/rest/api/playfab/client/characters/updatecharacterstatistics

	return _http_cli->request_append(
			"/Client/UpdateCharacterStatistics",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UpdatePlayerStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Updates the values of the specified title-specific statistics for the user. By default, clients are not permitted to
	// update statistics. Developers may override this setting in the Game Manager > Settings > API Features.
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/updateplayerstatistics

	return _http_cli->request_append(
			"/Client/UpdatePlayerStatistics",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UpdateSharedGroupData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Adds, updates, and removes data keys for a shared group object. If the permission is set to Public, all fields updated
	// or added in this call will be readable by users not in the group. By default, data permissions are set to Private.
	// Regardless of the permission setting, only members of the group can update the data. Shared Groups are designed for
	// sharing data between a very small number of players, please see our guide:
	// https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/client/shared-group-data/updatesharedgroupdata

	return _http_cli->request_append(
			"/Client/UpdateSharedGroupData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UpdateUserData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Creates and updates the title-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/updateuserdata

	return _http_cli->request_append(
			"/Client/UpdateUserData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UpdateUserPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Creates and updates the publisher-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/client/player-data-management/updateuserpublisherdata

	return _http_cli->request_append(
			"/Client/UpdateUserPublisherData",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int UpdateUserTitleDisplayName(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Updates the title specific display name for the user
	// https://docs.microsoft.com/rest/api/playfab/client/account-management/updateusertitledisplayname

	return _http_cli->request_append(
			"/Client/UpdateUserTitleDisplayName",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ValidateAmazonIAPReceipt(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Validates with Amazon that the receipt for an Amazon App Store in-app purchase is valid and that it matches the
	// purchased catalog item
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/validateamazoniapreceipt

	return _http_cli->request_append(
			"/Client/ValidateAmazonIAPReceipt",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ValidateGooglePlayPurchase(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Validates a Google Play purchase and gives the corresponding item to the player.
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/validategoogleplaypurchase

	return _http_cli->request_append(
			"/Client/ValidateGooglePlayPurchase",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ValidateIOSReceipt(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Validates with the Apple store that the receipt for an iOS in-app purchase is valid and that it matches the purchased
	// catalog item
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/validateiosreceipt

	return _http_cli->request_append(
			"/Client/ValidateIOSReceipt",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int ValidateWindowsStoreReceipt(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Validates with Windows that the receipt for an Windows App Store in-app purchase is valid and that it matches the
	// purchased catalog item
	// https://docs.microsoft.com/rest/api/playfab/client/platform-specific-methods/validatewindowsstorereceipt

	return _http_cli->request_append(
			"/Client/ValidateWindowsStoreReceipt",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int WriteCharacterEvent(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Writes a character-based event into PlayStream.
	// https://docs.microsoft.com/rest/api/playfab/client/analytics/writecharacterevent

	return _http_cli->request_append(
			"/Client/WriteCharacterEvent",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int WritePlayerEvent(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Writes a player-based event into PlayStream.
	// https://docs.microsoft.com/rest/api/playfab/client/analytics/writeplayerevent

	return _http_cli->request_append(
			"/Client/WritePlayerEvent",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

int WriteTitleEvent(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Writes a title-based event into PlayStream.
	// https://docs.microsoft.com/rest/api/playfab/client/analytics/writetitleevent

	return _http_cli->request_append(
			"/Client/WriteTitleEvent",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_SESSION_TICKET, USE_AUTH_AUTHORIZATION),
			Array());
}

} // namespace Client
