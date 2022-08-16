/*************************************************************************/
/*  PlayFabAPI_Insights.h                                                */
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

namespace Insights {


int GetDetails(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the current values for the Insights performance and data storage retention, list of pending operations, and the
	// performance and data storage retention limits.
	// https://docs.microsoft.com/rest/api/playfab/insights/analytics/getdetails
	

	return _http_cli->request_append(
		"/Insights/GetDetails",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetLimits(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves the range of allowed values for performance and data storage retention values as well as the submeter details
	// for each performance level.
	// https://docs.microsoft.com/rest/api/playfab/insights/analytics/getlimits
	

	return _http_cli->request_append(
		"/Insights/GetLimits",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetOperationStatus(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the status of a SetPerformance or SetStorageRetention operation.
	// https://docs.microsoft.com/rest/api/playfab/insights/analytics/getoperationstatus
	

	return _http_cli->request_append(
		"/Insights/GetOperationStatus",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetPendingOperations(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets a list of pending SetPerformance and/or SetStorageRetention operations for the title.
	// https://docs.microsoft.com/rest/api/playfab/insights/analytics/getpendingoperations
	

	return _http_cli->request_append(
		"/Insights/GetPendingOperations",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetPerformance(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the Insights performance level value for the title.
	// https://docs.microsoft.com/rest/api/playfab/insights/analytics/setperformance
	

	return _http_cli->request_append(
		"/Insights/SetPerformance",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetStorageRetention(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the Insights data storage retention days value for the title.
	// https://docs.microsoft.com/rest/api/playfab/insights/analytics/setstorageretention
	

	return _http_cli->request_append(
		"/Insights/SetStorageRetention",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

} // namespace Insights
