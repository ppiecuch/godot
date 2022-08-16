/*************************************************************************/
/*  PlayFabAPI_Leaderboards.h                                            */
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

namespace Leaderboards {


int CreateStatisticDefinition(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Create a new entity statistic definition.
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/createstatisticdefinition
	

	return _http_cli->request_append(
		"/Statistic/CreateStatisticDefinition",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteStatisticDefinition(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Delete an entity statistic definition. Will delete all statistics on entity profiles and leaderboards.
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/deletestatisticdefinition
	

	return _http_cli->request_append(
		"/Statistic/DeleteStatisticDefinition",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Delete statistics on an entity profile, will remove all rankings from associated leaderboards.
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/deletestatistics
	

	return _http_cli->request_append(
		"/Statistic/DeleteStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetLeaderboard(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get the leaderboard for a specific entity type and statistic.
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/getleaderboard
	

	return _http_cli->request_append(
		"/Leaderboard/GetLeaderboard",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetLeaderboardAroundEntity(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get the leaderboard around a specific entity.
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/getleaderboardaroundentity
	

	return _http_cli->request_append(
		"/Leaderboard/GetLeaderboardAroundEntity",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetLeaderboardForEntities(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get the leaderboard limited to a set of entities.
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/getleaderboardforentities
	

	return _http_cli->request_append(
		"/Leaderboard/GetLeaderboardForEntities",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetStatisticDefinition(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get current statistic definition information
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/getstatisticdefinition
	

	return _http_cli->request_append(
		"/Statistic/GetStatisticDefinition",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetStatisticDefinitions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get all current statistic definitions information
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/getstatisticdefinitions
	

	return _http_cli->request_append(
		"/Statistic/GetStatisticDefinitions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int IncrementStatisticVersion(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Increment an entity statistic definition version.
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/incrementstatisticversion
	

	return _http_cli->request_append(
		"/Statistic/IncrementStatisticVersion",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateStatistics(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update statistics on an entity profile, depending on the statistic definition may cause entity to be ranked on various
	// leaderboards.
	// https://docs.microsoft.com/rest/api/playfab/leaderboards/statistics-and-leaderboards/updatestatistics
	

	return _http_cli->request_append(
		"/Statistic/UpdateStatistics",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

} // namespace Leaderboards
