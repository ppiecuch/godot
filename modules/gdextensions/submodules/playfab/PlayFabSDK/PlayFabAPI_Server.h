/*************************************************************************/
/*  PlayFabAPI_Server.h                                                  */
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

/* Auto-generated at Tue Aug 16 2022 20:53:56 GMT+0000 (Coordinated Universal Time) */

namespace Server {


int AddCharacterVirtualCurrency(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Increments the character's balance of the specified virtual currency by the stated amount
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/addcharactervirtualcurrency
	

	return _http_cli->request_append(
		"/Server/AddCharacterVirtualCurrency",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddFriend(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds the Friend user to the friendlist of the user with PlayFabId. At least one of
	// FriendPlayFabId,FriendUsername,FriendEmail, or FriendTitleDisplayName should be initialized.
	// https://docs.microsoft.com/rest/api/playfab/server/friend-list-management/addfriend
	

	return _http_cli->request_append(
		"/Server/AddFriend",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddGenericID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds the specified generic service identifier to the player's PlayFab account. This is designed to allow for a PlayFab
	// ID lookup of any arbitrary service identifier a title wants to add. This identifier should never be used as
	// authentication credentials, as the intent is that it is easily accessible by other players.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/addgenericid
	

	return _http_cli->request_append(
		"/Server/AddGenericID",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddPlayerTag(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds a given tag to a player profile. The tag's namespace is automatically generated based on the source of the tag.
	// https://docs.microsoft.com/rest/api/playfab/server/playstream/addplayertag
	

	return _http_cli->request_append(
		"/Server/AddPlayerTag",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddSharedGroupMembers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds users to the set of those able to update both the shared data, as well as the set of users in the group. Only users
	// in the group (and the server) can add new members. Shared Groups are designed for sharing data between a very small
	// number of players, please see our guide:
	// https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/server/shared-group-data/addsharedgroupmembers
	

	return _http_cli->request_append(
		"/Server/AddSharedGroupMembers",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AddUserVirtualCurrency(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Increments the user's balance of the specified virtual currency by the stated amount
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/adduservirtualcurrency
	

	return _http_cli->request_append(
		"/Server/AddUserVirtualCurrency",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AuthenticateSessionTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Validated a client's session ticket, and if successful, returns details for that user
	// https://docs.microsoft.com/rest/api/playfab/server/authentication/authenticatesessionticket
	

	return _http_cli->request_append(
		"/Server/AuthenticateSessionTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int AwardSteamAchievement(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Awards the specified users the specified Steam achievements
	// https://docs.microsoft.com/rest/api/playfab/server/platform-specific-methods/awardsteamachievement
	

	return _http_cli->request_append(
		"/Server/AwardSteamAchievement",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int BanUsers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Bans users by PlayFab ID with optional IP address, or MAC address for the provided game.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/banusers
	

	return _http_cli->request_append(
		"/Server/BanUsers",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ConsumeItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Consume uses of a consumable item. When all uses are consumed, it will be removed from the player's inventory.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/consumeitem
	

	return _http_cli->request_append(
		"/Server/ConsumeItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int CreateSharedGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Requests the creation of a shared group object, containing key/value pairs which may be updated by all members of the
	// group. When created by a server, the group will initially have no members. Shared Groups are designed for sharing data
	// between a very small number of players, please see our guide:
	// https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/server/shared-group-data/createsharedgroup
	

	return _http_cli->request_append(
		"/Server/CreateSharedGroup",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteCharacterFromUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes the specific character ID from the specified user.
	// https://docs.microsoft.com/rest/api/playfab/server/characters/deletecharacterfromuser
	

	return _http_cli->request_append(
		"/Server/DeleteCharacterFromUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeletePlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes a user's player account from a title and deletes all associated data
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/deleteplayer
	

	return _http_cli->request_append(
		"/Server/DeletePlayer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeletePushNotificationTemplate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes push notification template for title
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/deletepushnotificationtemplate
	

	return _http_cli->request_append(
		"/Server/DeletePushNotificationTemplate",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeleteSharedGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes a shared group, freeing up the shared group ID to be reused for a new group. Shared Groups are designed for
	// sharing data between a very small number of players, please see our guide:
	// https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/server/shared-group-data/deletesharedgroup
	

	return _http_cli->request_append(
		"/Server/DeleteSharedGroup",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int DeregisterGame(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Inform the matchmaker that a Game Server Instance is removed.
	// https://docs.microsoft.com/rest/api/playfab/server/matchmaking/deregistergame
	

	return _http_cli->request_append(
		"/Server/DeregisterGame",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int EvaluateRandomResultTable(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Returns the result of an evaluation of a Random Result Table - the ItemId from the game Catalog which would have been
	// added to the player inventory, if the Random Result Table were added via a Bundle or a call to UnlockContainer.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/evaluaterandomresulttable
	

	return _http_cli->request_append(
		"/Server/EvaluateRandomResultTable",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ExecuteCloudScript(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Executes a CloudScript function, with the 'currentPlayerId' set to the PlayFab ID of the authenticated player. The
	// PlayFab ID is the entity ID of the player's master_player_account entity.
	// https://docs.microsoft.com/rest/api/playfab/server/server-side-cloud-script/executecloudscript
	

	return _http_cli->request_append(
		"/Server/ExecuteCloudScript",
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
	// https://docs.microsoft.com/rest/api/playfab/server/playstream/getallsegments
	

	return _http_cli->request_append(
		"/Server/GetAllSegments",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetAllUsersCharacters(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists all of the characters that belong to a specific user. CharacterIds are not globally unique; characterId must be
	// evaluated with the parent PlayFabId to guarantee uniqueness.
	// https://docs.microsoft.com/rest/api/playfab/server/characters/getalluserscharacters
	

	return _http_cli->request_append(
		"/Server/GetAllUsersCharacters",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCatalogItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the specified version of the title's catalog of virtual goods, including all defined properties
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/getcatalogitems
	

	return _http_cli->request_append(
		"/Server/GetCatalogItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCharacterData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/server/character-data/getcharacterdata
	

	return _http_cli->request_append(
		"/Server/GetCharacterData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCharacterInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user's character which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/server/character-data/getcharacterinternaldata
	

	return _http_cli->request_append(
		"/Server/GetCharacterInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCharacterInventory(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the specified character's current inventory of virtual goods
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/getcharacterinventory
	

	return _http_cli->request_append(
		"/Server/GetCharacterInventory",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCharacterLeaderboard(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a list of ranked characters for the given statistic, starting from the indicated point in the leaderboard
	// https://docs.microsoft.com/rest/api/playfab/server/characters/getcharacterleaderboard
	

	return _http_cli->request_append(
		"/Server/GetCharacterLeaderboard",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCharacterReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user's character which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/server/character-data/getcharacterreadonlydata
	

	return _http_cli->request_append(
		"/Server/GetCharacterReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetCharacterStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the details of all title-specific statistics for the specific character
	// https://docs.microsoft.com/rest/api/playfab/server/characters/getcharacterstatistics
	

	return _http_cli->request_append(
		"/Server/GetCharacterStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetContentDownloadUrl(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// This API retrieves a pre-signed URL for accessing a content file for the title. A subsequent HTTP GET to the returned
	// URL will attempt to download the content. A HEAD query to the returned URL will attempt to retrieve the metadata of the
	// content. Note that a successful result does not guarantee the existence of this content - if it has not been uploaded,
	// the query to retrieve the data will fail. See this post for more information:
	// https://community.playfab.com/hc/community/posts/205469488-How-to-upload-files-to-PlayFab-s-Content-Service. Also,
	// please be aware that the Content service is specifically PlayFab's CDN offering, for which standard CDN rates apply.
	// https://docs.microsoft.com/rest/api/playfab/server/content/getcontentdownloadurl
	

	return _http_cli->request_append(
		"/Server/GetContentDownloadUrl",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetFriendLeaderboard(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a list of ranked friends of the given player for the given statistic, starting from the indicated point in the
	// leaderboard
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getfriendleaderboard
	

	return _http_cli->request_append(
		"/Server/GetFriendLeaderboard",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetFriendsList(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the current friends for the user with PlayFabId, constrained to users who have PlayFab accounts. Friends from
	// linked accounts (Facebook, Steam) are also included. You may optionally exclude some linked services' friends.
	// https://docs.microsoft.com/rest/api/playfab/server/friend-list-management/getfriendslist
	

	return _http_cli->request_append(
		"/Server/GetFriendsList",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetLeaderboard(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a list of ranked users for the given statistic, starting from the indicated point in the leaderboard
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getleaderboard
	

	return _http_cli->request_append(
		"/Server/GetLeaderboard",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetLeaderboardAroundCharacter(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a list of ranked characters for the given statistic, centered on the requested user
	// https://docs.microsoft.com/rest/api/playfab/server/characters/getleaderboardaroundcharacter
	

	return _http_cli->request_append(
		"/Server/GetLeaderboardAroundCharacter",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetLeaderboardAroundUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a list of ranked users for the given statistic, centered on the currently signed-in user
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getleaderboardarounduser
	

	return _http_cli->request_append(
		"/Server/GetLeaderboardAroundUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetLeaderboardForUserCharacters(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a list of all of the user's characters for the given statistic.
	// https://docs.microsoft.com/rest/api/playfab/server/characters/getleaderboardforusercharacters
	

	return _http_cli->request_append(
		"/Server/GetLeaderboardForUserCharacters",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerCombinedInfo(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Returns whatever info is requested in the response for the user. Note that PII (like email address, facebook id) may be
	// returned. All parameters default to false.
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getplayercombinedinfo
	

	return _http_cli->request_append(
		"/Server/GetPlayerCombinedInfo",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerProfile(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the player's profile
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayerprofile
	

	return _http_cli->request_append(
		"/Server/GetPlayerProfile",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerSegments(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// List all segments that a player currently belongs to at this moment in time.
	// https://docs.microsoft.com/rest/api/playfab/server/playstream/getplayersegments
	

	return _http_cli->request_append(
		"/Server/GetPlayerSegments",
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
	// https://docs.microsoft.com/rest/api/playfab/server/playstream/getplayersinsegment
	

	return _http_cli->request_append(
		"/Server/GetPlayersInSegment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the current version and values for the indicated statistics, for the local player.
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getplayerstatistics
	

	return _http_cli->request_append(
		"/Server/GetPlayerStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerStatisticVersions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the information on the available versions of the specified statistic.
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getplayerstatisticversions
	

	return _http_cli->request_append(
		"/Server/GetPlayerStatisticVersions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayerTags(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get all tags with a given Namespace (optional) from a player profile.
	// https://docs.microsoft.com/rest/api/playfab/server/playstream/getplayertags
	

	return _http_cli->request_append(
		"/Server/GetPlayerTags",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromFacebookIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of Facebook identifiers.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfromfacebookids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromFacebookIDs",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromFacebookInstantGamesIds(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of Facebook Instant Games identifiers.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfromfacebookinstantgamesids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromFacebookInstantGamesIds",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromGenericIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of generic service identifiers. A generic identifier is the
	// service name plus the service-specific ID for the player, as specified by the title when the generic identifier was
	// added to the player account.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfromgenericids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromGenericIDs",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromNintendoServiceAccountIds(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of Nintendo Service Account identifiers.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfromnintendoserviceaccountids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromNintendoServiceAccountIds",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromNintendoSwitchDeviceIds(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of Nintendo Switch Device identifiers.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfromnintendoswitchdeviceids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromNintendoSwitchDeviceIds",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromPSNAccountIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of PlayStation :tm: Network identifiers.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfrompsnaccountids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromPSNAccountIDs",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromSteamIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of Steam identifiers. The Steam identifiers are the profile
	// IDs for the user accounts, available as SteamId in the Steamworks Community API calls.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfromsteamids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromSteamIDs",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromTwitchIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of Twitch identifiers. The Twitch identifiers are the IDs for
	// the user accounts, available as "_id" from the Twitch API methods (ex:
	// https://github.com/justintv/Twitch-API/blob/master/v3_resources/users.md#get-usersuser).
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfromtwitchids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromTwitchIDs",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPlayFabIDsFromXboxLiveIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the unique PlayFab identifiers for the given set of XboxLive identifiers.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getplayfabidsfromxboxliveids
	

	return _http_cli->request_append(
		"/Server/GetPlayFabIDsFromXboxLiveIDs",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the key-value store of custom publisher settings
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/getpublisherdata
	

	return _http_cli->request_append(
		"/Server/GetPublisherData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetRandomResultTables(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the configuration information for the specified random results tables for the title, including all ItemId
	// values and weights
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/getrandomresulttables
	

	return _http_cli->request_append(
		"/Server/GetRandomResultTables",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetServerCustomIDsFromPlayFabIDs(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the associated PlayFab account identifiers for the given set of server custom identifiers.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getservercustomidsfromplayfabids
	

	return _http_cli->request_append(
		"/Server/GetServerCustomIDsFromPlayFabIDs",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetSharedGroupData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves data stored in a shared group object, as well as the list of members in the group. The server can access all
	// public and private group data. Shared Groups are designed for sharing data between a very small number of players,
	// please see our guide: https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/server/shared-group-data/getsharedgroupdata
	

	return _http_cli->request_append(
		"/Server/GetSharedGroupData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetStoreItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the set of items defined for the specified store, including all prices defined, for the specified player
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/getstoreitems
	

	return _http_cli->request_append(
		"/Server/GetStoreItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetTime(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the current server time
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/gettime
	

	return _http_cli->request_append(
		"/Server/GetTime",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetTitleData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the key-value store of custom title settings
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/gettitledata
	

	return _http_cli->request_append(
		"/Server/GetTitleData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetTitleInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the key-value store of custom internal title settings
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/gettitleinternaldata
	

	return _http_cli->request_append(
		"/Server/GetTitleInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetTitleNews(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title news feed, as configured in the developer portal
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/gettitlenews
	

	return _http_cli->request_append(
		"/Server/GetTitleNews",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserAccountInfo(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the relevant details for a specified user
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getuseraccountinfo
	

	return _http_cli->request_append(
		"/Server/GetUserAccountInfo",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserBans(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets all bans for a user.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/getuserbans
	

	return _http_cli->request_append(
		"/Server/GetUserBans",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getuserdata
	

	return _http_cli->request_append(
		"/Server/GetUserData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getuserinternaldata
	

	return _http_cli->request_append(
		"/Server/GetUserInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserInventory(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the specified user's current inventory of virtual goods
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/getuserinventory
	

	return _http_cli->request_append(
		"/Server/GetUserInventory",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the publisher-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getuserpublisherdata
	

	return _http_cli->request_append(
		"/Server/GetUserPublisherData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserPublisherInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the publisher-specific custom data for the user which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getuserpublisherinternaldata
	

	return _http_cli->request_append(
		"/Server/GetUserPublisherInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserPublisherReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the publisher-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getuserpublisherreadonlydata
	

	return _http_cli->request_append(
		"/Server/GetUserPublisherReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GetUserReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the title-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/getuserreadonlydata
	

	return _http_cli->request_append(
		"/Server/GetUserReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GrantCharacterToUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Grants the specified character type to the user. CharacterIds are not globally unique; characterId must be evaluated
	// with the parent PlayFabId to guarantee uniqueness.
	// https://docs.microsoft.com/rest/api/playfab/server/characters/grantcharactertouser
	

	return _http_cli->request_append(
		"/Server/GrantCharacterToUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GrantItemsToCharacter(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds the specified items to the specified character's inventory
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/grantitemstocharacter
	

	return _http_cli->request_append(
		"/Server/GrantItemsToCharacter",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GrantItemsToUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds the specified items to the specified user's inventory
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/grantitemstouser
	

	return _http_cli->request_append(
		"/Server/GrantItemsToUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int GrantItemsToUsers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds the specified items to the specified user inventories
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/grantitemstousers
	

	return _http_cli->request_append(
		"/Server/GrantItemsToUsers",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int LinkPSNAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Links the PlayStation :tm: Network account associated with the provided access code to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/linkpsnaccount
	

	return _http_cli->request_append(
		"/Server/LinkPSNAccount",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int LinkServerCustomId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Links the custom server identifier, generated by the title, to the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/linkservercustomid
	

	return _http_cli->request_append(
		"/Server/LinkServerCustomId",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int LinkXboxAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Links the Xbox Live account associated with the provided access code to the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/linkxboxaccount
	

	return _http_cli->request_append(
		"/Server/LinkXboxAccount",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int LoginWithServerCustomId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Securely login a game client from an external server backend using a custom identifier for that player. Server Custom ID
	// and Client Custom ID are mutually exclusive and cannot be used to retrieve the same player account.
	// https://docs.microsoft.com/rest/api/playfab/server/authentication/loginwithservercustomid
	

	return _http_cli->request_append(
		"/Server/LoginWithServerCustomId",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int LoginWithSteamId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Signs the user in using an Steam ID, returning a session identifier that can subsequently be used for API calls which
	// require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/server/authentication/loginwithsteamid
	

	return _http_cli->request_append(
		"/Server/LoginWithSteamId",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int LoginWithXbox(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Signs the user in using a Xbox Live Token from an external server backend, returning a session identifier that can
	// subsequently be used for API calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/server/authentication/loginwithxbox
	

	return _http_cli->request_append(
		"/Server/LoginWithXbox",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int LoginWithXboxId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Signs the user in using an Xbox ID and Sandbox ID, returning a session identifier that can subsequently be used for API
	// calls which require an authenticated user
	// https://docs.microsoft.com/rest/api/playfab/server/authentication/loginwithxboxid
	

	return _http_cli->request_append(
		"/Server/LoginWithXboxId",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ModifyItemUses(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Modifies the number of remaining uses of a player's inventory item
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/modifyitemuses
	

	return _http_cli->request_append(
		"/Server/ModifyItemUses",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int MoveItemToCharacterFromCharacter(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Moves an item from a character's inventory into another of the users's character's inventory.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/moveitemtocharacterfromcharacter
	

	return _http_cli->request_append(
		"/Server/MoveItemToCharacterFromCharacter",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int MoveItemToCharacterFromUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Moves an item from a user's inventory into their character's inventory.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/moveitemtocharacterfromuser
	

	return _http_cli->request_append(
		"/Server/MoveItemToCharacterFromUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int MoveItemToUserFromCharacter(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Moves an item from a character's inventory into the owning user's inventory.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/moveitemtouserfromcharacter
	

	return _http_cli->request_append(
		"/Server/MoveItemToUserFromCharacter",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int NotifyMatchmakerPlayerLeft(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Informs the PlayFab match-making service that the user specified has left the Game Server Instance
	// https://docs.microsoft.com/rest/api/playfab/server/matchmaking/notifymatchmakerplayerleft
	

	return _http_cli->request_append(
		"/Server/NotifyMatchmakerPlayerLeft",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RedeemCoupon(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds the virtual goods associated with the coupon to the user's inventory. Coupons can be generated via the
	// Economy->Catalogs tab in the PlayFab Game Manager.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/redeemcoupon
	

	return _http_cli->request_append(
		"/Server/RedeemCoupon",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RedeemMatchmakerTicket(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Validates a Game Server session ticket and returns details about the user
	// https://docs.microsoft.com/rest/api/playfab/server/matchmaking/redeemmatchmakerticket
	

	return _http_cli->request_append(
		"/Server/RedeemMatchmakerTicket",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RefreshGameServerInstanceHeartbeat(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Set the state of the indicated Game Server Instance. Also update the heartbeat for the instance.
	// https://docs.microsoft.com/rest/api/playfab/server/matchmaking/refreshgameserverinstanceheartbeat
	

	return _http_cli->request_append(
		"/Server/RefreshGameServerInstanceHeartbeat",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RegisterGame(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Inform the matchmaker that a new Game Server Instance is added.
	// https://docs.microsoft.com/rest/api/playfab/server/matchmaking/registergame
	

	return _http_cli->request_append(
		"/Server/RegisterGame",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RemoveFriend(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes the specified friend from the the user's friend list
	// https://docs.microsoft.com/rest/api/playfab/server/friend-list-management/removefriend
	

	return _http_cli->request_append(
		"/Server/RemoveFriend",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RemoveGenericID(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes the specified generic service identifier from the player's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/removegenericid
	

	return _http_cli->request_append(
		"/Server/RemoveGenericID",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RemovePlayerTag(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Remove a given tag from a player profile. The tag's namespace is automatically generated based on the source of the tag.
	// https://docs.microsoft.com/rest/api/playfab/server/playstream/removeplayertag
	

	return _http_cli->request_append(
		"/Server/RemovePlayerTag",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RemoveSharedGroupMembers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes users from the set of those able to update the shared data and the set of users in the group. Only users in the
	// group can remove members. If as a result of the call, zero users remain with access, the group and its associated data
	// will be deleted. Shared Groups are designed for sharing data between a very small number of players, please see our
	// guide: https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/server/shared-group-data/removesharedgroupmembers
	

	return _http_cli->request_append(
		"/Server/RemoveSharedGroupMembers",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int ReportPlayer(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Submit a report about a player (due to bad bahavior, etc.) on behalf of another player, so that customer service
	// representatives for the title can take action concerning potentially toxic players.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/reportplayer
	

	return _http_cli->request_append(
		"/Server/ReportPlayer",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RevokeAllBansForUser(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Revoke all active bans for a user.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/revokeallbansforuser
	

	return _http_cli->request_append(
		"/Server/RevokeAllBansForUser",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RevokeBans(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Revoke all active bans specified with BanId.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/revokebans
	

	return _http_cli->request_append(
		"/Server/RevokeBans",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RevokeInventoryItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Revokes access to an item in a user's inventory
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/revokeinventoryitem
	

	return _http_cli->request_append(
		"/Server/RevokeInventoryItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int RevokeInventoryItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Revokes access for up to 25 items across multiple users and characters.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/revokeinventoryitems
	

	return _http_cli->request_append(
		"/Server/RevokeInventoryItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SavePushNotificationTemplate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Saves push notification template for title
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/savepushnotificationtemplate
	

	return _http_cli->request_append(
		"/Server/SavePushNotificationTemplate",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SendCustomAccountRecoveryEmail(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Forces an email to be sent to the registered contact email address for the user's account based on an account recovery
	// email template
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/sendcustomaccountrecoveryemail
	

	return _http_cli->request_append(
		"/Server/SendCustomAccountRecoveryEmail",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SendEmailFromTemplate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sends an email based on an email template to a player's contact email
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/sendemailfromtemplate
	

	return _http_cli->request_append(
		"/Server/SendEmailFromTemplate",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SendPushNotification(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sends an iOS/Android Push Notification to a specific user, if that user's device has been configured for Push
	// Notifications in PlayFab. If a user has linked both Android and iOS devices, both will be notified.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/sendpushnotification
	

	return _http_cli->request_append(
		"/Server/SendPushNotification",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SendPushNotificationFromTemplate(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sends an iOS/Android Push Notification template to a specific user, if that user's device has been configured for Push
	// Notifications in PlayFab. If a user has linked both Android and iOS devices, both will be notified.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/sendpushnotificationfromtemplate
	

	return _http_cli->request_append(
		"/Server/SendPushNotificationFromTemplate",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetFriendTags(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the tag list for a specified user in the friend list of another user
	// https://docs.microsoft.com/rest/api/playfab/server/friend-list-management/setfriendtags
	

	return _http_cli->request_append(
		"/Server/SetFriendTags",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetGameServerInstanceData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the custom data of the indicated Game Server Instance
	// https://docs.microsoft.com/rest/api/playfab/server/matchmaking/setgameserverinstancedata
	

	return _http_cli->request_append(
		"/Server/SetGameServerInstanceData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetGameServerInstanceState(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Set the state of the indicated Game Server Instance.
	// https://docs.microsoft.com/rest/api/playfab/server/matchmaking/setgameserverinstancestate
	

	return _http_cli->request_append(
		"/Server/SetGameServerInstanceState",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetGameServerInstanceTags(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Set custom tags for the specified Game Server Instance
	// https://docs.microsoft.com/rest/api/playfab/server/matchmaking/setgameserverinstancetags
	

	return _http_cli->request_append(
		"/Server/SetGameServerInstanceTags",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetPlayerSecret(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the player's secret if it is not already set. Player secrets are used to sign API requests. To reset a player's
	// secret use the Admin or Server API method SetPlayerSecret.
	// https://docs.microsoft.com/rest/api/playfab/server/authentication/setplayersecret
	

	return _http_cli->request_append(
		"/Server/SetPlayerSecret",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the key-value store of custom publisher settings
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/setpublisherdata
	

	return _http_cli->request_append(
		"/Server/SetPublisherData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetTitleData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the key-value store of custom title settings
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/settitledata
	

	return _http_cli->request_append(
		"/Server/SetTitleData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SetTitleInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the key-value store of custom title settings
	// https://docs.microsoft.com/rest/api/playfab/server/title-wide-data-management/settitleinternaldata
	

	return _http_cli->request_append(
		"/Server/SetTitleInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SubtractCharacterVirtualCurrency(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Decrements the character's balance of the specified virtual currency by the stated amount. It is possible to make a VC
	// balance negative with this API.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/subtractcharactervirtualcurrency
	

	return _http_cli->request_append(
		"/Server/SubtractCharacterVirtualCurrency",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int SubtractUserVirtualCurrency(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Decrements the user's balance of the specified virtual currency by the stated amount. It is possible to make a VC
	// balance negative with this API.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/subtractuservirtualcurrency
	

	return _http_cli->request_append(
		"/Server/SubtractUserVirtualCurrency",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UnlinkPSNAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Unlinks the related PSN account from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/unlinkpsnaccount
	

	return _http_cli->request_append(
		"/Server/UnlinkPSNAccount",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UnlinkServerCustomId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Unlinks the custom server identifier from the user's PlayFab account.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/unlinkservercustomid
	

	return _http_cli->request_append(
		"/Server/UnlinkServerCustomId",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UnlinkXboxAccount(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Unlinks the related Xbox Live account from the user's PlayFab account
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/unlinkxboxaccount
	

	return _http_cli->request_append(
		"/Server/UnlinkXboxAccount",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UnlockContainerInstance(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Opens a specific container (ContainerItemInstanceId), with a specific key (KeyItemInstanceId, when required), and
	// returns the contents of the opened container. If the container (and key when relevant) are consumable (RemainingUses >
	// 0), their RemainingUses will be decremented, consistent with the operation of ConsumeItem.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/unlockcontainerinstance
	

	return _http_cli->request_append(
		"/Server/UnlockContainerInstance",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UnlockContainerItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Searches Player or Character inventory for any ItemInstance matching the given CatalogItemId, if necessary unlocks it
	// using any appropriate key, and returns the contents of the opened container. If the container (and key when relevant)
	// are consumable (RemainingUses > 0), their RemainingUses will be decremented, consistent with the operation of
	// ConsumeItem.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/unlockcontaineritem
	

	return _http_cli->request_append(
		"/Server/UnlockContainerItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateAvatarUrl(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update the avatar URL of the specified player
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/updateavatarurl
	

	return _http_cli->request_append(
		"/Server/UpdateAvatarUrl",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateBans(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates information of a list of existing bans specified with Ban Ids.
	// https://docs.microsoft.com/rest/api/playfab/server/account-management/updatebans
	

	return _http_cli->request_append(
		"/Server/UpdateBans",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateCharacterData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user's character which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/server/character-data/updatecharacterdata
	

	return _http_cli->request_append(
		"/Server/UpdateCharacterData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateCharacterInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user's character which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/server/character-data/updatecharacterinternaldata
	

	return _http_cli->request_append(
		"/Server/UpdateCharacterInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateCharacterReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user's character which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/server/character-data/updatecharacterreadonlydata
	

	return _http_cli->request_append(
		"/Server/UpdateCharacterReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateCharacterStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the values of the specified title-specific statistics for the specific character
	// https://docs.microsoft.com/rest/api/playfab/server/characters/updatecharacterstatistics
	

	return _http_cli->request_append(
		"/Server/UpdateCharacterStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdatePlayerStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the values of the specified title-specific statistics for the user
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/updateplayerstatistics
	

	return _http_cli->request_append(
		"/Server/UpdatePlayerStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateSharedGroupData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Adds, updates, and removes data keys for a shared group object. If the permission is set to Public, all fields updated
	// or added in this call will be readable by users not in the group. By default, data permissions are set to Private.
	// Regardless of the permission setting, only members of the group (and the server) can update the data. Shared Groups are
	// designed for sharing data between a very small number of players, please see our guide:
	// https://docs.microsoft.com/gaming/playfab/features/social/groups/using-shared-group-data
	// https://docs.microsoft.com/rest/api/playfab/server/shared-group-data/updatesharedgroupdata
	

	return _http_cli->request_append(
		"/Server/UpdateSharedGroupData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/updateuserdata
	

	return _http_cli->request_append(
		"/Server/UpdateUserData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/updateuserinternaldata
	

	return _http_cli->request_append(
		"/Server/UpdateUserInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserInventoryItemCustomData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the key-value pair data tagged to the specified item, which is read-only from the client.
	// https://docs.microsoft.com/rest/api/playfab/server/player-item-management/updateuserinventoryitemcustomdata
	

	return _http_cli->request_append(
		"/Server/UpdateUserInventoryItemCustomData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserPublisherData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the publisher-specific custom data for the user which is readable and writable by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/updateuserpublisherdata
	

	return _http_cli->request_append(
		"/Server/UpdateUserPublisherData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserPublisherInternalData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the publisher-specific custom data for the user which cannot be accessed by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/updateuserpublisherinternaldata
	

	return _http_cli->request_append(
		"/Server/UpdateUserPublisherInternalData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserPublisherReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the publisher-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/updateuserpublisherreadonlydata
	

	return _http_cli->request_append(
		"/Server/UpdateUserPublisherReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int UpdateUserReadOnlyData(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the title-specific custom data for the user which can only be read by the client
	// https://docs.microsoft.com/rest/api/playfab/server/player-data-management/updateuserreadonlydata
	

	return _http_cli->request_append(
		"/Server/UpdateUserReadOnlyData",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int WriteCharacterEvent(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Writes a character-based event into PlayStream.
	// https://docs.microsoft.com/rest/api/playfab/server/analytics/writecharacterevent
	

	return _http_cli->request_append(
		"/Server/WriteCharacterEvent",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int WritePlayerEvent(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Writes a player-based event into PlayStream.
	// https://docs.microsoft.com/rest/api/playfab/server/analytics/writeplayerevent
	

	return _http_cli->request_append(
		"/Server/WritePlayerEvent",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

int WriteTitleEvent(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Writes a title-based event into PlayStream.
	// https://docs.microsoft.com/rest/api/playfab/server/analytics/writetitleevent
	

	return _http_cli->request_append(
		"/Server/WriteTitleEvent",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_SECRET_KEY, USE_AUTH_SECRET_KEY),
		Array()
	);
}

} // namespace Server
