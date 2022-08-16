/*************************************************************************/
/*  PlayFabAPI_Experimentation.h                                         */
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

namespace Experimentation {


int CreateExclusionGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a new experiment exclusion group for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/createexclusiongroup
	

	return _http_cli->request_append(
		"/Experimentation/CreateExclusionGroup",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateExperiment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a new experiment for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/createexperiment
	

	return _http_cli->request_append(
		"/Experimentation/CreateExperiment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteExclusionGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes an existing exclusion group for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/deleteexclusiongroup
	

	return _http_cli->request_append(
		"/Experimentation/DeleteExclusionGroup",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteExperiment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes an existing experiment for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/deleteexperiment
	

	return _http_cli->request_append(
		"/Experimentation/DeleteExperiment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetExclusionGroups(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the details of all exclusion groups for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/getexclusiongroups
	

	return _http_cli->request_append(
		"/Experimentation/GetExclusionGroups",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetExclusionGroupTraffic(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the details of all exclusion groups for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/getexclusiongrouptraffic
	

	return _http_cli->request_append(
		"/Experimentation/GetExclusionGroupTraffic",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetExperiments(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the details of all experiments for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/getexperiments
	

	return _http_cli->request_append(
		"/Experimentation/GetExperiments",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetLatestScorecard(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the latest scorecard of the experiment for the title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/getlatestscorecard
	

	return _http_cli->request_append(
		"/Experimentation/GetLatestScorecard",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetTreatmentAssignment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the treatment assignments for a player for every running experiment in the title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/gettreatmentassignment
	

	return _http_cli->request_append(
		"/Experimentation/GetTreatmentAssignment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int StartExperiment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Starts an existing experiment for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/startexperiment
	

	return _http_cli->request_append(
		"/Experimentation/StartExperiment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int StopExperiment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Stops an existing experiment for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/stopexperiment
	

	return _http_cli->request_append(
		"/Experimentation/StopExperiment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateExclusionGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates an existing exclusion group for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/updateexclusiongroup
	

	return _http_cli->request_append(
		"/Experimentation/UpdateExclusionGroup",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateExperiment(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates an existing experiment for a title.
	// https://docs.microsoft.com/rest/api/playfab/experimentation/experimentation/updateexperiment
	

	return _http_cli->request_append(
		"/Experimentation/UpdateExperiment",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

} // namespace Experimentation
